"""Pointe Sombre's island as numbers: a deterministic heightfield and outline helpers.

Pure Python (no engine), so the map script, the route-timing report, and the biome tool all read the same island,
and the same data always gives the same heights. Data: Tools/PointeSombre/island.json (metres; X north, Y east,
Z up; sea level 0). Integrator-owned (VerticalSlicePhasePlan.txt §13.2).

    island = Island.load()           # Tools/PointeSombre/island.json
    island.height(x, y)              # metres
    island.inside_coast(x, y)        # on land
    island.spec_hash()               # changes whenever the data changes (the map script skips unchanged tiles)
"""
import hashlib
import json
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_SPEC = os.path.normpath(os.path.join(HERE, "..", "..", "PointeSombre", "island.json"))
# Bump when the height function or the tile build changes, so tiles built the older way are rebuilt.
FORMULA_VERSION = 8   # 8: folds, rough causeway, no trail paint in the splash band (VS-08 critic pass); 7: graded paths, the path surface, height bands in metres (VS-08); 6: OBJ axes (X, -Y, Z); 5: OBJ with normals


def _smoothstep(t):
    t = 0.0 if t < 0.0 else 1.0 if t > 1.0 else t
    return t * t * (3.0 - 2.0 * t)


def _segment_distance(px, py, ax, ay, bx, by):
    dx, dy = bx - ax, by - ay
    length2 = dx * dx + dy * dy
    t = 0.0 if length2 == 0.0 else max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / length2))
    cx, cy = ax + t * dx, ay + t * dy
    return math.hypot(px - cx, py - cy)


def polyline_distance(px, py, points):
    return min(_segment_distance(px, py, *points[i], *points[i + 1]) for i in range(len(points) - 1))


def point_in_polygon(px, py, polygon):
    inside = False
    n = len(polygon)
    j = n - 1
    for i in range(n):
        xi, yi = polygon[i]
        xj, yj = polygon[j]
        if (yi > py) != (yj > py) and px < (xj - xi) * (py - yi) / (yj - yi) + xi:
            inside = not inside
        j = i
    return inside


def polygon_edge_distance(px, py, polygon):
    n = len(polygon)
    return min(_segment_distance(px, py, *polygon[i], *polygon[(i + 1) % n]) for i in range(n))


def signed_distance(px, py, polygon):
    """Positive inside the polygon, negative outside, in metres."""
    d = polygon_edge_distance(px, py, polygon)
    return d if point_in_polygon(px, py, polygon) else -d


class _ValueNoise:
    """Seeded 2D value noise on a square lattice, smoothly interpolated. Deterministic across runs and machines."""

    def __init__(self, seed, cell):
        self.seed = int(seed)
        self.cell = float(cell)

    def _lattice(self, ix, iy):
        h = hashlib.blake2b(f"{self.seed}:{ix}:{iy}".encode(), digest_size=4).digest()
        return int.from_bytes(h, "little") / 0xFFFFFFFF * 2.0 - 1.0

    def __call__(self, x, y):
        fx, fy = x / self.cell, y / self.cell
        ix, iy = math.floor(fx), math.floor(fy)
        tx, ty = _smoothstep(fx - ix), _smoothstep(fy - iy)
        a = self._lattice(ix, iy)
        b = self._lattice(ix + 1, iy)
        c = self._lattice(ix, iy + 1)
        d = self._lattice(ix + 1, iy + 1)
        return (a + (b - a) * tx) + ((c + (d - c) * tx) - (a + (b - a) * tx)) * ty


