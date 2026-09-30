"""Create or update quest Data Assets in /Game/Quests.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Quest ids and stage ids are stored in saves. Do not rename them once a quest has shipped.

The quests themselves are not defined here: every file in Tools/ContentSpecs/quests/ defines QUESTS
(see Tools/ContentSpecs/README.md) and content_specs.py loads them.
"""
import os
import sys

import unreal

_SCRIPTS = os.path.join(unreal.SystemLibrary.get_project_directory(), "Tools", "EditorScripts")
if _SCRIPTS not in sys.path:
    sys.path.insert(0, _SCRIPTS)
import content_specs

QUEST_PATH = "/Game/Quests"


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
    for quest in content_specs.load_specs("quests", "QUESTS"):
        write_quest(quest)


main()
