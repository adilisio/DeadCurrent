"""Create or update the item definition Data Assets in /Game/Items.

Safe to re-run: existing assets are updated in place. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

ITEMS_PATH = "/Game/Items"

# size_cm: for placeholder meshes, the item's real-world size; the mesh is scaled to fit.
# None keeps the mesh at its authored scale.
ITEMS = [
    dict(asset="DA_Item_Pistol", item_id="pistol_service", name="Service Pistol",
         description="A pre-collapse sidearm. Worn, but it still cycles.",
         category="Item.Weapon.Firearm", weight=1.2, value=120, stack=1,
         mesh="/Game/Weapons/Pistol/Meshes/SM_Pistol", size_cm=None),
    dict(asset="DA_Item_Ammo9mm", item_id="ammo_9mm", name="9mm Rounds",
         description="Loose pistol rounds, some hand-reloaded.",
         category="Item.Ammo", weight=0.01, value=1, stack=999,
         mesh="/Game/LevelPrototyping/Meshes/SM_ChamferCube", size_cm=(12, 8, 5)),
    dict(asset="DA_Item_FieldDressing", item_id="field_dressing", name="Field Dressing",
         description="Boiled cloth and a strip of tape. Stops bleeding, mostly.",
         category="Item.Consumable.Medical", weight=0.1, value=15, stack=10,
         mesh="/Game/LevelPrototyping/Meshes/SM_Cylinder", size_cm=(8, 8, 10)),
    dict(asset="DA_Item_SalvagedWiring", item_id="salvage_wiring", name="Salvaged Wiring",
         description="Copper wire stripped from dead machinery.",
         category="Item.Salvage", weight=0.25, value=4, stack=50,
         mesh="/Game/LevelPrototyping/Meshes/SM_Cylinder", size_cm=(16, 16, 5)),
]


def log(msg):
    unreal.log_warning("[DCITEMS] " + msg)


def make_tag(name):
    tag = unreal.GameplayTag()
    tag.import_text(name)
    if str(tag.get_editor_property("tag_name")) != name:
        raise RuntimeError(f"Unknown gameplay tag {name}")
    return tag


def mesh_scale(mesh, size_cm):
    if size_cm is None:
        return unreal.Vector(1, 1, 1)
    bounds = mesh.get_bounding_box()
    extent = bounds.max - bounds.min
    return unreal.Vector(size_cm[0] / extent.x, size_cm[1] / extent.y, size_cm[2] / extent.z)


def get_or_create(asset_name):
    path = f"{ITEMS_PATH}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.DCItemDefinition)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, ITEMS_PATH, unreal.DCItemDefinition, factory)
    log(f"created {path}")
    return asset


def main():
    for spec in ITEMS:
        item = get_or_create(spec["asset"])
        mesh = unreal.load_asset(spec["mesh"])
        item.set_editor_property("item_id", spec["item_id"])
        item.set_editor_property("display_name", unreal.Text(spec["name"]))
        item.set_editor_property("description", unreal.Text(spec["description"]))
        item.set_editor_property("category", make_tag(spec["category"]))
        item.set_editor_property("weight", spec["weight"])
        item.set_editor_property("value", spec["value"])
        item.set_editor_property("max_stack_size", spec["stack"])
        item.set_editor_property("world_mesh", mesh)
        item.set_editor_property("world_mesh_scale", mesh_scale(mesh, spec["size_cm"]))
        if not unreal.EditorAssetLibrary.save_loaded_asset(item, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save {spec['asset']} (is the file read-only?)")
        log(f"{spec['asset']}: {spec['item_id']} '{spec['name']}' {spec['category']}")


main()
