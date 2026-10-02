"""Warden Casimir Hale (sombre_hale), Lights Office. Present only once `sombre.hale_arrived` (his owner's presence).

Text: `Design/Narrative/SLICE_DIALOGUE.md` §8, built as written. Ids: `Design/POIs/sombre_ids.md` §3, §4, §6.
- Every `first` choice sets `sombre.hale_met`, so later talks open on `offer`.
- `offer` gives the card (`section_key_compact`) while the player has none and nothing is decided yet; sends his crew
  up (`sombre.hale_crew_up`, the zero-investment way to splice the feed); and gives oil (`lamp_oil`) while the player
  carries none.
- His `liv` line is shared by every tie (no tie leak). Hale never mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


SPEAKER = "Hale"
MET = [cons("SET_WORLD_FLAG", id="sombre.hale_met")]
BYE = ("Goodbye.", None)
GO_ON = ("Go on.", "offer")


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


SOMBRE_HALE = dict(
    asset="DA_Dialogue_SombreHale",
    dialogue_id="sombre_hale",
    entry="first",
    entries=[
        dict(id="after_line", conditions=[flag("sombre.meeting_done"), flag("sombre.light_line")]),
        dict(id="after_hand", conditions=[flag("sombre.meeting_done"), flag("sombre.light_hand")]),
        dict(id="after_flooded", conditions=[flag("sombre.meeting_done"), flag("sombre.node_destroyed")]),
        dict(id="after_power", conditions=[flag("sombre.meeting_done"), flag("sombre.power_settlement")]),
        dict(id="after", conditions=[flag("sombre.meeting_done")]),
        dict(id="offer", conditions=[flag("sombre.hale_met")]),
        dict(id="first"),
    ],
    nodes=[
        node("first",
             "Warden Hale, Lights Office. I've a card for this tower and a route that needs it lit by the next storm. "
             "You've been down there. Tell me what I'm looking at.",
             [
                 dict(text="Section Fourteen. It's trying to come back.", next="s14", consequences=MET),
                 dict(text="Why would I help the Compact?", next="why", consequences=MET),
                 dict(text="Goodbye.", next=None, consequences=MET),
             ]),
        node("s14",
             "Every light I've relit says that on its panel. It's a fault. A fault that sinks ships when the light's "
             "out. Seat the card and it's a light again.",
             [GO_ON]),
        node("why",
             "Because the Compact keeps a book of everyone this lake has drowned, and I'd like to stop writing in it.",
             [GO_ON]),
        node("offer", "The card, then?",
             [
                 dict(text="Give me the card.", next="card",
                      conditions=[cond("HAS_ITEM", id="section_key_compact", negate=True),
                                  flag("sombre.light_decided", True)],
                      consequences=[cons("GIVE_ITEM", id="section_key_compact")]),
                 dict(text="Send your crew up to splice the feed.", next="crew",
                      conditions=[flag("sombre.feed_spliced", True), flag("sombre.hale_crew_up", True),
                                  flag("sombre.node_destroyed", True)],
                      consequences=[cons("SET_WORLD_FLAG", id="sombre.hale_crew_up")]),
                 dict(text="I need oil for a hand light.", next="oil",
                      conditions=[cond("HAS_ITEM", id="lamp_oil", negate=True)],
                      consequences=[cons("GIVE_ITEM", id="lamp_oil")]),
                 ("The listener. Liv Kallio.", "liv"),
                 dict(text="The copper buyer.", next="copper", conditions=[flag("sombre.sigrun_revealed")]),
                 ("There's a laker on the reef. The Ashland Grey.", "grey"),
                 BYE,
             ]),
        node("card", "Seat it, splice the feed, and the Ida sails tonight. The Compact pays for lights that burn.",
             [GO_ON]),
        node("crew", "Done within the hour. They splice. You seat the card. I won't do that part for you.", [GO_ON]),
        node("oil", "Oil I have. Keepers I don't.", [GO_ON]),
        node("liv",
             "She's yours? She took my money too. Found the pattern's source, she said, then stopped writing. If you "
             "find her before I do, tell her the Loss Book has a new page since she left, and there's a boy's name "
             "on it.",
             [GO_ON]),
        node("copper",
             "I know what she is. Local Nine sends one to every light I relight. I've buried two linemen whose "
             "ladders were cut.",
             [GO_ON]),
        # The Ashland Grey's log itself is initial-production content, not the slice's.
        node("grey", "Her log, if you find it. Her dead aren't in my book yet.", [GO_ON]),

        node("after_line", "It showed something before it came right. I saw it. I'm going to write that down as a fault.",
             [BYE]),
        node("after_hand",
             "A hand light. Honest, and dim, and someone every four hours for the rest of their life. I'll enter it "
             "as lit.",
             [BYE]),
        node("after_flooded",
             "You flooded a vault the Authority built to outlast us. The Board will want your name. I'll give it.",
             [BYE]),
        node("after_power", "Warm houses and a dark reef. You chose who drowns. I do it every day. Welcome to the work.",
             [BYE]),
        node("after", "Dark, then. I'll be back with another card in spring.", [BYE]),
    ],
)

DIALOGUES = [SOMBRE_HALE]
