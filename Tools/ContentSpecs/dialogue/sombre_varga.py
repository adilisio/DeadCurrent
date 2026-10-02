"""Captain Ines Varga (sombre_varga), on the Ida at the quay. Her first conversation starts both slice quests.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §2, built as written. Ids: `Design/POIs/sombre_ids.md` §3, §5, §6.
- Every choice in `first` carries `StartQuest sombre.characteristic` and `StartQuest sombre.false_light`. These are the
  slice's only StartQuest calls. Flags set before she is met chain both quests forward when they start.
- Naming the Pruitts sets `sombre.varga_told` (False Light's `named` -> `done`). The lantern is shown, not handed over,
  so it can still be given at the net loft.
- "Where next?" sets `sombre.slice_end`: the ledger's default end-card trigger (plan §8.2 #11). Nothing reads the flag
  until VS-19's story card, which may move the trigger to a "Watch the light" interaction instead.
- Varga never mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


SPEAKER = "Varga"
CHARACTERISTIC = "sombre.characteristic"
FALSE_LIGHT = "sombre.false_light"
VARGA_TOLD = "sombre.varga_told"
START_BOTH = [cons("START_QUEST", id=CHARACTERISTIC), cons("START_QUEST", id=FALSE_LIGHT)]

BYE = ("Goodbye.", None)
ASK_LIV = ("Liv came through here.", "liv")
# The name, either way: Dell's confession, or the lantern in hand (only without the confession, so one line shows).
NAME_CONFESSED = dict(text="It was the Pruitts. Tem and Dell hold the lantern.", next="told",
                      conditions=[flag("sombre.dell_confessed"), flag(VARGA_TOLD, True)],
                      consequences=[cons("SET_WORLD_FLAG", id=VARGA_TOLD)])
NAME_LANTERN = dict(text="It was the Pruitts. Here's their lantern.", next="told",
                    conditions=[cond("HAS_ITEM", id="false_lantern"), flag("sombre.dell_confessed", True),
                                flag(VARGA_TOLD, True)],
                    consequences=[cons("SET_WORLD_FLAG", id=VARGA_TOLD)])
WHERE_NEXT = dict(text="Where next?", next="next", consequences=[cons("SET_WORLD_FLAG", id="sombre.slice_end")])


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


SOMBRE_VARGA = dict(
    asset="DA_Dialogue_SombreVarga",
    dialogue_id="sombre_varga",
    entry="first",
    entries=[
        dict(id="after_line", conditions=[flag("sombre.meeting_done"), flag("sombre.light_line")]),
        dict(id="after_hand", conditions=[flag("sombre.meeting_done"), flag("sombre.light_hand")]),
        dict(id="after_dark", conditions=[flag("sombre.meeting_done")]),
        dict(id="waiting", conditions=[cond("QUEST_ACTIVE", id=CHARACTERISTIC)]),
        dict(id="first"),
    ],
    nodes=[
        node("first",
             "I read that light on the west head as the point and put us on the reef for it. Somebody lit it on "
             "purpose. The Ida isn't leaving this harbor while the point's dark.",
             [
                 dict(text="What do you need?", next="need", consequences=START_BOTH),
                 dict(text="Liv came through here.", next="liv", consequences=START_BOTH),
                 dict(text="I'll find out why.", next=None, consequences=START_BOTH),
             ]),
        node("need",
             "Find out why the light failed, or find me a keeper who'll light it. And whoever lit that lantern on "
             "the west head, I want a name.",
             [ASK_LIV, ("I'll find out.", None)]),
        node("liv",
             "Your listener. Four months back, on the Packet. Ask the keeper what she did up there. Nobody else on "
             "this rock will say her name.",
             [BYE]),
        node("waiting", "Still dark. Still here. Pumps going.",
             [NAME_CONFESSED, NAME_LANTERN, ASK_LIV, BYE]),
        node("told", "Poor people with a lantern. I've been poorer. I'd still have drowned.",
             [BYE]),
        node("after_line",
             "Brightest light this side of the Narrows. For a moment there it was saying something else. You saw it. "
             "We sail tonight. You and your watcher have berths.",
             [NAME_CONFESSED, NAME_LANTERN, WHERE_NEXT, BYE]),
        node("after_hand", "Small light. Honest one. We go at first light, and I'll trust it.",
             [NAME_CONFESSED, NAME_LANTERN, WHERE_NEXT, BYE]),
        node("after_dark", "Dark as she left it. We go by daylight, and I'll steer wide of that head.",
             [NAME_CONFESSED, NAME_LANTERN, WHERE_NEXT, BYE]),
        node("next", "She went through Aubin Locks on the Packet. So will we, if the Local lets us.",
             [BYE]),
    ],
)

DIALOGUES = [SOMBRE_VARGA]
