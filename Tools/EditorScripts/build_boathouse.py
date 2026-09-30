"""Build /Game/Maps/Lvl_Boathouse, the first-playable scenario.

Regenerates the persistent map from scratch on every run. The hand-authored streaming
sublevel /Game/Maps/Lvl_Boathouse_Art is re-linked and never edited here: this script
does not destroy its actors and does not save it, except for the one-time create of an
empty level with a sentinel tagged ArtLayerSentinel.

Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Layout (X is out the door, Z is up, units are cm; the lake is -Y):
  Boathouse  X 0..720, Y -320..320     wake, inspect, pistol on the workbench, door out; west window
  Path       X 720..1800               shoreline walk to the scavenger
  Scavenger  X 2300..3000, Y -60..350 patrols the path; loot after death; relay rig + coil at his camp, on the beach
                                       side of a windbreak (Y -155..-130) his loop stays north of
  Cover      Y 550..650                wall so Mara is out of the scavenger's sight
  Mara       X 3100, Y 1300            Shore Watch quest giver; lookout crate reacts to the outcome
  West shore X -3200..-200             Exploration Loop POI: the Wrecked Survey Launch, bow on the
                                       stones at X -1500, stern in live water; not on any route
"""
import math
import unreal

MAP_PATH = "/Game/Maps/Lvl_Boathouse"
ART_MAP = "/Game/Maps/Lvl_Boathouse_Art"
ART_SENTINEL = "ArtLayerSentinel"
# Created by import_art.py. Resolved here so a rebuild without that script fails before it saves a map.
REQUIRED_SURFACES = [
    "/Game/Environment/Materials/MI_DC_CoastRock",
    "/Game/Environment/Materials/MI_DC_LandRock",
    "/Game/Environment/Materials/MI_DC_CoastSand",
    "/Game/Environment/Materials/MI_DC_Mud",
    "/Game/Environment/Materials/MI_DC_Concrete",
    "/Game/Environment/Materials/MI_DC_Plaster",
    "/Game/Environment/Materials/MI_DC_Steel",
    "/Game/Environment/Materials/MI_DC_RustPaint",
    "/Game/Environment/Materials/MI_DC_Gravel",
    "/Game/Environment/Materials/MI_DC_TernU1",
    "/Game/Environment/Materials/MI_DC_TernU2",
    "/Game/Environment/Materials/MI_DC_OpenLake",
    "/Game/Environment/Materials/MI_DC_Water",
]
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


def box(label, folder, center, size, pitch=0.0, material=None, actor_class=unreal.StaticMeshActor, hidden=False):
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
    chosen = material or block_mat
    for slot in range(mesh_comp.get_num_materials()):
        mesh_comp.set_material(slot, chosen)
    if hidden:
        mesh_comp.set_visibility(False)
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


def box_rot(label, folder, center, size, rot=(0.0, 0.0, 0.0), material=None, actor_class=unreal.StaticMeshActor, hidden=False):
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
    chosen = material or block_mat
    for slot in range(mesh_comp.get_num_materials()):
        mesh_comp.set_material(slot, chosen)
    if hidden:
        mesh_comp.set_visibility(False)
    return actor


class Frame:
    """A local coordinate frame (origin + rotation) for building a tilted object out of boxes."""

    def __init__(self, origin, pitch=0.0, yaw=0.0, roll=0.0):
        self.origin = origin
        self.rot = (pitch, yaw, roll)

    def world(self, local):
        v = rotate(unreal.Vector(*local), *self.rot)
        return (self.origin[0] + v.x, self.origin[1] + v.y, self.origin[2] + v.z)

    def part(self, label, folder, local_center, size, material=None, actor_class=unreal.StaticMeshActor, hidden=False):
        return box_rot(label, folder, self.world(local_center), size, self.rot, material, actor_class, hidden)


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


def inspectable(label, folder, center, size, display_name, description, variants=(), action=None, duration=None,
                material=None):
    actor = box(label, folder, center, size, material=material or interactable_mat, actor_class=unreal.DCInspectableActor)
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


def audio_asset(path):
    """A sound or attenuation asset from import_audio.py. Missing means that script did not run: fail before saving."""
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing {path}. Run import_audio.py first.")
    return asset


def conditional_audio(label, folder, location, sound_path, volume, conditions=(), attenuation=None, once=False):
    """Cosmetic sound that follows the rule language. Sets no flag. attenuation=None is a 2D sound."""
    actor = actors.spawn_actor_from_class(unreal.DCConditionalAudio, unreal.Vector(*location), unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    actor.set_editor_property("sound", audio_asset(sound_path))
    if attenuation:
        actor.set_editor_property("attenuation", audio_asset(f"/Game/Audio/{attenuation}"))
    actor.set_editor_property("volume_multiplier", volume)
    actor.set_editor_property("mode", unreal.DCConditionalAudioMode.ONCE_WHEN_TRUE if once
                              else unreal.DCConditionalAudioMode.WHILE_TRUE)
    if conditions:
        actor.set_editor_property("conditions", list(conditions))
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


def door(label, folder, hinge, width, height, thickness, display_name, persistent_id, material=None):
    actor = actors.spawn_actor_from_class(
        unreal.DCDoor, unreal.Vector(*hinge), unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0))
    actor.set_actor_label(label)
    actor.set_folder_path(folder)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    leaf = actor.get_editor_property("door_mesh")
    leaf.set_static_mesh(cube_mesh)
    leaf.set_material(0, material or interactable_mat)
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


SURVIVAL_MESH = "/Game/Survival_Character/Meshes/SK_Survival_Character"
SURVIVAL_JACKET_SLOT = 7
SURVIVAL_JEANS_SLOT = 8
SURVIVAL_EYE_SLOT = 3
SURVIVAL_SKIN_SLOT = 0
SURVIVAL_HAIR_SLOTS = (4, 5)


def assign_mannequin(actor, *mesh_paths, costume=None, skin=None, hair=None):
    """Point an actor at the first skeletal mesh that loads. costume=("MI_DC_XJacket", "MI_DC_XJeans") tints the
    Survival_Character mesh; it is ignored for a fallback mannequin."""
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
        if costume and mesh_path == SURVIVAL_MESH:
            for slot, name in zip((SURVIVAL_JACKET_SLOT, SURVIVAL_JEANS_SLOT), costume):
                mesh_comp.set_material(slot, surface(name))
            mesh_comp.set_material(SURVIVAL_EYE_SLOT, surface("MI_DC_Eye"))
            if skin:
                mesh_comp.set_material(SURVIVAL_SKIN_SLOT, surface(skin))
            if hair:
                for slot in SURVIVAL_HAIR_SLOTS:
                    mesh_comp.set_material(slot, surface(hair))
    else:
        log(f"{actor.get_actor_label()} no mannequin skeletal mesh found")
    if abp_path:
        abp_class = unreal.EditorAssetLibrary.load_blueprint_class(abp_path)
        mesh_comp.set_animation_mode(unreal.AnimationMode.ANIMATION_BLUEPRINT)
        mesh_comp.set_anim_class(abp_class)
        log(f"{actor.get_actor_label()} anim {abp_path}")


