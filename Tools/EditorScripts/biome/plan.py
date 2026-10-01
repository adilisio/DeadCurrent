"""The rocky-shoreline recipe's planner: pure Python (no engine), the only placement authority (plan §10).

    island = Island.load()
    recipe = load_recipe()                      # Tools/Biomes/great_lakes_rocky_shore.json
    zones = load_zones()                        # every Tools/Biomes/zones/*.json, by file name
    result = generate(island, recipe, zones, automatic)
    result["hash"], result["placements"]

Everything that decides where a rock goes is here and is a pure function of the recipe, the zone files, island.json,
and the automatic exclusion list (gathered from the map by scatter.py and recorded in the manifest). scatter.py
only writes what this returns: it never jitters, traces, or re-seeds. So two runs give the same hash, and the
committed manifest can be re-planned without Unreal (verify_plan.py --manifest).

How a point is chosen (per family, over every zone polygon):
  1. walk a 2 m lattice on the terrain grid; each cell may hold ceil(density x 4 m2) candidates
  2. each candidate draws its numbers from blake2b(recipe seed, zone seed, family offset, cell, k): no random module
  3. acceptance = density x zone scale x exposure weight x cluster mask x slope bias
  4. reject outside the polygon or the walkable outline, outside the family's slope / coast-distance / height
     windows, over a missing terrain triangle, inside an authored, pad, or automatic exclusion, or in the interior slots
  5. keep candidates in order of their draw: minimum spacing, then the family cap (uniform thinning, never "first N")
Seating: the instance stands on the terrain mesh's own triangles (the same diagonal split as terrain_mesh.py, so
the height is the rendered and colliding surface, not the analytic one), at the lowest of its footprint's corners,
sunk by sink_m, and lifted by the mesh's recorded bounds so its bottom meets that point.

Exposure ("fetch"): rays from the point out over the water; the open water each crosses before reaching land or a
reef is its fetch. Short mean fetch is a sheltered shore. Pointe Sombre's harbor is sheltered by its offshore
reef, not by a bay in the coastline, which is why the coast-turn heuristic was not used (VS-06 record).

Units: metres in the data and here. The placements are integers, already seated: x_mm, y_mm, z_mm, pitch/yaw/roll in
hundredths of a degree, scale in thousandths. Integers, so the C++ map test can re-hash the payload exactly.
"""
import hashlib
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SOMBRE = os.path.normpath(os.path.join(HERE, "..", "pointe_sombre"))
if SOMBRE not in sys.path:
    sys.path.insert(0, SOMBRE)

import island as island_mod  # noqa: E402

BIOMES = os.path.normpath(os.path.join(HERE, "..", "..", "Biomes"))
RECIPE_FILE = os.path.join(BIOMES, "great_lakes_rocky_shore.json")
ZONES_DIR = os.path.join(BIOMES, "zones")
PLANNER = "lattice-v1"

# Hard rejects no zone can override: the interior cell slots (2.5 km east, 400 m up) and anything high in the sky.
INTERIOR_Y_MIN_M = 2000.0


class RecipeError(ValueError):
    pass


# --- Data

def canonical(obj):
    return json.dumps(obj, sort_keys=True, separators=(",", ":"), ensure_ascii=False)


def fingerprint(text):
    """SHA-1 hex of UTF-8 text: a determinism fingerprint (the C++ map test re-computes it with FSHA1)."""
    return hashlib.sha1(text.encode("utf-8")).hexdigest()


def load_recipe(path=None):
    with open(path or RECIPE_FILE, encoding="utf-8") as handle:
        recipe = json.load(handle)
    validate_recipe(recipe)
    return recipe


def load_zones(directory=None):
    """Every *.json in the zones folder, in file-name order. A cell adds its own file; nothing here changes."""
    directory = directory or ZONES_DIR
    zones = []
    for name in sorted(os.listdir(directory)):
        if not name.endswith(".json"):
            continue
        with open(os.path.join(directory, name), encoding="utf-8") as handle:
            zone = json.load(handle)
        zone["_file"] = name
        zones.append(zone)
    ids = [z["id"] for z in zones]
    if len(set(ids)) != len(ids):
        raise RecipeError(f"duplicate zone ids in {directory}: {ids}")
    return zones


