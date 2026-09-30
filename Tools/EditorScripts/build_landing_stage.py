"""The Landing Stage (shore.landing_stage), Phase 5's production pilot POI.

Spec: Design/POIs/shore.landing_stage.md. Plan: WorldStatePhasePlan.txt.

This file owns only the stage. build_boathouse.py calls build(<its own helpers>) once, near the end of its main(),
after the rest of the persistent map exists; the stage's actors are in the persistent map (folder LandingStage, tag
LandingStage) because its presence rules reference them, and a rule cannot reference another level. Static dressing
with no state is in the art level (dress_landing.py). Do not run this file on its own.

Layout (cm; +X out of the boathouse door, the lake is -Y; beach Z 0, lake surface Z -6):
  gangway  X 950..1070,  Y -575..-820, top Z 24     from the beach, over the curb, onto the stage
  deck     X 850..1270,  Y -820..-1120, top Z 30    lantern post at the north-west corner, lean-to over the crate
  skiff    about (1060, -1205), alongside the deck's south edge; adrift about (1720, -2080), yaw 35
  Mara     coil route only: on the deck at (930, -960)

State comes only from existing flags (first match wins; no new flag, nothing saved):
  combat  shore.path_cleared      (Shore Watch done_killed)  lantern out, card gone, crate emptied, skiff cut adrift
  coil    shore.relay_recovered   (Shore Watch done_coil)    crate packed and lashed, skiff loaded, Mara and her pack here
  default neither                                           lantern lit, crate half packed, skiff moored
  power   wreck.power_cut (additive)                        the bulbs go dark and quiet
"""
import unreal

LOCATION_ID = "shore.landing_stage"
LOCATION_NAME = "Landing Stage"
TACKLE_ID = "landing.tackle"
PATH_CLEARED = "shore.path_cleared"        # Shore Watch finished by the kill route
RELAY_RECOVERED = "shore.relay_recovered"  # Shore Watch finished by the coil route (or a coil handed over after)
POWER_CUT = "wreck.power_cut"              # the Survey Launch's battery leads pulled
FOLDER = "LandingStage"
TAG = "LandingStage"

CRATE_MESHES = "/Game/Art/PolyHaven/wooden_crate_01/wooden_crate_01_2k/StaticMeshes"
LANTERN_MESH = "/Game/Art/PolyHaven/Lantern_01/Lantern_01_2k/StaticMeshes/Lantern_01"
LANTERN_GLASS = "/Game/Art/PolyHaven/Lantern_01/Lantern_01_2k/StaticMeshes/Lantern_01_glass"
CAN_MESH = "/Game/Art/PolyHaven/can_rusted/can_rusted_2k/StaticMeshes/can_rusted_2k"
HUM = "/Game/Audio/Ambience/S_DC_HumLiveWater"

# --- Narrative Builder: inspect text. All PROVISIONAL (spec, Environmental Story). Nothing here explains the
# Current, names who closed the landing, or says where anyone is going.
TEXT_CRATE = ("A supply crate, half packed: tins, a blanket rolled tight, room left for more. "
              "Somebody is getting ready to go somewhere by water.")
TEXT_CRATE_COIL = ("Packed, the lid nailed down, and lashed the way you lash a load for a boat. "
                   "Whoever packed it knew how things shift in a swell.")
TEXT_CRATE_COMBAT = "Pried open and empty. The lid lies on the boards where it was thrown."
TEXT_LANTERN = "A storm lantern, trimmed low. Lit for someone."
TEXT_LANTERN_COMBAT = ("The lantern is cold. The wick was pinched out, not burned down. "
                       "The card that hung under it is gone; the tack is still in the post.")
TEXT_CARD = "A card tacked to the post, pencil gone soft in the rain: DON'T TIE UP AFTER DARK UNLESS THE LAMP IS LIT."
TEXT_BULBS = ("Bare bulbs on a cable that runs off the end of the stage and down into the water. "
              "They hum. There is no generator.")