def build_lighting():
    """Overcast cold lake. Has to read with PlayTest's cvars: no volumetric fog, no bloom, no Lumen."""
    folder = "Lighting"
    sun = actors.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 1000),
        unreal.Rotator(pitch=-58.0, yaw=135.0, roll=0.0))
    sun.set_actor_label("Sun")
    sun.set_folder_path(folder)
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("atmosphere_sun_light", True)
    sun_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    # High and cool, not a raking sunset. Auto-exposure still meters the interior.
    # Lower than the first overcast pass. PlayTest has no bloom, so the sky and ground
    # have to sit under the emissives instead of competing with them.
    sun_comp.set_editor_property("intensity", 85.0)
    sun_comp.set_editor_property("light_color", unreal.Color(r=188, g=198, b=210, a=255))
    sun_comp.set_editor_property("temperature", 6800.0)
    sun_comp.set_editor_property("use_temperature", True)

    # PlayTest has no GI and the SkyLight adds nothing to faces the sun does not reach: captures
    # 1629 and 1638 showed ridge ends, the relay, and the lookout wall at RGB 1 to 6. A dim, shadowless
    # fill from the opposite side stands in for sky bounce. It is not an atmosphere sun.
    fill_dir = actors.spawn_actor_from_class(
        unreal.DirectionalLight, unreal.Vector(0, 0, 1100),
        unreal.Rotator(pitch=-28.0, yaw=315.0, roll=0.0))
    fill_dir.set_actor_label("SkyFill")
    fill_dir.set_folder_path(folder)
    fill_dir_comp = fill_dir.get_component_by_class(unreal.DirectionalLightComponent)
    fill_dir_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    fill_dir_comp.set_editor_property("atmosphere_sun_light", False)
    fill_dir_comp.set_editor_property("forward_shading_priority", 0)
    sun_comp.set_editor_property("forward_shading_priority", 1)
    fill_dir_comp.set_editor_property("cast_shadows", False)
    fill_dir_comp.set_editor_property("intensity", 24.0)
    fill_dir_comp.set_editor_property("light_color", unreal.Color(r=150, g=172, b=205, a=255))

    sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1000))
    sky.set_actor_label("SkyLight")
    sky.set_folder_path(folder)
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky_comp.set_editor_property("real_time_capture", True)
    # Sides facing away from the low sun read as black holes at 0.38. PlayTest has no GI or
    # reflections, so this ambient is the only light those faces get.
    sky_comp.set_editor_property("intensity", 0.9)
    sky_comp.set_editor_property("lower_hemisphere_is_black", False)
    sky_comp.set_editor_property("lower_hemisphere_color", unreal.LinearColor(0.11, 0.115, 0.125, 1.0))

    atmo = actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
    atmo.set_actor_label("SkyAtmosphere")
    atmo.set_folder_path(folder)
    atmo_comp = atmo.get_component_by_class(unreal.SkyAtmosphereComponent)
    # Even scattering so the sky is grey-blue instead of a saturated blue dome.
    atmo_comp.set_editor_property("rayleigh_scattering", unreal.LinearColor(0.22, 0.26, 0.30, 1.0))
    atmo_comp.set_editor_property("rayleigh_scattering_scale", 0.018)
    atmo_comp.set_editor_property("mie_scattering_scale", 0.16)
    atmo_comp.set_editor_property("sky_luminance_factor", unreal.LinearColor(0.28, 0.30, 0.34, 1.0))

    fog = actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0))
    fog.set_actor_label("HeightFog")
    fog.set_folder_path(folder)
    fog_comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fog_comp.set_editor_property("fog_density", 0.03)
    fog_comp.set_editor_property("fog_max_opacity", 0.72)
    fog_comp.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.18, 0.21, 0.24, 1.0))
    # Volumetric fog stays off. PlayTest also sets r.VolumetricFog=0. The height fog above is the distance tint.

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
    settings.set_editor_property("auto_exposure_min_brightness", 0.15)
    settings.set_editor_property("override_auto_exposure_max_brightness", True)
    settings.set_editor_property("auto_exposure_max_brightness", 1.15)
    settings.set_editor_property("override_auto_exposure_bias", True)
    settings.set_editor_property("auto_exposure_bias", -0.55)
    # Higher white balance is cooler. Saturation stays in the grade, which Low scalability does not strip.
    settings.set_editor_property("override_white_temp", True)
    settings.set_editor_property("white_temp", 6500.0)
    settings.set_editor_property("override_color_saturation", True)
    settings.set_editor_property("color_saturation", unreal.Vector4(0.78, 0.82, 0.88, 1.0))
    settings.set_editor_property("override_color_contrast", True)
    settings.set_editor_property("color_contrast", unreal.Vector4(1.12, 1.12, 1.12, 1.0))
    pp.set_editor_property("settings", settings)

    # PlayTest runs r.ShadowQuality=0, so the sun and the fill light the boathouse through its roof
    # and the interior clipped to white (captures 1629, 1638). A bounded grade stands in for the
    # missing shadow: the interior exposes darker and cooler, and the doorway blends back to daylight.
    # Phase 5 playtest: with a 90 cm blend the exposure changed in one step at the door, which was jarring. The
    # volume now stops about 2 m inside the walls (X 20..520, Y -200..200) and blends over 2.5 m, so the grade eases
    # off across the last steps to the door and is gone about 0.5 m outside it.
    inner = actors.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(270.0, 0.0, 125.0))
    inner.set_actor_label("InteriorGrade")
    inner.set_folder_path(folder)
    inner.set_editor_property("priority", 2.0)
    inner.set_editor_property("blend_weight", 1.0)
    inner.set_editor_property("blend_radius", 250.0)
    inner.set_actor_scale3d(unreal.Vector(2.5, 2.0, 1.3))
    inner_settings = inner.get_editor_property("settings")
    inner_settings.set_editor_property("override_auto_exposure_bias", True)
    inner_settings.set_editor_property("auto_exposure_bias", -1.7)
    inner_settings.set_editor_property("override_color_gain", True)
    inner_settings.set_editor_property("color_gain", unreal.Vector4(0.86, 0.92, 1.0, 1.0))
    inner.set_editor_property("settings", inner_settings)

    # The sun does not reach the ceiling. A local fill, not a second sun.
    fill = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(360.0, 0.0, 230.0))
    fill.set_actor_label("BoathouseFill")
    fill.set_folder_path(folder)
    fill_comp = fill.get_component_by_class(unreal.PointLightComponent)
    fill_comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    fill_comp.set_editor_property("intensity", 900.0)
    fill_comp.set_editor_property("attenuation_radius", 1600.0)
    fill_comp.set_editor_property("light_color", unreal.Color(r=186, g=196, b=204, a=255))
    fill_comp.set_editor_property("cast_shadows", False)


def surface(name):
    path = f"/Game/Environment/Materials/{name}"
    asset = unreal.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing {path}. Run import_art.py first.")
    return asset


