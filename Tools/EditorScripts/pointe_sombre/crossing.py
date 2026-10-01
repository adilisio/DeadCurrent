"""The crossing (sombre.crossing): the Ida's deck, offshore south-west of the island (VerticalSlicePhasePlan.txt §5.4).

VS-04 STUB, built by the Integrator to prove the architecture: a walkable deck facing the island, rails, a wheelhouse,
and the wheelhouse door, a one-way cell portal to the quay that sets sombre.reef_struck. The scene itself (Liv's
letter, the chart, Mara at the rail, the false light, the storm on the deck) is VS-10's, from its cell spec
(Design/POIs/sombre.crossing.md, VS-07). The new-game player start stands on this deck; the core script places it at
Anchor_NewGame_Deck.

Called by build_pointe_sombre.py as build(tk). Owns only actors tagged Cell:crossing.
"""
import math

CELL = "crossing"
REEF_STRUCK = "sombre.reef_struck"
# PROVISIONAL card text over the fade (the beat script's B0 "Sees" line). VS-10 writes the final text.
STRIKE_CARD = "The Ida turns for the burning light, then shudders on stone."

DECK_CENTER = (-33000.0, -36000.0)   # matches build_pointe_sombre.DECK_CENTER (the player start's anchor)
DECK_TOP = 300.0
HEADING = 25.0                        # the bow points at the west headland and, beyond it, the tower
LENGTH, BEAM = 2400.0, 720.0


def build(tk):
    yaw = math.radians(HEADING)
    fx, fy = math.cos(yaw), math.sin(yaw)          # forward (bow)
    rx, ry = -math.sin(yaw), math.cos(yaw)         # starboard

    def at(forward, right, z):
        return (DECK_CENTER[0] + fx * forward + rx * right, DECK_CENTER[1] + fy * forward + ry * right, z)

    rust = tk.surface("MI_DC_RustPaint")
    steel = tk.surface("MI_DC_Steel")

    # Hull (art, NoCollision) and the walkable deck under it (hidden collision), the Landing Stage pattern.
    tk.box("Ida_Hull", CELL, at(0.0, 0.0, DECK_TOP - 260.0), (LENGTH, BEAM, 520.0), rot=(0.0, HEADING, 0.0),
           material=rust, collision=False)
    tk.box("Ida_Deck", CELL, at(0.0, 0.0, DECK_TOP - 10.0), (LENGTH - 40.0, BEAM - 40.0, 20.0),
           rot=(0.0, HEADING, 0.0), hidden=True)
    for side, sign in (("Port", -1.0), ("Starboard", 1.0)):
        tk.box(f"Ida_Rail_{side}", CELL, at(0.0, sign * (BEAM / 2 - 10.0), DECK_TOP + 50.0), (LENGTH, 8.0, 100.0),
               rot=(0.0, HEADING, 0.0), material=steel, collision=False)
        tk.box(f"Ida_RailBlock_{side}", CELL, at(0.0, sign * (BEAM / 2 - 10.0), DECK_TOP + 100.0),
               (LENGTH, 30.0, 200.0), rot=(0.0, HEADING, 0.0), hidden=True)
    for end, f in (("Bow", LENGTH / 2 - 10.0), ("Stern", -LENGTH / 2 + 10.0)):
        tk.box(f"Ida_RailBlock_{end}", CELL, at(f, 0.0, DECK_TOP + 100.0), (30.0, BEAM, 200.0),
               rot=(0.0, HEADING, 0.0), hidden=True)

    # Wheelhouse at the stern; its forward face carries the door.
    house_back, house_front = -LENGTH / 2 + 60.0, -LENGTH / 2 + 560.0
    tk.box("Ida_Wheelhouse", CELL, at((house_back + house_front) / 2, 0.0, DECK_TOP + 150.0),
           (house_front - house_back, 520.0, 300.0), rot=(0.0, HEADING, 0.0), material=steel)
    door = tk.portal("Ida_WheelhouseDoor", CELL, at(house_front + 6.0, 0.0, DECK_TOP + 106.0), (8.0, 100.0, 210.0),
                     "Wheelhouse door",
                     [tk.portal_variant("strike", "Tell Varga about the light",
                                        consequences=[tk.cons("SET_WORLD_FLAG", id=REEF_STRUCK)])],
                     tk.anchor("Anchor_CrossingExit_Quay"), card_text=STRIKE_CARD, yaw=HEADING)
    tk.own(door, CELL, "Crossing:WheelhouseDoor")
    tk.log(f"crossing stub: deck at {DECK_CENTER}, heading {HEADING}")
