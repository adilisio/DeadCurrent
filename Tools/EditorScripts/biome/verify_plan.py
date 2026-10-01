"""Pure-Python checks of the shoreline recipe's planner (no engine). Run before any editor launch:

    py -3 Tools/EditorScripts/biome/verify_plan.py              the planner's rules, on the committed recipe and zones
    py -3 Tools/EditorScripts/biome/verify_plan.py --manifest   also re-plan the committed manifest from its own
                                                                recorded inputs and require the same hash

Exit code 0 when every check passes. Prints the plan's hash and counts.
"""
import copy
import json
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import plan  # noqa: E402

MANIFEST = os.path.join(plan.BIOMES, "out", "Lvl_PointeSombre.json")
FAILURES = []


def check(ok, message):
    print(("  ok   " if ok else "  FAIL ") + message)
    if not ok:
        FAILURES.append(message)


def in_window(value, window):
    return window["min"] - 1e-6 <= value <= window["max"] + 1e-6


def main():
    island = plan.island_mod.Island.load()
    recipe = plan.load_recipe()
    zones = plan.load_zones()
    test = next(z for z in zones if z["id"] == "shore_test")
    families = {f["id"]: f for f in recipe["families"] if f.get("enabled", True)}

    # A fake door inside the sheltered bight, in its densest pocket (23 instances within 4 m without it): its
    # automatic radius must leave a hole.
    fixture = [{"kind": "portal", "id": "fixture_door", "shape": {"shape": "circle", "center_cm": [-7100, -12900],
                                                                    "radius_cm": 0}}]

    print("Determinism")
    a = plan.generate(island, recipe, zones, fixture)
    b = plan.generate(island, recipe, zones, fixture)
    check(a["hash"] == b["hash"], f"two generations give the same hash ({a['hash'][:16]})")
    shuffled = json.loads(json.dumps(recipe), object_pairs_hook=lambda pairs: dict(reversed(pairs)))
    check(plan.generate(island, shuffled, zones, fixture)["hash"] == a["hash"],
          "key order in the recipe does not change the hash")
    reseeded = copy.deepcopy(recipe)
    reseeded["seed"] += 1
    check(plan.generate(island, reseeded, zones, fixture)["hash"] != a["hash"], "a new recipe seed changes the hash")
    check(plan.generate(island, recipe, zones, [])["hash"] != a["hash"],
          "the automatic exclusions are part of the hash")

    print("Placement rules")
    polys = {(z["id"], p["id"]): [tuple(q) for q in p["points"]] for z in zones for p in z["polygons"]}
    surface = plan.Surface(island)
    bad = {"polygon": 0, "bounds": 0, "missing": 0, "slope": 0, "coast": 0, "height": 0, "collision": 0}
    for p in a["placements"]:
        x, y = p["x_mm"] / 1000.0, p["y_mm"] / 1000.0
        fam = families[p["family"]]
        if not plan.island_mod.point_in_polygon(x, y, polys[(p["zone"], p["polygon"])]):
            bad["polygon"] += 1
        if not island.inside_bounds(x, y):
            bad["bounds"] += 1
        tri = surface.triangle(x, y)
        if tri is None:
            bad["missing"] += 1
            continue
        h, (gx, gy) = tri
        # The stored point is quantized to 1 mm; the windows are checked with that much slack.
        if not in_window(math.degrees(math.atan(math.hypot(gx, gy))), {"min": fam["slope_deg"]["min"] - 0.5,
                                                                      "max": fam["slope_deg"]["max"] + 0.5}):
            bad["slope"] += 1
        if not in_window(plan.island_mod.signed_distance(x, y, island.coast),
                         {"min": fam["coast_distance_m"]["min"] - 0.01, "max": fam["coast_distance_m"]["max"] + 0.01}):
            bad["coast"] += 1
        if not in_window(h - recipe["waterline_m"], {"min": fam["height_above_water_m"]["min"] - 0.01,
                                                     "max": fam["height_above_water_m"]["max"] + 0.01}):
            bad["height"] += 1
        if fam["collision"] != "NoCollision":
            bad["collision"] += 1
    for key, count in bad.items():
        check(count == 0, f"no placement fails the {key} rule ({count})")
    check(all(p["z_mm"] < 200000 and p["y_mm"] < plan.INTERIOR_Y_MIN_M * 1000 for p in a["placements"]),
          "nothing in the interior slots")
    check(all(isinstance(v, (int, str)) for p in a["placements"] for v in p.values()),
          "placements hold only integers and strings (the C++ test re-hashes them)")

    print("Exclusions")
    exclusions = plan.exclusion_set(island, recipe, test, fixture)
    hits = {}
    for p in a["placements"]:
        x, y = p["x_mm"] / 1000.0, p["y_mm"] / 1000.0
        for ex_id, shape, extra in exclusions:
            if plan._shape_hit(shape, x, y, extra - 0.001):
                hits[ex_id] = hits.get(ex_id, 0) + 1
    for ex_id, _shape, _extra in exclusions:
        check(hits.get(ex_id, 0) == 0, f"nothing inside {ex_id}")
    # Non-vacuous: without the exclusions, the same places would have been filled.
    bare_zones = copy.deepcopy(zones)
    for z in bare_zones:
        z["exclusions"] = []
        z["exclude_pads"] = []
    bare = plan.generate(island, recipe, bare_zones, [])
    for ex_id, shape, extra in exclusions:
        if ex_id.startswith("pad:"):
            continue
        filled = sum(1 for p in bare["placements"] if plan._shape_hit(shape, p["x_mm"] / 1000.0, p["y_mm"] / 1000.0, extra))
        check(filled > 0, f"{ex_id} would hold instances without the exclusion ({filled})")

    print("Exposure")
    expo = plan.Exposure(island, recipe["exposure"])
    for name, want in (("headland_post", 1.0), ("quay_arrival", 0.0)):
        x, y = island.places[name]
        score = expo.score(x, y)
        check((score > 0.8) if want else (score < 0.2), f"{name} scores {'exposed' if want else 'protected'} ({score:.2f})")
    count = {}
    for p in a["placements"]:
        count[(p["polygon"], p["family"])] = count.get((p["polygon"], p["family"]), 0) + 1
    def n(poly, fam):
        return count.get((poly, fam), 0)
    check(n("sheltered_bight", "cobble") > 1.5 * n("headland_tip", "cobble"),
          f"cobble: sheltered {n('sheltered_bight', 'cobble')} well above exposed {n('headland_tip', 'cobble')}")
    check(n("headland_tip", "talus") > 3 * max(1, n("sheltered_bight", "talus")),
          f"talus: exposed {n('headland_tip', 'talus')} well above sheltered {n('sheltered_bight', 'talus')}")
    check(n("sheltered_bight", "driftwood") > n("headland_tip", "driftwood"),
          f"driftwood: sheltered {n('sheltered_bight', 'driftwood')} above exposed {n('headland_tip', 'driftwood')}")

    print("Caps and policy")
    check(sum(a["counts"].values()) <= recipe["caps"]["instances_total"],
          f"{sum(a['counts'].values())} instances within the cap {recipe['caps']['instances_total']}")
    for fid, fam in families.items():
        check(a["counts"].get(fid, 0) <= fam["max_instances"], f"{fid} {a['counts'].get(fid, 0)} <= {fam['max_instances']}")
    bad_recipe = copy.deepcopy(recipe)
    bad_recipe["families"][0]["collision"] = "BlockAll"
    try:
        plan.validate_recipe(bad_recipe)
        check(False, "a family with collision is refused")
    except plan.RecipeError:
        check(True, "a family with collision is refused")

    if "--manifest" in sys.argv:
        print("Committed manifest")
        with open(MANIFEST, encoding="utf-8") as handle:
            manifest = json.load(handle)
        check(plan.payload_hash(manifest) == manifest["hash"], "the manifest's hash matches its own payload")
        again = plan.generate(island, recipe, zones, manifest["inputs"]["automatic"])
        check(again["hash"] == manifest["hash"],
              f"re-planning from the committed inputs gives the committed hash ({again['hash'][:16]})")

    print(f"plan {a['hash']} counts {a['counts']} total {sum(a['counts'].values())}")
    if FAILURES:
        print(f"{len(FAILURES)} check(s) failed")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