def find_largest_mesh(dest):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        return None
    best = None
    best_size = -1.0
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(asset_path)
        if not isinstance(asset, unreal.StaticMesh):
            continue
        bounds = asset.get_bounding_box()
        size = (bounds.max - bounds.min).length()
        if size > best_size:
            best = asset
            best_size = size
    return best


def wear_mesh(actor, mesh, longest_cm, center, yaw=0.0, keep_rotation=False, rotation=None, stretch=None):
    """Swap a greybox for an imported mesh. Uniform scale. Bounds center stays on center.

    rotation is (pitch, yaw, roll) in degrees and replaces yaw when given. stretch is a per-axis
    multiplier in mesh space for a mesh whose proportions are wrong (a plate that should be a board)."""
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(mesh)
    # inspectable() overrides every slot with the greybox material. An empty override list
    # lets the imported mesh's own materials render. A slot the FBX left empty renders black.
    comp.set_editor_property("override_materials", [])
    slots = mesh.get_editor_property("static_materials") or []
    fallback = None
    for slot in slots:
        fallback = slot.get_editor_property("material_interface") or fallback
    if fallback:
        for index, slot in enumerate(slots):
            if slot.get_editor_property("material_interface"):
                continue
            comp.set_material(index, fallback)
            log(f"{actor.get_actor_label()} filled empty slot {index} with {fallback.get_name()}")
    named = []
    for index in range(comp.get_num_materials()):
        mat = comp.get_material(index)
        named.append(mat.get_name() if mat else "None")
    log(f"{actor.get_actor_label()} materials {named}")
    bounds = mesh.get_bounding_box()
    extent = bounds.max - bounds.min
    longest = max(extent.x, extent.y, extent.z, 1.0)
    scale = longest_cm / max(longest, 1.0)
    mult = stretch or (1.0, 1.0, 1.0)
    actor.set_actor_scale3d(unreal.Vector(scale * mult[0], scale * mult[1], scale * mult[2]))
    if not keep_rotation:
        pitch, yaw, roll = rotation if rotation else (0.0, yaw, 0.0)
        actor.set_actor_rotation(unreal.Rotator(pitch=pitch, yaw=yaw, roll=roll), False)
    origin, _extent = actor.get_actor_bounds(False)
    loc = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(loc.x + (center[0] - origin.x), loc.y + (center[1] - origin.y), loc.z + (center[2] - origin.z)),
        False, True)
    comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
    log(f"{actor.get_actor_label()} wears {mesh.get_name()} scale={scale:.3f}")


def seat(actor, bottom_z=2.0):
    """Drop the mesh so its bounds sit on bottom_z. XY stays where wear_mesh put it."""
    origin, extent = actor.get_actor_bounds(False)
    loc = actor.get_actor_location()
    actor.set_actor_location(
        unreal.Vector(loc.x, loc.y, loc.z - (origin.z - extent.z) + bottom_z), False, True)


def assign_tern_materials(comp, mesh):
    """Both Tern slots: faded paint plus the rust-paint grime blend. Slot names from the FBX pick u1 or u2."""
    u1 = unreal.load_asset("/Game/Environment/Materials/MI_DC_TernU1")
    u2 = unreal.load_asset("/Game/Environment/Materials/MI_DC_TernU2")
    slots = mesh.get_editor_property("static_materials") if mesh else []
    for index in range(comp.get_num_materials()):
        name = ""
        if index < len(slots):
            name = str(slots[index].get_editor_property("material_slot_name")).lower()
        comp.set_material(index, u2 if "u2" in name else u1)


