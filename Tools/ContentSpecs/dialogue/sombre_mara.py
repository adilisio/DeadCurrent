"""Mara at Pointe Sombre (sombre_mara): placed, not following. A different asset from the shore's `mara_intro`.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §1, built as written. Ids: `Design/POIs/sombre_ids.md` §3, §6.
- In the standalone slice Mara asks the tie question on the crossing (`crossing_tie`). Each tie answer sets exactly one
  `player.tie.*` flag; "Does it matter?" sets none. All four set `sombre.mara_tie_asked`, so she asks once.
- Tie variant lines (`promise_*`) each have a NO_TIE partner (`promise_neutral`); the tie flags exclude each other, so
  each tie shows exactly one.
- Node ids are not saved, but builders and tests copy the entry nodes and the nodes that write state.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


def set_flag(name):
    return cons("SET_WORLD_FLAG", id=name)


SISTER, PARTNER, TOOK_IN = "player.tie.sister", "player.tie.partner", "player.tie.took_in"
NO_TIE = [flag(SISTER, True), flag(PARTNER, True), flag(TOOK_IN, True)]
QUEST = "sombre.characteristic"
TIE_ASKED = "sombre.mara_tie_asked"
PROMISE_TOLD = "shore.promise_told"

BYE = ("Goodbye.", None)
HOLD_ON = ("Hold on to something.", None)

SOMBRE_MARA = dict(
    asset="DA_Dialogue_SombreMara",
    dialogue_id="sombre_mara",
    entry="town",
    # First match wins; the last is unconditional.
    entries=[
        dict(id="crossing_tie", conditions=[cond("QUEST_NOT_STARTED", id=QUEST), *NO_TIE,
                                            flag("shore.liv_asked", True), flag(TIE_ASKED, True)]),
        dict(id="crossing", conditions=[cond("QUEST_NOT_STARTED", id=QUEST)]),
        dict(id="promise", conditions=[flag("sombre.player_named_tie"), flag(PROMISE_TOLD, True)]),
        dict(id="after_line", conditions=[flag("sombre.meeting_done"), flag("sombre.light_line")]),
        dict(id="after", conditions=[flag("sombre.meeting_done")]),
        dict(id="panel", conditions=[flag("sombre.panel_read"), flag("sombre.mara_panel", True)]),
        dict(id="ticks", conditions=[flag("sombre.ticks_seen"), flag("sombre.mara_ticks", True)]),
        dict(id="town"),
    ],
    nodes=[
        # The crossing. Shown only under NO_TIE (the entry), so each answer sets one tie flag, or none.
        dict(id="crossing_tie", line="Before we get there. You never said who she is to you.",
             choices=[
                 dict(text="My sister.", next="crossing", consequences=[set_flag(SISTER), set_flag(TIE_ASKED)]),
                 dict(text="My partner.", next="crossing", consequences=[set_flag(PARTNER), set_flag(TIE_ASKED)]),
                 dict(text="She took me in when nobody else would.", next="crossing",
                      consequences=[set_flag(TOOK_IN), set_flag(TIE_ASKED)]),
                 dict(text="Does it matter?", next="crossing_shrug", consequences=[set_flag(TIE_ASKED)]),
             ]),
        dict(id="crossing_shrug", line="Not to the lake.",
             choices=[("...", "crossing")]),
        dict(id="crossing", line="That light's on the wrong head. Tell your captain.",
             choices=[
                 dict(text="About the Tern. 'Two of them came up the beach.' That was sixty years ago.", next="watch",
                      conditions=[flag("wreck.mara_pressed"), flag("watch.revealed", True)],
                      consequences=[set_flag("watch.revealed")]),
                 dict(text="What is it you do, on that shore?", next="watch",
                      conditions=[flag("wreck.mara_pressed", True), flag("watch.revealed", True)],
                      consequences=[set_flag("watch.revealed")]),
                 HOLD_ON,
             ]),
        # PROVISIONAL (the watch is the prologue's office). Decides nothing about the Current or Mara's past.
        dict(id="watch",
             line="That wasn't me. That was the watch. It's the same thing. There's been a watch on that shore since "
                  "the night the Tern came in. Whoever keeps it keeps the log, and says what the log says as if they'd "
                  "seen it. So nobody gets to call it an old story. I'm the fourth.",
             choices=[("And the third?", "third"), HOLD_ON]),
        dict(id="third", line="Walked out to the Tern one storm night. The log says I didn't follow.",
             choices=[HOLD_ON]),

        # On the island.
        dict(id="town", line="Keep your voice down. This island's listening for somebody to blame.",
             choices=[BYE]),
        # The doc's stage direction, "a long look at the chalk rows", is the ellipsis; lines carry spoken text only.
        dict(id="ticks", line="...Write it down.",
             choices=[
                 dict(text="Same spacing as the Tern.", next="ticks_same",
                      conditions=[cond("HAS_ITEM", id="survey_chart")],
                      consequences=[set_flag("sombre.mara_ticks")]),
                 dict(text="Goodbye.", next=None, consequences=[set_flag("sombre.mara_ticks")]),
             ]),
        dict(id="ticks_same", line="Same spacing as everything. That's the part I don't write.",
             choices=[BYE]),
        dict(id="panel", line="Write it down. All of it. Nobody believes the second telling.",
             choices=[
                 dict(text="Liv left a note under the chart.", next="panel_note",
                      conditions=[flag("sombre.liv_note_found")],
                      consequences=[set_flag("sombre.mara_panel")]),
                 dict(text="Goodbye.", next=None, consequences=[set_flag("sombre.mara_panel")]),
             ]),
        dict(id="panel_note", line="I know her hand. Don't read it to me.",
             choices=[BYE]),

        # After the player named the tie at the loft. One tie line per tie flag; NO_TIE has its own.
        dict(id="promise",
             line="She asked me not to tell anyone from the Ida where she went. I kept it as long as I could.",
             choices=[
                 dict(text="She said I'd come?", next="promise_sister",
                      conditions=[flag(SISTER)], consequences=[set_flag(PROMISE_TOLD)]),
                 dict(text="She said I'd come?", next="promise_partner",
                      conditions=[flag(PARTNER)], consequences=[set_flag(PROMISE_TOLD)]),
                 dict(text="She said I'd come?", next="promise_took_in",
                      conditions=[flag(TOOK_IN)], consequences=[set_flag(PROMISE_TOLD)]),
                 dict(text="She said someone would come?", next="promise_neutral",
                      conditions=NO_TIE, consequences=[set_flag(PROMISE_TOLD)]),
                 dict(text="Goodbye.", next=None, consequences=[set_flag(PROMISE_TOLD)]),
             ]),
        dict(id="promise_sister", line="She said you'd come. She said you always did.", choices=[BYE]),
        dict(id="promise_partner", line="She said you'd come, and that she'd deserve it.", choices=[BYE]),
        dict(id="promise_took_in", line="She said you'd come, and that it'd be her fault.", choices=[BYE]),
        dict(id="promise_neutral", line="She said someone would. She didn't say who.", choices=[BYE]),

        # After the meeting.
        dict(id="after_line", line="It flashed the pattern before it came right. Three seconds. I counted.",
             choices=[BYE]),
        dict(id="after", line="Whatever that light does tonight, the watch would have written it down. So will I.",
             choices=[BYE]),
    ],
)

DIALOGUES = [SOMBRE_MARA]
