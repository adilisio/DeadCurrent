"""Odette Beaudry, the keeper (sombre_odette): the hatch key, and the keeper's post.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §3, built as written, plus one wiring fix (below). Ids:
`Design/POIs/sombre_ids.md` §3, §4, §6.
- The key, `sombre_vault_key`, comes by `[Persuasion 2]` (`key_press`) or by telling her who Liv is to you with Liv's
  letter in hand (`key_tie`). Each tie line has a NO_TIE partner ("She's why I came."). Neither shows once the player
  holds the key or the vault is open by any route.
- Keeper choices exclude each other and Dell's: each needs `!sombre.keeper_odette`, `!sombre.keeper_dell`, and
  `!sombre.light_line` (a light on the Line needs no keeper).
- Wiring fix (VS-09, recorded in SLICE_DIALOGUE.md's change list for Checkpoint B): the doc offers "Will you keep it?
  By hand." to a forgiven Odette only in `knows`, but forgiveness happens at the meeting, which ends `knows`, so that
  choice could never show. The beat script says a forgiven Odette can still keep the light (§5, and §4's "Choose a
  keeper" row), so the same choice is also offered in `after`, the node a forgiven Odette reaches after the meeting.
  It stays in `knows` as written (harmless there).
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


SPEAKER = "Odette"
SISTER, PARTNER, TOOK_IN = "player.tie.sister", "player.tie.partner", "player.tie.took_in"
NO_TIE = [flag(SISTER, True), flag(PARTNER, True), flag(TOOK_IN, True)]
KEY = "sombre_vault_key"
GIVE_KEY = [cons("GIVE_ITEM", id=KEY)]
# The key is offered only while it would still be useful.
KEY_STILL_NEEDED = [cond("HAS_ITEM", id=KEY, negate=True), flag("sombre.vault_opened", True)]
KEEPER = "sombre.keeper_odette"
KEEPER_OPEN = [flag("sombre.light_line", True), flag("sombre.keeper_dell", True), flag(KEEPER, True)]

BYE = ("Goodbye.", None)
KEEP_FORGIVEN = dict(text="Will you keep it? By hand.", next="keeps",
                     conditions=[*KEEPER_OPEN, flag("sombre.odette_forgiven")],
                     consequences=[cons("SET_WORLD_FLAG", id=KEEPER)])


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


def tie_key(text, tie_conditions):
    return dict(text=text, next="key_tie",
                conditions=[*tie_conditions, cond("HAS_ITEM", id="liv_letter"), *KEY_STILL_NEEDED],
                consequences=GIVE_KEY)


SOMBRE_ODETTE = dict(
    asset="DA_Dialogue_SombreOdette",
    dialogue_id="sombre_odette",
    entry="first",
    entries=[
        dict(id="after_keeper", conditions=[flag("sombre.meeting_done"), flag(KEEPER)]),
        dict(id="after_disgraced", conditions=[flag("sombre.meeting_done"), flag("sombre.exposed_odette"),
                                               flag("sombre.odette_forgiven", True)]),
        dict(id="after", conditions=[flag("sombre.meeting_done")]),
        dict(id="knows", conditions=[cond("QUEST_STAGE", id="sombre.characteristic", stage="knows")]),
        dict(id="first"),
    ],
    nodes=[
        node("first", "It failed. Lights fail. If you want the story, the store has more talkers than me.",
             [
                 ("A listener came through here four months ago.", "listener"),
                 dict(text="You're the keeper. You know more than 'it failed.'", next="key_press",
                      conditions=[cond("SKILL_AT_LEAST", id="Skill.Persuasion", quantity=2), *KEY_STILL_NEEDED],
                      consequences=GIVE_KEY),
                 BYE,
             ]),
        node("listener",
             "Lots of people come through. She wanted to see the vault. I have the key. That's all I say to a "
             "stranger.",
             [
                 tie_key("She's my sister.", [flag(SISTER)]),
                 tie_key("She's my partner.", [flag(PARTNER)]),
                 tie_key("She took me in when nobody would.", [flag(TOOK_IN)]),
                 tie_key("She's why I came.", NO_TIE),
                 BYE,
             ]),
        node("key_tie",
             "...Then you know what she's like when she's decided something. Here. She had the same look.",
             [("What did she do down there?", "ask_light"), BYE]),
        node("key_press",
             "I know it wasn't the storm. That's what I know. Here's the key. The hatch at the foot of the tower. Go "
             "and look, and don't tell me what you find.",
             [BYE]),
        node("ask_light", "Ask the light. I've stopped.", [BYE]),

        node("knows", "You've been down there. I can see it on you.",
             [
                 dict(text="Will you keep it? By hand. Oil and the old clockwork.", next="keeps",
                      conditions=[*KEEPER_OPEN, flag("sombre.exposed_odette", True)],
                      consequences=[cons("SET_WORLD_FLAG", id=KEEPER)]),
                 KEEP_FORGIVEN,
                 dict(text="Liv left you a note.", next="note", conditions=[cond("HAS_ITEM", id="liv_note")]),
                 BYE,
             ]),
        # The doc's "(a pause)" is the ellipsis; lines carry spoken text only.
        node("keeps",
             "Every four hours. Remy used to wind it with me. ...Yes. Tell whoever lights it I'll take the first "
             "watch.",
             [BYE]),
        node("note",
             "I know what it says. I read it the day she left, and pinned it back where she'd put it. I wanted "
             "someone else to find it.",
             [BYE]),

        node("after_keeper", "Every four hours. It's a good weight in the hand.", [BYE]),
        node("after_disgraced", "Go on. Everyone else has said it.", [BYE]),
        # KEEP_FORGIVEN here is the VS-09 wiring fix (see the module docstring).
        node("after", "Whatever you did to my light, it's done now.", [KEEP_FORGIVEN, BYE]),
    ],
)

DIALOGUES = [SOMBRE_ODETTE]