def build_ground():
    folder = "Ground"
    sand = surface("MI_DC_CoastSand")
    gravel = surface("MI_DC_Gravel")
    lake = surface("MI_DC_OpenLake")
    block("Floor", folder, -200, 3600, -600, 1800, -50, 0, material=sand)
    block("ShoreCurb", folder, -200, 3600, -600, -580, 0, 18, material=gravel)
    # A thin sheet. A deep slab's vertical face read as an untextured black wall at the far breakwater.
    block("Water", folder, -200, 3600, -1400, -600, -16, -6, material=lake)

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
    steel = surface("MI_DC_Steel")
    concrete = surface("MI_DC_Concrete")
    rust = surface("MI_DC_RustPaint")

    # Corrugated walls. Concrete sills and a rust-paint lintel are the base and trim.
    # Back (west) wall with a window: the survey launch's mast and lamp show through it (Exploration Loop).
    block("Wall_Back_North", folder, 0, 20, -110, 320, 0, wall_h, material=steel)
    block("Wall_Back_South", folder, 0, 20, -320, -250, 0, wall_h, material=steel)
    block("Wall_Back_Sill", folder, 0, 20, -250, -110, 0, 100, material=concrete)
    block("Wall_Back_Head", folder, 0, 20, -250, -110, 230, wall_h, material=steel)
    block("Wall_Right", folder, 0, 720, 300, 320, 0, wall_h, material=steel)
    block("Ceiling", folder, 0, 720, -320, 320, wall_h, wall_h + 20, material=steel)

    # Shore-side wall with a window looking at the lake.
    block("Wall_Left_West", folder, 0, 250, -320, -300, 0, wall_h, material=steel)
    block("Wall_Left_East", folder, 400, 720, -320, -300, 0, wall_h, material=steel)
    block("Wall_Left_Sill", folder, 250, 400, -320, -300, 0, 110, material=concrete)
    block("Wall_Left_Head", folder, 250, 400, -320, -300, 200, wall_h, material=steel)

    dx = 700
    block("Wall_Front_Left", folder, dx, dx + 20, -320, -door_half, 0, wall_h, material=steel)
    block("Wall_Front_Right", folder, dx, dx + 20, door_half, 320, 0, wall_h, material=steel)
    block("Wall_Front_Lintel", folder, dx, dx + 20, -door_half, door_half, door_h, wall_h, material=rust)
    door("Door", folder, (dx + 10, -door_half, 0), door_half * 2 - 2, door_h - 2, 6, "Boathouse Door", "boat.door",
         material=rust)

    bench_top = 75
    block("Workbench", folder, 400, 520, 160, 260, 0, bench_top, material=concrete)
    pickup("Pickup_Pistol", folder, "/Game/Items/DA_Item_Pistol", 1, 440, 200, bench_top,
           yaw=90.0, persistent_id="boat.pickup_pistol")
    pickup("Pickup_Ammo9mm", folder, "/Game/Items/DA_Item_Ammo9mm", 24, 490, 220, bench_top,
           persistent_id="boat.pickup_ammo")
    pickup("Pickup_FieldDressing", folder, "/Game/Items/DA_Item_FieldDressing", 1, 470, 180, bench_top,
           persistent_id="boat.pickup_dressing")

    cot = inspectable("Cot", folder, (160, -180, 25), (190, 80, 50), "Salt-stiff cot",
                      "The canvas is stiff with salt and old sweat. You slept here, or passed out here. Hard to tell which.")
    # The generated cot is taller than a real one (about half as tall as it is long), so it is squashed to a
    # camp cot's proportions. The interaction box keeps the old footprint.
    wear_mesh(cot, first_mesh("/Game/Art/Meshy/field_cot"), 190.0, (160.0, -180.0, 24.0), yaw=0.0,
              stretch=(1.0, 0.72, 0.5))
    seat(cot, 0.0)
    inspectable("Radio", folder, (80, 180, 18), (28, 22, 36), "Dead radio",
                "The casing is faintly warm. Nothing in this building should still be drawing power.")
    inspectable("Notice", folder, (30, 0, 150), (6, 80, 90), "Faded notice",
                "GREAT LAKES MARITIME AUTHORITY. Storm protocol. Stay inland during Current events. The date is torn off.")
    inspectable("WindowSill", folder, (325, -270, 150), (140, 16, 8), "Lake window",
                "The water sits too still. No birds. A hull lists in the shallows, paint long gone.",
                material=steel)
    inspectable("WestWindow", folder, (34, -180, 104), (28, 130, 8), "West window",
                "West along the shore, a mast leans out over the water. A light at the top of it comes and goes. "
                "Nothing out there should still have power.",
                material=steel,
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
        unreal.DCScavengerCharacter, unreal.Vector(2500.0, 0.0, 96.0),
        unreal.Rotator(pitch=0.0, yaw=180.0, roll=0.0))
    scav.set_actor_label("Scavenger")
    scav.set_folder_path(folder)
    scav.set_editor_property("patrol_points", [
        # Phase 5 playtest: the south leg used to run at Y -350, right past the relay, so the coil could not be
        # reached unseen. It now runs at Y -60, and a windbreak (build_camp_cover) stands between it and the relay.
        # The whole loop also sits 3 m further east than it did, so from the boathouse door he is about 16 m off,
        # beyond his 12 m sight: the player can watch him before choosing an approach.
        unreal.Vector(2300.0, -60.0, 0.0),
        unreal.Vector(3000.0, -60.0, 0.0),
        unreal.Vector(3000.0, 350.0, 0.0),
        unreal.Vector(2300.0, 350.0, 0.0),
    ])
    assign_mannequin(
        scav,
        SURVIVAL_MESH,
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
        costume=("MI_DC_ScavJacket", "MI_DC_ScavJeans"),
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
    # The hum matches the rig's silent variants: it stops when the player holds the coil, when the relay is
    # recovered, or when the scavenger is dead. Inspecting the rig does not stop it, and the actor sets nothing.
    conditional_audio("Audio_RelayHum", folder, (2480, -220, 60), "/Game/Audio/Ambience/S_DC_HumRelay", 0.6,
                      conditions=[cond("HAS_ITEM", id="radio_coil", negate=True),
                                  cond("WORLD_FLAG", id="shore.relay_recovered", negate=True),
                                  cond("ACTOR_DEAD", id="boat.scavenger", negate=True)],
                      attenuation="SA_DC_Hum")


CRATE_BODY = "/Game/Art/PolyHaven/wooden_crate_01/wooden_crate_01_2k/StaticMeshes/wooden_crate_01"
CRATE_LID = "/Game/Art/PolyHaven/wooden_crate_01/wooden_crate_01_2k/StaticMeshes/wooden_crate_01_lid"
CRATE_SCALE = 1.3  # 107 x 53 x 41 cm


def crate_visual(label, folder, x, y, bottom_z, yaw):
    """One closed PolyHaven crate, NoCollision (its cover block does the blocking)."""
    parts = []
    for suffix, path in (("", CRATE_BODY), ("_Lid", CRATE_LID)):
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, bottom_z),
                                              unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
        actor.set_actor_label(label + suffix)
        actor.set_folder_path(folder)
        actor.set_actor_scale3d(unreal.Vector(CRATE_SCALE, CRATE_SCALE, CRATE_SCALE))
        comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        comp.set_static_mesh(unreal.load_asset(path))
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        actor.set_actor_enable_collision(False)
        parts.append(actor)
    return parts


def build_camp_cover():
    """Cover for the coil route (Phase 5 playtest: the open camp forced a fight). Three crate stacks the player can
    move between, crouched, to reach the coil: one west of the camp off the path, two on the beach south of the
    patrol's south leg. Each is a hidden block (collision, which is what breaks the scavenger's sight trace) under
    closed crates. All stay outside the patrol square (X 2300..3000, Y -60..350) and the path."""
    folder = "CampCover"
    # centre x, y, yaw (degrees). A stack is two crates side by side with one on top: about 214 x 53 x 123 cm.
    for index, (x, y, yaw) in enumerate([(1880.0, 250.0, 90.0), (2250.0, -470.0, 0.0), (2560.0, -470.0, 0.0)]):
        across = unreal.Vector(math.cos(math.radians(yaw)), math.sin(math.radians(yaw)), 0.0)
        long_x, long_y = (214.0, 55.0) if yaw == 0.0 else (55.0, 214.0)
        cover = block(f"CampCover_{index}", folder, x - long_x / 2, x + long_x / 2, y - long_y / 2, y + long_y / 2,
                      0.0, 123.0)
        cover.get_component_by_class(unreal.StaticMeshComponent).set_visibility(False)
        cover.set_editor_property("tags", [unreal.Name("CampCover")])
        for side in (-1.0, 1.0):
            crate_visual(f"CampCover_{index}_Crate{int(side)}", folder, x + across.x * 53.5 * side,
                         y + across.y * 53.5 * side, 0.0, yaw)
        crate_visual(f"CampCover_{index}_Top", folder, x + across.x * 20.0, y + across.y * 20.0, 41.0, yaw + 8.0)

    # A windbreak of scrap sheet along the lake side of his camp (Phase 5 playtest: "I need a clear path"). His
    # loop now stays north of it; the relay and the coil are on the beach side, so a player coming along the
    # waterline, crouched, is out of his sight all the way to the coil. Overlapping panels, no gaps. Collision on:
    # it is what breaks his sight trace, and he walks around it when chasing.
    steel = surface("MI_DC_Steel")
    rust = surface("MI_DC_RustPaint")
    for index, (x0, dy, height) in enumerate([(1850, 0, 188), (2025, 6, 196), (2200, -4, 184), (2375, 5, 198),
                                              (2550, -3, 190), (2725, 4, 194), (2900, -5, 186)]):
        panel = block(f"CampWindbreak_{index}", folder, x0, x0 + 200, -155 + dy, -137 + dy, 0, height,
                      material=steel if index % 2 == 0 else rust)
        panel.set_editor_property("tags", [unreal.Name("CampWindbreak")])


