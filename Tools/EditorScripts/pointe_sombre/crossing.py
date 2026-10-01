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
    rust = tk.surface("MI_DC_RustPaint")
    steel = tk.surface("MI_DC_Steel")
    house_back, house_front = -LENGTH / 2 + 60.0, -LENGTH / 2 + 560.0

    def frame(center, heading):
        h = math.radians(heading)
        fx, fy, rx, ry = math.cos(h), math.sin(h), -math.sin(h), math.cos(h)

        def at(forward, right, z):
            return (center[0] + fx * forward + rx * right, center[1] + fy * forward + ry * right, z)
        return at

    def collision(tag, center, heading, deck_top):
        """The Ida's walkable shape, hidden and static: deck, keel, rails, wheelhouse. Built where she is at sea and
        again at her berth (VS-08). Nothing a player stands on is moved by presence: a load restores the player and
        then snaps presence, and a moved floor would carry a player who had been standing on it."""
        at = frame(center, heading)
        rot = (0.0, heading, 0.0)
        parts = [
            tk.box(f"Ida{tag}_Deck", CELL, at(0.0, 0.0, deck_top - 10.0), (LENGTH - 40.0, BEAM - 40.0, 20.0), rot=rot, hidden=True),
            # A keel under the deck, so at the quay nobody wades under the moored hull.
            tk.box(f"Ida{tag}_Keel", CELL, at(0.0, 0.0, deck_top - 240.0), (LENGTH - 40.0, BEAM - 40.0, 420.0), rot=rot, hidden=True),
            tk.box(f"Ida{tag}_WheelhouseBlock", CELL, at((house_back + house_front) / 2, 0.0, deck_top + 150.0),
                   (house_front - house_back, 520.0, 300.0), rot=rot, hidden=True),
        ]
        for side, sign in (("Port", -1.0), ("Starboard", 1.0)):
            parts.append(tk.box(f"Ida{tag}_RailBlock_{side}", CELL, at(0.0, sign * (BEAM / 2 - 10.0), deck_top + 100.0),
                                (LENGTH, 30.0, 200.0), rot=rot, hidden=True))
        for end, f in (("Bow", LENGTH / 2 - 10.0), ("Stern", -LENGTH / 2 + 10.0)):
            parts.append(tk.box(f"Ida{tag}_RailBlock_{end}", CELL, at(f, 0.0, deck_top + 100.0), (30.0, BEAM, 200.0),
                                rot=rot, hidden=True))
        return parts

    # At sea: the walkable deck for the crossing, and the art (hull, rails, wheelhouse; NoCollision), the Landing Stage
    # pattern.
    at = frame(DECK_CENTER, HEADING)
    collision("", DECK_CENTER, HEADING, DECK_TOP)
    art = [
        tk.box("Ida_Hull", CELL, at(0.0, 0.0, DECK_TOP - 260.0), (LENGTH, BEAM, 520.0), rot=(0.0, HEADING, 0.0),
               material=rust, collision=False),
        tk.box("Ida_Wheelhouse", CELL, at((house_back + house_front) / 2, 0.0, DECK_TOP + 150.0),
               (house_front - house_back, 520.0, 300.0), rot=(0.0, HEADING, 0.0), material=steel, collision=False),
    ]
    for side, sign in (("Port", -1.0), ("Starboard", 1.0)):
        art.append(tk.box(f"Ida_Rail_{side}", CELL, at(0.0, sign * (BEAM / 2 - 10.0), DECK_TOP + 50.0),
                          (LENGTH, 8.0, 100.0), rot=(0.0, HEADING, 0.0), material=steel, collision=False))

    # The wheelhouse door: the crossing's exit.
    door = tk.portal("Ida_WheelhouseDoor", CELL, at(house_front + 6.0, 0.0, DECK_TOP + 106.0), (8.0, 100.0, 210.0),
                     "Wheelhouse door",
                     [tk.portal_variant("strike", "Tell Varga about the light",
                                        consequences=[tk.cons("SET_WORLD_FLAG", id=REEF_STRUCK)])],
                     tk.anchor("Anchor_CrossingExit_Quay"), card_text=STRIKE_CARD, yaw=HEADING)
    tk.own(door, CELL, "Crossing:WheelhouseDoor")

    # VS-08: after the strike the Ida lies at her berth on the quay (Anchor_IdaBerth: the berth's deck-top centre and
    # the turn from this heading). The same art, moved by presence on the scene cut, not a second vessel (plan §5.4);
    # her collision at the berth is built there, static. The wheelhouse door is gone after the strike, so the strike
    # cannot be replayed.
    berth = tk.anchor("Anchor_IdaBerth")
    turn = berth.get_actor_rotation().yaw
    b = berth.get_actor_location()
    collision("Berth", (b.x, b.y), HEADING + turn, b.z)
    tk.presence("Ida", CELL, art, [
        tk.state("berth", [tk.flag(REEF_STRUCK)], place=tk.placement(b, berth.get_actor_rotation())),
    ], pivot=(DECK_CENTER[0], DECK_CENTER[1], DECK_TOP))
    tk.presence("IdaDoor", CELL, [door], [tk.state("gone", [tk.flag(REEF_STRUCK)], present=False)])
    tk.log(f"crossing stub: deck at {DECK_CENTER}, heading {HEADING}; berth at the quay after the strike")
