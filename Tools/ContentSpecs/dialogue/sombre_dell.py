"""Dell Pruitt (sombre_dell): the confession approach to False Light, and the keeper's post.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §6, built as written, with plan §8.2 #1. Ids: `Design/POIs/sombre_ids.md`
§3, §6. The same asset wherever he stands: at the shed, or at the headland post during the first storm (§8.2 #2:
catching him at the post is this conversation and this confession, not a separate approach).
- The confession: `[Persuasion 2]` or `[Survival 2]` (the wet boots) sets `sombre.dell_confessed`.
- §8.2 #1: the doc's Survival line said "It hasn't rained since the storm.", which contradicts a slice that opens in
  rain. Only that clause is reworded (default proposed in VS-09; Anthony may replace it at Checkpoint B). The new
  clause names wading, not weather, so it reads the same in the storm and in the calm after `knows`.
- Keeper choices exclude Odette's: each needs `!sombre.keeper_odette`, `!sombre.keeper_dell`, `!sombre.light_line`.
- Dell never mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


SPEAKER = "Dell"
CONFESSED = "sombre.dell_confessed"
KEEPER = "sombre.keeper_dell"
KEEPER_OPEN = [flag("sombre.light_line", True), flag("sombre.keeper_odette", True), flag(KEEPER, True)]
BYE = ("Goodbye.", None)


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


SOMBRE_DELL = dict(
    asset="DA_Dialogue_SombreDell",
    dialogue_id="sombre_dell",
    entry="first",
    entries=[
        dict(id="keeper", conditions=[flag(KEEPER)]),
        dict(id="confessed", conditions=[flag(CONFESSED)]),
        dict(id="first"),
    ],
    nodes=[
        node("first", "I found his boat. That's all.",
             [
                 dict(text="That isn't all.", next="confess",
                      conditions=[cond("SKILL_AT_LEAST", id="Skill.Persuasion", quantity=2)],
                      consequences=[cons("SET_WORLD_FLAG", id=CONFESSED)]),
                 # §8.2 #1: the rain clause reworded; the check and its outcome are unchanged.
                 dict(text="Your boots are wet to the knee. Nobody wades the reef for fish.", next="confess",
                      conditions=[cond("SKILL_AT_LEAST", id="Skill.Survival", quantity=2)],
                      consequences=[cons("SET_WORLD_FLAG", id=CONFESSED)]),
                 BYE,
             ]),
        node("confess",
             "Remy was my friend. After the light went, Ma said the lake was giving us something back. So I hold the "
             "lantern on the west head when the weather comes in. We didn't make the dark. We just ate off it.",
             [
                 dict(text="Stop. Keep the real light instead.", next="keeper_offer", conditions=KEEPER_OPEN),
                 BYE,
             ]),
        node("keeper_offer", "Me? ...If the lantern stops. If you tell Varga it stops.",
             [
                 dict(text="It stops.", next="keeper_yes", conditions=KEEPER_OPEN,
                      consequences=[cons("SET_WORLD_FLAG", id=KEEPER)]),
                 ("Think about it.", None),
             ]),
        node("keeper_yes", "Every four hours. I know the stair. I used to race him up it.", [BYE]),
        node("confessed", "You know, then. Say it at the loft if you're going to.",
             [
                 dict(text="Keep the real light instead.", next="keeper_offer", conditions=KEEPER_OPEN),
                 BYE,
             ]),
        node("keeper", "I'm keeping it. Four hours on, four off. I sleep in pieces now.", [BYE]),
    ],
)

DIALOGUES = [SOMBRE_DELL]