def build_cover_and_npc():
    folder = "Cover"
    # LOS wall: scavenger patrols south of this, Mara stands north.
    block("Ridge", folder, 1900, 3400, 550, 650, 0, 280, material=surface("MI_DC_LandRock"))
    block("RidgeEnd", folder, 3380, 3480, 550, 1500, 0, 280, material=surface("MI_DC_LandRock"))

    folder = "NPC"
    steel = surface("MI_DC_Steel")
    block("Shed_Back", folder, 3040, 3220, 1480, 1500, 0, 220, material=steel)
    block("Shed_Left", folder, 3040, 3060, 1180, 1500, 0, 220, material=steel)
    block("Shed_Roof", folder, 3040, 3220, 1180, 1500, 220, 240, material=steel)

    npc = actors.spawn_actor_from_class(
        unreal.DCFriendlyNPC, unreal.Vector(3100.0, 1280.0, 96.0),
        unreal.Rotator(pitch=0.0, yaw=-90.0, roll=0.0))
    npc.set_actor_label("Mara")
    npc.set_folder_path(folder)
    assign_mannequin(
        npc,
        SURVIVAL_MESH,
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
        costume=("MI_DC_MaraJacket", "MI_DC_MaraJeans"),
        skin="MI_DC_MaraSkin",
        hair="MI_DC_MaraHair",
    )
    # Her own head, so she does not read as the scavenger in a different jacket. PROVISIONAL: a face for the
    # provisional character, not a decision about her history. Bone space: X is up, Y is forward, Z is lateral.
    # The mesh is authored Z-up; pitch -90 turns its up onto the bone's up.
    swap = npc.get_editor_property("head_swap")
    swap.set_editor_property("head_mesh", first_mesh("/Game/Art/Meshy/mara_head_collar"))
    swap.set_editor_property("rotation", unreal.Rotator(pitch=-90.0, yaw=0.0, roll=0.0))
    # Phase 5 playtest ("a floating head attached to a coat"): a new head that brings its own knit collar ring, so
    # the join to the jacket is cloth against cloth. Its chin sits about 6 mesh units higher than the old bust's.
    swap.set_editor_property("offset", unreal.Vector(-4.8, 1.0, 0.0))
    swap.set_editor_property("scale", 0.22)
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
    rock = surface("MI_DC_CoastRock")
    gravel = surface("MI_DC_Gravel")
    lake = unreal.load_asset("/Game/Environment/Materials/MI_DC_OpenLake")
    block("Floor_West", folder, -3200, -200, -600, 1800, -50, 0, material=rock)
    # The curb stops where the launch's hull crosses the shoreline. Gravel is the waterline shingle.
    block("ShoreCurb_West_A", folder, -3200, -1720, -600, -580, 0, 18, material=gravel)
    block("ShoreCurb_West_B", folder, -1280, -200, -600, -580, 0, 18, material=gravel)
    block("Water_West", folder, -3200, -200, -2400, -600, -16, -6, material=lake)
    # East of the basin wall the sheet used to stop, and the channel mouth was a black void.
    block("Water_Channel", folder, -200, 2800, -4200, -1400, -16, -6, material=lake)
    # Keep the player on the map: a bluff to the west and north, rocks around the far water.
    block("Bluff_West", folder, -3240, -3200, -2440, 1840, -80, 420, material=surface("MI_DC_LandRock"))
    block("Bluff_North", folder, -3200, -200, 1800, 1840, -50, 300, material=surface("MI_DC_LandRock"))
    block("Breakwater_South", folder, -3200, -200, -2440, -2400, -80, 140, material=rock)
    block("Breakwater_East", folder, -220, -200, -2400, -1400, -80, 140, material=rock)
    # Basin-facing skins. The outer blocks' inner sides were reading as an untextured black wall.
    block("Breakwater_South_Inner", folder, -3200, -220, -2360, -2320, -20, 150, material=rock)
    block("Breakwater_East_Inner", folder, -300, -240, -2360, -1400, -20, 150, material=rock)
    # The east half had no edge: the player could walk off the north and east sides, or wade off the far side of
    # the channel, and fall. Invisible walls (collision, no mesh visibility) close it. The lake keeps its open
    # horizon; these stand in the water and at the land edges.
    wall_bottom, wall_top = -120, 700
    for label, x0, x1, y0, y1 in [
        ("Edge_North", -200, 3640, 1800, 1840),
        ("Edge_East", 3600, 3640, -1440, 1840),
        ("Edge_ChannelEast", 2800, 2840, -4240, -1400),
        ("Edge_ChannelNorth", 2800, 3640, -1440, -1400),
        ("Edge_South", -200, 2840, -4240, -4200),
        ("Edge_ChannelWest", -240, -160, -4240, -1400),
        ("Edge_BasinSouth", -3240, -160, -2440, -2400),
    ]:
        edge = block(label, folder, x0, x1, y0, y1, wall_bottom, wall_top)
        edge.get_component_by_class(unreal.StaticMeshComponent).set_visibility(False)
    # Beach stones.
    block("Boulder_A", folder, -2250, -2080, -520, -400, 0, 70, material=rock)
    block("Boulder_B", folder, -880, -760, -560, -470, 0, 45, material=rock)
    block("Boulder_C", folder, -2600, -2450, 200, 330, 0, 90, material=rock)


