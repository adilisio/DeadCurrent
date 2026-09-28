"""Build /Game/Maps/Lvl_Boathouse, the first-playable greybox scenario.

Regenerates the map from scratch on every run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Layout (X is out the door, Z is up, units are cm):
  Boathouse  X 0..720, Y -320..320     wake, inspect, pistol on the workbench, door out
  Path       X 720..1800               shoreline walk to the scavenger
  Scavenger  X 2000..2700, Y -350..350 patrols the path; loot after death
  Cover      Y 550..650                wall so Mara is out of the scavenger's sight
  Mara       X 3100, Y 1300            talk, then F5 / quit / F9
"""
import math
import unreal

MAP_PATH = "/Game/Maps/Lvl_Boathouse"
CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
MAT_FLOOR = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
MAT_BLOCK = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"
MAT_INTERACTABLE = "/Game/LevelPrototyping/Materials/MI_DefaultColorway"

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

cube_mesh = unreal.load_asset(CUBE)
floor_mat = unreal.load_asset(MAT_FLOOR)
block_mat = unreal.load_asset(MAT_BLOCK)
interactable_mat = unreal.load_asset(MAT_INTERACTABLE)

bounds = cube_mesh.get_bounding_box()
CUBE_MIN = bounds.min
CUBE_SIZE = bounds.max - bounds.min
CUBE_CENTER = (bounds.max + bounds.min) * 0.5


def log(msg):
    unreal.log_warning("[DCBOAT] " + msg)


def rotate_pitch(v, pitch_deg):
    p = math.radians(pitch_deg)
    return unreal.Vector(v.x * math.cos(p) - v.z * math.sin(p), v.y, v.x * math.sin(p) + v.z * math.cos(p))


def box(label, folder, center, size, pitch=0.0, material=None, actor_class=unreal.StaticMeshActor):
    scale = unreal.Vector(size[0] / CUBE_SIZE.x, size[1] / CUBE_SIZE.y, size[2] / CUBE_SIZE.z)
    scaled_center = unreal.Vector(CUBE_CENTER.x * scale.x, CUBE_CENTER.y * scale.y, CUBE_CENTER.z * scale.z)
    offset = rotate_pitch(scaled_center, pitch)
    location = unreal.Vector(center[0], center[1], center[2]) - offset
    actor = actors.spawn_actor_from_class(actor_class, location, unreal.Rotator(pitch=pitch, yaw=0.0, roll=0.0))
    actor.set_actor_scale3d(scale)
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    mesh_comp.set_static_mesh(cube_mesh)
    mesh_comp.set_material(0, material or block_mat)
    return actor


def inspectable(label, folder, center, size, display_name, description, world_flag=None, flag_description=None):
    actor = box(label, folder, center, size, material=interactable_mat, actor_class=unreal.DCInspectableActor)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    actor.set_editor_property("description", unreal.Text(description))
    if world_flag:
        actor.set_editor_property("world_flag", unreal.Name(world_flag))
        actor.set_editor_property("flag_description", unreal.Text(flag_description or ""))
    return actor


def block(label, folder, x0, x1, y0, y1, z0, z1, material=None):
    return box(label, folder,
               ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
               (x1 - x0, y1 - y0, z1 - z0), material=material)


def resolve_soft(value):
    if isinstance(value, unreal.Object):
        return value
    return unreal.load_asset(str(value))


def set_persistent_id(actor, persistent_id):
    comp = actor.get_component_by_class(unreal.DCPersistentIdComponent)
    if not comp:
        log(f"no PersistentId on {actor.get_actor_label()}")
        return
    comp.set_editor_property("persistent_id", persistent_id)
    log(f"id {actor.get_actor_label()}={persistent_id}")


def door(label, folder, hinge, width, height, thickness, display_name, persistent_id):
    actor = actors.spawn_actor_from_class(
        unreal.DCDoor, unreal.Vector(*hinge), unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0))
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    leaf = actor.get_editor_property("door_mesh")
    leaf.set_static_mesh(cube_mesh)
    leaf.set_material(0, interactable_mat)
    scale = unreal.Vector(thickness / CUBE_SIZE.x, width / CUBE_SIZE.y, height / CUBE_SIZE.z)
    leaf.set_editor_property("relative_scale3d", scale)
    leaf.set_editor_property("relative_location", unreal.Vector(
        -thickness / 2 - CUBE_MIN.x * scale.x,
        1.0 - CUBE_MIN.y * scale.y,
        1.0 - CUBE_MIN.z * scale.z))
    set_persistent_id(actor, persistent_id)
    return actor


