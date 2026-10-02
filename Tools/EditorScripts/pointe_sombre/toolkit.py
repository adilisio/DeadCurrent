"""The helpers every Pointe Sombre cell script is given: build(tk) receives this module, set up by the core script.

Integrator-owned (VerticalSlicePhasePlan.txt §13.2). A cell script uses only these helpers, its own constants, and the
shared anchors; it never looks up another cell's actors (ask the Integrator for an anchor instead).

Conventions (Design/technical_architecture.md, "Pointe Sombre: map architecture"):
  - Units are cm in the engine, metres in Tools/PointeSombre/island.json. X is north, Y is east, Z is up.
  - Every actor a cell makes goes through tk.own(actor, cell): outliner folder "Cells/<cell>", tags "Cell:<cell>"
    (plus any extra tags), so a rebuild, a report, or a reviewer can find it.
  - Exterior ground height: tk.ground_z(x_cm, y_cm) samples the same heightfield the terrain tiles are built from.
  - Interior cells are built at tk.interior_origin(cell) and passed through tk.make_interior(actor): lighting channel
    1 only (the exterior sun, moon, and lightning are on channel 0), and a draw distance so the exterior never pays
    for them.
  - Portals: tk.portal(...) with tk.portal_variant(...); destinations are anchors (tk.anchor(name)) or the cell's own
    markers (tk.marker(...)).
"""
import math

import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
CYLINDER = "/Game/LevelPrototyping/Meshes/SM_Cylinder"
MAT_FLAT = "/Game/LevelPrototyping/Materials/M_FlatCol"
ENV = "/Game/Environment/Materials"
SOMBRE_MATERIALS = "/Game/World/PointeSombre/Materials"
INTERIOR_DRAW_DISTANCE_CM = 12000.0   # an interior cell is drawn only from within 120 m (it is 2.5 km from the island)

# Set by the core script (configure()) before any cell hook runs.
island = None              # pointe_sombre.island.Island
_anchors = {}              # name -> actor
_slots = {}                # interior cell -> origin (cm)

cube_mesh = unreal.load_asset(CUBE)
_bounds = cube_mesh.get_bounding_box()
CUBE_MIN = _bounds.min
CUBE_SIZE = _bounds.max - _bounds.min
CUBE_CENTER = (_bounds.max + _bounds.min) * 0.5


def log(msg):
    unreal.log_warning("[DCSOMBRE] " + msg)


def configure(island_obj, anchors, slots):
    global island, _anchors, _slots
    island = island_obj
    _anchors = dict(anchors)
    _slots = dict(slots)


# --- Ownership

def own(actor, cell, *tags):
    actor.set_folder_path(f"Cells/{cell}")
    existing = [str(t) for t in actor.get_editor_property("tags")]
    wanted = existing + [t for t in (f"Cell:{cell}",) + tags if t not in existing]
    actor.set_editor_property("tags", [unreal.Name(t) for t in wanted])
    return actor


def anchor(name):
    if name not in _anchors:
        raise RuntimeError(f"No anchor {name}. Anchors are created by build_pointe_sombre.py (ANCHORS); ask the Integrator.")
    return _anchors[name]


def interior_origin(cell):
    if cell not in _slots:
        raise RuntimeError(f"No interior slot for {cell}. Slots are assigned in build_pointe_sombre.py (INTERIOR_SLOTS).")
    return _slots[cell]


def ground_z(x_cm, y_cm):
    """Terrain height (cm) at a map point, from the same heightfield as the terrain tiles."""
    return island.height(x_cm / 100.0, y_cm / 100.0) * 100.0


# --- Materials

def surface(name):
    asset = unreal.load_asset(f"{ENV}/{name}")
    if not asset:
        raise RuntimeError(f"Missing {ENV}/{name}. Run import_art.py first.")
    return asset


def blockout():
    """The default for a box with no material: a plain mid-dark grey (the prototype grid materials read as unfinished
    and the review capture reports them)."""
    return flat("MI_DC_Sombre_Block", (0.16, 0.16, 0.16, 1.0))