def build_survey_launch():
    """Exploration Loop POI: a Maritime Authority survey launch driven bow-first onto the stones.

    Notice: a leaning mast with a flickering lamp above the boathouse roofline (and through its west window).
    Discover: walking up to it (location shore.survey_launch). Read: name board, cut life jackets, depth
    sounder, gutted breaker panel, survey log, emergency beacon, dead fish. Danger: live water round the
    stern until the battery leads are pulled. Loot: the survey locker (obvious) and the kit in the tender
    tied off the stern, out in the live water (less obvious; the log mentions it).
    """
    folder = "SurveyLaunch"
    hull_mat = material_instance("MI_DC_WreckHull", MAT_FLAT, {"Base Color": (0.10, 0.13, 0.15, 1.0)})
    # Rails the mesh covers stay as collision. The boarding plank and cabin trim use the rust-paint instance.
    rust = surface("MI_DC_RustPaint")
    steel = surface("MI_DC_Steel")
    ensure_glow_material()
    amber = material_instance("MI_DC_GlowAmber", MAT_GLOW, {"Color": (9.0, 4.0, 0.9, 1.0)})
    spark = material_instance("MI_DC_GlowSpark", MAT_GLOW, {"Color": (2.0, 4.5, 10.0, 1.0)})
    live_water = surface("MI_DC_LiveWater")
    water_mat = surface("MI_DC_Water")
    fish_mat = material_instance("MI_DC_DeadFish", MAT_FLAT, {"Base Color": (0.78, 0.8, 0.72, 1.0)})
    live = [cond("WORLD_FLAG", id=WRECK_POWER_CUT, negate=True)]

    # Hull frame: local X = beam (+X is the east side, toward the boathouse), local Y = length (+Y is the
    # bow, pointing inland), Z = up from the deck. Bow raised on the stones, listing toward the east side.
    hull = Frame((-1500.0, -760.0, 95.0), pitch=-4.0, roll=-3.0)
    hull.part("Hull", folder, (0, 0, -80), (360, 1000, 160), material=hull_mat, hidden=True)
    hull.part("Bow", folder, (0, 540, -70), (220, 80, 180), material=hull_mat, hidden=True)
    # The mesh covers these. Collision stays; the brown planks were showing through the hull.
    hull.part("Rail_West", folder, (-174, 0, 30), (12, 1000, 60), material=rust, hidden=True)
    # Gap in the east rail where the plank comes aboard (local Y 180..320).
    hull.part("Rail_East_Aft", folder, (174, -160, 30), (12, 680, 60), material=rust, hidden=True)
    hull.part("Rail_East_Bow", folder, (174, 410, 30), (12, 180, 60), material=rust, hidden=True)
    hull.part("Rail_Bow", folder, (0, 494, 30), (360, 12, 60), material=rust, hidden=True)
    hull.part("Transom", folder, (0, -494, 30), (360, 12, 60), material=rust, hidden=True)

    # Boarding plank from the beach up to the gap in the east rail.
    top = hull.world((180, 250, 0))
    run = 320.0
    rise = top[2]
    plank_pitch = -math.degrees(math.atan2(rise, run))
    box_rot("Plank", folder, (top[0] + run / 2, top[1], rise / 2 - 2),
            (math.hypot(run, rise), 90, 6), (plank_pitch, 0.0, 0.0), material=rust)

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
    hull.part("Wheelhouse_Roof", folder, (0, -24, wall_h + 10), (336, 336, 20), material=rust)
    hull.part("Mast", folder, (0, -24, wall_h + 20 + 450), (16, 16, 900), material=steel)
    hull.part("Mast_Yard", folder, (0, -24, wall_h + 20 + 780), (160, 10, 10), material=steel)
    lamp = hull.world((0, -24, wall_h + 20 + 915))
    flicker_light("MastLamp", folder, lamp, (255, 170, 80), 60.0, 1400.0, glow_cm=60.0, glow_material=amber,
                  min_brightness=0.35, dropout=0.18, interval=(0.06, 0.9))

    # Inside the wheelhouse.
    hull.part("Console", folder, (-68, 94, 47), (150, 48, 94), material=rust)
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
    hull.part("BatteryLead", folder, (-105, -420, 45), (6, 150, 6), material=rust)
    hull.part("BatteryLead_Over", folder, (-105, -505, -30), (6, 6, 140), material=rust)
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
    # Top face 0.25 cm above the open lake (z -6) so its edge is not a visible step, and the same material so it
    # has no seam.
    water_sheet = box("WaterSurface", folder, (-1500, -1390, -6.25), (1040, 780, 1), material=water_mat)
    water_sheet.get_component_by_class(unreal.StaticMeshComponent).set_collision_enabled(
        unreal.CollisionEnabled.NO_COLLISION)
    # The map test finds the slab by this tag to prove the water is still there after the power is cut.
    water_sheet.set_editor_property("tags", [unreal.Name("WaterSurface")])
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
        snap = flicker_light(name, folder, (sx, sy, 24), (160, 200, 255), 40.0, 700.0, glow_cm=16.0,
                             glow_material=spark, conditions=live, min_brightness=0.0, dropout=0.4, interval=interval)
        snap.set_editor_property("flash_sounds", [audio_asset(f"/Game/Audio/SFX/S_DC_Spark0{n}") for n in range(1, 7)])
        snap.set_editor_property("flash_attenuation", audio_asset("/Game/Audio/SA_DC_Snap"))
    # The live water hums while it is live, and the breaker clunks once when the leads come off.
    # Levels are the starting points in C:\FO5_AssetLibrary\Audio\SOURCING_NOTES.md.
    conditional_audio("Audio_LiveWaterHum", folder, (-1500, -1390, 60), "/Game/Audio/Ambience/S_DC_HumLiveWater", 0.16,
                      conditions=live, attenuation="SA_DC_Hum")
    bank = battery.get_actor_location()
    conditional_audio("Audio_BreakerThrow", folder, (bank.x, bank.y, bank.z),
                      "/Game/Audio/SFX/S_DC_BreakerPull", 0.56,
                      conditions=[cond("WORLD_FLAG", id=WRECK_POWER_CUT)], attenuation="SA_DC_Clunk", once=True)
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
    # The box stays the container (class, id, contents). The mesh is the same wreck, scaled down.
    tender = box("Tender", folder, (-1500, -1650, 12), (120, 240, 45), material=interactable_mat,
                 actor_class=unreal.DCLootContainer)
    boat_mesh = find_largest_mesh("/Game/Art/Fab/motorboat_wreck")
    if boat_mesh:
        wear_mesh(tender, boat_mesh, 240.0, (-1500.0, -1650.0, 24.0), yaw=0.0)
        assign_tern_materials(tender.get_component_by_class(unreal.StaticMeshComponent), boat_mesh)
    setup_container(tender, "Tender", "boat.wreck_tender", [(ITEM_CHART, 1), (ITEM_DRESSING, 2), (ITEM_AMMO, 18)])
    stern = hull.world((0, -500, -20))
    rope_run = stern[1] - (-1530.0)
    box_rot("TenderLine", folder, (-1500, (stern[1] - 1530.0) / 2, (stern[2] + 20) / 2),
            (4, rope_run, 4), (0.0, 0.0, -math.degrees(math.atan2(stern[2] - 20, rope_run))), material=rust)

    # Discovery: walking up to the wreck from the boathouse, the beach, or the water.
    discovery = actors.spawn_actor_from_class(unreal.DCLocationVolume, unreal.Vector(-1650.0, -700.0, 300.0))
    discovery.set_actor_label("Location_SurveyLaunch")
    discovery.set_folder_path(folder)
    discovery.set_editor_property("location_id", WRECK_LOCATION)
    discovery.set_editor_property("display_name", unreal.Text("Wrecked Survey Launch"))
    discovery.get_editor_property("bounds").set_box_extent(unreal.Vector(950.0, 1150.0, 500.0))
    dress_library_clues()
    log(f"survey launch: deck gap at {top}, lamp at {lamp}")


def first_mesh(dest):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        raise RuntimeError(f"Missing {dest}. Run import_art.py first.")
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(asset_path)
        if isinstance(asset, unreal.StaticMesh):
            return asset
    raise RuntimeError(f"No static mesh in {dest}")


def actor_by_label(label):
    for actor in actors.get_all_level_actors():
        if actor.get_actor_label() == label:
            return actor
    raise RuntimeError(f"Missing actor {label}")


