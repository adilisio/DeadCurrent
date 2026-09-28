"""FP-02: build /Game/Maps/Lvl_TestGym, a greybox movement test map.

Regenerates the map from scratch on every run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Layout (X is forward from the spawn point, Z is up, units are cm):
  Center lane  (Y = 0)      sprint lane with a floor marker every 5 m
  Left lane    (Y = -1200)  stairs up to 1.8 m platforms separated by 2 m, 3.5 m and 5 m gaps
  Right lane   (Y = +1200)  30 degree walkable ramp and 50 degree unwalkable ramp
  Far right    (Y = +2600)  ledges at 20, 40, 60, 80 and 110 cm
  Far left     (Y = -2600)  1.4 m crouch tunnel, then a 1 x 2.1 m doorway into a 1 m corridor
"""
import math
import unreal

MAP_PATH = "/Game/Maps/Lvl_TestGym"
CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
MAT_FLOOR = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
MAT_BLOCK = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

cube_mesh = unreal.load_asset(CUBE)
floor_mat = unreal.load_asset(MAT_FLOOR)
block_mat = unreal.load_asset(MAT_BLOCK)

bounds = cube_mesh.get_bounding_box()
CUBE_MIN = bounds.min
CUBE_SIZE = bounds.max - bounds.min
CUBE_CENTER = (bounds.max + bounds.min) * 0.5


def log(msg):
    unreal.log_warning("[FP02] " + msg)


def rotate_pitch(v, pitch_deg):
    p = math.radians(pitch_deg)
    return unreal.Vector(v.x * math.cos(p) - v.z * math.sin(p), v.y, v.x * math.sin(p) + v.z * math.cos(p))


def box(label, folder, center, size, pitch=0.0, material=None):
    """Place the cube mesh so it covers `size` centered on `center`, pitched about Y."""
    scale = unreal.Vector(size[0] / CUBE_SIZE.x, size[1] / CUBE_SIZE.y, size[2] / CUBE_SIZE.z)
    scaled_center = unreal.Vector(CUBE_CENTER.x * scale.x, CUBE_CENTER.y * scale.y, CUBE_CENTER.z * scale.z)
    offset = rotate_pitch(scaled_center, pitch)
    location = unreal.Vector(center[0], center[1], center[2]) - offset
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, location, unreal.Rotator(roll=0.0, pitch=pitch, yaw=0.0))
    actor.set_actor_scale3d(scale)
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    mesh_comp = actor.static_mesh_component
    mesh_comp.set_static_mesh(cube_mesh)
    mesh_comp.set_material(0, material or block_mat)
    return actor


def block(label, folder, x0, x1, y0, y1, z0, z1, material=None):
    """Axis-aligned box from min to max corners."""
    return box(label, folder,
               ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2),
               (x1 - x0, y1 - y0, z1 - z0), material=material)


def ramp(label, folder, x0, y0, y1, rise, angle_deg, thickness=20.0):
    """Slab rising from the floor at x0 to `rise` height, top surface at `angle_deg`."""
    length = rise / math.sin(math.radians(angle_deg))
    run = rise / math.tan(math.radians(angle_deg))
    # Center of the slab's top surface is halfway up; push down half the thickness along the slab normal.
    top_center = unreal.Vector(x0 + run / 2, 0.0, rise / 2)
    normal = rotate_pitch(unreal.Vector(0.0, 0.0, 1.0), angle_deg)
    center = top_center - normal * (thickness / 2)
    box(label, folder, (center.x, (y0 + y1) / 2, center.z), (length, y1 - y0, thickness), pitch=angle_deg)
    return x0 + run


