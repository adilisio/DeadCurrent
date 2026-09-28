"""Create or update quest Data Assets in /Game/Quests.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Quest ids and stage ids are stored in saves. Do not rename them once a quest has shipped.
"""
import unreal

QUEST_PATH = "/Game/Quests"

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
    return c


# Shore Watch: Mara wants the scavenger's rigged relay silenced.
#   accepted --(boat.scavenger dead)--> return_killed --(Mara)--> done_killed   flag shore.path_cleared
#            --(carrying radio_coil)--> return_coil   --(Mara)--> done_coil     flag shore.relay_recovered
# Mara's dialogue moves the return stages to the outcome stages and hands out the rewards.
SHORE_WATCH = dict(
    asset="DA_Quest_ShoreWatch",
    quest_id="shore.watch",
    name="Shore Watch",
    start="accepted",
    stages=[
        dict(
            id="accepted",
            objective="Silence the relay at the scavenger's camp: kill him, or pull the coil from his rig without a fight.",
            transitions=[
                dict(next="return_killed", conditions=[cond("ACTOR_DEAD", id="boat.scavenger")]),
                dict(next="return_coil", conditions=[cond("HAS_ITEM", id="radio_coil")]),
            ],
        ),
        dict(
            id="return_killed",
            objective="The scavenger is dead. Tell Mara the relay has no one to tend it.",
        ),
        dict(
            id="return_coil",
            objective="You have the relay coil. Bring it to Mara.",
        ),
        dict(
            id="done_killed",
            completes=True,
            objective="You killed the scavenger. The shore path is clear and his relay has gone cold.",
            on_enter=[cons("SET_WORLD_FLAG", id="shore.path_cleared")],
        ),
        dict(
            id="done_coil",
            completes=True,
            objective="You took the relay coil without a fight. Mara is listening to it. The scavenger still walks the shore.",
            on_enter=[cons("SET_WORLD_FLAG", id="shore.relay_recovered")],
        ),
    ],
)

QUESTS = [SHORE_WATCH]


def log(msg):
    unreal.log_warning("[DCQUEST] " + msg)


def get_or_create(asset_name):
    path = f"{QUEST_PATH}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_directory_exist(QUEST_PATH):
        unreal.EditorAssetLibrary.make_directory(QUEST_PATH)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.DCQuestDefinition)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, QUEST_PATH, unreal.DCQuestDefinition, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    log(f"created {path}")
    return asset


def make_transition(spec):
    transition = unreal.DCQuestTransition()
    transition.set_editor_property("next_stage", spec["next"])
    transition.set_editor_property("conditions", spec.get("conditions", []))
    return transition


def make_stage(spec):
    stage = unreal.DCQuestStage()
    stage.set_editor_property("stage_id", spec["id"])
    stage.set_editor_property("objective_text", unreal.Text(spec.get("objective", "")))
    stage.set_editor_property("completes_quest", spec.get("completes", False))
    stage.set_editor_property("on_enter", spec.get("on_enter", []))
    stage.set_editor_property("transitions", [make_transition(t) for t in spec.get("transitions", [])])
    return stage


def write_quest(spec):
    asset = get_or_create(spec["asset"])
    asset.set_editor_property("quest_id", spec["quest_id"])
    asset.set_editor_property("display_name", unreal.Text(spec["name"]))
    asset.set_editor_property("start_stage", spec.get("start", ""))
    asset.set_editor_property("stages", [make_stage(s) for s in spec["stages"]])
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {spec['asset']}")
    log(f"saved {QUEST_PATH}/{spec['asset']} ({len(spec['stages'])} stages)")


def main():
    for quest in QUESTS:
        write_quest(quest)


main()
