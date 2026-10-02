"""The west headland (sombre.headland) and the Ashland Grey's stern (sombre.ashland_grey). Spec: Design/POIs/sombre.headland.md.

Owner: Claude. Built in VS-16 (the post, its hide, the footprints, the Grey, the wrecker, the storm lantern for Dell).
VS-10 adds only what the crossing needs to see from the Ida's deck (ledger Design/POIs/sombre_ids.md §3.1):

  Lantern_Crossing: the false light burning on the post before the strike. It is the only storm moment before
  sombre.storm exists, so it is its own light, burning while the reef has not been struck. After the strike the
  post is dark until VS-16's storm lantern (sombre.storm, !sombre.false_light_taken) takes over.

It replaces the greybox's always-lit glow box on the post (GREYBOX_RETIRE). The post, its arm, and the hide stay the
greybox's until VS-16.

Called by build_pointe_sombre.py as build(tk). Owns only actors tagged Cell:headland.
"""
import layout as layout_mod

CELL = "headland"
REEF_STRUCK = "sombre.reef_struck"
GREYBOX_RETIRE = ("FalseLight_Lantern",)

LANTERN_COLOR = (255, 170, 90)   # a warm oil flame, against the cold storm
LANTERN_CANDELAS = 6000.0        # throws a warm pool on the hide that reads from the Ida's deck, 345 m away
LANTERN_RADIUS = 3000.0
# Seen only from the deck (it goes out at the strike, before anyone can reach the post), so it is built for distance:
# a 45 cm lamp is under a pixel at 345 m, and a hotter one saturates to a white dot (VS-10 captures).
LANTERN_GLOW_CM = 120.0
LANTERN_GLOW = (16.0, 5.5, 1.0, 0.9)


def build(tk):
    lay = layout_mod.Layout.load(tk.island)
    p = lay.data["landmarks"]["false_light_post"]
    x, y = p["center"]
    top = lay.ground(x, y) * 100.0 + p["post_height"] * 100.0
    # Where the greybox hung its lantern: on the arm, 80 cm toward the lake, 70 cm below the post's top.
    where = (x * 100.0, y * 100.0 - 80.0, top - 70.0)
    tk.flicker_light("Lantern_Crossing", CELL, where, LANTERN_COLOR, LANTERN_CANDELAS, LANTERN_RADIUS,
                     conditions=[tk.flag(REEF_STRUCK, negate=True)], glow_cm=LANTERN_GLOW_CM,
                     glow_material=tk.glow("MI_DC_Sombre_FalseLight", LANTERN_GLOW),
                     min_brightness=0.6, dropout=0.04, interval=(0.08, 0.5))
    tk.log(f"headland: Lantern_Crossing at {where} (until the strike)")