def build_lighting():
    folder = "Lighting"
    sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1000),
                                        unreal.Rotator(roll=0.0, pitch=-40.0, yaw=35.0))
    sun.set_actor_label("Sun")
    sun.set_folder_path(folder)
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("atmosphere_sun_light", True)
    sun_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1000))
    sky.set_actor_label("SkyLight")
    sky.set_folder_path(folder)
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky_comp.set_editor_property("real_time_capture", True)

    for cls, label in [(unreal.SkyAtmosphere, "SkyAtmosphere"),
                       (unreal.ExponentialHeightFog, "HeightFog"),
                       (unreal.VolumetricCloud, "VolumetricCloud")]:
        a = actors.spawn_actor_from_class(cls, unreal.Vector(0, 0, 0))
        a.set_actor_label(label)
        a.set_folder_path(folder)


def build_ground():
    block("Floor", "Ground", -1500, 5500, -4000, 4000, -50, 0, material=floor_mat)

    start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0, 0, 100), unreal.Rotator(0, 0, 0))
    start.set_folder_path("Ground")


def build_sprint_lane():
    folder = "SprintLane"
    for i in range(1, 9):
        x = i * 500
        block(f"Marker_{x // 100}m", folder, x - 5, x + 5, -200, 200, 0, 1)


def build_jump_lane():
    folder = "JumpLane"
    y0, y1 = -1350, -1050
    step_rise, step_run, steps = 18, 35, 10
    x = 500
    for i in range(steps):
        block(f"Step_{i + 1:02d}", folder, x, x + step_run, y0, y1, 0, step_rise * (i + 1))
        x += step_run
    top = step_rise * steps

    gaps = [0, 200, 350, 500]
    for i, gap in enumerate(gaps):
        x += gap
        label = "Platform_Start" if gap == 0 else f"Platform_AfterGap{gap}cm"
        block(label, folder, x, x + 500, y0, y1, 0, top)
        x += 500


def build_ramp_lane():
    folder = "RampLane"
    top = 200
    end = ramp("Ramp_30deg", folder, 500, 900, 1200, top, 30)
    block("Ramp_30deg_Platform", folder, end, end + 500, 900, 1200, 0, top)

    end = ramp("Ramp_50deg_Unwalkable", folder, 500, 1350, 1650, top, 50)
    block("Ramp_50deg_Backstop", folder, end, end + 300, 1350, 1650, 0, top)


def build_ledge_lane():
    folder = "LedgeLane"
    for i, height in enumerate([20, 40, 60, 80, 110]):
        x = 600 + i * 400
        block(f"Ledge_{height}cm", folder, x, x + 200, 2500, 2700, 0, height)


def build_crouch_and_door_lane():
    folder = "CrouchAndDoorLane"
    cy = -2600

    # Crouch tunnel: 2 m wide, 1.4 m clearance.
    tx0, tx1, clearance, inner = 800, 1600, 140, 100
    block("Tunnel_WallLeft", folder, tx0, tx1, cy - inner - 20, cy - inner, 0, clearance)
    block("Tunnel_WallRight", folder, tx0, tx1, cy + inner, cy + inner + 20, 0, clearance)
    block("Tunnel_Ceiling", folder, tx0, tx1, cy - inner - 20, cy + inner + 20, clearance, clearance + 30)

    # Doorway: 1 m wide, 2.1 m tall, in a 3 m wall.
    dx, door_half, door_h, wall_h = 2400, 50, 210, 300
    block("Door_WallLeft", folder, dx, dx + 20, cy - 500, cy - door_half, 0, wall_h)
    block("Door_WallRight", folder, dx, dx + 20, cy + door_half, cy + 500, 0, wall_h)
    block("Door_Lintel", folder, dx, dx + 20, cy - door_half, cy + door_half, door_h, wall_h)

    # 1 m corridor behind the door.
    block("Corridor_WallLeft", folder, dx + 20, dx + 620, cy - door_half - 20, cy - door_half, 0, 250)
    block("Corridor_WallRight", folder, dx + 20, dx + 620, cy + door_half, cy + door_half + 20, 0, 250)


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
    build_sprint_lane()
    build_jump_lane()
    build_ramp_lane()
    build_ledge_lane()
    build_crouch_and_door_lane()

    levels.save_current_level()
    log(f"saved {MAP_PATH} with {len(actors.get_all_level_actors())} actors")


main()