def validate_recipe(recipe):
    for key in ("id", "version", "seed", "waterline_m", "caps", "exposure", "automatic_radii_m",
                "families"):
        if key not in recipe:
            raise RecipeError(f"recipe has no '{key}'")
    for fam in recipe["families"]:
        if not fam.get("enabled", True):
            continue
        if fam.get("collision") != "NoCollision":
            raise RecipeError(f"family {fam['id']}: collision must be 'NoCollision' (Tier C never blocks)")
        if not fam.get("meshes"):
            raise RecipeError(f"family {fam['id']} has no meshes")
        for mesh in fam["meshes"]:
            if "bounds_cm" not in mesh:
                raise RecipeError(f"family {fam['id']}: mesh {mesh.get('path')} has no recorded bounds_cm")
    enabled = [f for f in recipe["families"] if f.get("enabled", True)]
    components = sum(len(f["meshes"]) for f in enabled)
    if components > recipe["caps"]["hism_components"]:
        raise RecipeError(f"{components} meshes would make more HISM components than caps.hism_components")


# --- Deterministic numbers

def draws(*key, count=8):
    """count floats in [0, 1) from one blake2b of the key."""
    digest = hashlib.blake2b(":".join(str(k) for k in key).encode(), digest_size=4 * count).digest()
    return [int.from_bytes(digest[4 * i:4 * i + 4], "little") / 4294967296.0 for i in range(count)]


def lerp(a, b, t):
    return a + (b - a) * t


def clamp01(t):
    return 0.0 if t < 0.0 else 1.0 if t > 1.0 else t


# --- The terrain mesh's own surface

class Surface:
    """Heights on the terrain grid's vertices, interpolated on the same triangles terrain_mesh.py builds."""

    def __init__(self, island):
        self.island = island
        g = island.spec["grid"]
        self.x0, self.y0, self.step = float(g["x_min"]), float(g["y_min"]), float(g["step"])
        self.drop = float(g["drop_below"])
        self.ni = int(round((float(g["x_max"]) - self.x0) / self.step))
        self.nj = int(round((float(g["y_max"]) - self.y0) / self.step))
        self._v = {}

    def vertex(self, i, j):
        key = (i, j)
        h = self._v.get(key)
        if h is None:
            h = self.island.height(self.x0 + i * self.step, self.y0 + j * self.step)
            self._v[key] = h
        return h

    def triangle(self, x, y):
        """(height, (dhdx, dhdy)) of the triangle under (x, y), or None where the mesh has no triangle."""
        fx, fy = (x - self.x0) / self.step, (y - self.y0) / self.step
        i, j = math.floor(fx), math.floor(fy)
        if i < 0 or j < 0 or i >= self.ni or j >= self.nj:
            return None   # off the terrain grid: no tile there
        u, v = fx - i, fy - j
        h00, h10 = self.vertex(i, j), self.vertex(i + 1, j)
        h11, h01 = self.vertex(i + 1, j + 1), self.vertex(i, j + 1)
        # terrain_mesh._tile_geometry: split along the diagonal with the smaller height difference.
        if abs(h00 - h11) <= abs(h10 - h01):
            if u >= v:   # (00, 10, 11)
                tri = (h00, h10, h11)
                h = h00 + (h10 - h00) * u + (h11 - h10) * v
                grad = (h10 - h00, h11 - h10)
            else:        # (00, 11, 01)
                tri = (h00, h11, h01)
                h = h00 + (h11 - h01) * u + (h01 - h00) * v
                grad = (h11 - h01, h01 - h00)
        else:
            if u + v <= 1.0:   # (00, 10, 01)
                tri = (h00, h10, h01)
                h = h00 + (h10 - h00) * u + (h01 - h00) * v
                grad = (h10 - h00, h01 - h00)
            else:              # (10, 11, 01)
                tri = (h10, h11, h01)
                h = h11 + (h11 - h01) * (u - 1.0) + (h11 - h10) * (v - 1.0)
                grad = (h11 - h01, h11 - h10)
        if all(t < self.drop for t in tri):
            return None
        return h, (grad[0] / self.step, grad[1] / self.step)

    def height(self, x, y):
        tri = self.triangle(x, y)
        return None if tri is None else tri[0]


# --- Exposure

