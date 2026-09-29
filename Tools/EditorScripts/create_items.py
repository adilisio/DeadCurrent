"""Create or update the item definition Data Assets in /Game/Items.

Safe to re-run: existing assets are updated in place. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

ITEMS_PATH = "/Game/Items"

# size_cm: for placeholder meshes, the item's real-world size; the mesh is scaled to fit.
# fit_cm: for an imported prop, the longest side. The scale is uniform so the mesh keeps its shape.
# None keeps the mesh at its authored scale.
CRATE = "/Game/Art/PolyHaven/wooden_crate_01/wooden_crate_01_2k/StaticMeshes/wooden_crate_01"
ITEMS = [
    dict(asset="DA_Item_Ammo9mm", item_id="ammo_9mm", name="9mm Rounds",
         description="Loose pistol rounds, some hand-reloaded.",
         category="Item.Ammo", weight=0.01, value=1, stack=999,
         mesh=CRATE, size_cm=None, fit_cm=14),
    dict(asset="DA_Item_Pistol", item_id="pistol_service", name="Service Pistol",
         description="A pre-collapse sidearm. Worn, but it still cycles.",
         category="Item.Weapon.Firearm", weight=1.2, value=120, stack=1,
         mesh="/Game/Weapons/Pistol/Meshes/SM_Pistol", size_cm=None,
         firearm=dict(ammo="/Game/Items/DA_Item_Ammo9mm", mag=15, damage=25.0,
                      range=10000.0, fire_interval=0.18, reload=1.3,
                      recoil_pitch=1.4, recoil_yaw=0.4, recoil_recovery=12.0,
                      equip_offset=(38.0, 12.0, -20.0), equip_rot=(6.0, -90.0, 4.0),
                      fire_sound="/Game/Weapons/GrenadeLauncher/Audio/FirstPersonTemplateWeaponFire02")),
    dict(asset="DA_Item_FieldDressing", item_id="field_dressing", name="Field Dressing",
         description="Boiled cloth and a strip of tape. Stops bleeding, mostly.",
         category="Item.Consumable.Medical", weight=0.1, value=15, stack=10,
         mesh="/Game/Art/Meshy/field_dressing/SM_field_dressing", size_cm=None, fit_cm=10),
    dict(asset="DA_Item_SalvagedWiring", item_id="salvage_wiring", name="Salvaged Wiring",
         description="Copper wire stripped from dead machinery.",
         category="Item.Salvage", weight=0.25, value=4, stack=50,
         mesh="/Game/LevelPrototyping/Meshes/SM_Cylinder", size_cm=(16, 16, 5)),
    dict(asset="DA_Item_RadioCoil", item_id="radio_coil", name="Relay Coil",
         description="A hand-wound copper coil from a Maritime Authority relay. It is warm, and it hums when you hold it close.",
         category="Item.Quest", weight=0.2, value=8, stack=1,
         mesh="/Game/Art/Meshy/radio_coil/SM_radio_coil", size_cm=None, fit_cm=12),
    dict(asset="DA_Item_SurveyChart", item_id="survey_chart", name="Sounder Chart",
         description="A roll of the Tern's depth-sounder paper, torn off at the mark. Regular spikes, evenly spaced, and someone has pencilled AGAIN beside the last one.",
         category="Item.Quest", weight=0.05, value=6, stack=1,
         mesh="/Game/Art/Meshy/sounder_chart/SM_sounder_chart", size_cm=None, fit_cm=18),
]


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
    for spec in ITEMS:
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
        if not unreal.EditorAssetLibrary.save_loaded_asset(item, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save {spec['asset']} (is the file read-only?)")
        log(f"{spec['asset']}: {spec['item_id']} '{spec['name']}' {spec['category']}")


main()
