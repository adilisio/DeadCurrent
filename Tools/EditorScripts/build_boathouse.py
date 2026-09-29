"""Build /Game/Maps/Lvl_Boathouse, the first-playable greybox scenario.

Regenerates the map from scratch on every run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Layout (X is out the door, Z is up, units are cm; the lake is -Y):
  Boathouse  X 0..720, Y -320..320     wake, inspect, pistol on the workbench, door out; west window
  Path       X 720..1800               shoreline walk to the scavenger
  Scavenger  X 2000..2700, Y -350..350 patrols the path; loot after death; relay rig + coil at his camp
  Cover      Y 550..650                wall so Mara is out of the scavenger's sight
  Mara       X 3100, Y 1300            Shore Watch quest giver; lookout crate reacts to the outcome
  West shore X -3200..-200             Exploration Loop POI: the Wrecked Survey Launch, bow on the
                                       stones at X -1500, stern in live water; not on any route
"""
import math
import unreal

MAP_PATH = "/Game/Maps/Lvl_Boathouse"
CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
MAT_FLOOR = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray"
MAT_BLOCK = "/Game/LevelPrototyping/Materials/MI_PrototypeGrid_TopDark"
MAT_INTERACTABLE = "/Game/LevelPrototyping/Materials/MI_DefaultColorway"
MAT_FLAT = "/Game/LevelPrototyping/Materials/M_FlatCol"
ENV_MATERIALS = "/Game/Environment/Materials"
# Our own unlit translucent glow (parameter "Color": rgb = emissive, a = opacity). The prototype's M_SimpleGlow
# multiplies by particle color, which is black on a static mesh, so it can't be used here.
MAT_GLOW = ENV_MATERIALS + "/M_DC_Glow"

ITEM_AMMO = "/Game/Items/DA_Item_Ammo9mm"
ITEM_DRESSING = "/Game/Items/DA_Item_FieldDressing"
ITEM_WIRING = "/Game/Items/DA_Item_SalvagedWiring"
ITEM_CHART = "/Game/Items/DA_Item_SurveyChart"

# Exploration Loop: the Wrecked Survey Launch. Ids are saved; never rename them once shipped.
WRECK_LOCATION = "shore.survey_launch"
WRECK_LOG_READ = "wreck.log_read"            # read the survey log (opens a line with Mara)
WRECK_BATTERY_SEEN = "wreck.battery_seen"    # inspected the battery bank (offers "Pull the leads")
WRECK_POWER_CUT = "wreck.power_cut"          # pulled the leads: the live water and sparks stop
SHORE_RELAY_INSPECTED = "shore.relay_inspected"  # Shore Watch clue; the beacon reacts to it

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


def rotate(v, pitch=0.0, yaw=0.0, roll=0.0):
    """Rotate v by an Unreal rotator (same matrix as FRotationMatrix)."""
    p, y, r = math.radians(pitch), math.radians(yaw), math.radians(roll)
    sp, cp, sy, cy, sr, cr = math.sin(p), math.cos(p), math.sin(y), math.cos(y), math.sin(r), math.cos(r)
    ax = (cp * cy, cp * sy, sp)
    ay = (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp)
    az = (-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp)
    return unreal.Vector(v.x * ax[0] + v.y * ay[0] + v.z * az[0],
                         v.x * ax[1] + v.y * ay[1] + v.z * az[1],
                         v.x * ax[2] + v.y * ay[2] + v.z * az[2])


def box_rot(label, folder, center, size, rot=(0.0, 0.0, 0.0), material=None, actor_class=unreal.StaticMeshActor):
    """Like box(), with a full (pitch, yaw, roll) rotation about the box center."""
    scale = unreal.Vector(size[0] / CUBE_SIZE.x, size[1] / CUBE_SIZE.y, size[2] / CUBE_SIZE.z)
    scaled_center = unreal.Vector(CUBE_CENTER.x * scale.x, CUBE_CENTER.y * scale.y, CUBE_CENTER.z * scale.z)
    offset = rotate(scaled_center, *rot)
    location = unreal.Vector(center[0], center[1], center[2]) - offset
    actor = actors.spawn_actor_from_class(actor_class, location, unreal.Rotator(pitch=rot[0], yaw=rot[1], roll=rot[2]))
    actor.set_actor_scale3d(scale)
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    mesh_comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    mesh_comp.set_static_mesh(cube_mesh)
    mesh_comp.set_material(0, material or block_mat)
    return actor