TEXT_BULBS_CUT = "The bulbs are dark. The cable still runs down into the water."
TEXT_SKIFF = "A plank skiff, seams tarred, oars shipped. Tied off short, the way you tie a boat you mean to use."
TEXT_SKIFF_COIL = "The skiff rides lower now: a bundle under a tarp amidships, a water can wedged in the bow."
TEXT_SKIFF_COMBAT = "The skiff turns slowly out on the open water, empty, a length of line trailing from its bow."
TEXT_CUT_LINE = "The mooring line ends a hand's width from the cleat. Cut, not frayed."

# --- Layout.
DECK_TOP = 30.0
DECK = (850.0, 1270.0, -1120.0, -820.0)          # x0, x1, y0, y1
GANGWAY = (950.0, 1070.0, -820.0, -575.0)
CRATE_AT = (1180.0, -930.0)
LANTERN_POST = (865.0, -835.0)
LANTERN_POST_HEIGHT = 175.0
# Moored along the deck's west side, the side that faces the boathouse door, so it reads from the path.
SKIFF_AT = (765.0, -985.0, 38.0)                  # bounds center; the floor stays above the lake sheet
SKIFF_YAW = 90.0
SKIFF_ADRIFT = ((1720.0, -2080.0, 18.0), (0.0, 35.0, 0.0))  # deeper than moored: no draft reads as hovering (V-01)
MARA_ON_STAGE = ((960.0, -990.0, DECK_TOP + 96.0), (0.0, 100.0, 0.0))
PACK_AT_LOOKOUT = (3205.0, 1335.0, 0.0)
PACK_ON_STAGE = (1085.0, -895.0, DECK_TOP)
CLEAT_AT = (858.0, -930.0, DECK_TOP + 3.0)

BOAT_MESH = "/Game/Smugglers_cove/meshes/ships/SM_boat_dutch_small_02"
PLANK_MESH = "/Game/Smugglers_cove/meshes/structures/SM_wooden_pier_planks"
CYLINDER_MESH = "/Game/LevelPrototyping/Meshes/SM_Cylinder"
BULB_Z = DECK_TOP + 205.0  # the bulb string hangs above a standing player's head (capsule top about DECK_TOP + 188)

T = None  # build_boathouse.py's helpers, set by build()


def log(msg):
    unreal.log_warning("[DCLANDING] " + msg)


def combat():
    return [T.cond("WORLD_FLAG", id=PATH_CLEARED)]


def coil():
    return [T.cond("WORLD_FLAG", id=RELAY_RECOVERED)]


def tag(actor, *extra):
    actor.set_editor_property("tags", [unreal.Name(TAG)] + [unreal.Name(t) for t in extra])
    return actor


def movable(actor):
    root = actor.get_editor_property("root_component")
    if root:
        root.set_mobility(unreal.ComponentMobility.MOVABLE)
    return actor


def no_collision(actor):
    for comp in actor.get_components_by_class(unreal.PrimitiveComponent):
        comp.set_collision_profile_name("NoCollision")
    return actor


def flat(name, rgba, roughness=0.9):
    """A flat-colour instance of M_FlatCol. Under the shore's exposure these read far lighter than their values, so
    cloth and timber are authored dark; high roughness keeps a tarp from mirroring the sky."""
    mi = T.material_instance(name, T.MAT_FLAT, {"Base Color": rgba})
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi, "Roughness", roughness)
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi, "Metallic", 0.0)
    unreal.MaterialEditingLibrary.update_material_instance(mi)
    unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False)
    return mi


def dressing_box(label, center, size, material, rot=None):
    """A NoCollision greybox part. rot is (pitch, yaw, roll)."""
    if rot:
        actor = T.box_rot(label, FOLDER, center, size, rot=rot, material=material)
    else:
        actor = T.box(label, FOLDER, center, size, material=material)
    return tag(no_collision(actor))