def pickup(label, folder, item_path, quantity, x, y, surface_z, yaw=0.0, persistent_id=None):
    item = unreal.load_asset(item_path)
    mesh = resolve_soft(item.get_editor_property("world_mesh"))
    scale = item.get_editor_property("world_mesh_scale")
    mesh_bounds = mesh.get_bounding_box()
    center = (mesh_bounds.max + mesh_bounds.min) * 0.5
    location = unreal.Vector(
        x - center.x * scale.x,
        y - center.y * scale.y,
        surface_z - mesh_bounds.min.z * scale.z)

    actor = actors.spawn_actor_from_class(
        unreal.DCItemPickup, location, unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    actor.set_editor_property("item", item)
    actor.set_editor_property("quantity", quantity)
    mesh_comp = actor.get_editor_property("mesh")
    mesh_comp.set_static_mesh(mesh)
    mesh_comp.set_editor_property("relative_scale3d", scale)
    if persistent_id:
        set_persistent_id(actor, persistent_id)
    return actor


def first_existing(*paths):
    for path in paths:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            return path
    return None


def first_of_class(asset_class, *paths):
    for path in paths:
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            continue
        asset = unreal.load_asset(path)
        if isinstance(asset, asset_class):
            return path, asset
        log(f"skip {path} type={asset.get_class().get_name() if asset else 'None'}")
    return None, None


def assign_mannequin(actor, *mesh_paths):
    mesh_path, mesh_asset = first_of_class(unreal.SkeletalMesh, *mesh_paths)
    abp_path = first_existing(
        "/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed",
        "/Game/Characters/Mannequins/Anims/Manny/ABP_Manny",
        "/Game/Characters/Mannequins/Rigs/ABP_Manny",
    )
    mesh_comp = actor.get_editor_property("mesh")
    if mesh_asset:
        mesh_comp.set_skeletal_mesh_asset(mesh_asset)
        log(f"{actor.get_actor_label()} mesh {mesh_path}")
    else:
        log(f"{actor.get_actor_label()} no mannequin skeletal mesh found")
    if abp_path:
        abp_class = unreal.EditorAssetLibrary.load_blueprint_class(abp_path)
        mesh_comp.set_animation_mode(unreal.AnimationMode.ANIMATION_BLUEPRINT)
        mesh_comp.set_anim_class(abp_class)
        log(f"{actor.get_actor_label()} anim {abp_path}")


def build_lighting():
    folder = "Lighting"
    sun = actors.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 1000),
        unreal.Rotator(pitch=-40.0, yaw=35.0, roll=0.0))
    sun.set_actor_label("Sun")
    sun.set_folder_path(folder)
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("atmosphere_sun_light", True)
    sun_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    # Between moonlight (10) and the 12000 lux blowout. Auto-exposure meters the rest.
    sun_comp.set_editor_property("intensity", 300.0)

    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1000))
    sky.set_actor_label("SkyLight")
    sky.set_folder_path(folder)
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky_comp.set_editor_property("real_time_capture", True)
    sky_comp.set_editor_property("intensity", 0.8)

    atmo = actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
    atmo.set_actor_label("SkyAtmosphere")
    atmo.set_folder_path(folder)

    fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0))
    fog.set_actor_label("HeightFog")
    fog.set_folder_path(folder)
    fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_comp.set_editor_property("fog_density", 0.005)
    fog_comp.set_editor_property("fog_max_opacity", 0.35)

    pp = actors.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0.0, 0.0, 0.0))
    pp.set_actor_label("PostProcess")
    pp.set_folder_path(folder)
    pp.set_editor_property("unbound", True)
    pp.set_editor_property("priority", 1.0)
    pp.set_editor_property("blend_weight", 1.0)
    settings = pp.get_editor_property("settings")
    settings.set_editor_property("override_auto_exposure_method", True)
    settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
    settings.set_editor_property("override_auto_exposure_min_brightness", True)
    settings.set_editor_property("auto_exposure_min_brightness", 0.5)
    settings.set_editor_property("override_auto_exposure_max_brightness", True)
    settings.set_editor_property("auto_exposure_max_brightness", 12.0)
    settings.set_editor_property("override_auto_exposure_bias", True)
    settings.set_editor_property("auto_exposure_bias", 0.0)
    pp.set_editor_property("settings", settings)


