"""Place NoCollision shoreline dressing in Lvl_Boathouse_Art.

Owns only actors tagged ShoreDress. Safe to re-run: those actors are replaced.
build_boathouse.py does not load this level for editing and does not save it, so a
content rebuild leaves the dressing in place. Nothing here has collision.

Run after build_boathouse.py:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import os
import unreal

LIBRARY = r"C:\FO5_AssetLibrary\CC0\polyhaven"
MAP_PATH = "/Game/Maps/Lvl_Boathouse"
ART_MAP = "/Game/Maps/Lvl_Boathouse_Art"
TAG = "ShoreDress"

MODELS = [
    dict(asset_id="dead_quiver_branch_01", gltf="dead_quiver_branch_01_2k.gltf"),
    dict(asset_id="dead_tree_trunk", gltf="dead_tree_trunk_2k.gltf"),
    dict(asset_id="tree_stump_01", gltf="tree_stump_01_2k.gltf"),
]

# x, y, yaw, longest-axis cm. Sits on the waterline or north of the ridge.
# Kept off the doorway, the boarding plank, the scavenger patrol square, and the live water.
PLACEMENTS = [
    ("dead_quiver_branch_01", 1400, -530, 25, 160),
    ("tree_stump_01", 1850, -510, 0, 70),
    ("dead_tree_trunk", 3200, -470, -20, 240),
    ("dead_quiver_branch_01", 2920, 40, 70, 140),
    ("dead_quiver_branch_01", -2500, -500, 80, 150),
    ("tree_stump_01", -2860, -470, 15, 75),
    ("dead_tree_trunk", -3050, 420, 35, 220),
    ("tree_stump_01", 2200, 800, 0, 70),
    ("dead_quiver_branch_01", 2550, 860, 120, 150),
]

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCDRESS] " + msg)


def package_name(obj):
    return str(obj.get_outermost().get_name()) if obj else ""


def import_model(spec):
    dest = f"/Game/Art/PolyHaven/{spec['asset_id']}"
    existing = find_mesh(dest)
    if existing:
        log(f"reuse {existing.get_path_name()}")
        return existing
    filename = os.path.join(LIBRARY, spec["asset_id"], spec["gltf"])
    if not os.path.isfile(filename):
        raise RuntimeError(f"Missing {filename}")
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = find_mesh(dest)
    if not mesh:
        raise RuntimeError(f"glTF import produced no static mesh in {dest}")
    log(f"imported {mesh.get_path_name()}")
    return mesh


def find_mesh(dest):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        return None
    best = None
    best_size = -1.0
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(asset_path)
        if not isinstance(asset, unreal.StaticMesh):
            continue
        box = asset.get_bounding_box()
        size = (box.max - box.min).length()
        if size > best_size:
            best = asset
            best_size = size
    return best


def clear_dressing():
    doomed = []
    for actor in actors.get_all_level_actors():
        if package_name(actor.get_outer()) != ART_MAP:
            continue
        if TAG in [str(tag) for tag in actor.tags]:
            doomed.append(actor)
    if doomed:
        actors.destroy_actors(doomed)
    log(f"cleared {len(doomed)} previous dressing actors")


def place(mesh, x, y, yaw, longest_cm):
    box = mesh.get_bounding_box()
    extent = box.max - box.min
    longest = max(extent.x, extent.y, extent.z)
    if longest < 1.0:
        raise RuntimeError(f"{mesh.get_name()} has no bounds")
    scale = longest_cm / longest
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, 0.0), unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    actor.set_actor_label(f"ShoreDress_{actor.get_name()}")
    actor.set_editor_property("tags", [unreal.Name(TAG)])
    mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    mesh_comp.set_static_mesh(mesh)
    mesh_comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_actor_enable_collision(False)
    origin, extent = actor.get_actor_bounds(False)
    bottom = origin.z - extent.z
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(loc.x, loc.y, loc.z - bottom + 1.0), False, True)
    if package_name(actor.get_outer()) != ART_MAP:
        raise RuntimeError(f"{actor.get_actor_label()} landed in {package_name(actor.get_outer())}")
    return actor


def main():
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        raise RuntimeError(f"Missing {MAP_PATH}")
    levels.load_level(MAP_PATH)
    if not levels.set_current_level_by_name("Lvl_Boathouse_Art"):
        raise RuntimeError("Could not edit Lvl_Boathouse_Art. Run build_boathouse.py first.")
    meshes = {spec["asset_id"]: import_model(spec) for spec in MODELS}
    clear_dressing()
    placed = []
    for asset_id, x, y, yaw, longest in PLACEMENTS:
        placed.append(place(meshes[asset_id], x, y, yaw, longest))
    package = placed[0].get_outer().get_outermost()
    if not unreal.EditorLoadingAndSavingUtils.save_packages([package], False):
        raise RuntimeError(f"Could not save {ART_MAP}")
    log(f"saved {len(placed)} dressing actors in {ART_MAP}")


main()
