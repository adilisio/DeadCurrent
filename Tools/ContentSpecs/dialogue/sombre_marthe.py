"""Marthe (sombre_marthe): the store, the call to the net loft, and the meeting she chairs.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §4, built as written, with plan §8.2 #4. Ids: `Design/POIs/sombre_ids.md`
§3, §6. There is no separate `sombre_meeting` asset (an alias): the meeting is this conversation's `loft` entry.
- §8.2 #4, the gathering: at stage `knows`, the `store` node gains "Call the island to the loft." It sets
  `sombre.meeting_called`, and the `loft` entry needs it. The attendees' owners move them to the loft on the stair's
  scene cut. Marthe's one-line reply (`call`) is new VS-09 wording, PROVISIONAL, read aloud at Checkpoint B.
- The meeting: "It's done." needs `sombre.light_decided` (set by every physical decision act); "Leave it as she left
  it." needs it unset. Each evidence choice needs its item, removes it, and sets its `exposed_*` flag. The tie line
  (`r_liv`) has one choice per tie flag and a NO_TIE partner. "That's all." sets `sombre.meeting_done`.
- Other people speak their reactions inside this asset (`speaker` per node).
- Outside the meeting Marthe never mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


def set_flag(name):
    return cons("SET_WORLD_FLAG", id=name)


SISTER, PARTNER, TOOK_IN = "player.tie.sister", "player.tie.partner", "player.tie.took_in"
NO_TIE = [flag(SISTER, True), flag(PARTNER, True), flag(TOOK_IN, True)]
AT_KNOWS = cond("QUEST_STAGE", id="sombre.characteristic", stage="knows")
CALLED = "sombre.meeting_called"
NAMED_TIE = "sombre.player_named_tie"
FORGIVEN = "sombre.odette_forgiven"

BYE = ("Goodbye.", None)
GO_ON = ("Go on.", "evidence")


def node(id, line, choices, speaker="Marthe"):
    return dict(id=id, speaker=speaker, line=line, choices=choices)


def evidence(text, next_node, item, exposed):
    return dict(text=text, next=next_node, conditions=[cond("HAS_ITEM", id=item)],
                consequences=[cons("REMOVE_ITEM", id=item), set_flag(exposed)])


def name_tie(text, tie_conditions):
    return dict(text=text, next="r_tie", conditions=tie_conditions, consequences=[set_flag(NAMED_TIE)])


SOMBRE_MARTHE = dict(
    asset="DA_Dialogue_SombreMarthe",
    dialogue_id="sombre_marthe",
    entry="store",
    entries=[
        dict(id="after_line", conditions=[flag("sombre.meeting_done"), flag("sombre.light_line")]),
        dict(id="after_hand", conditions=[flag("sombre.meeting_done"), flag("sombre.light_hand")]),
        dict(id="after_power", conditions=[flag("sombre.meeting_done"), flag("sombre.power_settlement")]),
        dict(id="after_dark", conditions=[flag("sombre.meeting_done")]),
        # §8.2 #4: the meeting needs the call as well as the stage.
        dict(id="loft", conditions=[AT_KNOWS, flag(CALLED)]),
        dict(id="store"),
    ],
    nodes=[
        node("store", "No light, no ships, no stock. Do the arithmetic.",
             [
                 ("Who keeps the light?", "who"),
                 ("I need oil for a lamp.", "oil"),
                 ("Where does the island meet?", "loft_where"),
                 # §8.2 #4.
                 dict(text="Call the island to the loft.", next="call",
                      conditions=[AT_KNOWS, flag(CALLED, True)], consequences=[set_flag(CALLED)]),
                 BYE,
             ]),
        node("who", "Odette. Her family's had it three generations. Since her boy, she doesn't go up.", [BYE]),
        node("oil",
             "Smokehouse can render you a can, if you can stand the smell. The Compact sells better, when the "
             "Compact comes.",
             [BYE]),
        node("loft_where", "Net loft, over this store. When there's something to decide.", [BYE]),
        # New VS-09 wording (§8.2 #4), PROVISIONAL.
        node("call", "I'll send round for them. Go on up.", [BYE]),

        # The meeting (beat script §5).
        node("loft", "Is the light burning tonight, or isn't it?",
             [
                 ("Not yet.", None),
                 dict(text="It's done.", next="evidence", conditions=[flag("sombre.light_decided")]),
                 dict(text="Leave it as she left it.", next="evidence",
                      conditions=[flag("sombre.light_decided", True)]),
             ]),
        node("evidence", "Then say what you came to say. Everyone's here.",
             [
                 evidence("This was burning on the west head the night the Ida hit.", "r_pruitts",
                          "false_lantern", "sombre.exposed_pruitts"),
                 evidence("Four months ago someone let a stranger into the vault.", "r_odette",
                          "vault_access_log", "sombre.exposed_odette"),
                 evidence("The listener pulled the card. She wrote to Odette.", "r_liv",
                          "liv_note", "sombre.exposed_liv"),
                 evidence("Someone made sure it could never be fixed.", "r_sigrun",
                          "cut_cable_end", "sombre.exposed_sigrun"),
                 dict(text="That's all.", next="close", consequences=[set_flag("sombre.meeting_done")]),
             ]),
        node("r_pruitts", "The lake provides. We only held the lantern.", [GO_ON], speaker="Tem"),
        node("r_odette", "I told you it failed. I lied. Remy was the first boat out.",
             [
                 dict(text="She let her in. Liv's the one who pulled the card.", next="forgive",
                      conditions=[flag("sombre.exposed_liv"), flag(FORGIVEN, True)],
                      consequences=[set_flag(FORGIVEN)]),
                 GO_ON,
             ], speaker="Odette"),
        node("r_liv", "A listener pulled it. For what?",
             [
                 name_tie("She's my sister.", [flag(SISTER)]),
                 name_tie("She's my partner.", [flag(PARTNER)]),
                 name_tie("She took me in.", [flag(TOOK_IN)]),
                 name_tie("She's why I came.", NO_TIE),
                 dict(text="Odette let her in. Liv pulled the card.", next="forgive",
                      conditions=[flag("sombre.exposed_odette"), flag(FORGIVEN, True)],
                      consequences=[set_flag(FORGIVEN)]),
                 GO_ON,
             ]),
        node("r_tie", "Then she owes me a son. And you came all this way for her.", [GO_ON], speaker="Odette"),
        node("forgive", "...Then it's the listener this island owes a grudge, not Odette.", [GO_ON]),
        # Hale is always present by the knows stage.
        node("r_sigrun", "Copper buyer. You're coming with me.", [("Go on.", "r_sigrun_reply")], speaker="Hale"),
        node("r_sigrun_reply",
             "Somebody had to make sure it couldn't be fixed. You'll thank us when the tables come back.",
             [GO_ON], speaker="Sigrun"),
        node("close", "Then that's decided. Whatever it is, we live under it.", [BYE]),

        # After the meeting.
        node("after_line", "Ships by morning. That arithmetic I can do.", [BYE]),
        node("after_hand", "A light we have to feed. I'll put oil on the list, and a name for every four hours.",
             [BYE]),
        node("after_power", "Warm houses and a dark reef. I'll sell more candles than oil.", [BYE]),
        node("after_dark", "No light, no ships, no stock. Same arithmetic as before.", [BYE]),
    ],
)

DIALOGUES = [SOMBRE_MARTHE]