class Exposure:
    """0 = sheltered, 1 = exposed, from the mean open-water fetch of rays leaving the shore (cached on a grid)."""

    def __init__(self, island, spec):
        self.island = island
        self.spec = spec
        self._cache = {}

    def fetch(self, x, y):
        s = self.spec
        rays = int(s["rays"])
        step = float(s["step_m"])
        reach = float(s["seaward_within_m"])
        deep = float(s["deep_below_m"])
        block = float(s["block_above_m"])
        limit = float(s["max_fetch_m"])
        values = []
        for r in range(rays):
            angle = 2.0 * math.pi * r / rays
            dx, dy = math.cos(angle), math.sin(angle)
            d, start = 0.0, None
            while d <= reach + limit:
                d += step
                h = self.island.height(x + dx * d, y + dy * d)
                if start is None:
                    if h < deep:
                        start = d
                    elif d > reach:
                        break
                elif h >= block or d - start >= limit:
                    values.append(min(limit, d - start))
                    break
        return sum(values) / len(values) if values else None

    def score(self, x, y):
        cell = float(self.spec["cache_cell_m"])
        key = (math.floor(x / cell), math.floor(y / cell))
        if key not in self._cache:
            cx, cy = (key[0] + 0.5) * cell, (key[1] + 0.5) * cell
            f = self.fetch(cx, cy)
            if f is None:
                self._cache[key] = 1.0
            else:
                lo, hi = float(self.spec["protected_below_m"]), float(self.spec["exposed_above_m"])
                self._cache[key] = clamp01((f - lo) / (hi - lo))
        return self._cache[key]


# --- Shapes

def _shape_hit(shape, x, y, extra=0.0):
    kind = shape["shape"]
    if kind == "circle":
        cx, cy = shape["center"]
        return math.hypot(x - cx, y - cy) <= float(shape["radius"]) + extra
    if kind == "box":
        cx, cy = shape["center"]
        hx, hy = shape["half"]
        a = math.radians(float(shape.get("yaw", 0.0)))
        lx = (x - cx) * math.cos(a) + (y - cy) * math.sin(a)
        ly = -(x - cx) * math.sin(a) + (y - cy) * math.cos(a)
        return abs(lx) <= float(hx) + extra and abs(ly) <= float(hy) + extra
    if kind == "polygon":
        pts = [tuple(p) for p in shape["points"]]
        if island_mod.point_in_polygon(x, y, pts):
            return True
        return extra > 0.0 and island_mod.polygon_edge_distance(x, y, pts) <= extra
    raise RecipeError(f"unknown exclusion shape {kind}")


def exclusion_set(island, recipe, zone, automatic):
    """[(id, shape, extra_margin)] for one zone: its authored shapes, its pads, and every automatic one."""
    out = []
    for ex in zone.get("exclusions", []):
        out.append((f"zone:{ex['id']}", ex, float(ex.get("margin_m", 0.0))))
    pads = {p["name"]: p for p in island.spec.get("pads", [])}
    for ref in zone.get("exclude_pads", []):
        if ref["name"] not in pads:
            raise RecipeError(f"zone {zone['id']}: no pad named '{ref['name']}' in island.json")
        pad = pads[ref["name"]]
        out.append((f"pad:{ref['name']}", {"shape": "circle", "center": pad["center"], "radius": pad["radius"]},
                    float(ref.get("margin_m", 0.0))))
    radii = recipe["automatic_radii_m"]
    for item in automatic:
        if item["kind"] not in radii:
            raise RecipeError(f"no automatic radius for kind '{item['kind']}'")
        out.append((f"auto:{item['kind']}:{item['id']}", shape_m(item["shape"]), float(radii[item["kind"]])))
    return out


def shape_m(shape_cm):
    """An automatic exclusion's shape, recorded in integer cm (and centidegrees), in metres for the tests here."""
    kind = shape_cm["shape"]
    if kind == "circle":
        return {"shape": "circle", "center": [v / 100.0 for v in shape_cm["center_cm"]],
                "radius": shape_cm["radius_cm"] / 100.0}
    if kind == "box":
        return {"shape": "box", "center": [v / 100.0 for v in shape_cm["center_cm"]],
                "half": [v / 100.0 for v in shape_cm["half_cm"]], "yaw": shape_cm.get("yaw_cdeg", 0) / 100.0}
    raise RecipeError(f"unknown automatic shape {kind}")


# --- Planning

def _family_mask(seed, cluster, x, y):
    noise = island_mod._ValueNoise(seed, float(cluster["cell_m"]))
    n01 = (noise(x, y) + 1.0) * 0.5
    # Full inside a pocket, fading over its rim. The noise is smooth and sits near 0.5 (median 0.50, 80th
    # percentile about 0.70), so a threshold of 0.5 makes pockets of about half the area.
    return island_mod._smoothstep((n01 - float(cluster["threshold"])) / max(1e-6, float(cluster["softness"])))


def _slope_bias(bias, slope, lo, hi):
    if bias == "flat":
        return 1.0
    t = clamp01((slope - lo) / max(1e-6, hi - lo))
    return (1.0 - t) if bias == "prefer_low" else t


