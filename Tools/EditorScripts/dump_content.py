"""Read-only dump of every generated item, quest, and dialogue Data Asset's meaningful properties.

Used to prove a change to the content generators (create_items.py, create_quest.py, create_dialogue.py, or the spec
files they load) is behavior-preserving: dump before, change, regenerate, dump after, compare. It changes nothing.

Output path: the DC_DUMP_OUT environment variable, else Saved/ContentDumps/content.json (under the project).
The file is sorted and stable, so two dumps compare with a plain diff (`git diff --no-index before.json after.json`).
`Tools\DumpContent.bat <label>` runs it and writes Saved/ContentDumps/<label>.json.

Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import json
import os

import unreal

ITEM_PATH = "/Game/Items"
QUEST_PATH = "/Game/Quests"
DIALOGUE_PATH = "/Game/Dialogue"


def log(msg):
    unreal.log_warning("[DCDUMP] " + msg)


def norm(value):
    """Turn an editor property value into plain JSON data."""
    if value is None:
        return None
    if isinstance(value, (bool, int, float, str)):
        return value
    if isinstance(value, unreal.Vector):
        return [round(value.x, 4), round(value.y, 4), round(value.z, 4)]
    if isinstance(value, unreal.Rotator):
        return [round(value.pitch, 4), round(value.yaw, 4), round(value.roll, 4)]
    if isinstance(value, unreal.GameplayTag):
        return str(value.get_editor_property("tag_name"))
    if isinstance(value, (unreal.Name, unreal.Text)):
        return str(value)
    if isinstance(value, unreal.Object):
        return value.get_path_name()
    if isinstance(value, (list, tuple)):
        return [norm(v) for v in value]
    return str(value)


def props(obj, names):
    return {name: norm(obj.get_editor_property(name)) for name in names}


ITEM_FIELDS = [
    "item_id", "display_name", "description", "category", "weight", "value", "max_stack_size",
    "world_mesh", "world_mesh_scale",
    "ammo_item", "magazine_size", "damage", "range", "fire_interval", "reload_duration",
    "recoil_pitch", "recoil_yaw_variance", "recoil_recovery_speed",
    "equipped_offset", "equipped_rotation", "fire_sound", "dry_fire_sound",
]
RULE_COND_FIELDS = ["type", "id", "stage", "quantity", "negate"]
RULE_CONS_FIELDS = ["type", "id", "stage", "quantity", "item"]


def dump_conditions(conditions):
    return [props(c, RULE_COND_FIELDS) for c in conditions]


def dump_consequences(consequences):
    return [props(c, RULE_CONS_FIELDS) for c in consequences]


def dump_item(asset):
    return props(asset, ITEM_FIELDS)


def dump_quest(asset):
    stages = []
    for stage in asset.get_editor_property("stages"):
        stages.append(dict(
            stage_id=norm(stage.get_editor_property("stage_id")),
            objective_text=norm(stage.get_editor_property("objective_text")),
            completes_quest=stage.get_editor_property("completes_quest"),
            on_enter=dump_consequences(stage.get_editor_property("on_enter")),
            transitions=[dict(next_stage=norm(t.get_editor_property("next_stage")),
                              conditions=dump_conditions(t.get_editor_property("conditions")))
                         for t in stage.get_editor_property("transitions")],
        ))
    return dict(quest_id=norm(asset.get_editor_property("quest_id")),
                display_name=norm(asset.get_editor_property("display_name")),
                start_stage=norm(asset.get_editor_property("start_stage")),
                stages=stages)


def dump_dialogue(asset):
    nodes = []
    for node in asset.get_editor_property("nodes"):
        nodes.append(dict(
            node_id=norm(node.get_editor_property("node_id")),
            speaker=norm(node.get_editor_property("speaker")),
            line=norm(node.get_editor_property("line")),
            choices=[dict(text=norm(c.get_editor_property("text")),
                          next_node_id=norm(c.get_editor_property("next_node_id")),
                          conditions=dump_conditions(c.get_editor_property("conditions")),
                          consequences=dump_consequences(c.get_editor_property("consequences")))
                     for c in node.get_editor_property("choices")],
        ))
    return dict(dialogue_id=norm(asset.get_editor_property("dialogue_id")),
                entry_node_id=norm(asset.get_editor_property("entry_node_id")),
                entries=[dict(node_id=norm(e.get_editor_property("node_id")),
                              conditions=dump_conditions(e.get_editor_property("conditions")))
                         for e in asset.get_editor_property("entries")],
                nodes=nodes)


def dump_folder(folder, dumper):
    result = {}
    if not unreal.EditorAssetLibrary.does_directory_exist(folder):
        return result
    for path in sorted(unreal.EditorAssetLibrary.list_assets(folder, recursive=False, include_folder=False)):
        asset = unreal.load_asset(path)
        if asset is None:
            continue
        result[asset.get_name()] = dumper(asset)
    return result


def main():
    out = os.environ.get("DC_DUMP_OUT") or os.path.join(
        unreal.SystemLibrary.get_project_directory(), "Saved", "ContentDumps", "content.json")
    os.makedirs(os.path.dirname(out), exist_ok=True)
    data = dict(
        items=dump_folder(ITEM_PATH, dump_item),
        quests=dump_folder(QUEST_PATH, dump_quest),
        dialogues=dump_folder(DIALOGUE_PATH, dump_dialogue),
    )
    with open(out, "w", encoding="utf-8") as handle:
        json.dump(data, handle, indent=1, sort_keys=True, ensure_ascii=False)
    log(f"wrote {out}: {len(data['items'])} items, {len(data['quests'])} quests, {len(data['dialogues'])} dialogues")


main()
