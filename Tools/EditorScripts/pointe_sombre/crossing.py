"""The crossing (sombre.crossing): the Ida's deck, offshore south-west of the island (VerticalSlicePhasePlan.txt §5.4).

Owner: Claude. Spec: Design/POIs/sombre.crossing.md. VS-04 built the stub (a walkable deck facing the island, rails, a
wheelhouse, the one-way door); VS-10 (2026-10-02) makes it the played B0:
  - the wheelhouse door "Tell Varga about the light" sets sombre.reef_struck AND sombre.storm (ledger §3.1: the storm is
    gameplay state, set by the strike; the atmosphere looks do not read it), then fades to the quay with the strike card
  - Liv's letter on the hatch cover in the lee of the wheelhouse: the first read gives liv_letter and sets
    watch.mara_travelling (the prologue's P1 text); later reads give the prologue's re-read line
  - the recap chart pinned to the wheelhouse front: where the player is (PROVISIONAL wording)
Mara at the rail is the harbor's actor (one Mara, placed by presence; harbor.py). The false light seen ahead is the
headland's Lantern_Crossing. The new-game player start stands on this deck; the core script places it at
Anchor_NewGame_Deck.

Called by build_pointe_sombre.py as build(tk). Owns only actors tagged Cell:crossing.
"""
import math

CELL = "crossing"
REEF_STRUCK = "sombre.reef_struck"
STORM = "sombre.storm"
LETTER_READ = "watch.mara_travelling"   # the letter's flag in the prologue doc (P1, P7); the ledger's §3.3
# The card over the fade: the beat script's B0 "Sees" line, kept as the final default (VS-10). Anthony may replace it
# at Checkpoint B.
STRIKE_CARD = "The Ida turns for the burning light, then shudders on stone."

# Liv's letter: the prologue doc's P1 text, tie-neutral, and its re-read line (PROVISIONAL, as written there).
LETTER_TEXT = ("Posted from the old Authority landing, west end of the Narrows.\n"
               "The relay here talks if you feed it. I've fed it. I think I know where it's coming from.\n"
               "Don't come after me. You'll want to, so I'm saying it plainly: don't.\n"
               "Tell Varga she'll need a new listener for the season. Tell her I'm sorry.\n"
               "\u2014 L.")
LETTER_REREAD = ("Liv's letter. You know it by heart. Her capital A has no crossbar; she always said a crossbar reads "
                 "as a dash in a transcript.")
# The recap chart (VS-10 wording, PROVISIONAL): only the run's places; it explains nothing.
CHART_TEXT = ("Varga's chart of the run, pinned in the lee of the wheelhouse. Her pencil line runs from the Authority "
              "Shore, east through the Narrows, to Pointe Sombre. She has circled the light on the Pointe.")