def build_ground():
    folder = "Ground"
    block("Floor", folder, -200, 3600, -600, 1800, -50, 0, material=floor_mat)
    block("ShoreCurb", folder, -200, 3600, -600, -580, 0, 18)
    block("Water", folder, -200, 3600, -1400, -600, -80, -10)

    start = actors.spawn_actor_from_class(
        unreal.PlayerStart, unreal.Vector(220.0, 0.0, 100.0),
        unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0))
    start.set_actor_label("PlayerStart")
    start.set_folder_path(folder)


def build_boathouse():
    folder = "Boathouse"
    wall_h = 280
    door_h = 210
    door_half = 50

    block("Wall_Back", folder, 0, 20, -320, 320, 0, wall_h)
    block("Wall_Right", folder, 0, 720, 300, 320, 0, wall_h)
    block("Ceiling", folder, 0, 720, -320, 320, wall_h, wall_h + 20)

    # Shore-side wall with a window looking at the lake.
    block("Wall_Left_West", folder, 0, 250, -320, -300, 0, wall_h)
    block("Wall_Left_East", folder, 400, 720, -320, -300, 0, wall_h)
    block("Wall_Left_Sill", folder, 250, 400, -320, -300, 0, 110)
    block("Wall_Left_Head", folder, 250, 400, -320, -300, 200, wall_h)

    dx = 700
    block("Wall_Front_Left", folder, dx, dx + 20, -320, -door_half, 0, wall_h)
    block("Wall_Front_Right", folder, dx, dx + 20, door_half, 320, 0, wall_h)
    block("Wall_Front_Lintel", folder, dx, dx + 20, -door_half, door_half, door_h, wall_h)
    door("Door", folder, (dx + 10, -door_half, 0), door_half * 2 - 2, door_h - 2, 6, "Boathouse Door", "boat.door")

    bench_top = 75
    block("Workbench", folder, 400, 520, 160, 260, 0, bench_top)
    pickup("Pickup_Pistol", folder, "/Game/Items/DA_Item_Pistol", 1, 440, 200, bench_top,
           yaw=90.0, persistent_id="boat.pickup_pistol")
    pickup("Pickup_Ammo9mm", folder, "/Game/Items/DA_Item_Ammo9mm", 24, 490, 220, bench_top,
           persistent_id="boat.pickup_ammo")
    pickup("Pickup_FieldDressing", folder, "/Game/Items/DA_Item_FieldDressing", 1, 470, 180, bench_top,
           persistent_id="boat.pickup_dressing")

    inspectable("Cot", folder, (160, -180, 25), (190, 80, 50), "Salt-stiff cot",
                "The canvas is stiff with salt and old sweat. You slept here, or passed out here. Hard to tell which.")
    inspectable("Radio", folder, (80, 180, 18), (28, 22, 36), "Dead radio",
                "The casing is faintly warm. Nothing in this building should still be drawing power.")
    inspectable("Notice", folder, (30, 0, 150), (6, 80, 90), "Faded notice",
                "GREAT LAKES MARITIME AUTHORITY. Storm protocol. Stay inland during Current events. The date is torn off.")
    inspectable("WindowSill", folder, (325, -270, 150), (140, 16, 8), "Lake window",
                "The water sits too still. No birds. A hull lists in the shallows, paint long gone.")