def dress_library_clues():
    """Library meshes on the existing clue actors. No ammo box in the library; the rounds stay a small stack."""
    log_actor = actor_by_label("SurveyLog")
    origin, _extent = log_actor.get_actor_bounds(False)
    wear_mesh(log_actor, first_mesh("/Game/Art/Meshy/lighthouse_logbook"), 28.0,
              (origin.x, origin.y, origin.z), keep_rotation=True)

    jackets = actor_by_label("LifeJackets")
    # The library jacket is modelled upright, as if worn. Rolled onto its back it lies on the stones, and the
    # thickness (mesh Y) is flattened to a stuffed vest's.
    wear_mesh(jackets, first_mesh("/Game/Art/PolyHaven/life_jacket"), 70.0, (-1080.0, -380.0, 16.0),
              rotation=(0.0, 25.0, 90.0), stretch=(1.0, 0.5, 1.0))
    origin, extent = jackets.get_actor_bounds(False)
    loc = jackets.get_actor_location()
    jackets.set_actor_location(
        unreal.Vector(loc.x, loc.y, loc.z - (origin.z - extent.z) + 2.0), False, True)

    chalk = actor_by_label("ChalkWarning")
    board = unreal.load_asset("/Game/Environment/Materials/M_DC_Chalk")
    if not board:
        raise RuntimeError("Missing M_DC_Chalk. Run import_art.py first.")
    comp = chalk.get_component_by_class(unreal.StaticMeshComponent)
    for slot in range(comp.get_num_materials()):
        comp.set_material(slot, board)

    # One mesh for every relay inspect state (live, cold, empty). The coil stays its own pickup.
    # Yaw 270 turns the open side (lid hinges, side gland) toward the camp camera and the path.
    # The cavity polygons stay black under this grade; a fill light only lit the sand, so it is not used.
    relay = actor_by_label("RelayRig")
    wear_mesh(relay, first_mesh("/Game/Art/Meshy/relay_housing"), 40.0, (2480.0, -220.0, 20.0), yaw=270.0)
    seat(relay)

    # Greybox inspectables that were still on MI_DefaultColorway. Library meshes, same actors.
    wear_mesh(actor_by_label("Can"), first_mesh("/Game/Art/PolyHaven/can_rusted"), 44.0, (980.0, 80.0, 22.0))
    seat(actor_by_label("Can"))
    wear_mesh(actor_by_label("Crates"), first_mesh("/Game/Art/PolyHaven/wooden_crate_01"), 80.0, (1550.0, 220.0, 35.0))
    seat(actor_by_label("Crates"))
    wear_mesh(actor_by_label("Lookout"), first_mesh("/Game/Art/PolyHaven/wooden_crate_01"), 80.0, (3160.0, 1220.0, 40.0))
    seat(actor_by_label("Lookout"))
    hull_actor = actor_by_label("Hull")
    boat = find_largest_mesh("/Game/Art/Fab/motorboat_wreck")
    if not boat:
        raise RuntimeError("Missing the motorboat mesh for the beached hull")
    wear_mesh(hull_actor, boat, 220.0, (1200.0, -280.0, 40.0), yaw=90.0)
    assign_tern_materials(hull_actor.get_component_by_class(unreal.StaticMeshComponent), boat)
    seat(hull_actor)
    dress_meshy_clues()


# Meshy prop placement: label -> (prop id, longest side in cm, (pitch, yaw, roll)).
# The size is the readable footprint the trace already hits, not the real-world size. The rotation
# turns the mesh's front toward the way the player meets it. Tuned against the Tools\ReviewCapture.bat clue shots.
MESHY_CLUES = {
    "DepthSounder": ("depth_sounder", 34.0, (0.0, 0.0, 0.0), None),
    "BreakerPanel": ("breaker_panel", 70.0, (0.0, 270.0, 0.0), None),
    "BatteryBank": ("battery_bank", 76.0, (0.0, 0.0, 0.0), None),
    "Beacon": ("emergency_beacon", 32.0, (0.0, 0.0, 0.0), None),
    # Meshy made a square plate. Stand it on its edge, and squeeze it into a board.
    "NameBoard": ("name_board_tern", 120.0, (0.0, 180.0, 90.0), (1.0, 0.28, 0.2)),
    "DeadFish": ("dead_fish", 36.0, (0.0, 0.0, 0.0), None),
}
FISH_RING = 9


def place_name_letters():
    """The flaked T_RN on the bow board: a thin NoCollision plane just off the board's bow face.
    The board's own UVs are not one face, so the letters cannot ride on its texture."""
    board = actor_by_label("NameBoard")
    origin, extent = board.get_actor_bounds(False)
    material = unreal.load_asset("/Game/Environment/Materials/M_DC_Letters")
    plane_mesh = unreal.load_asset("/Engine/BasicShapes/Plane")
    if not material or not plane_mesh:
        raise RuntimeError("Missing M_DC_Letters or the engine plane. Run import_art.py first.")
    plane = actors.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(origin.x, origin.y + extent.y + 0.6, origin.z + 1.0),
        unreal.Rotator(pitch=LETTER_ROT[0], yaw=LETTER_ROT[1], roll=LETTER_ROT[2]))
    plane.set_actor_label("NameBoardLetters")
    plane.set_folder_path("SurveyLaunch")
    comp = plane.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(plane_mesh)
    comp.set_material(0, material)
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    comp.set_editor_property("cast_shadow", False)
    # The plane is 100 x 100 cm. The board is about 120 x 34 cm; the name fills the middle of it.
    plane.set_actor_scale3d(unreal.Vector(LETTER_SCALE[0], LETTER_SCALE[1], 1.0))

    # The board hung in the air: the generated hull does not reach the blockout bow it was placed on. It is now
    # propped on two stakes driven into the stones behind it (PROVISIONAL: someone stood the name board up on
    # the beach). Stakes have no collision, so they change no route and block no trace.
    stake_mat = surface("MI_DC_RustPaint")
    top = origin.z + extent.z - 6.0
    for side, offset in (("L", -(extent.x - 14.0)), ("R", extent.x - 14.0)):
        stake = block(f"NameBoardStake_{side}", "SurveyLaunch", origin.x + offset - 4.0, origin.x + offset + 4.0,
                      origin.y - extent.y - 9.0, origin.y - extent.y - 1.0, -4.0, top, material=stake_mat)
        stake.get_component_by_class(unreal.StaticMeshComponent).set_collision_enabled(
            unreal.CollisionEnabled.NO_COLLISION)


LETTER_ROT = (0.0, 0.0, -90.0)
LETTER_SCALE = (0.96, -0.24)


def meshy_mesh(prop_id):
    path = f"/Game/Art/Meshy/{prop_id}/SM_{prop_id}"
    mesh = unreal.load_asset(path)
    if not mesh:
        raise RuntimeError(f"Missing {path}. Run import_art.py first.")
    return mesh


def dress_meshy_clues():
    """Bespoke Meshy meshes on the greybox clue actors. Same actors, ids, and variants; the mesh keeps
    the greybox's center and sits on the greybox's bottom, so the trace still lands on it."""
    for label, (prop_id, longest, rotation, stretch) in MESHY_CLUES.items():
        actor = actor_by_label(label)
        origin, extent = actor.get_actor_bounds(False)
        bottom = origin.z - extent.z
        wear_mesh(actor, meshy_mesh(prop_id), longest, (origin.x, origin.y, origin.z),
                  rotation=rotation, stretch=stretch)
        seat(actor, bottom)
    # The ring of dead fish is scenery, so it lies on the water at the greybox's height.
    fish = meshy_mesh("dead_fish")
    for index in range(FISH_RING):
        actor = actor_by_label(f"Fish_{index}")
        origin, extent = actor.get_actor_bounds(False)
        bottom = origin.z - extent.z
        wear_mesh(actor, fish, 32.0, (origin.x, origin.y, origin.z), rotation=(0.0, 40.0 * index + 65.0, 0.0))
        seat(actor, bottom)
    place_name_letters()
    # Steel and painted board rather than the prototype grid: no library mesh fits these two.
    for label, name in (("SurveyLocker", "MI_DC_RustPaint"), ("KeepOut", "MI_DC_Plaster")):
        comp = actor_by_label(label).get_component_by_class(unreal.StaticMeshComponent)
        for slot in range(comp.get_num_materials()):
            comp.set_material(slot, surface(name))