def mesh_actor(label, mesh, scale=1.0):
    actor = T.actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0.0, 0.0, 0.0),
                                            unreal.Rotator(pitch=0.0, yaw=0.0, roll=0.0))
    actor.set_actor_label(label)
    actor.set_folder_path(FOLDER)
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(mesh)
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    return tag(actor)


def roll(label, center, length, diameter, material, yaw=0.0):
    """A NoCollision cylinder lying on its side: a rolled blanket, a duffel, a bedroll."""
    mesh = unreal.load_asset(CYLINDER_MESH)
    bounds = mesh.get_bounding_box()
    ext = bounds.max - bounds.min
    actor = no_collision(mesh_actor(label, mesh))
    actor.set_actor_scale3d(unreal.Vector(diameter / ext.x, diameter / ext.y, length / ext.z))
    actor.get_component_by_class(unreal.StaticMeshComponent).set_material(0, material)
    pose(actor, center, (90.0, yaw, 0.0))
    return actor


def pose(actor, center, rotation):
    """Rotate, then move so the actor's bounds center lands on center. Returns (location, rotator)."""
    actor.set_actor_rotation(unreal.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]), False)
    origin, _extent = actor.get_actor_bounds(False)
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(loc.x + center[0] - origin.x, loc.y + center[1] - origin.y,
                                           loc.z + center[2] - origin.z), False, True)
    return actor.get_actor_location(), actor.get_actor_rotation()


def rest_on(actor, z):
    """Drop so the bounds bottom sits on z."""
    origin, extent = actor.get_actor_bounds(False)
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(loc.x, loc.y, loc.z - (origin.z - extent.z) + z), False, True)


def placement(location, rotation):
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


def presence(name, targets, states, pivot_on_origin=False):
    """A conditional presence rule pivoting on the first target, so a state's placement is simply where that first
    target goes (the others keep their offsets from it). The pivot is the first target's bounds center (greybox
    boxes have their origin at a corner), or its actor origin with pivot_on_origin (Mara, the crate lid)."""
    first = targets[0]
    if pivot_on_origin:
        where = first.get_actor_location()
    else:
        where, _extent = first.get_actor_bounds(False)
    rule = T.actors.spawn_actor_from_class(unreal.DCConditionalPresence, where, first.get_actor_rotation())
    rule.set_actor_label(f"Presence_{name}")
    rule.set_folder_path(FOLDER)
    tag(rule, "LandingPresence", f"Presence_{name}")
    for target in targets:
        movable(target)
    rule.set_editor_property("targets", list(targets))
    rule.set_editor_property("states", list(states))
    log(f"presence {name}: {len(targets)} targets, states {[str(s.get_editor_property('state_id')) for s in states]}")
    return rule


# --- Pieces.

def planks(label, x0, x1, y0, y1, top, sections_x=1, sections_y=1, yaw=0.0):
    """Visual decking from the pier kit's plank section, stretched to fill the rectangle. NoCollision: the walkable
    surface is the hidden block under it, so the kit's own collision can never trip the player."""
    mesh = unreal.load_asset(PLANK_MESH)
    bounds = mesh.get_bounding_box()
    ext = bounds.max - bounds.min
    w = (x1 - x0) / sections_x
    d = (y1 - y0) / sections_y
    along_x, along_y = (d, w) if yaw else (w, d)
    for i in range(sections_x):
        for j in range(sections_y):
            section = no_collision(mesh_actor(f"{label}_{i}{j}", mesh))
            section.set_actor_scale3d(unreal.Vector(along_x / ext.x, along_y / ext.y, 1.0))
            pose(section, (x0 + w * (i + 0.5), y0 + d * (j + 0.5), top - ext.z / 2.0), (0.0, yaw, 0.0))


def walkable(label, x0, x1, y0, y1, z0, z1, material):
    """The walkable collision block: a dark bed 3 cm under the plank tops, so the gaps between the kit's boards show
    timber underneath rather than open water. The planks on top are what the player reads."""
    return tag(T.block(label, FOLDER, x0, x1, y0, y1, z0, z1 - 3.0, material=material))