def _coast_tangent_deg(island, x, y):
    coast = island.coast
    best, yaw = None, 0.0
    for i in range(len(coast)):
        a, b = coast[i], coast[(i + 1) % len(coast)]
        d = island_mod._segment_distance(x, y, *a, *b)
        if best is None or d < best:
            best, yaw = d, math.degrees(math.atan2(b[1] - a[1], b[0] - a[0]))
    return yaw


def _footprint(mesh, scale, yaw_deg):
    """XY corners (m, relative to the pivot) of the mesh's bounds, scaled, yawed, pulled in by 20%."""
    b = mesh["bounds_cm"]
    a = math.radians(yaw_deg)
    ca, sa = math.cos(a), math.sin(a)
    out = []
    for px in (b["min"][0], b["max"][0]):
        for py in (b["min"][1], b["max"][1]):
            lx, ly = px * scale * 0.008, py * scale * 0.008   # cm -> m, x 0.8
            out.append((lx * ca - ly * sa, lx * sa + ly * ca))
    return out


def _int(value, per_unit):
    """The manifest's integer units (so the hashed payload has no floats and C++ can reproduce it exactly)."""
    return int(round(value * per_unit))


def generate(island, recipe, zones, automatic=()):
    """Plan every placement. Returns the manifest body (without engine fields) including its hash."""
    validate_recipe(recipe)
    surface = Surface(island)
    exposure = Exposure(island, recipe["exposure"])
    weights = recipe["exposure"]["weights"]
    rseed = int(recipe["seed"])
    automatic = sorted(automatic, key=lambda a: (a["kind"], a["id"]))
    for item in automatic:
        if item["kind"] not in recipe["automatic_radii_m"]:
            raise RecipeError(f"no automatic radius for kind '{item['kind']}'")

    families = [f for f in recipe["families"] if f.get("enabled", True)]
    candidates = {f["id"]: [] for f in families}
    step = surface.step
    for zone in sorted(zones, key=lambda z: z["id"]):
        if zone.get("recipe") != recipe["id"]:
            continue
        exclusions = exclusion_set(island, recipe, zone, automatic)
        zseed = int(zone["seed"])
        zscale = float(zone.get("density_scale", 1.0))
        for poly in zone["polygons"]:
            pts = [tuple(p) for p in poly["points"]]
            forced = poly.get("exposure", "auto")
            xs, ys = [p[0] for p in pts], [p[1] for p in pts]
            i0 = math.floor((min(xs) - surface.x0) / step)
            i1 = math.ceil((max(xs) - surface.x0) / step)
            j0 = math.floor((min(ys) - surface.y0) / step)
            j1 = math.ceil((max(ys) - surface.y0) / step)
            for fam in families:
                fid = fam["id"]
                fseed = rseed + 7919 * int(fam["seed_offset"]) + 104729 * zseed
                lam = float(fam["density_per_m2"]) * step * step * zscale
                per_cell = max(1, math.ceil(lam))
                slope_w, coast_w, height_w = fam["slope_deg"], fam["coast_distance_m"], fam["height_above_water_m"]
                for i in range(i0, i1):
                    for j in range(j0, j1):
                        for k in range(per_cell):
                            r = draws(rseed, zseed, fam["seed_offset"], i, j, k, count=10)
                            x = surface.x0 + (i + r[1]) * step
                            y = surface.y0 + (j + r[2]) * step
                            if not island_mod.point_in_polygon(x, y, pts):
                                continue
                            if y > INTERIOR_Y_MIN_M or not island.inside_bounds(x, y):
                                continue
                            tri = surface.triangle(x, y)
                            if tri is None:
                                continue
                            h, (gx, gy) = tri
                            above = h - float(recipe["waterline_m"])
                            if not (height_w["min"] <= above <= height_w["max"]):
                                continue
                            slope = math.degrees(math.atan(math.hypot(gx, gy)))
                            if not (slope_w["min"] <= slope <= slope_w["max"]):
                                continue
                            coast_d = island_mod.signed_distance(x, y, island.coast)
                            if not (coast_w["min"] <= coast_d <= coast_w["max"]):
                                continue
                            if any(_shape_hit(shape, x, y, extra) for _id, shape, extra in exclusions):
                                continue
                            t = 0.0 if forced == "protected" else 1.0 if forced == "exposed" else exposure.score(x, y)
                            w_exp = lerp(float(weights["protected"][fid]), float(weights["exposed"][fid]), t)
                            accept = (lam / per_cell) * w_exp * _family_mask(fseed, fam["cluster"], x, y) * \
                                _slope_bias(slope_w.get("bias", "flat"), slope, slope_w["min"], slope_w["max"])
                            if r[0] >= accept:
                                continue
                            candidates[fid].append({
                                "order": r[0] / max(accept, 1e-9), "zone": zone["id"], "polygon": poly["id"],
                                "x": x, "y": y, "r": r, "exposure": t})

    placements = []
    counts = {}
    capped = False
    total_cap = int(recipe["caps"]["instances_total"])
    for fam in families:
        fid = fam["id"]
        spacing = float(fam.get("min_spacing_m", 0.0))
        grid = {}
        kept = []
        for c in sorted(candidates[fid], key=lambda c: (c["order"], c["zone"], c["x"], c["y"])):
            if len(kept) >= int(fam["max_instances"]) or len(placements) + len(kept) >= total_cap:
                capped = True
                break
            gx, gy = math.floor(c["x"] / max(spacing, 1e-6)), math.floor(c["y"] / max(spacing, 1e-6))
            if spacing > 0.0 and any(math.hypot(c["x"] - o["x"], c["y"] - o["y"]) < spacing
                                     for di in (-1, 0, 1) for dj in (-1, 0, 1) for o in grid.get((gx + di, gy + dj), ())):
                continue
            seated = _seat(island, surface, fam, c)
            if seated is None:
                continue
            kept.append(seated)
            if spacing > 0.0:
                grid.setdefault((gx, gy), []).append(c)
        counts[fid] = len(kept)
        placements.extend(kept)

    placements.sort(key=lambda p: (p["zone"], p["family"], p["mesh"], p["x_mm"], p["y_mm"], p["z_mm"]))
    body = {
        "planner": PLANNER,
        "recipe_id": recipe["id"],
        "recipe_version": recipe["version"],
        "island_hash": island.spec_hash(),
        "inputs": {
            "recipe": fingerprint(canonical(recipe)),
            "zones": fingerprint(canonical([{k: v for k, v in z.items() if k != "_file"} for z in
                                       sorted(zones, key=lambda z: z["id"])])),
            "automatic": automatic,
        },
        "counts": counts,
        "placements": placements,
    }
    body["capped"] = capped
    body["hash"] = payload_hash(body)
    return body