DECK_CENTER = (-33000.0, -36000.0)   # matches build_pointe_sombre.DECK_CENTER (the player start's anchor)
DECK_TOP = 300.0
HEADING = 25.0                        # the bow points at the west headland and, beyond it, the tower
LENGTH, BEAM = 2400.0, 720.0
# The hatch cover in the lee of the wheelhouse, in the deck's frame (cm forward of midships, cm to starboard), and its
# size; the letter lies on it. The chart hangs on the wheelhouse front, to starboard of the door.
HATCH = (-420.0, -170.0)
HATCH_SIZE = (130.0, 160.0, 42.0)
CHART_RIGHT = 175.0


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
        parts.append(tk.box(f"Ida{tag}_HatchBlock", CELL, at(HATCH[0], HATCH[1], deck_top + HATCH_SIZE[2] / 2.0),
                            HATCH_SIZE, rot=rot, hidden=True))
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
    # The rails: a low rusted toe board, stanchions every 2 m, and a dark top rail (VS-10: a solid steel wall read as a
    # band of water in the capture). Art only; the hidden rail blocks in collision() stop the player.
    paint = tk.flat("MI_DC_Sombre_RailPaint", (0.05, 0.055, 0.055, 1.0), roughness=0.6)
    rot = (0.0, HEADING, 0.0)
    for side, sign in (("Port", -1.0), ("Starboard", 1.0)):
        r = sign * (BEAM / 2 - 10.0)
        art.append(tk.box(f"Ida_ToeBoard_{side}", CELL, at(0.0, r, DECK_TOP + 14.0), (LENGTH, 10.0, 28.0), rot=rot,
                          material=rust, collision=False))
        art.append(tk.box(f"Ida_TopRail_{side}", CELL, at(-80.0, r, DECK_TOP + 100.0), (LENGTH - 160.0, 8.0, 8.0),
                          rot=rot, material=paint, collision=False))
        for k in range(12):
            art.append(tk.box(f"Ida_Stanchion_{side}_{k:02d}", CELL, at(-LENGTH / 2 + 80.0 + k * 200.0, r, DECK_TOP + 50.0),
                              (6.0, 6.0, 100.0), rot=rot, material=paint, collision=False))
    glass = tk.flat("MI_DC_Sombre_WheelhouseGlass", (0.02, 0.025, 0.03, 1.0), roughness=0.1)
    for k, right in enumerate((-180.0, 180.0)):
        art.append(tk.box(f"Ida_WheelhouseWindow_{k}", CELL, at(house_front + 3.0, right, DECK_TOP + 225.0),
                          (4.0, 110.0, 55.0), rot=rot, material=glass, collision=False))
    art.append(tk.box("Ida_TopRail_Bow", CELL, at(LENGTH / 2 - 10.0, 0.0, DECK_TOP + 100.0), (8.0, BEAM - 20.0, 8.0),
                      rot=rot, material=paint, collision=False))
    art.append(tk.box("Ida_ToeBoard_Bow", CELL, at(LENGTH / 2 - 10.0, 0.0, DECK_TOP + 14.0), (10.0, BEAM, 28.0),
                      rot=rot, material=rust, collision=False))

    # VS-10: the hatch cover (art; its collision is in collision()), Liv's letter on it, and the recap chart. They are
    # part of the vessel, so they move with her to the berth; the letter still reads there (its re-read line).
    timber = tk.surface("MI_DC_TimberDark")
    art.append(tk.box("Ida_HatchCover", CELL, at(HATCH[0], HATCH[1], DECK_TOP + HATCH_SIZE[2] / 2.0), HATCH_SIZE,
                      rot=(0.0, HEADING, 0.0), material=timber, collision=False))
    paper = tk.flat("MI_DC_Sombre_Paper", (0.62, 0.59, 0.50, 1.0))
    letter = tk.inspectable("Ida_Letter", CELL, at(HATCH[0] + 10.0, HATCH[1] + 15.0, DECK_TOP + HATCH_SIZE[2] + 0.6),
                            (22.0, 28.0, 1.2), "Liv's letter", LETTER_REREAD, action="Read", duration=14.0,
                            rot=(0.0, HEADING + 12.0, 0.0), material=paper,
                            variants=[tk.variant(LETTER_TEXT, [tk.flag(LETTER_READ, negate=True)],
                                                 [tk.cons("GIVE_ITEM", id="liv_letter"),
                                                  tk.cons("SET_WORLD_FLAG", id=LETTER_READ)], action="Read")])
    tk.own(letter, CELL, "Crossing:Letter")
    chart_paper = tk.flat("MI_DC_Sombre_Chart", (0.50, 0.53, 0.50, 1.0))
    chart = tk.inspectable("Ida_Chart", CELL, at(house_front + 2.0, CHART_RIGHT, DECK_TOP + 150.0), (2.0, 70.0, 52.0),
                           "Chart", CHART_TEXT, action="Look", duration=9.0, rot=(0.0, HEADING, 0.0),
                           material=chart_paper)
    tk.own(chart, CELL, "Crossing:Chart")
    art += [letter, chart]

    # The wheelhouse door: the crossing's exit. The strike sets the storm as world state (ledger §3.1).
    door = tk.portal("Ida_WheelhouseDoor", CELL, at(house_front + 6.0, 0.0, DECK_TOP + 106.0), (8.0, 100.0, 210.0),
                     "Wheelhouse door",
                     [tk.portal_variant("strike", "Tell Varga about the light",
                                        consequences=[tk.cons("SET_WORLD_FLAG", id=REEF_STRUCK),
                                                      tk.cons("SET_WORLD_FLAG", id=STORM)])],
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
    tk.log(f"crossing: deck at {DECK_CENTER}, heading {HEADING}; the letter and chart; berth at the quay after the strike")
