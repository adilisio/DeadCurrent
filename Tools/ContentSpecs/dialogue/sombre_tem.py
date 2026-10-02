"""Tem Pruitt (sombre_tem), at the salvage shed. One of the two Pruitt conversations (`sombre_pruitts` is an alias).

Text: `Design/Narrative/SLICE_DIALOGUE.md` §5, built as written. Ids: `Design/POIs/sombre_ids.md` §6. Writes no state.
- Plan §8.2 #3: there is no force branch ("Tem keeps a shotgun" is cut). Tem is never hostile.
- Tem never mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""

SPEAKER = "Tem"
BYE = ("Goodbye.", None)


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


SOMBRE_TEM = dict(
    asset="DA_Dialogue_SombreTem",
    dialogue_id="sombre_tem",
    entry="first",
    entries=[
        dict(id="exposed", conditions=[cond("WORLD_FLAG", id="sombre.exposed_pruitts")]),
        dict(id="first"),
    ],
    nodes=[
        node("first", "The lake provides. Salvage is honest work. You want anything off a wreck, you come to me.",
             [
                 ("Someone lit a lantern on the west head the night the Ida hit.", "threat"),
                 ("The Ashland Grey was yours?", "grey"),
                 BYE,
             ]),
        node("threat", "Someone should be careful out on that head in weather. It's a bad place to be seen.", [BYE]),
        node("grey", "The Ashland Grey was the lake's. We just got there first.", [BYE]),
        node("exposed", "You'll be wanting me sorry. I'm poor, not sorry.", [BYE]),
    ],
)

DIALOGUES = [SOMBRE_TEM]