def editor_world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def package_name(obj):
    if not obj:
        return ""
    outer = obj.get_outermost()
    return outer.get_name() if outer else ""


def focus_persistent():
    """Gameplay actors have to be spawned into the persistent map, not the art sublevel."""
    current = levels.get_current_level()
    if package_name(current) == MAP_PATH:
        return
    short = MAP_PATH.rsplit("/", 1)[-1]
    for name in (short, "PersistentLevel"):
        levels.set_current_level_by_name(name)
        if package_name(levels.get_current_level()) == MAP_PATH:
            return
    for level in unreal.EditorLevelUtils.get_levels(editor_world()):
        log(f"level name={level.get_name()} package={package_name(level)}")
    raise RuntimeError("Could not return to the persistent boathouse level")


def art_levels():
    """Loaded ULevels whose package is the art map. Each ULevel is named PersistentLevel, so the package is the id."""
    found = []
    for level in unreal.EditorLevelUtils.get_levels(editor_world()):
        if package_name(level) == ART_MAP:
            found.append(level)
    return found


def destroy_persistent_actors():
    """Drop generated actors only. A loaded art sublevel shares get_all_level_actors()."""
    persistent = levels.get_current_level()
    if not persistent:
        raise RuntimeError("No current level to rebuild")
    persistent_package = package_name(persistent)
    if persistent_package != MAP_PATH:
        raise RuntimeError(f"Refusing to rebuild: current level package is '{persistent_package}'")
    doomed = []
    spared = 0
    for actor in actors.get_all_level_actors():
        outer = actor.get_outer()
        if package_name(outer) != persistent_package:
            spared += 1
            continue
        # Spare the level's builder brush only. Volumes are brushes too; skipping every Brush left each rebuild's
        # post-process and nav-bounds volumes behind (Phase 5 playtest found 42 interior grades and 78 unbound
        # post-process volumes stacked in the map).
        if isinstance(actor, unreal.WorldSettings) or (
                isinstance(actor, unreal.Brush) and not isinstance(actor, unreal.Volume)):
            continue
        doomed.append(actor)
    if not doomed and spared > 3:
        raise RuntimeError(
            f"Refusing to rebuild: no actors in package '{persistent_package}' "
            f"({spared} actors were in other levels). The art level filter is wrong.")
    actors.destroy_actors(doomed)
    log(f"cleared {len(doomed)} persistent actors; left {spared} in other levels")


def ensure_art_level():
    """Re-link Lvl_Boathouse_Art. Create it once. Never save it on a later run."""
    world = editor_world()
    linked = art_levels()
    if len(linked) > 1:
        raise RuntimeError(f"Lvl_Boathouse_Art is linked {len(linked)} times")
    if linked:
        sentinels = []
        for actor in actors.get_all_level_actors():
            if package_name(actor.get_outer()) != ART_MAP:
                continue
            if ART_SENTINEL in [str(tag) for tag in actor.get_editor_property("tags")]:
                sentinels.append(actor)
        if len(sentinels) != 1:
            raise RuntimeError(
                f"Art level is linked but has {len(sentinels)} ArtLayerSentinel actors")
        log("art level already linked; sentinel intact")
        focus_persistent()
        return

    if not unreal.EditorAssetLibrary.does_asset_exist(ART_MAP):
        streaming = unreal.EditorLevelUtils.create_new_streaming_level(
            unreal.LevelStreamingAlwaysLoaded, ART_MAP, False)
        if not streaming:
            raise RuntimeError(f"Could not create {ART_MAP}")
        if not levels.set_current_level_by_name("Lvl_Boathouse_Art"):
            raise RuntimeError("Could not make the new art level current")
        sentinel = actors.spawn_actor_from_class(
            unreal.TargetPoint, unreal.Vector(0.0, 0.0, -2000.0))
        sentinel.set_actor_label("ArtLayerSentinel")
        sentinel.set_editor_property("tags", [unreal.Name(ART_SENTINEL)])
        sentinel.set_actor_hidden_in_game(True)
        sentinel.set_actor_enable_collision(False)
        loaded = streaming.get_loaded_level()
        if loaded and sentinel.get_outer() != loaded:
            unreal.EditorLevelUtils.move_actors_to_level([sentinel], loaded)
        package = (loaded or sentinel.get_outer()).get_outermost()
        if not unreal.EditorLoadingAndSavingUtils.save_packages([package], False):
            raise RuntimeError(f"Could not save {ART_MAP}")
        log("created Lvl_Boathouse_Art with ArtLayerSentinel")
    else:
        streaming = unreal.EditorLevelUtils.add_level_to_world(
            world, ART_MAP, unreal.LevelStreamingAlwaysLoaded)
        if not streaming:
            raise RuntimeError(f"Could not link {ART_MAP}")
        log("relinked Lvl_Boathouse_Art")

    focus_persistent()


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


def build_landing_stage_poi():
    """The Landing Stage (Phase 5 pilot) lives in its own script, which owns only its actors
    (Design/POIs/shore.landing_stage.md). This hook hands it this script's helpers; it is the only coupling."""
    import os
    import sys
    import types
    scripts = os.path.join(unreal.SystemLibrary.get_project_directory(), "Tools", "EditorScripts")
    if scripts not in sys.path:
        sys.path.insert(0, scripts)
    import build_landing_stage
    build_landing_stage.build(types.SimpleNamespace(**globals()))


def require_surfaces():
    missing = [path for path in REQUIRED_SURFACES if not unreal.EditorAssetLibrary.does_asset_exist(path)]
    if missing:
        raise RuntimeError("Missing surface materials from import_art.py: " + ", ".join(missing))


def main():
    require_surfaces()
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        levels.load_level(MAP_PATH)
        destroy_persistent_actors()
    elif not levels.new_level(MAP_PATH, False):
        raise RuntimeError(f"Could not create {MAP_PATH}")

    focus_persistent()

    log(f"cube bounds min={CUBE_MIN} size={CUBE_SIZE}")
    build_lighting()
    build_ground()
    build_boathouse()
    build_exterior()
    build_scavenger()
    build_camp_cover()
    build_cover_and_npc()
    build_west_shore()
    build_survey_launch()
    build_landing_stage_poi()
    build_nav()
    ensure_art_level()

    focus_persistent()
    if not levels.save_current_level():
        raise RuntimeError(f"Could not save {MAP_PATH} (is the file read-only?)")
    linked = art_levels()
    if len(linked) != 1:
        raise RuntimeError(f"Expected one art-level link after save, found {len(linked)}")
    log(f"saved {MAP_PATH} with {len(actors.get_all_level_actors())} actors; art level linked")


main()