class _Path:
    """A trail (island.json "paths"): a polyline the terrain paints with the path surface and, when graded, builds up so
    its slope never exceeds max_grade_deg. The profile is the ungraded ground sampled every metre along the line, then
    filled (never cut) to the grade (deterministic), so the trail follows the land and only its steep places are built
    up. Across the trail the full profile holds within half_width_m and fades to the natural ground over
    falloff_m."""

    def __init__(self, spec):
        self.id = spec["id"]
        self.points = [tuple(p) for p in spec["points"]]
        self.half_width = float(spec.get("half_width_m", 1.5))
        self.falloff = float(spec.get("falloff_m", 3.0))
        self.grade = spec.get("max_grade_deg") is not None
        self.max_grade = float(spec.get("max_grade_deg") or 0.0)
        self._cum = [0.0]
        for (ax, ay), (bx, by) in zip(self.points, self.points[1:]):
            self._cum.append(self._cum[-1] + math.hypot(bx - ax, by - ay))
        self.length = self._cum[-1]
        self._profile = []
        self._last_cap = False
        xs = [p[0] for p in self.points]
        ys = [p[1] for p in self.points]
        reach = self.half_width + self.falloff
        self._box = (min(xs) - reach, max(xs) + reach, min(ys) - reach, max(ys) + reach)

    def point_at(self, s):
        for i in range(1, len(self._cum)):
            if s <= self._cum[i] or i == len(self._cum) - 1:
                seg = self._cum[i] - self._cum[i - 1]
                t = 0.0 if seg == 0 else max(0.0, min(1.0, (s - self._cum[i - 1]) / seg))
                (ax, ay), (bx, by) = self.points[i - 1], self.points[i]
                return ax + (bx - ax) * t, ay + (by - ay) * t
        return self.points[-1]

    def build_profile(self, ground):
        n = max(1, int(math.ceil(self.length)))
        stations = [self.length * k / n for k in range(n + 1)]
        h = [ground(*self.point_at(st)) for st in stations]
        # Fill only: the lowest profile at or above the ground whose slope never exceeds the grade (the maximum of the
        # ground's cones). It never cuts below a pad, so a trail always arrives level with the place it climbs to, and a
        # scarp becomes a ramp built up against it.
        step = math.tan(math.radians(self.max_grade)) * (self.length / n)
        for i in range(1, len(h)):
            h[i] = max(h[i], h[i - 1] - step)
        for i in range(len(h) - 2, -1, -1):
            h[i] = max(h[i], h[i + 1] - step)
        self._stations = stations
        self._profile = h

    def nearest(self, x, y):
        """(distance to the centre line, arc length of the nearest point)."""
        x0, x1, y0, y1 = self._box
        if x < x0 or x > x1 or y < y0 or y > y1:
            return float("inf"), 0.0
        best, best_s, best_cap = float("inf"), 0.0, False
        last = len(self.points) - 1
        for i in range(1, len(self.points)):
            (ax, ay), (bx, by) = self.points[i - 1], self.points[i]
            dx, dy = bx - ax, by - ay
            l2 = dx * dx + dy * dy
            raw = 0.0 if l2 == 0 else ((x - ax) * dx + (y - ay) * dy) / l2
            t = max(0.0, min(1.0, raw))
            d = math.hypot(x - (ax + t * dx), y - (ay + t * dy))
            if d < best:
                # Beyond the trail's two ends (not at a bend) the point is "off the end".
                cap = (i == 1 and raw < 0.0) or (i == last and raw > 1.0)
                best, best_s, best_cap = d, self._cum[i - 1] + t * math.sqrt(l2), cap
        self._last_cap = best_cap
        return best, best_s

    def profile_at(self, s):
        n = len(self._profile) - 1
        f = 0.0 if self.length == 0 else s / self.length * n
        i = max(0, min(n - 1, int(f)))
        t = f - i
        return self._profile[i] + (self._profile[i + 1] - self._profile[i]) * t

    def apply(self, x, y, h):
        d, s = self.nearest(x, y)
        if d >= self.half_width + self.falloff or self._last_cap:
            return h   # square ends: a trail grades only what lies alongside it, so a spur cannot bend a trail it joins
        w = 1.0 if d <= self.half_width else 1.0 - _smoothstep((d - self.half_width) / self.falloff)
        # Fill only here too: a trail raises ground to its profile and never cuts it, so a later trail cannot lower an
        # earlier one where they meet.
        return max(h, h + (self.profile_at(s) - h) * w)