def build_structure(timber, pile, tarp):
    x0, x1, y0, y1 = DECK
    walkable("Landing_Deck", x0, x1, y0, y1, DECK_TOP - 12.0, DECK_TOP, timber)
    planks("Landing_DeckPlanks", x0, x1, y0, y1, DECK_TOP, sections_x=2)
    gx0, gx1, gy0, gy1 = GANGWAY
    walkable("Landing_Gangway", gx0, gx1, gy0, gy1, 12.0, 24.0, timber)
    planks("Landing_GangwayPlanks", gx0, gx1, gy0, gy1, 24.0, yaw=90.0)
    for index, (x, y) in enumerate([(x0 + 10, y0 + 10), (x1 - 10, y0 + 10), (x0 + 10, y1 - 10), (x1 - 10, y1 - 10)]):
        dressing_box(f"Landing_Pile_{index}", (x, y, (DECK_TOP - 12.0 - 90.0) / 2.0), (18, 18, 90 + DECK_TOP - 12.0), pile)
    # The lean-to over the crate: four posts and a tarp roof falling toward the water.
    # The front posts are tall enough to carry the bulb string above head height.
    for index, (x, y, h) in enumerate([(1120, -845, 218), (1262, -845, 218), (1120, -1005, 165), (1262, -1005, 165)]):
        dressing_box(f"Landing_LeanToPost_{index}", (x, y, DECK_TOP + h / 2.0), (9, 9, h), pile)
    dressing_box("Landing_LeanToRoof", (1191.0, -925.0, DECK_TOP + 194.0), (170, 185, 4), tarp, rot=(0.0, 0.0, 16.0))
    # The lantern post at the head of the gangway, tall enough that the lantern shows against the water from the path.
    dressing_box("Landing_LanternPost", (LANTERN_POST[0], LANTERN_POST[1], DECK_TOP + LANTERN_POST_HEIGHT / 2.0),
                 (12, 12, LANTERN_POST_HEIGHT), pile)