def interactable_material():
    """Greybox interactables (doors, portals, test props): a plain painted grey-green, not the prototype colourway
    (which the review capture reports as a default material)."""
    return flat("MI_DC_Sombre_Greybox", (0.20, 0.23, 0.21, 1.0), roughness=0.85)


def flat(name, rgba, roughness=0.9):
    """A flat-colour child of M_FlatCol in the Pointe Sombre materials folder. Use it for faces the triplanar surfaces
    light wrongly: a ceiling or any face looking down gets the triplanar top projection's upward normal and renders
    unlit from below (seen on the VS-04 fixture's ceiling). Made once per run."""
    if name in _flat_made:
        return _flat_made[name]
    path = f"{SOMBRE_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        if not unreal.EditorAssetLibrary.does_directory_exist(SOMBRE_MATERIALS):
            unreal.EditorAssetLibrary.make_directory(SOMBRE_MATERIALS)
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, SOMBRE_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, unreal.load_asset(MAT_FLAT))
    mel.set_material_instance_vector_parameter_value(mi, "Base Color", unreal.LinearColor(*rgba))
    mel.set_material_instance_scalar_parameter_value(mi, "Roughness", roughness)
    mel.set_material_instance_scalar_parameter_value(mi, "Metallic", 0.0)
    mel.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    return mi


def tinted_surface(name, parent_name, tint, tile_cm=None):
    """A Pointe Sombre child of one of the shore's surface instances, with its own tint (and tile size)."""
    path = f"{SOMBRE_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        if not unreal.EditorAssetLibrary.does_directory_exist(SOMBRE_MATERIALS):
            unreal.EditorAssetLibrary.make_directory(SOMBRE_MATERIALS)
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, SOMBRE_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, surface(parent_name))
    mel.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(tint[0], tint[1], tint[2], 1.0))
    if tile_cm:
        mel.set_material_instance_scalar_parameter_value(mi, "TileSizeCm", tile_cm)
    mel.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    _flat_made[name] = mi
    return mi


_flat_made = {}

GLOW_MASTER = f"{ENV}/M_DC_Glow"   # the shore's unlit, translucent glow: emissive = Color.rgb, opacity = Color.a


def glow(name, rgba):
    """A Pointe Sombre instance of M_DC_Glow (lamps, lanterns, windows). Values above 1 bloom; distance needs more."""
    if name in _flat_made:
        return _flat_made[name]
    path = f"{SOMBRE_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, SOMBRE_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, unreal.load_asset(GLOW_MASTER))
    mel.set_material_instance_vector_parameter_value(mi, "Color", unreal.LinearColor(*rgba))
    mel.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    _flat_made[name] = mi
    return mi


# --- Geometry

def _rotate(v, pitch=0.0, yaw=0.0, roll=0.0):
    """Rotate v by an Unreal rotator (same matrix as FRotationMatrix)."""
    p, y, r = math.radians(pitch), math.radians(yaw), math.radians(roll)
    sp, cp, sy, cy, sr, cr = math.sin(p), math.cos(p), math.sin(y), math.cos(y), math.sin(r), math.cos(r)
    ax = (cp * cy, cp * sy, sp)
    ay = (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, -sr * cp)
    az = (-(cr * sp * cy + sr * sy), cy * sr - cr * sp * sy, cr * cp)
    return unreal.Vector(v.x * ax[0] + v.y * ay[0] + v.z * az[0],
                         v.x * ax[1] + v.y * ay[1] + v.z * az[1],
                         v.x * ax[2] + v.y * ay[2] + v.z * az[2])


