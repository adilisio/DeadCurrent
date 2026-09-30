"""Create or update the item definition Data Assets in /Game/Items.

Safe to re-run: existing assets are updated in place. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

The items themselves are not defined here: every file in Tools/ContentSpecs/items/ defines ITEMS
(see Tools/ContentSpecs/README.md) and content_specs.py loads them.
"""
import os
import sys

import unreal

_SCRIPTS = os.path.join(unreal.SystemLibrary.get_project_directory(), "Tools", "EditorScripts")
if _SCRIPTS not in sys.path:
    sys.path.insert(0, _SCRIPTS)
import content_specs

ITEMS_PATH = "/Game/Items"


def log(msg):
    unreal.log_warning("[DCITEMS] " + msg)


def make_tag(name):
    tag = unreal.GameplayTag()
    tag.import_text(name)
    if str(tag.get_editor_property("tag_name")) != name:
        raise RuntimeError(f"Unknown gameplay tag {name}")
    return tag


def mesh_scale(mesh, size_cm, fit_cm=None):
    bounds = mesh.get_bounding_box()
    extent = bounds.max - bounds.min
    if fit_cm:
        uniform = fit_cm / max(extent.x, extent.y, extent.z, 1.0)
        return unreal.Vector(uniform, uniform, uniform)
    if size_cm is None:
        return unreal.Vector(1, 1, 1)
    return unreal.Vector(size_cm[0] / extent.x, size_cm[1] / extent.y, size_cm[2] / extent.z)


def get_or_create(asset_name):
    path = f"{ITEMS_PATH}/{asset_name}"
    leftover = f"{ITEMS_PATH}/{asset_name}__reclass"
    if unreal.EditorAssetLibrary.does_asset_exist(leftover):
        unreal.EditorAssetLibrary.delete_asset(leftover)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.DCItemDefinition)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, ITEMS_PATH, unreal.DCItemDefinition, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    log(f"created {path}")
    return asset


def main():
    for spec in content_specs.load_specs("items", "ITEMS"):
        item = get_or_create(spec["asset"])
        mesh = unreal.load_asset(spec["mesh"])
        if not mesh:
            raise RuntimeError(f"Missing {spec['mesh']} for {spec['asset']}. Run import_art.py first.")
        item.set_editor_property("item_id", spec["item_id"])
        item.set_editor_property("display_name", unreal.Text(spec["name"]))
        item.set_editor_property("description", unreal.Text(spec["description"]))
        item.set_editor_property("category", make_tag(spec["category"]))
        item.set_editor_property("weight", spec["weight"])
        item.set_editor_property("value", spec["value"])
        item.set_editor_property("max_stack_size", spec["stack"])
        item.set_editor_property("world_mesh", mesh)
        item.set_editor_property("world_mesh_scale", mesh_scale(mesh, spec["size_cm"], spec.get("fit_cm")))
        if "firearm" in spec:
            gun = spec["firearm"]
            item.set_editor_property("ammo_item", unreal.load_asset(gun["ammo"]))
            item.set_editor_property("magazine_size", gun["mag"])
            item.set_editor_property("damage", gun["damage"])
            item.set_editor_property("range", gun["range"])
            item.set_editor_property("fire_interval", gun["fire_interval"])
            item.set_editor_property("reload_duration", gun["reload"])
            item.set_editor_property("recoil_pitch", gun["recoil_pitch"])
            item.set_editor_property("recoil_yaw_variance", gun["recoil_yaw"])
            item.set_editor_property("recoil_recovery_speed", gun["recoil_recovery"])
            item.set_editor_property("equipped_offset", unreal.Vector(*gun["equip_offset"]))
            item.set_editor_property("equipped_rotation", unreal.Rotator(
                pitch=gun["equip_rot"][0], yaw=gun["equip_rot"][1], roll=gun["equip_rot"][2]))
            item.set_editor_property("fire_sound", unreal.load_asset(gun["fire_sound"]))
            item.set_editor_property("dry_fire_sound", unreal.load_asset(gun["dry_fire_sound"]))
        if not unreal.EditorAssetLibrary.save_loaded_asset(item, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save {spec['asset']} (is the file read-only?)")
        log(f"{spec['asset']}: {spec['item_id']} '{spec['name']}' {spec['category']}")


main()