def build_lantern_and_card():
    top = DECK_TOP + LANTERN_POST_HEIGHT
    lantern = T.box("Landing_Lantern", FOLDER, (LANTERN_POST[0], LANTERN_POST[1], top + 22.0), (20, 16, 44),
                    actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(lantern, "Storm lantern", TEXT_LANTERN,
                        variants=[T.variant(TEXT_LANTERN_COMBAT, combat())])
    T.wear_mesh(lantern, unreal.load_asset(LANTERN_MESH), 44.0, (LANTERN_POST[0], LANTERN_POST[1], top + 22.0))
    rest_on(lantern, top)
    tag(lantern)
    glass = mesh_actor("Landing_LanternGlass", unreal.load_asset(LANTERN_GLASS))
    glass.set_actor_transform(lantern.get_actor_transform(), False, False)
    no_collision(glass)
    glow = T.material_instance("MI_DC_GlowLantern", T.MAT_GLOW, {"Color": (7.0, 3.4, 0.8, 1.0)})
    origin, _extent = glass.get_actor_bounds(False)
    light = T.flicker_light("Landing_LanternLight", FOLDER, (origin.x, origin.y, origin.z), (255, 168, 90), 90.0, 1200.0,
                            glow_cm=8.0, glow_material=glow, min_brightness=0.82, dropout=0.0, interval=(0.08, 0.3))
    tag(light)
    card = T.box("Landing_Card", FOLDER, (LANTERN_POST[0], LANTERN_POST[1] + 7.0, DECK_TOP + 105.0), (16, 1.2, 11),
                 material=flat("MI_DC_Card", (0.30, 0.28, 0.22, 1.0)), actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(card, "Card", TEXT_CARD, duration=6.0)
    tag(card)
    return light, card


def build_crate(cloth, rope):
    x, y = CRATE_AT
    body_mesh = unreal.load_asset(f"{CRATE_MESHES}/wooden_crate_01")
    lid_mesh = unreal.load_asset(f"{CRATE_MESHES}/wooden_crate_01_lid")
    latch_mesh = unreal.load_asset(f"{CRATE_MESHES}/wooden_crate_01_latch")
    crate = T.box("Landing_Crate", FOLDER, (x, y, DECK_TOP + 22.0), (107, 55, 44), actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(crate, "Crate", TEXT_CRATE, variants=[
        T.variant(TEXT_CRATE_COMBAT, combat()),
        T.variant(TEXT_CRATE_COIL, coil()),
    ])
    T.wear_mesh(crate, body_mesh, 107.0, (x, y, DECK_TOP + 22.0))
    rest_on(crate, DECK_TOP)
    tag(crate)
    scale = crate.get_actor_scale3d().x
    closed = crate.get_actor_transform()  # body, lid, and latch share one origin: the lid here is shut

    latch = mesh_actor("Landing_CrateLatch", latch_mesh, scale)
    latch.set_actor_transform(closed, False, False)
    no_collision(latch)

    origin, extent = crate.get_actor_bounds(False)
    lid = mesh_actor("Landing_CrateLid", lid_mesh, scale)
    no_collision(lid)
    # Combat: thrown flat on the boards, resting just above the plank tops so no edge dips into them (V-08).
    pose(lid, (1005.0, -1050.0, DECK_TOP + 3.0), (0.0, 28.0, 0.0))
    rest_on(lid, DECK_TOP + 1.0)
    thrown = (lid.get_actor_location(), lid.get_actor_rotation())
    # Default: stood against the crate's west end, its face toward the gangway, so it reads as a lid (V-05).
    pose(lid, (origin.x - extent.x - 9.0, y, DECK_TOP + 24.0), (0.0, 90.0, -72.0))
    rest_on(lid, DECK_TOP + 0.5)

    # Contents, seen with the lid off: a folded tarp filling the bottom, three tins and a rolled blanket on top of it,
    # high enough to show over the rim (V-05).
    can = unreal.load_asset(CAN_MESH)
    fill_top = DECK_TOP + 22.0
    contents = [dressing_box("Landing_CrateFill", (x, y, (DECK_TOP + 3.0 + fill_top) / 2.0),
                             (extent.x * 2.0 - 14.0, extent.y * 2.0 - 12.0, fill_top - DECK_TOP - 3.0), cloth)]
    for index, (dx, dy) in enumerate([(-34.0, -8.0), (-18.0, 7.0), (-6.0, -9.0)]):
        tin = no_collision(mesh_actor(f"Landing_CrateTin_{index}", can, 1.1))
        pose(tin, (x + dx, y + dy, fill_top + 9.0), (0.0, 25.0 * index, 0.0))
        rest_on(tin, fill_top)
        contents.append(tin)
    contents.append(roll("Landing_CrateBlanket", (x + 24.0, y, fill_top + 10.0), 46.0, 20.0,
                         flat("MI_DC_BlanketRoll", (0.10, 0.035, 0.03, 1.0)), yaw=90.0))

    # Packed for a boat: two rope bands round the shut crate, thick and dark enough to read against the lid (V-06).
    lashing = [dressing_box(f"Landing_CrateLashing_{index}", (x + dx, origin.y, origin.z),
                            (5.0, extent.y * 2.0 + 3.0, extent.z * 2.0 + 3.0), rope)
               for index, dx in enumerate([-30.0, 30.0])]
    return lid, closed, thrown, contents, lashing


def build_bulbs(cable_mat):
    """The bulb string hangs along the lean-to's front eave, the side that faces the door and the path, so the
    power cut reads as a change from there (V-04). The cable drops off the east end into the lake."""
    glow = T.material_instance("MI_DC_GlowBulb", T.MAT_GLOW, {"Color": (11.0, 12.0, 15.0, 1.0)})
    x0, x1, y = 1120.0, 1262.0, -841.0
    dressing_box("Landing_BulbCable", ((x0 + x1) / 2.0, y, BULB_Z + 9.0), (x1 - x0, 2, 2), cable_mat)
    dressing_box("Landing_BulbCableDrop", (x1 + 8.0, y, (BULB_Z + 9.0 - 30.0) / 2.0), (2, 2, BULB_Z + 39.0), cable_mat)
    unpowered = [T.cond("WORLD_FLAG", id=POWER_CUT, negate=True)]
    count = 5
    for index in range(count):
        bx = x0 + 12.0 + (x1 - x0 - 24.0) * index / (count - 1)
        light = T.flicker_light(f"Landing_Bulb_{index}", FOLDER, (bx, y, BULB_Z), (205, 222, 255),
                                45.0 if index == count // 2 else 0.0, 700.0, glow_cm=9.0, glow_material=glow,
                                conditions=unpowered, min_brightness=0.6, dropout=0.05, interval=(0.05, 0.5))
        tag(light)
    bulbs = T.box("Landing_Bulbs", FOLDER, ((x0 + x1) / 2.0, y, BULB_Z + 4.0), (x1 - x0, 10, 14),
                  actor_class=unreal.DCInspectableActor, hidden=True)
    T.setup_inspectable(bulbs, "Bulbs", TEXT_BULBS, variants=[T.variant(TEXT_BULBS_CUT, [T.cond("WORLD_FLAG", id=POWER_CUT)])])
    tag(bulbs)
    hum = T.conditional_audio("Landing_BulbHum", FOLDER, ((x0 + x1) / 2.0, y, BULB_Z), HUM, 0.08,
                              conditions=unpowered, attenuation="SA_DC_Hum")
    tag(hum)


def build_skiff(timber_dark, rope, rust, tarp):
    sx, sy, sz = SKIFF_AT
    skiff = T.box("Landing_Skiff", FOLDER, (sx, sy, sz), (130, 400, 60), material=timber_dark,
                  actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(skiff, "Skiff", TEXT_SKIFF, variants=[
        T.variant(TEXT_SKIFF_COMBAT, combat()),
        T.variant(TEXT_SKIFF_COIL, coil()),
    ])
    boat = unreal.load_asset(BOAT_MESH)
    if boat:
        # Library rowing boat (Smugglers_cove, migrated at 1K), cut to a 4 m skiff. It sits high enough that the
        # lake sheet does not show through its floor.
        T.wear_mesh(skiff, boat, 400.0, (sx, sy, sz), yaw=SKIFF_YAW)
    tag(skiff)
    cleat = dressing_box("Landing_Cleat", CLEAT_AT, (6, 18, 6), rust)
    line = dressing_box("Landing_MooringLine", (CLEAT_AT[0] - 22.0, CLEAT_AT[1] + 12.0, DECK_TOP + 6.0), (48, 3, 3), rope,
                        rot=(-8.0, -28.0, 0.0))
    stub = T.box("Landing_CutLine", FOLDER, (CLEAT_AT[0] - 5.0, CLEAT_AT[1], DECK_TOP - 9.0), (4, 4, 22),
                 material=rope, actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(stub, "Cut line", TEXT_CUT_LINE)
    tag(stub)
    water_can = no_collision(mesh_actor("Landing_SkiffWaterCan", unreal.load_asset(CAN_MESH), 2.2))
    pose(water_can, (sx, sy + 115.0, sz), (0.0, 25.0, 0.0))
    rest_on(water_can, sz - 40.0)
    cargo = [dressing_box("Landing_SkiffBundle", (sx, sy - 35.0, sz - 30.0), (62, 95, 32), tarp), water_can]
    return skiff, line, stub, cargo, cleat


def build_tackle(rust):
    tackle = T.box("Landing_Tackle", FOLDER, (905.0, -1082.0, DECK_TOP + 11.0), (46, 26, 22), material=rust,
                   actor_class=unreal.DCLootContainer)
    T.setup_container(tackle, "Tackle box", TACKLE_ID, [(T.ITEM_AMMO, 6), (T.ITEM_WIRING, 1)])
    return tag(tackle)


def build_pack(cloth_dark, cloth):
    """Mara's pack: a canvas duffel with a bedroll strapped along the top. Two pieces that move as one. The owned
    library has no backpack mesh; whether to generate one is parked for Anthony (V-06)."""
    x, y, z = PACK_AT_LOOKOUT
    return [
        roll("Landing_MaraPack", (x, y, z + 16.0), 62.0, 32.0, cloth_dark),
        roll("Landing_MaraBedroll", (x, y, z + 40.0), 50.0, 16.0, cloth),
    ]


def build_discovery():
    volume = T.actors.spawn_actor_from_class(unreal.DCLocationVolume, unreal.Vector(1060.0, -875.0, 175.0))
    volume.set_actor_label("Location_LandingStage")
    volume.set_folder_path(FOLDER)
    volume.set_editor_property("location_id", LOCATION_ID)
    volume.set_editor_property("display_name", unreal.Text(LOCATION_NAME))
    volume.get_editor_property("bounds").set_box_extent(unreal.Vector(230.0, 285.0, 225.0))
    return tag(volume)


def build(map_tools):
    """Called by build_boathouse.py with its own helpers (box, inspectable, cond, flicker_light, ...)."""
    global T
    T = map_tools

    timber = flat("MI_DC_Timber", (0.05, 0.04, 0.03, 1.0))
    timber_dark = flat("MI_DC_TimberDark", (0.03, 0.025, 0.02, 1.0))
    pile = flat("MI_DC_Pile", (0.025, 0.021, 0.018, 1.0))
    tarp = flat("MI_DC_Tarp", (0.030, 0.036, 0.026, 1.0))
    rope = flat("MI_DC_Rope", (0.05, 0.038, 0.022, 1.0))
    cloth = flat("MI_DC_Blanket", (0.08, 0.05, 0.035, 1.0))
    cloth_dark = flat("MI_DC_PackCanvas", (0.035, 0.040, 0.025, 1.0))
    cable = flat("MI_DC_Cable", (0.02, 0.02, 0.02, 1.0))
    rust = T.surface("MI_DC_RustPaint")

    build_structure(timber, pile, tarp)
    lantern_light, card = build_lantern_and_card()
    lid, closed, thrown, contents, lashing = build_crate(cloth, rope)
    build_bulbs(cable)
    skiff, line, stub, cargo, _cleat = build_skiff(timber_dark, rope, rust, tarp)
    build_tackle(rust)
    pack = build_pack(cloth_dark, cloth)
    build_discovery()

    mara = T.actor_by_label("Mara")

    # The rules. Combat is listed first everywhere, so killing him and then handing over the coil anyway stays
    # the combat picture (a drifted skiff does not come back). Nothing here writes a flag or saves anything.
    presence("OpenLanding", [lantern_light, card, line], [state("combat", combat(), present=False)])
    presence("CrateLid", [lid], pivot_on_origin=True, states=[
        state("combat", combat(), place=placement(*thrown)),
        state("coil", coil(), place=placement(closed.translation, closed.rotation.rotator())),
    ])
    presence("CrateContents", contents, [state("combat", combat(), present=False), state("coil", coil(), present=False)])
    presence("Packed", lashing + cargo, [
        state("combat", combat(), present=False),
        state("coil", coil()),
        state("default", [], present=False),
    ])
    presence("CutLine", [stub], [state("combat", combat()), state("default", [], present=False)])
    presence("Skiff", [skiff], [state("combat", combat(), place=placement(*SKIFF_ADRIFT))])
    presence("Mara", [mara], [state("combat", combat()), state("coil", coil(), place=placement(*MARA_ON_STAGE))],
             pivot_on_origin=True)
    pack_rule = presence("MaraPack", pack, [
        state("combat", combat()),
        state("coil", coil(), place=placement((PACK_ON_STAGE[0], PACK_ON_STAGE[1], PACK_ON_STAGE[2] + 16.0), (90.0, 15.0, 0.0))),  # lying, like at the lookout
    ])
    log(f"built {LOCATION_ID}: pack pivot {pack_rule.get_actor_location()}")
