"""Jonas Leclair (sombre_jonas), on his porch. The children are not modeled (plan §8.2 #10); he carries them.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §9, built as written. Ids: `Design/POIs/sombre_ids.md` §6. Writes no state.
In the hand-lit ending the tower is lit but does not flash the pattern, so `after` is right there too. Jonas never
mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name):
    return cond("WORLD_FLAG", id=name)


SPEAKER = "Jonas"
BYE = ("Goodbye.", None)


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


SOMBRE_JONAS = dict(
    asset="DA_Dialogue_SombreJonas",
    dialogue_id="sombre_jonas",
    entry="first",
    entries=[
        dict(id="after_line", conditions=[flag("sombre.meeting_done"), flag("sombre.light_line")]),
        dict(id="after", conditions=[flag("sombre.meeting_done")]),
        dict(id="first"),
    ],
    nodes=[
        node("first",
             "They don't sleep when the weather comes in. They sit up and draw those lines. The little one says the "
             "light used to draw them too.",
             [("The light drew them?", "drew"), ("Do you want the light back?", "want"), BYE]),
        node("drew", "Before it went dark it flashed wrong in storms, and she'd count along. Now she counts in the dark.",
             [BYE]),
        node("want", "I want my kids to sleep. Whatever that takes.", [BYE]),
        node("after_line", "It flashed the lines again tonight. She saw it. She's awake.", [BYE]),
        node("after", "Dark and quiet tonight. She slept a little.", [BYE]),
    ],
)

DIALOGUES = [SOMBRE_JONAS]
