"""Create or update dialogue Data Assets in /Game/Dialogue.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

DIALOGUE_PATH = "/Game/Dialogue"
QUEST = "shore.watch"
COIL = "radio_coil"
FLAG = "shore.cleared"
AMMO = "/Game/Items/DA_Item_Ammo9mm"
COIL_ITEM = "/Game/Items/DA_Item_RadioCoil"

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


def cons(type_name, id=None, stage=None, quantity=1, item_path=None):
    c = unreal.DCGameplayConsequence()
    c.set_editor_property("type", getattr(CONS, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("quantity", quantity)
    if item_path:
        c.set_editor_property("item", unreal.load_asset(item_path))
    return c


REWARD_KILL = [
    cons("GIVE_ITEM", id="ammo_9mm", quantity=24, item_path=AMMO),
    cons("COMPLETE_QUEST", id=QUEST),
    cons("SET_WORLD_FLAG", id=FLAG),
]
REWARD_COIL = [
    cons("REMOVE_ITEM", id=COIL, quantity=1, item_path=COIL_ITEM),
    cons("GIVE_ITEM", id="ammo_9mm", quantity=24, item_path=AMMO),
    cons("COMPLETE_QUEST", id=QUEST),
    cons("SET_WORLD_FLAG", id=FLAG),
]
ACCEPT = [cons("START_QUEST", id=QUEST, stage="accepted")]
SCAV_TALK = [
    cond("QUEST_NOT_STARTED", id=QUEST),
    cond("HOSTILE_DEAD", negate=True),
]

MARA_INTRO = dict(
    asset="DA_Dialogue_MaraIntro",
    dialogue_id="mara_intro",
    entry="greeting",
    entries=[
        dict(id="done", conditions=[cond("QUEST_COMPLETE", id=QUEST)]),
        dict(id="turnin_kill", conditions=[cond("QUEST_ACTIVE", id=QUEST), cond("HOSTILE_DEAD")]),
        dict(id="turnin_coil", conditions=[cond("QUEST_ACTIVE", id=QUEST), cond("HAS_ITEM", id=COIL)]),
        dict(id="inprogress", conditions=[cond("QUEST_ACTIVE", id=QUEST)]),
        dict(id="already_dead", conditions=[cond("QUEST_NOT_STARTED", id=QUEST), cond("HOSTILE_DEAD")]),
        dict(id="greeting"),
    ],
    nodes=[
        dict(
            id="greeting",
            speaker="Mara",
            line="Keep your voice down. That scavenger still works this stretch of shore.",
            choices=[
                ("Who are you?", "who"),
                ("What is this place?", "place"),
                dict(text="About that scavenger...", next="offer", conditions=SCAV_TALK),
                ("Goodbye", None),
            ],
        ),
        dict(
            id="who",
            speaker="Mara",
            line="Name's Mara. I watch the shore for people who still listen before they shoot.",
            choices=[
                ("What is this place?", "place"),
                dict(text="About that scavenger...", next="offer", conditions=SCAV_TALK),
                ("Goodbye", None),
            ],
        ),
        dict(
            id="place",
            speaker="Mara",
            line="Old Great Lakes Maritime ground. The boathouse still stands. The shore doesn't stay empty for long.",
            choices=[
                ("Who are you?", "who"),
                dict(text="About that scavenger...", next="offer", conditions=SCAV_TALK),
                ("Goodbye", None),
            ],
        ),
        dict(
            id="offer",
            speaker="Mara",
            line="He walks the same loop every day. Put him down and the path stays quieter. If you'd rather not shoot, he keeps a radio coil at that camp. Bring me that and he loses his friends.",
            choices=[
                dict(text="I'll deal with him.", next="accept", consequences=ACCEPT),
                dict(text="I'll look for the coil.", next="accept_sneak", consequences=ACCEPT),
                ("Not now.", None),
            ],
        ),
        dict(
            id="accept",
            speaker="Mara",
            line="Then do it quiet if you can. Come back when the path is clear.",
            choices=[("Goodbye", None)],
        ),
        dict(
            id="accept_sneak",
            speaker="Mara",
            line="Camp's on his loop. Don't let him see you take it. Come back with the coil.",
            choices=[("Goodbye", None)],
        ),
        dict(
            id="inprogress",
            speaker="Mara",
            line="He's still working that stretch. I can hear him.",
            choices=[
                ("I'll get it done.", None),
                ("Remind me about the coil.", "coil_hint"),
                ("Goodbye", None),
            ],
        ),
        dict(
            id="coil_hint",
            speaker="Mara",
            line="Small copper piece in his kit at the camp. He won't miss it until he tries to call someone.",
            choices=[("Goodbye", None)],
        ),
        dict(
            id="turnin_kill",
            speaker="Mara",
            line="I heard it stop. The path is quieter. Don't get comfortable — something else will smell the gap.",
            choices=[
                dict(text="I took care of it.", next="reward", consequences=REWARD_KILL),
            ],
        ),
        dict(
            id="turnin_coil",
            speaker="Mara",
            line="That's the coil off his radio. He'll have a harder time calling friends. The path won't stay empty, but it's quieter tonight.",
            choices=[
                dict(text="Here.", next="reward", consequences=REWARD_COIL),
            ],
        ),
        dict(
            id="already_dead",
            speaker="Mara",
            line="The path is already quiet. That was you?",
            choices=[
                dict(text="It was.", next="reward", consequences=REWARD_KILL),
                ("Wasn't me.", None),
            ],
        ),
        dict(
            id="reward",
            speaker="Mara",
            line="Twenty-four rounds. Don't waste them.",
            choices=[("Goodbye", None)],
        ),
        dict(
            id="done",
            speaker="Mara",
            line="The path is quieter now. Don't get comfortable.",
            choices=[
                ("Who are you?", "who"),
                ("What is this place?", "place"),
                ("Goodbye", None),
            ],
        ),
    ],
)


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
    node.set_editor_property("speaker", unreal.Text(spec["speaker"]))
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
    write_dialogue(MARA_INTRO)


main()