class Island:
    def __init__(self, spec, source=None):
        self.spec = spec
        self.source = source
        self.coast = [tuple(p) for p in spec["coast"]]
        self.bounds = [tuple(p) for p in spec["bounds"]["points"]]
        self.base = float(spec["base_height"])
        self.sea_floor = float(spec["sea_floor"])
        shore = spec["shore"]
        self.default_ramp = float(shore["default_ramp"])
        self.sea_per_ramp = float(shore["sea_width_per_ramp"])
        self.sea_min = float(shore["sea_width_min"])
        self.sea_max = float(shore["sea_width_max"])
        noise = spec["noise"]
        self.noise = _ValueNoise(noise["seed"], noise["cell"])
        self.noise_amp = float(noise["amplitude"])
        # VS-08: terrain folds, a second, broader octave (a few metres over tens of metres), so the land breaks up into
        # rises and dips that hide and reveal, instead of reading as a smooth bank. Pads and the shore ramp hold it off
        # the same way as the fine noise.
        folds = spec.get("folds")
        self.folds = _ValueNoise(folds["seed"], folds["cell"]) if folds else None
        self.folds_amp = float(folds["amplitude"]) if folds else 0.0
        self._rough_noise = {}
        for line in spec.get("causeways", []) + spec.get("reefs", []):
            r = line.get("roughness")
            if r:
                self._rough_noise[line["name"]] = (_ValueNoise(r["seed"], r["cell"]),
                                                   _ValueNoise(int(r["seed"]) + 1, float(r["cell"]) * 1.7))
        self.places = {k: tuple(v) for k, v in spec["places"].items() if k != "about"}
        self.paths = []
        # Each trail is graded on the ground as the trails before it left it, so where a spur meets a trail it starts
        # from that trail's surface (a junction has no step).
        for p in spec.get("paths", []):
            path = _Path(p)
            if path.grade:
                path.build_profile(self.height)
            self.paths.append(path)

    @classmethod
    def load(cls, path=None):
        path = path or DEFAULT_SPEC
        with open(path, encoding="utf-8") as handle:
            return cls(json.load(handle), path)

    def spec_hash(self):
        text = json.dumps(self.spec, sort_keys=True, separators=(",", ":"))
        return hashlib.sha1(f"{FORMULA_VERSION}|{text}".encode()).hexdigest()[:16]

    def inside_coast(self, x, y):
        return point_in_polygon(x, y, self.coast)

    def inside_bounds(self, x, y):
        return point_in_polygon(x, y, self.bounds)

    def _ramp(self, x, y):
        """Shore steepness here: the distance (m) over which land rises from the waterline. Small is a cliff."""
        ramp = self.default_ramp
        for zone in self.spec.get("ramp_zones", []):
            cx, cy = zone["center"]
            w = 1.0 - _smoothstep(math.hypot(x - cx, y - cy) / float(zone["radius"]))
            ramp = ramp + (float(zone["ramp"]) - ramp) * w
        return ramp

    def _relief(self, x, y):
        """Land height before the shore ramp: base, hills, and ridges."""
        h = self.base
        for hill in self.spec.get("hills", []):
            cx, cy = hill["center"]
            d = math.hypot(x - cx, y - cy) / float(hill["radius"])
            h += float(hill["height"]) * math.exp(-2.5 * d * d)
        for ridge in self.spec.get("ridges", []):
            d = _segment_distance(x, y, *ridge["from"], *ridge["to"]) / (float(ridge["width"]) * 0.5)
            h += float(ridge["height"]) * math.exp(-1.6 * d * d)
        return h

    def _pad_weight(self, x, y):
        """Strongest pad pull here and its height, or (0, None)."""
        best_w, best_h = 0.0, None
        for pad in self.spec.get("pads", []):
            cx, cy = pad["center"]
            d = math.hypot(x - cx, y - cy)
            r, blend = float(pad["radius"]), float(pad["blend"])
            w = 1.0 if d <= r else 1.0 - _smoothstep((d - r) / blend)
            if w > best_w:
                best_w, best_h = w, float(pad["height"])
        return best_w, best_h

    def height(self, x, y):
        """Terrain height (m): the ungraded island, then each graded path's cut-and-fill."""
        h = self._height_ungraded(x, y)
        for path in self.paths:
            if path.grade:
                h = path.apply(x, y, h)
        return h

    def path_distance(self, x, y):
        """(distance to the nearest path's centre line, that path) in metres, or (inf, None) with no paths."""
        best, best_path = float("inf"), None
        for path in self.paths:
            d = path.nearest(x, y)[0]
            if d < best:
                best, best_path = d, path
        return best, best_path

    def _height_ungraded(self, x, y):
        d = signed_distance(x, y, self.coast)
        ramp = self._ramp(x, y)
        if d >= 0.0:
            t = _smoothstep(d / ramp)
            h = self._relief(x, y) * t
            w, pad_h = self._pad_weight(x, y)
            rough = self.noise_amp * self.noise(x, y)
            if self.folds:
                rough += self.folds_amp * self.folds(x, y)
            h += rough * t * (1.0 - w)
            if pad_h is not None:
                h = h + (pad_h - h) * w * _smoothstep(d / max(ramp * 0.5, 0.5))
        else:
            sea_w = min(self.sea_max, max(self.sea_min, ramp * self.sea_per_ramp))
            h = self.sea_floor * _smoothstep(-d / sea_w)
        # Walkable or awash features in the water: the reef causeway, the Grey's reef, the reef line.
        for line in self.spec.get("causeways", []) + self.spec.get("reefs", []):
            dist = polyline_distance(x, y, [tuple(p) for p in line["points"]])
            h = max(h, self._feature(dist, line, rough=self._roughness(line, x, y)))
        for pad in self.spec.get("rock_pads", []):
            cx, cy = pad["center"]
            h = max(h, self._feature(math.hypot(x - cx, y - cy) - float(pad["radius"]), pad, radial=True))
        return h

    def _roughness(self, feature, x, y):
        """(crest height offset, half-width offset) in metres from a feature's optional "roughness" (VS-08: the reef
        causeway is a broken bedrock spine whose crest rises and dips and whose edge wanders, not a level bank)."""
        noise = self._rough_noise.get(feature["name"])
        if noise is None:
            return 0.0, 0.0
        r = feature["roughness"]
        return float(r["height_m"]) * noise[0](x, y), float(r["width_m"]) * noise[1](x, y)

    def _feature(self, dist, feature, radial=False, rough=(0.0, 0.0)):
        """A crest at feature height across its width (or radius), falling to the sea floor over its falloff."""
        crest = float(feature["height"]) + rough[0]
        half = 0.0 if radial else float(feature["width"]) * 0.5 + rough[1]
        over = dist - half
        if over <= 0.0:
            return crest
        t = _smoothstep(over / float(feature["falloff"]))
        return crest + (self.sea_floor - crest) * t

    def sample_grid(self):
        """Heights on the grid as rows over X: [(x, [h(y0), h(y1), ...]), ...], plus the Y list."""
        g = self.spec["grid"]
        step = float(g["step"])
        xs = [g["x_min"] + i * step for i in range(int(round((g["x_max"] - g["x_min"]) / step)) + 1)]
        ys = [g["y_min"] + j * step for j in range(int(round((g["y_max"] - g["y_min"]) / step)) + 1)]
        return xs, ys, [[self.height(x, y) for y in ys] for x in xs]


class MeshSurface:
    """Heights on the terrain grid's vertices, interpolated on the same triangles terrain_mesh.py builds: the surface the
    player walks and traces meet, not the analytic one (they differ by up to one 2 m quad off a vertex). Used by the
    shoreline recipe (biome/plan.py) and the route-timing report (pointe_sombre/routes.py)."""

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