def _seat(island, surface, fam, c):
    r = c["r"]
    meshes = fam["meshes"]
    total_w = sum(float(m.get("weight", 1.0)) for m in meshes)
    pick, acc, index = r[3] * total_w, 0.0, 0
    for index, m in enumerate(meshes):
        acc += float(m.get("weight", 1.0))
        if pick < acc:
            break
    mesh = meshes[index]
    scale = lerp(float(fam["scale"]["min"]), float(fam["scale"]["max"]), r[4])
    yaw = lerp(float(fam["yaw_deg"]["min"]), float(fam["yaw_deg"]["max"]), r[5])
    if fam.get("align_to_coast"):
        yaw += _coast_tangent_deg(island, c["x"], c["y"]) + (180.0 if r[6] < 0.5 else 0.0)
    yaw = yaw % 360.0
    tilt = float(fam.get("tilt_deg", 0.0))
    pitch = (r[7] * 2.0 - 1.0) * tilt
    roll = (r[8] * 2.0 - 1.0) * tilt
    corners = [(c["x"] + dx, c["y"] + dy) for dx, dy in _footprint(mesh, scale, yaw)] + [(c["x"], c["y"])]
    heights = [surface.height(x, y) for x, y in corners]
    if any(h is None for h in heights):
        return None
    relief = max(heights) - min(heights)
    if relief > float(fam.get("max_footprint_relief_m", 1e9)):
        return None
    ground = min(heights)
    lift_cm = -float(mesh["bounds_cm"]["min"][2]) * scale
    z_cm = (ground - float(fam.get("sink_m", 0.0))) * 100.0 + lift_cm
    return {
        "zone": c["zone"], "polygon": c["polygon"], "family": fam["id"], "mesh": index,
        "x_mm": _int(c["x"], 1000.0), "y_mm": _int(c["y"], 1000.0), "z_mm": _int(z_cm, 10.0),
        "pitch_cdeg": _int(pitch, 100.0), "yaw_cdeg": _int(yaw, 100.0), "roll_cdeg": _int(roll, 100.0),
        "scale_milli": _int(scale, 1000.0),
    }


def payload_hash(body):
    """SHA-1 over the canonical JSON of the fields that decide the instances (not the hash, not engine bookkeeping)."""
    keys = ("planner", "island_hash", "inputs", "counts", "placements")
    return fingerprint(canonical({k: body[k] for k in keys}))