def box(label, cell, center, size, rot=(0.0, 0.0, 0.0), material=None, actor_class=unreal.StaticMeshActor,
        hidden=False, collision=True):
    """A cube scaled to size (cm), centred on center, rotated (pitch, yaw, roll) about that centre."""
    scale = unreal.Vector(size[0] / CUBE_SIZE.x, size[1] / CUBE_SIZE.y, size[2] / CUBE_SIZE.z)
    scaled_center = unreal.Vector(CUBE_CENTER.x * scale.x, CUBE_CENTER.y * scale.y, CUBE_CENTER.z * scale.z)
    offset = _rotate(scaled_center, *rot)
    location = unreal.Vector(center[0], center[1], center[2]) - offset
    actor = actors.spawn_actor_from_class(actor_class, location, unreal.Rotator(pitch=rot[0], yaw=rot[1], roll=rot[2]))
    actor.set_actor_scale3d(scale)
    actor.set_actor_label(label)
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(cube_mesh)
    chosen = material or blockout()
    for slot in range(comp.get_num_materials()):
        comp.set_material(slot, chosen)
    if hidden:
        comp.set_visibility(False)
    if not collision:
        comp.set_collision_profile_name("NoCollision")
    return own(actor, cell)


def block(label, cell, x0, x1, y0, y1, z0, z1, **kwargs):
    return box(label, cell, ((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2), (x1 - x0, y1 - y0, z1 - z0), **kwargs)


def wall_segment(label, cell, a, b, z0, z1, thickness=40.0, **kwargs):
    """A wall standing on the segment a->b (cm, XY), from z0 to z1."""
    dx, dy = b[0] - a[0], b[1] - a[1]
    length = math.hypot(dx, dy)
    yaw = math.degrees(math.atan2(dy, dx))
    center = ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2, (z0 + z1) / 2)
    return box(label, cell, center, (length + thickness, thickness, z1 - z0), rot=(0.0, yaw, 0.0), **kwargs)


def movable(actor):
    root = actor.get_editor_property("root_component")
    if root:
        root.set_mobility(unreal.ComponentMobility.MOVABLE)
    return actor


def _channels(c0, c1):
    channels = unreal.LightingChannels()
    channels.set_editor_property("channel0", c0)
    channels.set_editor_property("channel1", c1)
    return channels


def make_interior(actor):
    """For the fixed parts of an interior cell: lighting channel 1 only (out of reach of the exterior sun, moon, and
    lightning, which are channel 0 and, with shadows off at low spec, would otherwise light a sealed room), and drawn
    only from nearby. A light gets channels 0 and 1, so it also lights the player's arms and anyone who walks in:
    characters stay on channel 0 because presence moves them between the exterior and the cells."""
    for comp in actor.get_components_by_class(unreal.PrimitiveComponent):
        comp.set_editor_property("lighting_channels", _channels(False, True))
        comp.set_editor_property("ld_max_draw_distance", INTERIOR_DRAW_DISTANCE_CM)
    for comp in actor.get_components_by_class(unreal.LightComponent):
        comp.set_editor_property("lighting_channels", _channels(True, True))
    return actor