class Frame:
    """A local coordinate frame (origin + rotation) for building a tilted object out of boxes."""

    def __init__(self, origin, pitch=0.0, yaw=0.0, roll=0.0):
        self.origin = origin
        self.rot = (pitch, yaw, roll)

    def world(self, local):
        v = rotate(unreal.Vector(*local), *self.rot)
        return (self.origin[0] + v.x, self.origin[1] + v.y, self.origin[2] + v.z)

    def part(self, label, folder, local_center, size, material=None, actor_class=unreal.StaticMeshActor):
        return box_rot(label, folder, self.world(local_center), size, self.rot, material, actor_class)


def ensure_glow_material():
    """Create (once) M_DC_Glow: unlit, translucent, emissive = Color.rgb, opacity = Color.a."""
    if unreal.EditorAssetLibrary.does_asset_exist(MAT_GLOW):
        return
    if not unreal.EditorAssetLibrary.does_directory_exist(ENV_MATERIALS):
        unreal.EditorAssetLibrary.make_directory(ENV_MATERIALS)
    mel = unreal.MaterialEditingLibrary
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_DC_Glow", ENV_MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Could not create {MAT_GLOW}")
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("two_sided", True)
    color = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -300, 0)
    color.set_editor_property("parameter_name", "Color")
    color.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(color, "A", unreal.MaterialProperty.MP_OPACITY)
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {MAT_GLOW}")


def material_instance(name, parent_path, vectors):
    """Create or update a material instance in ENV_MATERIALS with the given vector parameters."""
    path = f"{ENV_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        if not unreal.EditorAssetLibrary.does_directory_exist(ENV_MATERIALS):
            unreal.EditorAssetLibrary.make_directory(ENV_MATERIALS)
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, ENV_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    unreal.MaterialEditingLibrary.set_material_instance_parent(mi, unreal.load_asset(parent_path))
    for param, rgba in vectors.items():
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mi, param, unreal.LinearColor(*rgba))
    unreal.MaterialEditingLibrary.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    return mi


