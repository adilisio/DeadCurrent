"""Create or update quest Data Assets in /Game/Quests.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

QUEST_PATH = "/Game/Quests"

SHORE_WATCH = dict(
    asset="DA_Quest_ShoreWatch",
    quest_id="shore.watch",
    name="Shore Watch",
    completed="done",
    stages=[
        dict(
            id="accepted",
            objective="Deal with the scavenger on the shore, or bring Mara the radio coil from his camp.",
            advance_on_hostile_death=True,
            next_on_hostile_death="return",
        ),
        dict(
            id="return",
            objective="Return to Mara.",
        ),
        dict(id="done"),
    ],
)


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


def make_stage(spec):
    stage = unreal.DCQuestStage()
    stage.set_editor_property("stage_id", spec["id"])
    stage.set_editor_property("objective_text", unreal.Text(spec.get("objective", "")))
    stage.set_editor_property("advance_on_hostile_death", spec.get("advance_on_hostile_death", False))
    nxt = spec.get("next_on_hostile_death")
    stage.set_editor_property("next_stage_on_hostile_death", unreal.Name(nxt) if nxt else unreal.Name())
    return stage


def write_quest(spec):
    asset = get_or_create(spec["asset"])
    asset.set_editor_property("quest_id", spec["quest_id"])
    asset.set_editor_property("display_name", unreal.Text(spec["name"]))
    asset.set_editor_property("completed_stage", spec["completed"])
    asset.set_editor_property("stages", [make_stage(s) for s in spec["stages"]])
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {spec['asset']}")
    log(f"saved {QUEST_PATH}/{spec['asset']} ({len(spec['stages'])} stages)")


def main():
    write_quest(SHORE_WATCH)


main()
