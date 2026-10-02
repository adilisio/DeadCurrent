"""Sigrun Dahl (sombre_sigrun), at the net racks, "buying copper". She carries the Copper Buyer story.

Text: `Design/Narrative/SLICE_DIALOGUE.md` §7, built as written. Ids: `Design/POIs/sombre_ids.md` §3, §5, §6.
- Plan §8.2 #9: Copper Buyer is this conversation and its flags only. There is no quest asset, and no
  `sombre.copper_buyer` id.
- Confronted with `cut_cable_end`, she is revealed (`sombre.sigrun_revealed`). "Get me into the vault." sets
  `sombre.vault_opened` (one of the four ways in; she is not a portal: the existing hatch and door open by the flag).
  "Do it with me." sets `sombre.sigrun_helping` (she brings the wrench for the sea cock).
- Sigrun never mentions the tie.
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


SPEAKER = "Sigrun"
VAULT_OPENED = "sombre.vault_opened"
BYE = ("Goodbye.", None)
GET_ME_IN = dict(text="Get me into the vault.", next="unjam", conditions=[flag(VAULT_OPENED, True)],
                 consequences=[cons("SET_WORLD_FLAG", id=VAULT_OPENED)])


def node(id, line, choices):
    return dict(id=id, speaker=SPEAKER, line=line, choices=choices)


SOMBRE_SIGRUN = dict(
    asset="DA_Dialogue_SombreSigrun",
    dialogue_id="sombre_sigrun",
    entry="first",
    entries=[
        # The fallback line if she is still placed after being taken (her presence rule hides her).
        dict(id="taken", conditions=[flag("sombre.exposed_sigrun")]),
        dict(id="helping", conditions=[flag("sombre.sigrun_helping")]),
        dict(id="revealed", conditions=[flag("sombre.sigrun_revealed")]),
        dict(id="first"),
    ],
    nodes=[
        node("first", "Buying copper. You got any? I pay in cells. Local cells, full charge.",
             [
                 ("What do you want copper for?", "copper"),
                 dict(text="Those are lineman's shears on your belt.", next="shears",
                      conditions=[cond("SKILL_AT_LEAST", id="Skill.Engineering", quantity=2)]),
                 dict(text="I found the lamp's feed cut. Clean. Shears, not a saw.", next="reveal",
                      conditions=[cond("HAS_ITEM", id="cut_cable_end")],
                      consequences=[cons("SET_WORLD_FLAG", id="sombre.sigrun_revealed")]),
                 BYE,
             ]),
        node("copper",
             "Everyone wants copper. Copper's the only thing on this lake that remembers where it's supposed to go.",
             [BYE]),
        node("shears", "Good eye. Lot of linemen in the world.", [BYE]),
        node("reveal",
             "...Fine. Local Nine. I cut it. Your listener pulled their card, and the Compact's coming with a new one. "
             "Every light they relight, that thing under the rock tries to phone home. I made sure it can't.",
             [GET_ME_IN, ("What would you do with the vault?", "sea"), BYE]),
        node("unjam", "Lower door's jammed because I jammed it. Come on.",
             [("What would you do with it?", "sea"), BYE]),
        node("sea", "Open the sea cock. Flood it. Nothing rejoins from under the lake.",
             [
                 dict(text="Do it with me.", next="with_me",
                      conditions=[flag("sombre.light_line", True), flag("sombre.node_destroyed", True)],
                      consequences=[cons("SET_WORLD_FLAG", id="sombre.sigrun_helping")]),
                 ("Not yet.", None),
             ]),
        node("with_me", "Good. I've got the wrench. Tell me when.", [BYE]),
        node("revealed", "Still here. Still buying copper, officially.",
             [
                 GET_ME_IN,
                 ("About the vault.", "sea"),
                 dict(text="The Warden's in the harbor.", next="warden", conditions=[flag("sombre.hale_arrived")]),
                 BYE,
             ]),
        node("warden", "Hale. I know. I'd rather not meet him on a quay.", [BYE]),
        node("helping", "Wrench is ready. Your call.", [BYE]),
        node("taken", "Tell Nine I kept my mouth shut.", [BYE]),
    ],
)

DIALOGUES = [SOMBRE_SIGRUN]
