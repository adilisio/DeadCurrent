"""Create or update dialogue Data Assets in /Game/Dialogue.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Conditions and consequences are the shared rule language (FDCGameplayCondition /
FDCGameplayConsequence), the same one quests and inspectables use.
"""
import unreal

DIALOGUE_PATH = "/Game/Dialogue"
QUEST = "shore.watch"
SCAV = "boat.scavenger"
COIL = "radio_coil"
AMMO = "ammo_9mm"
DRESSING = "field_dressing"
ITEM_PATHS = {
    COIL: "/Game/Items/DA_Item_RadioCoil",
    AMMO: "/Game/Items/DA_Item_Ammo9mm",
    DRESSING: "/Game/Items/DA_Item_FieldDressing",
}
# World flags this conversation reads or writes.
RELAY_INSPECTED = "shore.relay_inspected"    # player inspected the live relay (set by the relay rig)
VOICE_DISCUSSED = "shore.voice_discussed"    # player told Mara the relay speaks
RELAY_RECOVERED = "shore.relay_recovered"    # Mara has the coil (quest outcome, or handed over later)
HEARD_KILL = "shore.mara_heard_kill"         # Mara commented on a kill after the coil route
# Exploration Loop (the Wrecked Survey Launch, see build_boathouse.py). Not part of Shore Watch.
WRECK_LOG_READ = "wreck.log_read"            # player read the survey log at the wreck
WRECK_TOLD = "wreck.mara_told"               # player told Mara about the wreck (asked once)
WRECK_PRESSED = "wreck.mara_pressed"         # Persuasion: she admitted she saw two people leave the beach

COND = unreal.DCConditionType
CONS = unreal.DCConsequenceType


