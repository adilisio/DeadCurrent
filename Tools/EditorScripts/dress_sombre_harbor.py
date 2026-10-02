"""Dress the harbor (sombre.harbor): the stateless props of Lvl_PointeSombre_Art_Harbor (VerticalSlicePhasePlan.txt §5.6).

Owner: the harbor cell (Claude, VS-10). Spec: Design/POIs/sombre.harbor.md. Run with
Tools\\RebuildContent.bat dress_sombre_harbor (also part of a full rebuild, after the map). Idempotent: the sublevel
is cleared and rebuilt each run. Everything here is NoCollision dressing; the quay's people, the Ida, the berth fender,
and every rule live in the persistent map (pointe_sombre/harbor.py, crossing.py, greybox.py).

  - the quay's timber face: piles along the water side of the berth fender
  - working clutter (barrels, crates, a bucket) in three small groups on the quay apron, off the three trails that
    leave the arrival, clear of Mara's spot, and clear of the stretch of quay where the player talks to Varga
Positions are metres in the island frame (X north, Y east), from Tools/PointeSombre/greybox.json (the fender, the
sheds) and island.json (the trails); ground heights come from the island heightfield.
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SOMBRE = os.path.join(HERE, "pointe_sombre")
if SOMBRE not in sys.path:
    sys.path.insert(0, SOMBRE)

import art  # noqa: E402

POLES = "/Game/Smugglers_cove/meshes/structures/SM_wooden_pier_poles"
BARREL_A = "/Game/Smugglers_cove/meshes/props/SM_wooden_barrels_01_a"
BARREL_B = "/Game/Smugglers_cove/meshes/props/SM_wooden_barrels_01_b"
CRATE = "/Game/Smugglers_cove/meshes/props/SM_wooden_crate_02"
BUCKET = "/Game/Smugglers_cove/meshes/props/SM_wooden_bucket_01"

# The berth fender runs x -89.2..-87.0, y -1..25 (greybox.json). The piles stand just off its water side, from the
# lake bed (z -1.6 m) to 0.6 m above the quay (2.4 m).
PILE_X = -89.75
PILE_Y = [-1.0 + 2.6 * k for k in range(11)]
PILE_BASE_CM, PILE_HEIGHT_CM = -160.0, 400.0

# (label, mesh, x m, y m, yaw, height cm or None, fit cm or None). Three groups on the apron (pad "quay apron",
# centre (-78, 10), radius 20 m): by the east end of the berth, by the west shed, and at the berth's west end.
CLUTTER = [
    ("Quay_BarrelsEast", BARREL_A, -85.6, 25.6, 20.0, 110.0, None),
    ("Quay_CrateEast", CRATE, -84.4, 26.8, -12.0, None, 85.0),
    ("Quay_BucketEast", BUCKET, -85.0, 24.2, 0.0, None, 32.0),
    ("Quay_BarrelsWest", BARREL_B, -84.6, -14.2, -35.0, 110.0, None),
    ("Quay_CrateWest", CRATE, -83.4, -15.6, 8.0, None, 85.0),
    ("Quay_CrateBerth", CRATE, -86.6, 1.2, 4.0, None, 80.0),
    ("Quay_CrateBerthTop", CRATE, -86.6, 1.2, 31.0, None, 70.0),
]


def main():
    sub = art.ArtLevel("Harbor")
    for k, y in enumerate(PILE_Y):
        sub.prop(f"Quay_Pile_{k:02d}", POLES, (PILE_X * 100.0, y * 100.0, PILE_BASE_CM), yaw=90.0 + 7.0 * (k % 3),
                 height_cm=PILE_HEIGHT_CM)
    crate_top = {}
    for label, mesh, x, y, yaw, height, fit in CLUTTER:
        where = (x * 100.0, y * 100.0)
        if label.endswith("Top"):
            where = (x * 100.0, y * 100.0, crate_top[(x, y)])
        actor = sub.prop(label, mesh, where, yaw=yaw, height_cm=height, fit_cm=fit)
        if mesh == CRATE and not label.endswith("Top"):
            origin, extent = actor.get_actor_bounds(False)
            crate_top[(x, y)] = origin.z + extent.z
    sub.save()


main()