def build_exterior():
    folder = "Exterior"
    inspectable("KeepOut", folder, (740, 140, 130), (8, 70, 90), "Painted board",
                "KEEP OUT is brushed on in tar. Someone added, smaller, underneath: HEAR IT TOO.")
    inspectable("Hull", folder, (1200, -280, 40), (220, 70, 80), "Beached hull",
                "A workboat, rolled and gutted. The engine bay is empty. Wiring was cut, not torn.")
    inspectable("Can", folder, (980, 80, 22), (22, 22, 44), "Fuel can",
                "Light. The cap is missing. A sour chemical smell, not gasoline.")
    inspectable("Crates", folder, (1550, 220, 35), (80, 80, 70), "Nailed crates",
                "Stenciled GREAT LAKES MARITIME SUPPLY, same as the crate in every other ruin on this shore.")


def build_scavenger():
    folder = "Combat"
    scav = actors.spawn_actor_from_class(
        unreal.DCScavengerCharacter, unreal.Vector(2300.0, 0.0, 96.0),
        unreal.Rotator(pitch=0.0, yaw=180.0, roll=0.0))
    scav.set_actor_label("Scavenger")
    scav.set_folder_path(folder)
    scav.set_editor_property("patrol_points", [
        unreal.Vector(2000.0, -350.0, 0.0),
        unreal.Vector(2700.0, -350.0, 0.0),
        unreal.Vector(2700.0, 350.0, 0.0),
        unreal.Vector(2000.0, 350.0, 0.0),
    ])
    assign_mannequin(
        scav,
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
    )
    set_persistent_id(scav, "boat.scavenger")

    inspectable("CampJunk", folder, (2480, -220, 20), (50, 40, 40), "Scavenger kit",
                "A torn pack, empty cans, a radio housing with one coil still seated. Whoever walks this loop has been here a while.")
    pickup("Pickup_RadioCoil", folder, "/Game/Items/DA_Item_RadioCoil", 1, 2520, -280, 20,
           persistent_id="boat.pickup_coil")


def build_cover_and_npc():
    folder = "Cover"
    # LOS wall: scavenger patrols south of this, Mara stands north.
    block("Ridge", folder, 1900, 3400, 550, 650, 0, 280)
    block("RidgeEnd", folder, 3380, 3480, 550, 1500, 0, 280)

    folder = "NPC"
    block("Shed_Back", folder, 3040, 3220, 1480, 1500, 0, 220)
    block("Shed_Left", folder, 3040, 3060, 1180, 1500, 0, 220)
    block("Shed_Roof", folder, 3040, 3220, 1180, 1500, 220, 240)

    npc = actors.spawn_actor_from_class(
        unreal.DCFriendlyNPC, unreal.Vector(3100.0, 1280.0, 96.0),
        unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0))
    npc.set_actor_label("Mara")
    npc.set_folder_path(folder)
    assign_mannequin(
        npc,
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
    )
    set_persistent_id(npc, "boat.mara")

    inspectable("Lookout", folder, (3160, 1220, 40), (40, 30, 80), "Lookout crate",
                "Someone has been watching the path from here. F5 saves. F9 loads after you quit.",
                world_flag="shore.cleared",
                flag_description="The radio on this shore is quieter. Mara was right it wouldn't last.")


def build_nav():
    folder = "Ground"
    vol = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(1700.0, 400.0, 0.0))
    vol.set_actor_label("NavBounds")
    vol.set_folder_path(folder)
    vol.set_actor_scale3d(unreal.Vector(28.0, 18.0, 8.0))

    world = unreal.EditorLevelLibrary.get_editor_world()
    unreal.SystemLibrary.execute_console_command(world, "RebuildNavigation")
    log("nav mesh rebuild requested")


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        levels.load_level(MAP_PATH)
        actors.destroy_actors([a for a in actors.get_all_level_actors()
                               if not isinstance(a, (unreal.WorldSettings, unreal.Brush))])
    elif not levels.new_level(MAP_PATH, False):
        raise RuntimeError(f"Could not create {MAP_PATH}")

    log(f"cube bounds min={CUBE_MIN} size={CUBE_SIZE}")
    build_lighting()
    build_ground()
    build_boathouse()
    build_exterior()
    build_scavenger()
    build_cover_and_npc()
    build_nav()

    if not levels.save_current_level():
        raise RuntimeError(f"Could not save {MAP_PATH} (is the file read-only?)")
    log(f"saved {MAP_PATH} with {len(actors.get_all_level_actors())} actors")


main()