def cond(type_name, id=None, stage=None, negate=False, quantity=1):
    c = unreal.DCGameplayCondition()
    c.set_editor_property("type", getattr(unreal.DCConditionType, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("negate", negate)
    c.set_editor_property("quantity", quantity)
    return c


def cons(type_name, id=None):
    c = unreal.DCGameplayConsequence()
    c.set_editor_property("type", getattr(unreal.DCConsequenceType, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    return c


def variant(description, conditions=(), consequences=(), action=None):
    """One conditional reading of an inspectable (first match wins). action renames the prompt verb."""
    v = unreal.DCInspectVariant()
    v.set_editor_property("description", unreal.Text(description))
    v.set_editor_property("conditions", list(conditions))
    v.set_editor_property("consequences", list(consequences))
    if action:
        v.set_editor_property("action", unreal.Text(action))
    return v


def setup_inspectable(actor, display_name, description, variants=(), action=None, duration=None):
    actor.set_editor_property("display_name", unreal.Text(display_name))
    actor.set_editor_property("description", unreal.Text(description))
    if variants:
        actor.set_editor_property("variants", list(variants))
    if action:
        actor.set_editor_property("action", unreal.Text(action))
    if duration:
        actor.set_editor_property("description_duration", duration)
    return actor


def inspectable(label, folder, center, size, display_name, description, variants=(), action=None, duration=None):
    actor = box(label, folder, center, size, material=interactable_mat, actor_class=unreal.DCInspectableActor)
    return setup_inspectable(actor, display_name, description, variants, action, duration)


def setup_container(actor, display_name, persistent_id, contents):
    """contents: [(item asset path, quantity)], the container's starting stacks, oldest first."""
    actor.set_editor_property("display_name", unreal.Text(display_name))
    stacks = []
    for item_path, quantity in contents:
        stack = unreal.DCItemStack()
        stack.set_editor_property("item", unreal.load_asset(item_path))
        stack.set_editor_property("quantity", quantity)
        stacks.append(stack)
    actor.get_component_by_class(unreal.DCInventoryComponent).set_editor_property("stacks", stacks)
    set_persistent_id(actor, persistent_id)
    return actor


def flicker_light(label, folder, location, color, candelas, radius, glow_cm=0.0, glow_material=None,
                  conditions=(), min_brightness=0.25, dropout=0.12, interval=(0.05, 0.6)):
    """Cosmetic flickering point light, with an optional glowing cube of glow_cm."""
    actor = actors.spawn_actor_from_class(unreal.DCFlickerLight, unreal.Vector(*location), unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    light = actor.get_editor_property("light")
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", candelas)
    light.set_editor_property("light_color", unreal.Color(r=color[0], g=color[1], b=color[2], a=255))
    light.set_editor_property("attenuation_radius", radius)
    if glow_cm > 0.0:
        glow = actor.get_editor_property("glow")
        scale = unreal.Vector(glow_cm / CUBE_SIZE.x, glow_cm / CUBE_SIZE.y, glow_cm / CUBE_SIZE.z)
        glow.set_static_mesh(cube_mesh)
        glow.set_material(0, glow_material)
        glow.set_editor_property("relative_scale3d", scale)
        glow.set_editor_property("relative_location", unreal.Vector(
            -CUBE_CENTER.x * scale.x, -CUBE_CENTER.y * scale.y, -CUBE_CENTER.z * scale.z))
    actor.set_editor_property("min_brightness", min_brightness)
    actor.set_editor_property("dropout_chance", dropout)
    actor.set_editor_property("min_interval", interval[0])
    actor.set_editor_property("max_interval", interval[1])
    if conditions:
        actor.set_editor_property("active_conditions", list(conditions))
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

    # Back (west) wall with a window: the survey launch's mast and lamp show through it (Exploration Loop).
    block("Wall_Back_North", folder, 0, 20, -110, 320, 0, wall_h)
    block("Wall_Back_South", folder, 0, 20, -320, -250, 0, wall_h)
    block("Wall_Back_Sill", folder, 0, 20, -250, -110, 0, 100)
    block("Wall_Back_Head", folder, 0, 20, -250, -110, 230, wall_h)
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
    inspectable("WestWindow", folder, (34, -180, 104), (28, 130, 8), "West window",
                "West along the shore, a mast leans out over the water. A light at the top of it comes and goes. "
                "Nothing out there should still have power.",
                variants=[
                    variant("The survey launch's mast, leaning over the water. The lamp at the top is still flickering, "
                            "with nothing left to power it.",
                            [cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
                    variant("The survey launch's mast, leaning out over the water. Its lamp comes and goes.",
                            [cond("LOCATION_DISCOVERED", id=WRECK_LOCATION)]),
                ], action="Look out", duration=6.0)


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

    # Shore Watch: the relay Mara wants silenced. Inspecting it while it is live is a clue
    # (shore.relay_inspected) that opens an extra line with Mara.
    empty = "The relay housing sits open and empty. Without the coil it is a box of cold wire."
    inspectable("RelayRig", folder, (2480, -220, 20), (50, 40, 40), "Relay rig",
                "The relay still hums. You would rather not listen to it again.",
                variants=[
                    variant(empty, [cond("HAS_ITEM", id="radio_coil")]),
                    variant(empty, [cond("WORLD_FLAG", id="shore.relay_recovered")]),
                    variant("The battery leads have been kicked loose in the struggle. The relay is silent, its coil gone cold.",
                            [cond("ACTOR_DEAD", id="boat.scavenger")]),
                    variant("A Maritime Authority relay housing, wired to a truck battery beside a torn pack. "
                            "The coil hums against your fingers, and under the hum, almost, words.",
                            [cond("WORLD_FLAG", id="shore.relay_inspected", negate=True)],
                            [cons("SET_WORLD_FLAG", id="shore.relay_inspected")]),
                    variant("The coil is seated on the right pins, taped the way a yard electrician tapes, not a "
                            "scavenger in a hurry. Somebody who knew this housing put it back in service. "
                            "The hum is the set running, not a short. You can hear it in the pinout.",
                            [cond("HAS_PERK", id="Perk.RelayEar"), cond("WORLD_FLAG", id="shore.relay_inspected")]),
                ])
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
                "Someone has been watching the path from here. F5 saves. F9 loads.",
                variants=[
                    variant("Mara's notebook lies open on the crate: the same six words in pencil, over and over. You don't read them twice.",
                            [cond("WORLD_FLAG", id="shore.relay_recovered")]),
                    variant("A pencil tally on the lid: one walker, crossed out. Under it: RELAY COLD.",
                            [cond("WORLD_FLAG", id="shore.path_cleared")]),
                ])


def build_west_shore():
    """Ground for the Exploration Loop POI, west of (behind) the boathouse."""
    folder = "WestShore"
    block("Floor_West", folder, -3200, -200, -600, 1800, -50, 0, material=floor_mat)
    # The curb stops where the launch's hull crosses the shoreline.
    block("ShoreCurb_West_A", folder, -3200, -1720, -600, -580, 0, 18)
    block("ShoreCurb_West_B", folder, -1280, -200, -600, -580, 0, 18)
    block("Water_West", folder, -3200, -200, -2400, -600, -80, -10)
    # Keep the player on the map: a bluff to the west and north, rocks around the far water.
    block("Bluff_West", folder, -3240, -3200, -2440, 1840, -80, 420)
    block("Bluff_North", folder, -3200, -200, 1800, 1840, -50, 300)
    block("Breakwater_South", folder, -3200, -200, -2440, -2400, -80, 140)
    block("Breakwater_East", folder, -220, -200, -2400, -1400, -80, 140)
    # Beach stones.
    block("Boulder_A", folder, -2250, -2080, -520, -400, 0, 70)
    block("Boulder_B", folder, -880, -760, -560, -470, 0, 45)
    block("Boulder_C", folder, -2600, -2450, 200, 330, 0, 90)


def build_survey_launch():
    """Exploration Loop POI: a Maritime Authority survey launch driven bow-first onto the stones.

    Notice: a leaning mast with a flickering lamp above the boathouse roofline (and through its west window).
    Discover: walking up to it (location shore.survey_launch). Read: name board, cut life jackets, depth
    sounder, gutted breaker panel, survey log, emergency beacon, dead fish. Danger: live water round the
    stern until the battery leads are pulled. Loot: the survey locker (obvious) and the kit in the tender
    tied off the stern, out in the live water (less obvious; the log mentions it).
    """
    folder = "SurveyLaunch"
    hull_mat = material_instance("MI_DC_WreckHull", MAT_FLAT, {"Base Color": (0.13, 0.19, 0.23, 1.0)})
    trim_mat = material_instance("MI_DC_WreckRust", MAT_FLAT, {"Base Color": (0.36, 0.16, 0.07, 1.0)})
    ensure_glow_material()
    amber = material_instance("MI_DC_GlowAmber", MAT_GLOW, {"Color": (9.0, 4.0, 0.9, 1.0)})
    spark = material_instance("MI_DC_GlowSpark", MAT_GLOW, {"Color": (2.0, 4.5, 10.0, 1.0)})
    live_water = material_instance("MI_DC_LiveWater", MAT_GLOW, {"Color": (0.2, 1.4, 4.0, 0.5)})
    water_mat = material_instance("MI_DC_Water", MAT_FLAT, {"Base Color": (0.02, 0.07, 0.09, 1.0)})
    fish_mat = material_instance("MI_DC_DeadFish", MAT_FLAT, {"Base Color": (0.78, 0.8, 0.72, 1.0)})
    live = [cond("WORLD_FLAG", id=WRECK_POWER_CUT, negate=True)]

    # Hull frame: local X = beam (+X is the east side, toward the boathouse), local Y = length (+Y is the
    # bow, pointing inland), Z = up from the deck. Bow raised on the stones, listing toward the east side.
    hull = Frame((-1500.0, -760.0, 95.0), pitch=-4.0, roll=-3.0)
    hull.part("Hull", folder, (0, 0, -80), (360, 1000, 160), material=hull_mat)
    hull.part("Bow", folder, (0, 540, -70), (220, 80, 180), material=hull_mat)
    hull.part("Rail_West", folder, (-174, 0, 30), (12, 1000, 60), material=trim_mat)
    # Gap in the east rail where the plank comes aboard (local Y 180..320).
    hull.part("Rail_East_Aft", folder, (174, -160, 30), (12, 680, 60), material=trim_mat)
    hull.part("Rail_East_Bow", folder, (174, 410, 30), (12, 180, 60), material=trim_mat)
    hull.part("Rail_Bow", folder, (0, 494, 30), (360, 12, 60), material=trim_mat)
    hull.part("Transom", folder, (0, -494, 30), (360, 12, 60), material=trim_mat)

    # Boarding plank from the beach up to the gap in the east rail.
    top = hull.world((180, 250, 0))
    run = 320.0
    rise = top[2]
    plank_pitch = -math.degrees(math.atan2(rise, run))
    box_rot("Plank", folder, (top[0] + run / 2, top[1], rise / 2 - 2),
            (math.hypot(run, rise), 90, 6), (plank_pitch, 0.0, 0.0), material=trim_mat)

    # Wheelhouse: local X -162..162, Y -180..132, walls 240 high, doorways fore and aft on the east side.
    wall_h, door_h = 240, 215
    hull.part("Wheelhouse_Fore_W", folder, (-65, 126, wall_h / 2), (170, 12, wall_h), material=hull_mat)
    hull.part("Wheelhouse_Fore_E", folder, (146, 126, wall_h / 2), (32, 12, wall_h), material=hull_mat)
    hull.part("Wheelhouse_Fore_Lintel", folder, (75, 126, (door_h + wall_h) / 2), (110, 12, wall_h - door_h), material=hull_mat)
    hull.part("Wheelhouse_Aft_W", folder, (-65, -174, wall_h / 2), (170, 12, wall_h), material=hull_mat)
    hull.part("Wheelhouse_Aft_E", folder, (146, -174, wall_h / 2), (32, 12, wall_h), material=hull_mat)
    hull.part("Wheelhouse_Aft_Lintel", folder, (75, -174, (door_h + wall_h) / 2), (110, 12, wall_h - door_h), material=hull_mat)
    hull.part("Wheelhouse_West", folder, (-156, -24, wall_h / 2), (12, 312, wall_h), material=hull_mat)
    # East wall has a long window so the inside is lit and visible from the plank.
    hull.part("Wheelhouse_East_Low", folder, (156, -24, 55), (12, 312, 110), material=hull_mat)
    hull.part("Wheelhouse_East_High", folder, (156, -24, 215), (12, 312, 50), material=hull_mat)
    hull.part("Wheelhouse_East_PostAft", folder, (156, -159, 150), (12, 42, 80), material=hull_mat)
    hull.part("Wheelhouse_East_PostFore", folder, (156, 111, 150), (12, 42, 80), material=hull_mat)
    hull.part("Wheelhouse_Roof", folder, (0, -24, wall_h + 10), (336, 336, 20), material=trim_mat)
    hull.part("Mast", folder, (0, -24, wall_h + 20 + 450), (16, 16, 900), material=trim_mat)
    hull.part("Mast_Yard", folder, (0, -24, wall_h + 20 + 780), (160, 10, 10), material=trim_mat)
    lamp = hull.world((0, -24, wall_h + 20 + 915))
    flicker_light("MastLamp", folder, lamp, (255, 170, 80), 60.0, 1400.0, glow_cm=60.0, glow_material=amber,
                  min_brightness=0.35, dropout=0.18, interval=(0.06, 0.9))

    # Inside the wheelhouse.
    hull.part("Console", folder, (-68, 94, 47), (150, 48, 94), material=trim_mat)
    survey_log = hull.part("SurveyLog", folder, (-100, 94, 98), (36, 26, 6), material=interactable_mat,
                    actor_class=unreal.DCInspectableActor)
    log_text = ("Survey log, last entry, in pencil: \"Sounder has the pattern again, same mark off the point. "
                "Radio has it too, on a channel with no station. Cut every breaker on the boat. Still feel it "
                "through my boots. Running her onto the stones. Kit's in the tender, tied off the stern. "
                "We walk from here.\" The rest of the book is blank.")
    setup_inspectable(survey_log, "Survey log", log_text, variants=[
        variant(log_text, [cond("WORLD_FLAG", id=WRECK_LOG_READ, negate=True)], [cons("SET_WORLD_FLAG", id=WRECK_LOG_READ)]),
    ], action="Read", duration=14.0)
    sounder = hull.part("DepthSounder", folder, (-28, 100, 115), (40, 30, 40), material=interactable_mat,
                        actor_class=unreal.DCInspectableActor)
    chart_item = cond("HAS_ITEM", id="survey_chart")
    setup_inspectable(sounder, "Depth sounder",
                      "A depth sounder, its paper roll still threaded. The bottom trace runs flat, then breaks into "
                      "tight, even spikes: too regular for rock, too regular for fish. Someone has circled them in "
                      "grease pencil and written AGAIN.",
                      variants=[
                          variant("You hold the sounder chart up to the roll. The spikes match the chart's margin ticks, "
                                  "and the ticks are numbered like a bearing, not a depth. The last tick is marked with a "
                                  "cross and the words NOT A SHOAL. Same spacing on both. Whatever they were drawing, they "
                                  "already knew it was not the bottom.",
                                  [cond("HAS_PERK", id="Perk.SchematicEye"), chart_item]),
                          variant("With the chart in hand the spikes line up with marks along its edge. Same spacing, "
                                  "same count. The trace and the paper are the same pattern, copied down. It still does "
                                  "not say what the pattern is.",
                                  [cond("SKILL_AT_LEAST", id="Skill.Engineering", quantity=2), chart_item]),
                      ], duration=10.0)
    panel = hull.part("BreakerPanel", folder, (-147, -60, 140), (6, 70, 90), material=interactable_mat,
                      actor_class=unreal.DCInspectableActor)
    engineering = ("The cuts are clean, and they start at the shore-power breaker, not the mast. "
                   "Whoever did this knew the panel. The mast lamp is not on these lugs. "
                   "Cutting them could not be what is still flickering up there.")
    setup_inspectable(panel, "Breaker panel",
                      "The breaker panel has been gutted, but not by scavengers. The cables are cut clean and the "
                      "ends taped off, one by one. Careful work, done fast.",
                      variants=[
                          variant(engineering + " The log says cutting them did not stop what they felt through their boots. "
                                  "That matches the panel: this bank was never feeding the mast.",
                                  [cond("SKILL_AT_LEAST", id="Skill.Engineering", quantity=2),
                                   cond("WORLD_FLAG", id=WRECK_LOG_READ)]),
                          variant(engineering, [cond("SKILL_AT_LEAST", id="Skill.Engineering", quantity=2)]),
                          variant("Every breaker thrown, every cable cut and taped. The crew did this themselves. "
                                  "The log says it didn't help.", [cond("WORLD_FLAG", id=WRECK_LOG_READ)]),
                      ], duration=8.0)
    locker = hull.part("SurveyLocker", folder, (-115, -140, 55), (60, 44, 110), material=interactable_mat,
                       actor_class=unreal.DCLootContainer)
    setup_container(locker, "Survey locker", "boat.wreck_locker",
                    [(ITEM_AMMO, 12), (ITEM_WIRING, 3), (ITEM_DRESSING, 1)])

    # Aft deck: the battery bank whose leads run into the lake, and the emergency beacon on the transom.
    battery = hull.part("BatteryBank", folder, (-105, -340, 30), (90, 60, 60), material=interactable_mat,
                        actor_class=unreal.DCInspectableActor)
    setup_inspectable(battery, "Battery bank", "A bank of old marine cells in a split box.", variants=[
        variant("The leads hang loose where you tore them free. The water around the stern has gone flat and quiet.",
                [cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
        variant("You brace a boot against the box and wrench the leads free. A crack, a smell of hot metal, "
                "and the shimmer around the stern dies.",
                [cond("WORLD_FLAG", id=WRECK_BATTERY_SEEN)], [cons("SET_WORLD_FLAG", id=WRECK_POWER_CUT)],
                action="Pull the leads"),
        variant("A bank of old marine cells in a split box. The only cables on this boat nobody cut run from it, "
                "over the transom and into the lake. Sixty years on, they are still live. "
                "The corroded leads look like they'd tear free with a hard pull.",
                [], [cons("SET_WORLD_FLAG", id=WRECK_BATTERY_SEEN)]),
    ], duration=8.0)
    hull.part("BatteryLead", folder, (-105, -420, 45), (6, 150, 6), material=trim_mat)
    hull.part("BatteryLead_Over", folder, (-105, -505, -30), (6, 6, 140), material=trim_mat)
    beacon = hull.part("Beacon", folder, (110, -494, 78), (22, 22, 36), material=interactable_mat,
                       actor_class=unreal.DCInspectableActor)
    setup_inspectable(beacon, "Emergency beacon",
                      "An emergency beacon, its switch taped down to TRANSMIT. The battery compartment is solid "
                      "white crust. It has been dead for decades.",
                      variants=[
                          variant("The beacon's switch is still taped down to TRANSMIT. You pulled the last live leads "
                                  "on this boat. Up on the mast, the lamp is still flickering.",
                                  [cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
                          variant("An emergency beacon, its switch taped down to TRANSMIT, its battery dead for decades. "
                                  "Put your ear to the casing anyway: a faint hum, the same one the scavenger's relay makes.",
                                  [cond("WORLD_FLAG", id=SHORE_RELAY_INSPECTED)]),
                      ], duration=8.0)
    name_board = hull.part("NameBoard", folder, (0, 582, -22), (140, 6, 40), material=interactable_mat,
                           actor_class=unreal.DCInspectableActor)
    setup_inspectable(name_board, "Name board",
                      "GREAT LAKES MARITIME AUTHORITY - SURVEY LAUNCH, and under it a name, mostly flaked away: T_RN. "
                      "The keel is split where she hit the stones. She came in bow-first and fast. "
                      "Someone put her here on purpose.", duration=8.0)

    # Signs the crew left in a hurry, on the beach toward the boathouse.
    inspectable("LifeJackets", folder, (-1080, -380, 6), (64, 44, 12), "Life jackets",
                "Two life jackets on the stones, still buckled. The straps were cut through, not unclipped. "
                "Whoever wore them was in a hurry, and walked inland.",
                variants=[
                    variant("The straps are cut, not unclipped, and the two jackets lie in a line pointing off the "
                            "stones toward the treeline, not back along the beach. They left inland, and they left "
                            "together. The ground just under the treeline is the way they went.",
                            [cond("SKILL_AT_LEAST", id="Skill.Survival", quantity=2)]),
                ], duration=8.0)

    # The water itself: dark, always there, no collision. Pulling the leads must never remove it.
    surface = box("WaterSurface", folder, (-1500, -1390, 3), (1040, 780, 8), material=water_mat)
    surface.get_component_by_class(unreal.StaticMeshComponent).set_collision_enabled(
        unreal.CollisionEnabled.NO_COLLISION)
    # The electricity: a bright glow layer over the water that is also the damage volume. It (and the
    # sparks) go out with the power; the water stays.
    hazard = box("LiveWater", folder, (-1500, -1390, 8), (1040, 780, 20), material=live_water,
                 actor_class=unreal.DCDamageVolume)
    hazard.set_editor_property("display_name", unreal.Text("The water is live."))
    hazard.set_editor_property("damage_per_second", 20.0)
    hazard.set_editor_property("active_conditions", live)
    for name, (sx, sy), interval in [("Sparks_A", (-1230, -1470), (0.03, 0.25)), ("Sparks_B", (-1760, -1600), (0.03, 0.3)),
                                     ("Sparks_C", (-1120, -1180), (0.04, 0.35)), ("Sparks_D", (-1900, -1250), (0.03, 0.4)),
                                     ("Sparks_E", (-1700, -1730), (0.05, 0.3)), ("Sparks_F", (-1300, -1650), (0.03, 0.25))]:
        flicker_light(name, folder, (sx, sy, 24), (160, 200, 255), 40.0, 700.0, glow_cm=16.0,
                      glow_material=spark, conditions=live, min_brightness=0.0, dropout=0.4, interval=interval)
    # Pale dead fish ring the edge, lying on the surface where they can be seen.
    for index, (fx, fy) in enumerate([(-960, -990), (-960, -1250), (-960, -1520), (-1200, -1795), (-1500, -1795),
                                      (-1800, -1795), (-2040, -1560), (-2040, -1280), (-2040, -1010)]):
        block(f"Fish_{index}", folder, fx - 22, fx + 22, fy - 8, fy + 8, 6, 12, material=fish_mat)
    # A chalked warning on the beach at the edge of the water (the danger should not be a pure surprise).
    inspectable("ChalkWarning", folder, (-1040, -930, 30), (8, 60, 60), "Chalk warning",
                "Chalked on a plank stuck upright in the stones, in a hurried hand: KEEP OUT OF THE WATER. "
                "IT'S STILL ON. Under it, smaller: cut the breakers, it doesn't matter.",
                variants=[
                    variant("The chalk is fresh enough to rub off, and the plank was driven in from the water side. "
                            "Whoever wrote it was standing in the shallows, looking back at the boat, not warning "
                            "people away from the shore. The water beyond it is only water now.",
                            [cond("ATTRIBUTE_AT_LEAST", id="Attribute.Fieldcraft", quantity=2),
                             cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
                    variant("The chalk is fresh enough to rub off, and the plank was driven in from the water side. "
                            "Whoever wrote it was standing in the shallows, looking back at the boat, not warning "
                            "people away from the shore.",
                            [cond("ATTRIBUTE_AT_LEAST", id="Attribute.Fieldcraft", quantity=2)]),
                    variant("The chalk warning is still on the plank. The water beyond it is only water now.",
                            [cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
                ], duration=8.0)
    inspectable("DeadFish", folder, (-950, -1000, 12), (40, 14, 10), "Dead fish",
                "Dead fish, belly-up, in a ring around the stern, every one the same distance out. "
                "Inside the ring the water has a faint, crawling shimmer.",
                variants=[
                    variant("These fish did not drift here. The eyes are all burst the same way, and nothing has fed "
                            "on them. One shock, all at once, standing off the hull. Not a tide. "
                            "The shimmer has gone out of the water inside the ring.",
                            [cond("HAS_PERK", id="Perk.PulseRead"), cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
                    variant("These fish did not drift here. The eyes are all burst the same way, and nothing has fed "
                            "on them. One shock, all at once, standing off the hull. Not a tide.",
                            [cond("HAS_PERK", id="Perk.PulseRead")]),
                    variant("The ring of dead fish is still there. The shimmer has gone out of the water inside it.",
                            [cond("WORLD_FLAG", id=WRECK_POWER_CUT)]),
                ], duration=8.0)

    # The crew's kit, in the tender tied off the stern: out in the live water.
    tender = box("Tender", folder, (-1500, -1650, 12), (120, 240, 45), material=interactable_mat,
                 actor_class=unreal.DCLootContainer)
    setup_container(tender, "Tender", "boat.wreck_tender", [(ITEM_CHART, 1), (ITEM_DRESSING, 2), (ITEM_AMMO, 18)])
    stern = hull.world((0, -500, -20))
    rope_run = stern[1] - (-1530.0)
    box_rot("TenderLine", folder, (-1500, (stern[1] - 1530.0) / 2, (stern[2] + 20) / 2),
            (4, rope_run, 4), (0.0, 0.0, -math.degrees(math.atan2(stern[2] - 20, rope_run))), material=trim_mat)

    # Discovery: walking up to the wreck from the boathouse, the beach, or the water.
    discovery = actors.spawn_actor_from_class(unreal.DCLocationVolume, unreal.Vector(-1650.0, -700.0, 300.0))
    discovery.set_actor_label("Location_SurveyLaunch")
    discovery.set_folder_path(folder)
    discovery.set_editor_property("location_id", WRECK_LOCATION)
    discovery.set_editor_property("display_name", unreal.Text("Wrecked Survey Launch"))
    discovery.get_editor_property("bounds").set_box_extent(unreal.Vector(950.0, 1150.0, 500.0))
    log(f"survey launch: deck gap at {top}, lamp at {lamp}")


def build_nav():
    folder = "Ground"
    vol = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(1700.0, 400.0, 0.0))
    vol.set_actor_label("NavBounds")
    vol.set_folder_path(folder)
    vol.set_actor_scale3d(unreal.Vector(28.0, 18.0, 8.0))
    west = actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(-1700.0, -300.0, 0.0))
    west.set_actor_label("NavBounds_West")
    west.set_folder_path(folder)
    west.set_actor_scale3d(unreal.Vector(15.0, 21.0, 8.0))

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
    build_west_shore()
    build_survey_launch()
    build_nav()

    if not levels.save_current_level():
        raise RuntimeError(f"Could not save {MAP_PATH} (is the file read-only?)")
    log(f"saved {MAP_PATH} with {len(actors.get_all_level_actors())} actors")


main()
