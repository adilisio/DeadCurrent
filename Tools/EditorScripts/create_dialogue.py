"""Create or update dialogue Data Assets in /Game/Dialogue.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Conditions and consequences are the shared rule language (FDCGameplayCondition /
FDCGameplayConsequence), the same one quests and inspectables use.

The conversations themselves are not defined here: every file in Tools/ContentSpecs/dialogue/ defines DIALOGUES
(see Tools/ContentSpecs/README.md) and content_specs.py loads them.
"""
import os
import sys

import unreal

_SCRIPTS = os.path.join(unreal.SystemLibrary.get_project_directory(), "Tools", "EditorScripts")
if _SCRIPTS not in sys.path:
    sys.path.insert(0, _SCRIPTS)
import content_specs

DIALOGUE_PATH = "/Game/Dialogue"


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
    for dialogue in content_specs.load_specs("dialogue", "DIALOGUES"):
        write_dialogue(dialogue)


main()