def cond(type_name, id=None, stage=None, quantity=1, negate=False):
    c = unreal.DCGameplayCondition()
    c.set_editor_property("type", getattr(COND, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("quantity", quantity)
    c.set_editor_property("negate", negate)
    return c


def cons(type_name, id=None, stage=None, quantity=1):
    c = unreal.DCGameplayConsequence()
    c.set_editor_property("type", getattr(CONS, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("quantity", quantity)
    if id in ITEM_PATHS and type_name in ("GIVE_ITEM", "REMOVE_ITEM"):
        c.set_editor_property("item", unreal.load_asset(ITEM_PATHS[id]))
    return c


def stage_is(stage):
    return cond("QUEST_STAGE", id=QUEST, stage=stage)


NOT_STARTED = cond("QUEST_NOT_STARTED", id=QUEST)
START = cons("START_QUEST", id=QUEST)
BYE = ("Goodbye.", None)
ASK_WHO = ("Who are you?", "who")
ASK_PLACE = ("What is this place?", "place")
ASK_OFFER = dict(text="You keep looking toward his camp.", next="offer", conditions=[NOT_STARTED])
TELL_VOICE = dict(text="I looked at his relay. It's saying words.", next="voice",
                  conditions=[cond("WORLD_FLAG", id=RELAY_INSPECTED), cond("WORLD_FLAG", id=VOICE_DISCUSSED, negate=True)])
# Offered in Mara's everyday nodes (never in Shore Watch turn-ins) once the survey log has been read.
TELL_WRECK = dict(text="There's a wrecked survey launch west of the boathouse. I read her log.", next="wreck",
                  conditions=[cond("WORLD_FLAG", id=WRECK_LOG_READ), cond("WORLD_FLAG", id=WRECK_TOLD, negate=True)])

MARA_INTRO = dict(
    asset="DA_Dialogue_MaraIntro",
    dialogue_id="mara_intro",
    entry="greeting",
    # First match wins.
    entries=[
        dict(id="done_killed", conditions=[stage_is("done_killed")]),
        dict(id="done_coil", conditions=[stage_is("done_coil")]),
        dict(id="turnin_kill", conditions=[stage_is("return_killed")]),
        dict(id="turnin_coil", conditions=[stage_is("return_coil")]),
        dict(id="inprogress", conditions=[stage_is("accepted")]),
        dict(id="already_dead", conditions=[NOT_STARTED, cond("ACTOR_DEAD", id=SCAV)]),
        dict(id="greeting"),
    ],
    nodes=[
        dict(id="greeting", line="Keep your voice down. That scavenger still works this stretch of shore.",
             choices=[ASK_WHO, ASK_PLACE, ASK_OFFER, TELL_WRECK, BYE]),
        dict(id="who", line="Name's Mara. I watch the shore for people who still listen before they shoot.",
             choices=[ASK_PLACE, ASK_OFFER, TELL_WRECK, BYE]),
        dict(id="place", line="Great Lakes Maritime ground, once. The boathouse still stands. The shore doesn't stay empty for long.",
             choices=[ASK_WHO, ASK_OFFER, TELL_WRECK, BYE]),

        # The survey launch (Exploration Loop). One exchange, then it's done. She doesn't explain it.
        dict(id="wreck",
             line="The Tern. She's been on those stones since the lights went out. Everybody on this shore reads that "
                  "last page once, and everybody decides it was a storm.",
             choices=[dict(text="Was it a storm?", next="wreck_storm", consequences=[cons("SET_WORLD_FLAG", id=WRECK_TOLD)])]),
        dict(id="wreck_storm", line="It's always a storm. Stay out of the water round her stern. It bites.",
             choices=[
                 dict(text="You're leaving something out.", next="wreck_pressed",
                      conditions=[cond("SKILL_AT_LEAST", id="Skill.Persuasion", quantity=2)],
                      consequences=[cons("SET_WORLD_FLAG", id=WRECK_PRESSED)]),
                 BYE,
             ]),
        # PROVISIONAL. A limited admission. It does not explain the Current or decide who Mara is.
        dict(id="wreck_pressed",
             line="Two of them came up off that beach the night she grounded. They would not look at the water, "
                  "and they would not say what they had heard. I didn't follow. Don't ask me to.",
             choices=[BYE]),

        # Offer. The route is not locked by the reply; what the player does in the world decides it.
        dict(id="offer",
             line="Listen. Under the wind. He's wired an old Maritime Authority relay at his camp. "
                  "Dead sixty years, and three nights now it's been talking. Things come to a signal like that. I want it quiet.",
             choices=[
                 dict(text="I'll put him down.", next="accept_kill", consequences=[START]),
                 dict(text="I'll pull the coil out of his rig. No shooting.", next="accept_sneak", consequences=[START]),
                 dict(text="This coil? I already pulled it.", next="turnin_coil",
                      conditions=[cond("HAS_ITEM", id=COIL)], consequences=[START]),
                 ("Not my problem.", None),
             ]),
        dict(id="accept_kill", line="Then do it clean. He walks a square past the beached hull. Come back when he's down.",
             choices=[BYE]),
        dict(id="accept_sneak",
             line="The coil sits in the relay housing by his pack, right on his loop. Stay low, time his walk, and don't let him see you take it.",
             choices=[BYE]),

        # In progress.
        dict(id="inprogress", line="Still hear it? Every night it comes in a little clearer.",
             choices=[("Remind me what you need.", "recap"), TELL_VOICE, TELL_WRECK, BYE]),
        dict(id="recap", line="His relay goes quiet. Kill him, or pull the coil from the rig at his camp. Your choice. Just make it quiet.",
             choices=[BYE]),
        dict(id="voice", line="...Yeah. I've heard them too. Don't say them out loud, and don't repeat them on the boats.",
             choices=[dict(text="I won't.", next=None, consequences=[cons("SET_WORLD_FLAG", id=VOICE_DISCUSSED)])]),

        # Turn-ins. The reply moves the quest to its outcome stage and pays out.
        dict(id="turnin_kill", line="It stopped. I heard it stop, right about when the shooting did.",
             choices=[
                 dict(text="He's dead. His relay has no one to tend it.", next="reward_kill", consequences=[
                     cons("GIVE_ITEM", id=AMMO, quantity=24),
                     cons("SET_QUEST_STAGE", id=QUEST, stage="done_killed"),
                 ]),
                 TELL_VOICE,
             ]),
        dict(id="turnin_coil", line="That's the coil. Still warm. Give it here.",
             choices=[
                 dict(text="Here. It's yours.", next="reward_coil", consequences=[
                     cons("REMOVE_ITEM", id=COIL, quantity=1),
                     cons("GIVE_ITEM", id=DRESSING, quantity=2),
                     cons("SET_QUEST_STAGE", id=QUEST, stage="done_coil"),
                 ]),
                 TELL_VOICE,
             ]),
        dict(id="reward_kill", line="Twenty-four rounds. He won't need them. You will.", choices=[BYE]),
        dict(id="reward_coil",
             # Phase 5 (Gameplay Critic G-01): the old closing clause promised she would sit up with the coil, but on
             # this route she packs and waits at the landing. PROVISIONAL; it names no destination.
             line="Two dressings. All I can spare. He's still out there, so don't get careless. I've got packing to do.",
             choices=[BYE]),

        # After the quest: what Mara says depends on how it ended.
        dict(id="done_killed", line="Path's quiet. His relay went cold with him. Don't get comfortable.",
             choices=[
                 dict(text="I pulled the coil from his relay, too.", next="coil_after_kill",
                      conditions=[cond("HAS_ITEM", id=COIL)],
                      consequences=[cons("REMOVE_ITEM", id=COIL, quantity=1), cons("SET_WORLD_FLAG", id=RELAY_RECOVERED)]),
                 ASK_WHO, ASK_PLACE, TELL_WRECK, BYE,
             ]),
        dict(id="coil_after_kill", line="Better in my hands than rusting on his shore. I'll see what it has left to say.",
             choices=[BYE]),
        dict(id="done_coil",
             line="The coil talked all night. I'm writing down what I can. He's still walking the shore. Keep clear of him.",
             choices=[
                 dict(text="He won't be walking anywhere now.", next="went_back",
                      conditions=[cond("ACTOR_DEAD", id=SCAV), cond("WORLD_FLAG", id=HEARD_KILL, negate=True)],
                      consequences=[cons("SET_WORLD_FLAG", id=HEARD_KILL)]),
                 ASK_WHO, ASK_PLACE, TELL_WRECK, BYE,
             ]),
        dict(id="went_back", line="You went back for him anyway. ...I suppose that's one way to keep a shore quiet.",
             choices=[BYE]),

        # The scavenger died before Mara asked.
        dict(id="already_dead", line="The walker on the path went quiet. That you?",
             choices=[
                 dict(text="It was me.", next="turnin_kill", consequences=[START]),
                 ("Wasn't me.", "already_dead_denied"),
             ]),
        dict(id="already_dead_denied",
             line="Hm. Somebody did. He'd rigged an old relay at his camp. If it's still humming, that's somebody else's business now.",
             choices=[BYE]),
    ],
)

DIALOGUES = [MARA_INTRO]


def log(msg):
    unreal.log_warning("[DCDIALOGUE] " + msg)


def get_or_create(asset_name):
    path = f"{DIALOGUE_PATH}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_directory_exist(DIALOGUE_PATH):
        unreal.EditorAssetLibrary.make_directory(DIALOGUE_PATH)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.DCDialogueAsset)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, DIALOGUE_PATH, unreal.DCDialogueAsset, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    log(f"created {path}")
    return asset


def make_choice(spec):
    choice = unreal.DCDialogueChoice()
    if isinstance(spec, dict):
        text = spec["text"]
        next_id = spec.get("next")
        conditions = spec.get("conditions", [])
        consequences = spec.get("consequences", [])
    else:
        text, next_id = spec[0], spec[1]
        conditions, consequences = [], []
    choice.set_editor_property("text", unreal.Text(text))
    choice.set_editor_property("next_node_id", unreal.Name(next_id) if next_id else unreal.Name())
    choice.set_editor_property("conditions", conditions)
    choice.set_editor_property("consequences", consequences)
    return choice


def make_node(spec):
    node = unreal.DCDialogueNode()
    node.set_editor_property("node_id", spec["id"])
    node.set_editor_property("speaker", unreal.Text(spec.get("speaker", "Mara")))
    node.set_editor_property("line", unreal.Text(spec["line"]))
    node.set_editor_property("choices", [make_choice(c) for c in spec["choices"]])
    return node


def make_entry(spec):
    entry = unreal.DCDialogueEntry()
    entry.set_editor_property("node_id", spec["id"])
    entry.set_editor_property("conditions", spec.get("conditions", []))
    return entry


def write_dialogue(spec):
    asset = get_or_create(spec["asset"])
    asset.set_editor_property("dialogue_id", spec["dialogue_id"])
    asset.set_editor_property("entry_node_id", spec["entry"])
    asset.set_editor_property("entries", [make_entry(e) for e in spec.get("entries", [])])
    asset.set_editor_property("nodes", [make_node(n) for n in spec["nodes"]])
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {spec['asset']}")
    log(f"saved {DIALOGUE_PATH}/{spec['asset']} ({len(spec['nodes'])} nodes)")


def main():
    for dialogue in DIALOGUES:
        write_dialogue(dialogue)


main()
