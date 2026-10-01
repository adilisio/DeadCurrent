"""Pointe Sombre's route-timing report (plan §5.5, VS-08): every compressed-geography leg, walked on the real terrain.

Pure Python, no engine:

    py -3 Tools/EditorScripts/pointe_sombre/routes.py            the table, and Tools/PointeSombre/out/routes.json
    py -3 Tools/EditorScripts/pointe_sombre/routes.py --check    exit 1 if a leg is off target or not walkable

A leg (Tools/PointeSombre/routes.json) names a path (island.json "paths": the trails the terrain paints and the player
follows). The report samples the path every 0.5 m on the terrain mesh's own triangles (island.MeshSurface, the surface
the player walks), and gives:
  - walked length (3D) and time at the player's walk speed (ADCPlayerCharacter MaxWalkSpeed, 450 cm/s)
  - the steepest 2 m of the path (the engine's walkable floor is 44.76°; the report fails a leg over walkable_max_deg)
  - the lowest point (a leg may cross the causeway awash, never deep water) and any point outside the fence
  - the target window and whether the time is within ±20% of it (plan VS-08's done-when)
The engine check is DeadCurrent.Map.Sombre.Greybox, which walks the real player along each path and times it.
"""
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import island as island_mod  # noqa: E402

ROUTES_FILE = os.path.normpath(os.path.join(HERE, "..", "..", "PointeSombre", "routes.json"))
OUT_FILE = os.path.normpath(os.path.join(HERE, "..", "..", "PointeSombre", "out", "routes.json"))
SAMPLE_M = 0.5


def load_routes(path=None):
    with open(path or ROUTES_FILE, encoding="utf-8") as handle:
        return json.load(handle)


def path_points(island, path_id):
    for p in island.spec.get("paths", []):
        if p["id"] == path_id:
            return [tuple(q) for q in p["points"]]
    raise KeyError(f"no path '{path_id}' in island.json")


def walk(surface, island, points, slope_window_m=2.0):
    """Sample the polyline; return the measures."""
    samples = []
    for (ax, ay), (bx, by) in zip(points, points[1:]):
        seg = math.hypot(bx - ax, by - ay)
        n = max(1, int(math.ceil(seg / SAMPLE_M)))
        for k in range(n):
            t = k / n
            samples.append((ax + (bx - ax) * t, ay + (by - ay) * t))
    samples.append(points[-1])
    heights = []
    missing = 0
    outside = 0
    for x, y in samples:
        h = surface.height(x, y)
        if h is None:
            missing += 1
            h = float("nan")
        if not island.inside_bounds(x, y):
            outside += 1
        heights.append(h)
    length = 0.0
    flat = 0.0
    for i in range(1, len(samples)):
        dx = samples[i][0] - samples[i - 1][0]
        dy = samples[i][1] - samples[i - 1][1]
        d = math.hypot(dx, dy)
        flat += d
        length += math.sqrt(d * d + (heights[i] - heights[i - 1]) ** 2) if not math.isnan(heights[i]) else d
    window = max(1, int(round(slope_window_m / SAMPLE_M)))
    steepest = 0.0
    steepest_at = samples[0]
    for i in range(window, len(samples)):
        run = math.hypot(samples[i][0] - samples[i - window][0], samples[i][1] - samples[i - window][1])
        rise = abs(heights[i] - heights[i - window])
        if run > 0 and not math.isnan(rise):
            deg = math.degrees(math.atan2(rise, run))
            if deg > steepest:
                steepest, steepest_at = deg, samples[i]
    valid = [h for h in heights if not math.isnan(h)]
    return {
        "length_m": round(length, 1),
        "plan_length_m": round(flat, 1),
        "climb_m": round(sum(max(0.0, heights[i] - heights[i - 1]) for i in range(1, len(heights))
                             if not math.isnan(heights[i]) and not math.isnan(heights[i - 1])), 1),
        "steepest_deg": round(steepest, 1),
        "steepest_at": [round(steepest_at[0], 1), round(steepest_at[1], 1)],
        "lowest_m": round(min(valid), 2) if valid else None,
        "start_z_m": round(heights[0], 2),
        "end_z_m": round(heights[-1], 2),
        "missing_samples": missing,
        "outside_fence_samples": outside,
    }


def report(island=None, routes=None):
    island = island or island_mod.Island.load()
    routes = routes or load_routes()
    surface = island_mod.MeshSurface(island)
    speed = float(routes["walk_speed_cm_s"]) / 100.0
    tolerance = float(routes["tolerance"])
    legs = []
    for leg in routes["legs"]:
        pts = path_points(island, leg["path"])
        if leg.get("reverse"):
            pts = list(reversed(pts))
        m = walk(surface, island, pts)
        seconds = m["length_m"] / speed
        lo, hi = leg["target_s"]
        ok_time = lo * (1.0 - tolerance) <= seconds <= hi * (1.0 + tolerance)
        in_window = lo <= seconds <= hi
        ok_walk = (m["steepest_deg"] <= float(routes["walkable_max_deg"]) and m["missing_samples"] == 0
                   and m["outside_fence_samples"] == 0 and m["lowest_m"] >= float(routes["lowest_allowed_m"]))
        legs.append(dict(id=leg["id"], path=leg["path"], target_s=[lo, hi], seconds=round(seconds, 1),
                         within_window=in_window, within_tolerance=ok_time, walkable=ok_walk, **m))
    return {"about": "Generated by Tools/EditorScripts/pointe_sombre/routes.py from island.json and routes.json. "
                     "Times are walked length / walk speed on the terrain mesh surface.",
            "island_hash": island.spec_hash(), "walk_speed_m_s": speed, "tolerance": tolerance, "legs": legs}


def main():
    result = report()
    print(f"{'leg':22} {'target':>9} {'time':>6} {'len':>7} {'climb':>6} {'steep':>6} {'low':>6}  result")
    bad = 0
    for leg in result["legs"]:
        flag = "ok" if leg["within_tolerance"] and leg["walkable"] else "FAIL"
        bad += flag != "ok"
        note = "" if leg["within_window"] else " (outside the window, within ±20%)" if leg["within_tolerance"] else ""
        print(f"{leg['id']:22} {leg['target_s'][0]:>3}-{leg['target_s'][1]:<3}s {leg['seconds']:>5}s "
              f"{leg['length_m']:>6}m {leg['climb_m']:>5}m {leg['steepest_deg']:>5}° {leg['lowest_m']:>5}m  "
              f"{flag}{note}{'' if leg['walkable'] else ' NOT WALKABLE at ' + str(leg['steepest_at'])}")
    os.makedirs(os.path.dirname(OUT_FILE), exist_ok=True)
    with open(OUT_FILE, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(result, handle, indent=1)
        handle.write("\n")
    if "--check" in sys.argv and bad:
        print(f"{bad} leg(s) fail")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
