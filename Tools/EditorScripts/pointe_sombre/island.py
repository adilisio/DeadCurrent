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
FORMULA_VERSION = 6   # 6: OBJ axes (X, -Y, Z); 5: tiles imported from OBJ with explicit normals


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
        self.places = {k: tuple(v) for k, v in spec["places"].items() if k != "about"}

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
        d = signed_distance(x, y, self.coast)
        ramp = self._ramp(x, y)
        if d >= 0.0:
            t = _smoothstep(d / ramp)
            h = self._relief(x, y) * t
            w, pad_h = self._pad_weight(x, y)
            h += self.noise_amp * self.noise(x, y) * t * (1.0 - w)
            if pad_h is not None:
                h = h + (pad_h - h) * w * _smoothstep(d / max(ramp * 0.5, 0.5))
        else:
            sea_w = min(self.sea_max, max(self.sea_min, ramp * self.sea_per_ramp))
            h = self.sea_floor * _smoothstep(-d / sea_w)
        # Walkable or awash features in the water: the reef causeway, the Grey's reef, the reef line.
        for line in self.spec.get("causeways", []) + self.spec.get("reefs", []):
            dist = polyline_distance(x, y, [tuple(p) for p in line["points"]])
            h = max(h, self._feature(dist, line))
        for pad in self.spec.get("rock_pads", []):
            cx, cy = pad["center"]
            h = max(h, self._feature(math.hypot(x - cx, y - cy) - float(pad["radius"]), pad, radial=True))
        return h

    def _feature(self, dist, feature, radial=False):
        """A crest at feature height across its width (or radius), falling to the sea floor over its falloff."""
        half = 0.0 if radial else float(feature["width"]) * 0.5
        over = dist - half
        if over <= 0.0:
            return float(feature["height"])
        t = _smoothstep(over / float(feature["falloff"]))
        return float(feature["height"]) + (self.sea_floor - float(feature["height"])) * t

    def sample_grid(self):
        """Heights on the grid as rows over X: [(x, [h(y0), h(y1), ...]), ...], plus the Y list."""
        g = self.spec["grid"]
        step = float(g["step"])
        xs = [g["x_min"] + i * step for i in range(int(round((g["x_max"] - g["x_min"]) / step)) + 1)]
        ys = [g["y_min"] + j * step for j in range(int(round((g["y_max"] - g["y_min"]) / step)) + 1)]
        return xs, ys, [[self.height(x, y) for y in ys] for x in xs]