def marker(label, cell, location, yaw=0.0, *tags):
    """An empty, named point: a portal's arrival, a respawn point, a reference for tests."""
    actor = actors.spawn_actor_from_class(unreal.TargetPoint, unreal.Vector(*location),
                                          unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_label(label)
    return own(actor, cell, *tags)


# --- Rules

def cond(type_name, id=None, stage=None, negate=False, quantity=1):
    c = unreal.DCGameplayCondition()
    c.set_editor_property("type", getattr(unreal.DCConditionType, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("stage", unreal.Name(stage) if stage else unreal.Name())
    c.set_editor_property("negate", negate)
    c.set_editor_property("quantity", quantity)
    return c


def flag(id, negate=False):
    return cond("WORLD_FLAG", id=id, negate=negate)


def cons(type_name, id=None, quantity=1):
    """A consequence. GIVE_ITEM / REMOVE_ITEM of an item defined in Tools/ContentSpecs/items also set the asset
    reference (as the content specs do), so the map holds a hard reference and the item is always loaded with it."""
    c = unreal.DCGameplayConsequence()
    c.set_editor_property("type", getattr(unreal.DCConsequenceType, type_name))
    c.set_editor_property("id", unreal.Name(id) if id else unreal.Name())
    c.set_editor_property("quantity", quantity)
    if type_name in ("GIVE_ITEM", "REMOVE_ITEM"):
        path = _item_paths().get(id)
        if not path:
            raise RuntimeError(f"No item '{id}' in Tools/ContentSpecs/items (ask the Integrator for a ledger id)")
        c.set_editor_property("item", unreal.load_asset(path))
    return c


_items = None


def _item_paths():
    """{item_id: asset path} from the content specs (the same loader the item generator uses)."""
    global _items
    if _items is None:
        import os
        import sys
        scripts = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
        if scripts not in sys.path:
            sys.path.insert(0, scripts)
        import content_specs
        _items = content_specs.item_paths()
    return _items


# --- Portals

def portal_variant(variant_id, verb, conditions=(), consequences=()):
    v = unreal.DCPortalVariant()
    v.set_editor_property("variant_id", unreal.Name(variant_id))
    v.set_editor_property("verb", unreal.Text(verb))
    v.set_editor_property("conditions", list(conditions))
    v.set_editor_property("consequences", list(consequences))
    return v


def portal(label, cell, center, size, display_name, variants, destination, locked_text=None, locked_verb=None,
           card_text=None, material=None, yaw=0.0):
    """A cell portal whose mesh is a box (the door leaf, hatch, or stair) of size cm, centred on center."""
    actor = box(label, cell, center, size, rot=(0.0, yaw, 0.0), material=material or interactable_material(),
                actor_class=unreal.DCCellPortal)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    actor.set_editor_property("variants", list(variants))
    actor.set_editor_property("destination", destination)
    if locked_text:
        actor.set_editor_property("locked_text", unreal.Text(locked_text))
    if locked_verb:
        actor.set_editor_property("locked_verb", unreal.Text(locked_verb))
    if card_text:
        actor.set_editor_property("card_text", unreal.Text(card_text))
    return own(actor, cell, "CellPortal")


# --- Presence

def placement(location, rotation=(0.0, 0.0, 0.0)):
    rot = rotation if isinstance(rotation, unreal.Rotator) else unreal.Rotator(
        pitch=rotation[0], yaw=rotation[1], roll=rotation[2])
    loc = location if isinstance(location, unreal.Vector) else unreal.Vector(*location)
    return unreal.Transform(location=loc, rotation=rot, scale=unreal.Vector(1.0, 1.0, 1.0))


def state(state_id, conditions, present=True, place=None):
    s = unreal.DCPresenceState()
    s.set_editor_property("state_id", unreal.Name(state_id))
    s.set_editor_property("conditions", list(conditions))
    s.set_editor_property("present", present)
    if place is not None:
        s.set_editor_property("move", True)
        s.set_editor_property("placement", place)
    return s


def presence(name, cell, targets, states, pivot=None, defer=True):
    """A conditional presence rule. Its pivot is pivot (cm) or the first target's actor origin; a state's placement
    is where the pivot goes, and the targets keep their offsets from it."""
    first = targets[0]
    where = unreal.Vector(*pivot) if pivot is not None else first.get_actor_location()
    rule = actors.spawn_actor_from_class(unreal.DCConditionalPresence, where, unreal.Rotator(0.0, 0.0, 0.0))
    rule.set_actor_label(f"Presence_{name}")
    for target in targets:
        movable(target)
    rule.set_editor_property("targets", list(targets))
    rule.set_editor_property("states", list(states))
    rule.set_editor_property("defer_while_observed", defer)
    log(f"presence {name}: {len(targets)} targets, states {[str(s.get_editor_property('state_id')) for s in states]}")
    return own(rule, cell, f"Presence_{name}")


# --- Lights, volumes

def point_light(label, cell, location, color, candelas, radius, shadows=False):
    actor = actors.spawn_actor_from_class(unreal.PointLight, unreal.Vector(*location), unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(label)
    comp = actor.get_component_by_class(unreal.PointLightComponent)
    comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    comp.set_editor_property("intensity", candelas)
    comp.set_editor_property("attenuation_radius", radius)
    comp.set_editor_property("light_color", unreal.Color(r=color[0], g=color[1], b=color[2], a=255))
    comp.set_editor_property("cast_shadows", shadows)
    return own(actor, cell)


def post_process_box(label, cell, center, extent, priority, settings_fn, blend_radius=0.0):
    """A bounded post-process volume (cm). settings_fn(settings) sets the overrides."""
    actor = actors.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(*center))
    actor.set_actor_label(label)
    actor.set_editor_property("unbound", False)
    actor.set_editor_property("priority", priority)
    actor.set_editor_property("blend_weight", 1.0)
    actor.set_editor_property("blend_radius", blend_radius)
    # The default brush is a 200 cm cube.
    actor.set_actor_scale3d(unreal.Vector(extent[0] / 100.0, extent[1] / 100.0, extent[2] / 100.0))
    settings = actor.get_editor_property("settings")
    settings_fn(settings)
    actor.set_editor_property("settings", settings)
    return own(actor, cell)


def location_volume(label, cell, location_id, display_name, center, extent):
    volume = actors.spawn_actor_from_class(unreal.DCLocationVolume, unreal.Vector(*center))
    volume.set_actor_label(label)
    volume.set_editor_property("location_id", location_id)
    volume.set_editor_property("display_name", unreal.Text(display_name))
    volume.get_editor_property("bounds").set_box_extent(unreal.Vector(*extent))
    return own(volume, cell)


# --- Story actors (VS-10): inspectables, people, lights and sounds that follow the rules. The same setup as the
# accepted shore scripts (build_boathouse.py), so a cell builder never sets engine properties directly.

def _vec(v):
    return v if isinstance(v, unreal.Vector) else unreal.Vector(*v)


def set_persistent_id(actor, persistent_id):
    """Saved actors (NPCs, doors, containers, pickups) carry the ledger's persistent id (Design/POIs/sombre_ids.md §7)."""
    comp = actor.get_component_by_class(unreal.DCPersistentIdComponent)
    if not comp:
        raise RuntimeError(f"{actor.get_actor_label()} has no persistent id component")
    comp.set_editor_property("persistent_id", persistent_id)
    return actor


def variant(description, conditions=(), consequences=(), action=None):
    """One inspect variant (first match wins; the actor's own description is the fallback)."""
    v = unreal.DCInspectVariant()
    v.set_editor_property("description", unreal.Text(description))
    v.set_editor_property("conditions", list(conditions))
    v.set_editor_property("consequences", list(consequences))
    if action:
        v.set_editor_property("action", unreal.Text(action))
    return v


def inspectable(label, cell, center, size, display_name, description, variants=(), action=None, duration=None,
                material=None, rot=(0.0, 0.0, 0.0), hidden=False):
    """An inspectable whose mesh is a box (or, hidden=True, an invisible box around the dressing that shows it)."""
    actor = box(label, cell, center, size, rot=rot, material=material or interactable_material(),
                actor_class=unreal.DCInspectableActor, hidden=hidden)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    actor.set_editor_property("description", unreal.Text(description))
    if variants:
        actor.set_editor_property("variants", list(variants))
    if action:
        actor.set_editor_property("action", unreal.Text(action))
    if duration:
        actor.set_editor_property("description_duration", duration)
    return own(actor, cell, "Inspectable")


SURVIVAL = "/Game/Survival_Character"
SURVIVAL_MESH = SURVIVAL + "/Meshes/SK_Survival_Character"
QUINN_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"
ANIM_BLUEPRINTS = ("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed",
                   "/Game/Characters/Mannequins/Anims/Manny/ABP_Manny")
SURVIVAL_JACKET_SLOT, SURVIVAL_JEANS_SLOT, SURVIVAL_EYE_SLOT = 7, 8, 3
NPC_HALF_HEIGHT = 96.0   # a standing character's capsule centre above the ground (as placed on the shore)


def costume(name, part, tint):
    """A Pointe Sombre tint of the Survival_Character pack's jacket or jeans (part "Jacket" or "Jeans"), made the way
    import_art.py makes the shore's costumes. A costume, not a decision about anyone's face or history."""
    path = f"{SOMBRE_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, SOMBRE_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, unreal.load_asset(f"{SURVIVAL}/Materials/MI_Survival_Character_{part}"))
    mel.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(tint[0], tint[1], tint[2], 1.0))
    mel.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    return mi


def npc(label, cell, location, yaw, persistent_id, display_name, dialogue, body="survival", jacket=None, jeans=None,
        paints=None, greeting=None, notice_range=None):
    """A friendly NPC whose capsule centre stands at location (cm: ground + NPC_HALF_HEIGHT), with its dialogue (a
    /Game/Dialogue asset name) and the ledger's persistent id.
    body "survival": the Survival_Character pack body, with jacket and jeans tints (tk.costume).
    body "quinn": the one-piece Quinn mannequin with a flat paint per slot (Mara's accepted placeholder, Phase 5)."""
    actor = actors.spawn_actor_from_class(unreal.DCFriendlyNPC, _vec(location),
                                          unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_label(label)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    asset = unreal.load_asset(f"/Game/Dialogue/{dialogue}")
    if not asset:
        raise RuntimeError(f"Missing /Game/Dialogue/{dialogue}. Run create_dialogue.py first.")
    actor.set_editor_property("dialogue", asset)
    if greeting:
        actor.set_editor_property("greeting", unreal.Text(greeting))
    if notice_range is not None:
        actor.set_editor_property("notice_range", notice_range)
    mesh_comp = actor.get_editor_property("mesh")
    mesh_path = SURVIVAL_MESH if body == "survival" else QUINN_MESH
    mesh_asset = unreal.load_asset(mesh_path)
    if not mesh_asset:
        raise RuntimeError(f"Missing {mesh_path}")
    mesh_comp.set_skeletal_mesh_asset(mesh_asset)
    if body == "survival":
        if jacket:
            mesh_comp.set_material(SURVIVAL_JACKET_SLOT, jacket)
        if jeans:
            mesh_comp.set_material(SURVIVAL_JEANS_SLOT, jeans)
        mesh_comp.set_material(SURVIVAL_EYE_SLOT, surface("MI_DC_Eye"))
    for slot, paint in enumerate(paints or ()):
        mesh_comp.set_material(slot, paint)
    for path in ANIM_BLUEPRINTS:
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            mesh_comp.set_animation_mode(unreal.AnimationMode.ANIMATION_BLUEPRINT)
            mesh_comp.set_anim_class(unreal.EditorAssetLibrary.load_blueprint_class(path))
            break
    set_persistent_id(actor, persistent_id)
    return own(actor, cell, "NPC", f"NPC:{persistent_id}")


def flicker_light(label, cell, location, color, candelas, radius, conditions=(), glow_cm=0.0, glow_material=None,
                  min_brightness=0.25, dropout=0.12, interval=(0.05, 0.6)):
    """An ADCFlickerLight: a point light (and an optional glowing cube) that burns while its conditions pass."""
    actor = actors.spawn_actor_from_class(unreal.DCFlickerLight, _vec(location), unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(label)
    light = actor.get_editor_property("light")
    light.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    light.set_editor_property("intensity", candelas)
    light.set_editor_property("light_color", unreal.Color(r=color[0], g=color[1], b=color[2], a=255))
    light.set_editor_property("attenuation_radius", radius)
    if glow_cm > 0.0:
        glow = actor.get_editor_property("glow")
        scale = unreal.Vector(glow_cm / CUBE_SIZE.x, glow_cm / CUBE_SIZE.y, glow_cm / CUBE_SIZE.z)
        glow.set_static_mesh(cube_mesh)
        glow.set_material(0, glow_material or surface("MI_DC_GlowLantern"))
        glow.set_editor_property("relative_scale3d", scale)
        glow.set_editor_property("relative_location", unreal.Vector(
            -CUBE_CENTER.x * scale.x, -CUBE_CENTER.y * scale.y, -CUBE_CENTER.z * scale.z))
    actor.set_editor_property("min_brightness", min_brightness)
    actor.set_editor_property("dropout_chance", dropout)
    actor.set_editor_property("min_interval", interval[0])
    actor.set_editor_property("max_interval", interval[1])
    if conditions:
        actor.set_editor_property("active_conditions", list(conditions))
    return own(actor, cell, "FlickerLight")


def conditional_audio(label, cell, location, sound_path, volume, conditions=(), attenuation=None, once=False):
    """An ADCConditionalAudio: a sound that plays while (or once when) its conditions pass. It sets no flag.
    attenuation None is a 2D sound; otherwise the name of an attenuation asset in /Game/Audio."""
    actor = actors.spawn_actor_from_class(unreal.DCConditionalAudio, _vec(location), unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(label)
    sound = unreal.load_asset(sound_path)
    if not sound:
        raise RuntimeError(f"Missing {sound_path}. Run import_audio.py first.")
    actor.set_editor_property("sound", sound)
    if attenuation:
        att = unreal.load_asset(f"/Game/Audio/{attenuation}")
        if not att:
            raise RuntimeError(f"Missing /Game/Audio/{attenuation}. Run import_audio.py first.")
        actor.set_editor_property("attenuation", att)
    actor.set_editor_property("volume_multiplier", volume)
    actor.set_editor_property("mode", unreal.DCConditionalAudioMode.ONCE_WHEN_TRUE if once
                              else unreal.DCConditionalAudioMode.WHILE_TRUE)
    if conditions:
        actor.set_editor_property("conditions", list(conditions))
    return own(actor, cell, "ConditionalAudio")


def damage_volume(label, cell, center, size, display_name, damage_per_second, conditions=(), material=None,
                  rot=(0.0, 0.0, 0.0)):
    """An ADCDamageVolume (a hazard such as live water): hurts whoever stands in it while its conditions pass. The
    box is its visible surface; pass material=None for an invisible one over dressing that shows the hazard."""
    actor = box(label, cell, center, size, rot=rot, material=material or blockout(), actor_class=unreal.DCDamageVolume,
                hidden=material is None)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    actor.set_editor_property("damage_per_second", damage_per_second)
    if conditions:
        actor.set_editor_property("active_conditions", list(conditions))
    return own(actor, cell, "Hazard")


def loot_container(label, cell, center, size, display_name, persistent_id, contents, material=None,
                   rot=(0.0, 0.0, 0.0)):
    """An ADCLootContainer with the ledger's persistent id and its starting stacks: contents [(item_id, quantity)],
    item ids from Tools/ContentSpecs/items (the slice's loot reuses ammo_9mm, field_dressing, salvage_wiring)."""
    actor = box(label, cell, center, size, rot=rot, material=material or interactable_material(),
                actor_class=unreal.DCLootContainer)
    actor.set_editor_property("display_name", unreal.Text(display_name))
    stacks = []
    for item_id, quantity in contents:
        path = _item_paths().get(item_id)
        if not path:
            raise RuntimeError(f"No item '{item_id}' in Tools/ContentSpecs/items")
        stack = unreal.DCItemStack()
        stack.set_editor_property("item", unreal.load_asset(path))
        stack.set_editor_property("quantity", quantity)
        stacks.append(stack)
    actor.get_component_by_class(unreal.DCInventoryComponent).set_editor_property("stacks", stacks)
    set_persistent_id(actor, persistent_id)
    return own(actor, cell, "Container")
