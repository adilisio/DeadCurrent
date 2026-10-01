"""Pointe Sombre's greybox layout (Tools/PointeSombre/greybox.json), resolved to engine coordinates. Pure Python.

Integrator-owned (VS-08). The core script uses it for the shared anchors; pointe_sombre/greybox.py for the blockouts.
Heights come from the terrain mesh's own surface (island.MeshSurface), so a marker off a 2 m grid vertex stands on the
triangle the player walks, not on the analytic height (Codex VS-04 review, off-lattice anchors).

    lay = Layout.load(island)
    lay.structure_origin("store")      -> (x_cm, y_cm, z_cm, yaw): the kit's local origin for compose.place
    lay.anchors(slots)                 -> [(name, (x, y, z) cm, yaw, held)]
"""
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import island as island_mod  # noqa: E402

DATA = os.path.normpath(os.path.join(HERE, "..", "..", "PointeSombre", "greybox.json"))
KIT_STRUCTURES = os.path.normpath(os.path.join(HERE, "..", "..", "Kits", "great_lakes_settlement", "structures"))
STANDING_UP_M = 1.0   # a marker stands at capsule-centre height: floor + 100 cm (VS-04 portal rule)


def _rot(x, y, yaw_deg):
    a = math.radians(yaw_deg)
    return x * math.cos(a) - y * math.sin(a), x * math.sin(a) + y * math.cos(a)


class Layout:
    def __init__(self, data, island):
        self.data = data
        self.island = island
        self.surface = island_mod.MeshSurface(island)
        self.structures = {s["id"]: s for s in data["structures"]}

    @classmethod
    def load(cls, island, path=None):
        with open(path or DATA, encoding="utf-8") as handle:
            return cls(json.load(handle), island)

    def ground(self, x, y):
        h = self.surface.height(x, y)
        return self.island.height(x, y) if h is None else h

    # --- Structures

    def kit_spec(self, sid):
        s = self.structures[sid]
        with open(os.path.join(KIT_STRUCTURES, s["kit"].replace("kit_", "") + ".json"), encoding="utf-8") as handle:
            spec = json.load(handle)
        for key, value in s.get("override", {}).items():
            spec[key] = value
        spec["id"] = f"greybox_{sid}"
        return spec

    def footprint_m(self, sid):
        spec = self.kit_spec(sid)
        return float(spec["footprint"][0]), float(spec["footprint"][1])

    def structure_origin(self, sid):
        """The kit frame's origin (its footprint corner) for a structure centred at 'center', in cm, and the ground
        relief under the footprint: (x, y, z, yaw, relief_m). z is the lowest ground under the footprint, so the kit's
        pilings reach the rock and nothing is flattened (Gemini VS-05/VS-08 briefs)."""
        s = self.structures[sid]
        w, d = self.footprint_m(sid)
        cx, cy = s["center"]
        yaw = float(s["yaw"])
        ox, oy = _rot(-w / 2.0, -d / 2.0, yaw)
        corners = [_rot(px, py, yaw) for px in (-w / 2.0, 0.0, w / 2.0) for py in (-d / 2.0, 0.0, d / 2.0)]
        heights = [self.ground(cx + px, cy + py) for px, py in corners]
        low, high = min(heights), max(heights)
        return (cx + ox) * 100.0, (cy + oy) * 100.0, low * 100.0, yaw, high - low

    def structure_local(self, sid, local, local_yaw=0.0):
        """A point (m) in a structure's kit frame -> (x, y) m and world yaw."""
        x, y, _z, yaw, _relief = self.structure_origin(sid)
        px, py = _rot(local[0], local[1], yaw)
        return x / 100.0 + px, y / 100.0 + py, yaw + local_yaw

    # --- Anchors

    def anchors(self, slots_cm, deck=None):
        """[(name, (x, y, z) cm, yaw, held)] for every anchor in the data. slots_cm: interior cell -> (x, y, z) cm.
        deck: (center_xy_cm, top_cm, heading_deg) of the Ida's deck, for anchors on it."""
        tower = self.data["tower"]
        out = []
        for name, a in self.data["anchors"].items():
            up = float(a.get("up", STANDING_UP_M))
            yaw = float(a.get("yaw", 0.0))
            if "slot" in a:
                ox, oy, oz = slots_cm[a["slot"]]
                dx, dy, dz = a["offset"]
                loc = (ox + dx * 100.0, oy + dy * 100.0, oz + (dz + up) * 100.0)
            elif "tower" in a:
                cx, cy = tower["center"]
                ang = math.radians(float(a["angle"]))
                x = cx + float(a["radius"]) * math.cos(ang)
                y = cy + float(a["radius"]) * math.sin(ang)
                floor = tower["pad_z"] + (tower["shaft_top"] if a["tower"] == "gallery" else 0.0)
                loc = (x * 100.0, y * 100.0, (floor + up) * 100.0)
            elif "structure" in a:
                x, y, yaw = self.structure_local(a["structure"], a["local"], float(a.get("local_yaw", 0.0)))
                if a.get("on_floor"):
                    # On the structure's raised floor (its porch or deck): the kit's floor stands at base above the
                    # structure's origin, and greybox.py raises base by the ground's relief under the footprint.
                    _x, _y, oz, _yaw, relief = self.structure_origin(a["structure"])
                    floor_cm = oz + float(self.kit_spec(a["structure"]).get("base", 0.0)) + relief * 100.0
                    loc = (x * 100.0, y * 100.0, floor_cm + up * 100.0)
                else:
                    loc = (x * 100.0, y * 100.0, (self.ground(x, y) + up) * 100.0)
            elif "deck" in a:
                (dcx, dcy), top, heading = deck
                f, r = a["deck"]
                h = math.radians(heading)
                x = dcx + (math.cos(h) * f - math.sin(h) * r) * 100.0
                y = dcy + (math.sin(h) * f + math.cos(h) * r) * 100.0
                loc = (x, y, top + up * 100.0)
            else:
                x, y = a["at"]
                z = float(a["z"]) if "z" in a else self.ground(x, y)
                loc = (x * 100.0, y * 100.0, (z + up) * 100.0)
            out.append((name, loc, yaw, bool(a.get("held", False))))
        return out
