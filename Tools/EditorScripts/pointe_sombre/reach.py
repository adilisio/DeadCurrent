"""Pointe Sombre's reachable-space check (VS-08; plan "inspect_bounds.py-style flood fill"). Pure Python.

    py -3 Tools/EditorScripts/pointe_sombre/reach.py          report; exit 1 on a void, an escape, or an unreachable place

Flood-fills the 2 m terrain grid from the quay arrival, on foot: a step to a neighbouring cell is walkable when the climb
between them is under the engine's walkable floor angle (44.76°). Every cell is inside the fence (the walls along
island.json bounds, 55 m tall) or it is not entered. Ground is the terrain mesh where it has a triangle, otherwise the
hidden safety floor (drop_below − 0.05 m), so "void" is a reachable cell with neither. Places that must be reachable on
foot: the store, the tower door, the cable hut, the west-headland post, the reef by the Ashland Grey, the vault's lower
door. Interiors are reached by portal (the map test uses them).
"""
import math
import os
import sys
from collections import deque

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import island as island_mod  # noqa: E402

WALKABLE_DEG = 44.76
MUST_REACH = {
    "store": (-45.0, -6.0),
    "tower_door": (17.0, 155.0),
    "cable_hut": (76.0, 98.0),
    "headland_post": (-20.0, -208.0),
    "grey_reef": (-110.0, -228.0),
    "lower_door": (62.0, 145.0),
    "settlement": (-26.0, -8.0),
}


def main():
    island = island_mod.Island.load()
    surface = island_mod.MeshSurface(island)
    g = island.spec["grid"]
    step = float(g["step"])
    floor = float(g["drop_below"]) - 0.05
    nx = int(round((g["x_max"] - g["x_min"]) / step))
    ny = int(round((g["y_max"] - g["y_min"]) / step))

    def center(i, j):
        return g["x_min"] + (i + 0.5) * step, g["y_min"] + (j + 0.5) * step

    ground = {}

    def ground_at(i, j):
        key = (i, j)
        if key not in ground:
            x, y = center(i, j)
            if not island.inside_bounds(x, y):
                ground[key] = None
            else:
                h = surface.height(x, y)
                ground[key] = (h, "terrain") if h is not None else (floor, "floor")
        return ground[key]

    def cell(x, y):
        return int((x - g["x_min"]) // step), int((y - g["y_min"]) // step)

    start = cell(*island.places["quay_arrival"])
    rise = math.tan(math.radians(WALKABLE_DEG)) * step
    seen = {start}
    queue = deque([start])
    while queue:
        i, j = queue.popleft()
        here = ground_at(i, j)
        for di, dj in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            n = (i + di, j + dj)
            if n in seen or not (0 <= n[0] < nx and 0 <= n[1] < ny):
                continue
            there = ground_at(*n)
            if there is None:
                continue   # outside the fence: the wall stops the player
            if abs(there[0] - here[0]) > rise:
                continue
            seen.add(n)
            queue.append(n)

    terrain = sum(1 for c in seen if ground[c][1] == "terrain")
    on_floor = sum(1 for c in seen if ground[c][1] == "floor")
    void = [c for c in seen if ground[c] is None]
    print(f"reachable cells from the quay: {len(seen)} ({len(seen) * step * step / 10000.0:.2f} ha); on terrain {terrain}, "
          f"on the safety floor {on_floor} (shallows off a cliff), void {len(void)}")
    bad = len(void)
    for name, (x, y) in MUST_REACH.items():
        ok = cell(x, y) in seen
        bad += not ok
        print(f"  {'ok  ' if ok else 'FAIL'} {name} reachable on foot")
    high = max(ground[c][0] for c in seen)
    wall = float(island.spec["bounds"]["height"])
    print(f"  highest reachable ground {high:.1f} m; fence top {wall:.0f} m ({'ok' if high + 2.0 < wall else 'FAIL'})")
    bad += not (high + 2.0 < wall)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
