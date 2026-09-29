"""NoCollision structure dressing in Lvl_Boathouse_Art.

Owns only actors tagged StructureDress. Safe to re-run: those actors are replaced.
Does not touch ShoreDress, and does not move gameplay actors. The *Tern* is the
motorboat wreck lined up on the hull blocks. Camp scrap stays outside the
scavenger patrol square (X 2000..2700, Y -350..350).

Run after the meshes exist (this script imports them) and after build_boathouse.py:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import os
import unreal

POLY = r"C:\FO5_AssetLibrary\CC0\polyhaven"
BOAT_FBX = r"C:\FO5_AssetLibrary\Fab\motorboat_wreck\source\tires2.fbx"
MAP_PATH = "/Game/Maps/Lvl_Boathouse"
ART_MAP = "/Game/Maps/Lvl_Boathouse_Art"
TAG = "StructureDress"
MAX_TEXTURE = 2048

GLTF = [
    dict(asset_id="wooden_crate_01", gltf="wooden_crate_01_2k.gltf"),
    dict(asset_id="Lantern_01", gltf="Lantern_01_2k.gltf"),
    dict(asset_id="hanging_industrial_lamp", gltf="hanging_industrial_lamp_2k.gltf"),
    dict(asset_id="can_rusted", gltf="can_rusted_2k.gltf"),
]

# asset, x, y, yaw, longest cm. Sits on the ground. Kept off the door, the plank, and the patrol square.
GROUND = [
    ("wooden_crate_01", 160, 210, 15, 70),
    ("can_rusted", 860, 220, 40, 40),
    ("wooden_crate_01", 3280, 1400, -20, 80),
    ("can_rusted", 2980, 1320, 10, 36),
    ("Lantern_01", 3180, 1460, 0, 45),
    ("wooden_crate_01", 2300, 370, 20, 140),
    ("wooden_crate_01", 2660, 370, -30, 120),
    ("can_rusted", 2480, 390, 70, 55),
    ("Lantern_01", 2140, 380, 20, 70),
    ("can_rusted", 2780, -80, -15, 48),
    ("wooden_crate_01", 1880, 80, 50, 90),
]

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCSTRUCT] " + msg)


def package_name(obj):
    return str(obj.get_outermost().get_name()) if obj else ""


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


def cap_textures(dest):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        return
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(asset_path)
        if not isinstance(asset, unreal.Texture):
            continue
        asset.set_editor_property("max_texture_size", MAX_TEXTURE)
        unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
        log(f"texture {asset.get_name()} {asset.blueprint_get_size_x()} cap {MAX_TEXTURE}")


def import_gltf(spec):
    dest = f"/Game/Art/PolyHaven/{spec['asset_id']}"
    existing = find_mesh(dest)
    if existing:
        log(f"reuse {existing.get_path_name()}")
        return existing
    filename = os.path.join(POLY, spec["asset_id"], spec["gltf"])
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
    cap_textures(dest)
    log(f"imported {mesh.get_path_name()}")
    return mesh


def import_boat():
    dest = "/Game/Art/Fab/motorboat_wreck"
    existing = find_mesh(dest)
    if existing:
        box = existing.get_bounding_box()
        log(f"reuse boat {existing.get_path_name()} size {box.max - box.min}")
        cap_textures(dest)
        return existing
    if not os.path.isfile(BOAT_FBX):
        raise RuntimeError(f"Missing {BOAT_FBX}")
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", BOAT_FBX)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    cap_textures(dest)
    mesh = find_mesh(dest)
    if not mesh:
        raise RuntimeError(f"FBX import produced no static mesh in {dest}")
    box = mesh.get_bounding_box()
    log(f"imported boat {mesh.get_path_name()} size {box.max - box.min}")
    return mesh


def clear_dressing():
    doomed = []
    for actor in actors.get_all_level_actors():
        if package_name(actor.get_outer()) != ART_MAP:
            continue
        if TAG in [str(tag) for tag in actor.tags]:
            doomed.append(actor)
    if doomed:
        actors.destroy_actors(doomed)
    log(f"cleared {len(doomed)} previous structure actors")


def finish(actor, mesh):
    actor.set_editor_property("tags", [unreal.Name(TAG)])
    mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    mesh_comp.set_static_mesh(mesh)
    mesh_comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_actor_enable_collision(False)
    if package_name(actor.get_outer()) != ART_MAP:
        raise RuntimeError(f"{actor.get_actor_label()} landed in {package_name(actor.get_outer())}")
    return actor


def place_ground(mesh, x, y, yaw, longest_cm):
    box = mesh.get_bounding_box()
    extent = box.max - box.min
    longest = max(extent.x, extent.y, extent.z, 1.0)
    scale = longest_cm / longest
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, 0.0), unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    actor.set_actor_label(f"StructureDress_{mesh.get_name()}")
    finish(actor, mesh)
    origin, extent = actor.get_actor_bounds(False)
    bottom = origin.z - extent.z
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(loc.x, loc.y, loc.z - bottom + 1.0), False, True)
    return actor


def place_hanging_lamp(mesh):
    box = mesh.get_bounding_box()
    extent = box.max - box.min
    longest = max(extent.x, extent.y, extent.z, 1.0)
    scale = 70.0 / longest
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(360.0, -40.0, 250.0), unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0))
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    actor.set_actor_label("StructureDress_BoathouseLamp")
    finish(actor, mesh)
    origin, extent = actor.get_actor_bounds(False)
    top = origin.z + extent.z
    loc = actor.get_actor_location()
    # Hang just under the ceiling (z 280).
    actor.set_actor_location(unreal.Vector(loc.x, loc.y, loc.z + (275.0 - top)), False, True)
    return actor


def place_tern(mesh):
    """Line the wreck up with the hull blocks. Local +Y of the hull is the bow, inland."""
    box = mesh.get_bounding_box()
    extent = box.max - box.min
    axes = (extent.x, extent.y, extent.z)
    longest = max(axes)
    # Hull blocks run about 11 m along world Y. Yaw so the mesh's long axis follows that.
    yaw = 0.0
    if axes[0] >= axes[1] and axes[0] >= axes[2]:
        yaw = 90.0
    elif axes[2] >= axes[0] and axes[2] >= axes[1]:
        yaw = 90.0
    scale_xy = 1100.0 / max(longest, 1.0)
    # Keep the mesh as the outer hull, below the wheelhouse clues. Full height buried the deck cameras.
    scale_z = 140.0 / max(axes[2], 1.0)
    # Hull frame origin (-1500, -760). Bow toward y=-300, stern toward the water.
    actor = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(-1500.0, -830.0, 40.0),
        unreal.Rotator(pitch=-4.0, yaw=yaw, roll=-3.0))
    actor.set_actor_scale3d(unreal.Vector(scale_xy, scale_xy, scale_z))
    actor.set_actor_label("StructureDress_Tern")
    finish(actor, mesh)
    assign_tern_materials(actor.get_component_by_class(unreal.StaticMeshComponent), mesh)
    origin, bounds_extent = actor.get_actor_bounds(False)
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(
        loc.x + (-1500.0 - origin.x),
        loc.y + (-830.0 - origin.y),
        loc.z + (5.0 - (origin.z - bounds_extent.z))), False, True)
    origin, bounds_extent = actor.get_actor_bounds(False)
    log(f"Tern center {origin} extent {bounds_extent} yaw {yaw} mesh {axes} scale {(scale_xy, scale_xy, scale_z)}")
    return actor


def assign_tern_materials(comp, mesh):
    u1 = unreal.load_asset("/Game/Environment/Materials/MI_DC_TernU1")
    u2 = unreal.load_asset("/Game/Environment/Materials/MI_DC_TernU2")
    if not u1 or not u2:
        raise RuntimeError("Missing MI_DC_TernU1/U2. Run import_art.py first.")
    slots = mesh.get_editor_property("static_materials") if mesh else []
    for index in range(comp.get_num_materials()):
        name = ""
        if index < len(slots):
            name = str(slots[index].get_editor_property("material_slot_name")).lower()
        chosen = u2 if "u2" in name else u1
        comp.set_material(index, chosen)
        log(f"Tern slot {index} {name or '(unnamed)'} -> {chosen.get_name()}")


def main():
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        raise RuntimeError(f"Missing {MAP_PATH}")
    meshes = {spec["asset_id"]: import_gltf(spec) for spec in GLTF}
    boat = import_boat()
    levels.load_level(MAP_PATH)
    if not levels.set_current_level_by_name("Lvl_Boathouse_Art"):
        raise RuntimeError("Could not edit Lvl_Boathouse_Art. Run build_boathouse.py first.")
    clear_dressing()
    placed = [place_tern(boat), place_hanging_lamp(meshes["hanging_industrial_lamp"])]
    for asset_id, x, y, yaw, longest in GROUND:
        placed.append(place_ground(meshes[asset_id], x, y, yaw, longest))
    package = placed[0].get_outer().get_outermost()
    if not unreal.EditorLoadingAndSavingUtils.save_packages([package], False):
        raise RuntimeError(f"Could not save {ART_MAP}")
    log(f"saved {len(placed)} structure actors in {ART_MAP}")


main()
