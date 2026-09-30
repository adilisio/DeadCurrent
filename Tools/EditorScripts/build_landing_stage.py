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
SKIFF_AT = (1060.0, -1205.0, 8.0)
SKIFF_ADRIFT = ((1720.0, -2080.0, 8.0), (0.0, 35.0, 0.0))
MARA_ON_STAGE = ((930.0, -960.0, DECK_TOP + 96.0), (0.0, 60.0, 0.0))
PACK_AT_LOOKOUT = (3205.0, 1335.0, 0.0)
PACK_ON_STAGE = (1085.0, -895.0, DECK_TOP)
CLEAT_AT = (1180.0, -1116.0, DECK_TOP + 3.0)

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


def flat(name, rgba):
    return T.material_instance(name, T.MAT_FLAT, {"Base Color": rgba})


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

def build_structure(timber, pile, tarp):
    x0, x1, y0, y1 = DECK
    tag(T.block("Landing_Deck", FOLDER, x0, x1, y0, y1, DECK_TOP - 12.0, DECK_TOP, material=timber))
    gx0, gx1, gy0, gy1 = GANGWAY
    tag(T.block("Landing_Gangway", FOLDER, gx0, gx1, gy0, gy1, 12.0, 24.0, material=timber))
    for index, (x, y) in enumerate([(x0 + 10, y0 + 10), (x1 - 10, y0 + 10), (x0 + 10, y1 - 10), (x1 - 10, y1 - 10)]):
        dressing_box(f"Landing_Pile_{index}", (x, y, (DECK_TOP - 12.0 - 90.0) / 2.0), (18, 18, 90 + DECK_TOP - 12.0), pile)
    # The lean-to over the crate: four posts and a tarp roof falling toward the water.
    for index, (x, y, h) in enumerate([(1120, -845, 190), (1262, -845, 190), (1120, -1005, 150), (1262, -1005, 150)]):
        dressing_box(f"Landing_LeanToPost_{index}", (x, y, DECK_TOP + h / 2.0), (9, 9, h), pile)
    dressing_box("Landing_LeanToRoof", (1191.0, -925.0, DECK_TOP + 172.0), (170, 185, 4), tarp, rot=(0.0, 0.0, 13.0))
    # The lantern post at the head of the gangway.
    dressing_box("Landing_LanternPost", (LANTERN_POST[0], LANTERN_POST[1], DECK_TOP + 52.0), (12, 12, 104), pile)


def build_lantern_and_card():
    lantern = T.box("Landing_Lantern", FOLDER, (LANTERN_POST[0], LANTERN_POST[1], DECK_TOP + 122.0), (16, 14, 36),
                    actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(lantern, "Storm lantern", TEXT_LANTERN,
                        variants=[T.variant(TEXT_LANTERN_COMBAT, combat())])
    T.wear_mesh(lantern, unreal.load_asset(LANTERN_MESH), 36.0, (LANTERN_POST[0], LANTERN_POST[1], DECK_TOP + 122.0))
    rest_on(lantern, DECK_TOP + 104.0)
    tag(lantern)
    glass = mesh_actor("Landing_LanternGlass", unreal.load_asset(LANTERN_GLASS))
    glass.set_actor_transform(lantern.get_actor_transform(), False, False)
    no_collision(glass)
    glow = T.material_instance("MI_DC_GlowLantern", T.MAT_GLOW, {"Color": (3.2, 1.6, 0.35, 1.0)})
    origin, _extent = glass.get_actor_bounds(False)
    light = T.flicker_light("Landing_LanternLight", FOLDER, (origin.x, origin.y, origin.z), (255, 168, 90), 45.0, 900.0,
                            glow_cm=5.0, glow_material=glow, min_brightness=0.82, dropout=0.0, interval=(0.08, 0.3))
    tag(light)
    card = T.box("Landing_Card", FOLDER, (LANTERN_POST[0], LANTERN_POST[1] + 7.0, DECK_TOP + 78.0), (16, 1.2, 11),
                 material=flat("MI_DC_Card", (0.62, 0.58, 0.48, 1.0)), actor_class=unreal.DCInspectableActor)
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

    lid = mesh_actor("Landing_CrateLid", lid_mesh, scale)
    no_collision(lid)
    pose(lid, (1005.0, -1050.0, DECK_TOP + 3.0), (0.0, 28.0, 4.0))  # combat: thrown down on the boards
    rest_on(lid, DECK_TOP)
    thrown = (lid.get_actor_location(), lid.get_actor_rotation())
    pose(lid, (x, y + 36.0, DECK_TOP + 24.0), (0.0, 0.0, -74.0))  # default: leaning on the crate's north face
    rest_on(lid, DECK_TOP)

    # Contents, seen with the lid off: two tins and a rolled blanket.
    can = unreal.load_asset(CAN_MESH)
    contents = []
    for index, (dx, dy) in enumerate([(-30.0, -6.0), (-12.0, 8.0)]):
        tin = no_collision(mesh_actor(f"Landing_CrateTin_{index}", can, 1.1))
        pose(tin, (x + dx, y + dy, DECK_TOP + 12.0), (0.0, 20.0 * index, 0.0))
        rest_on(tin, DECK_TOP + 3.0)
        contents.append(tin)
    contents.append(dressing_box("Landing_CrateBlanket", (x + 25.0, y, DECK_TOP + 14.0), (46, 20, 20), cloth))

    # Packed for a boat: two rope bands round the shut crate.
    origin, extent = crate.get_actor_bounds(False)
    lashing = [dressing_box(f"Landing_CrateLashing_{index}", (x + dx, origin.y, origin.z),
                            (3.0, extent.y * 2.0 + 2.0, extent.z * 2.0 + 2.0), rope)
               for index, dx in enumerate([-28.0, 28.0])]
    return lid, closed, thrown, contents, lashing


def build_bulbs(cable_mat):
    glow = T.material_instance("MI_DC_GlowBulb", T.MAT_GLOW, {"Color": (2.6, 3.0, 3.8, 1.0)})
    dressing_box("Landing_BulbCable", (1272.0, -978.0, DECK_TOP + 168.0), (2, 270, 2), cable_mat, rot=(0.0, 0.0, 4.0))
    dressing_box("Landing_BulbCableDrop", (1276.0, -1117.0, (DECK_TOP + 160.0 - 6.0) / 2.0), (2, 2, DECK_TOP + 166.0),
                 cable_mat)
    unpowered = [T.cond("WORLD_FLAG", id=POWER_CUT, negate=True)]
    for index, y in enumerate([-900.0, -978.0, -1056.0]):
        light = T.flicker_light(f"Landing_Bulb_{index}", FOLDER, (1272.0, y, DECK_TOP + 160.0), (205, 222, 255),
                                18.0 if index == 1 else 0.0, 500.0, glow_cm=4.5, glow_material=glow,
                                conditions=unpowered, min_brightness=0.55, dropout=0.06, interval=(0.05, 0.5))
        tag(light)
    bulbs = T.box("Landing_Bulbs", FOLDER, (1272.0, -978.0, DECK_TOP + 162.0), (10, 190, 12),
                  actor_class=unreal.DCInspectableActor, hidden=True)
    T.setup_inspectable(bulbs, "Bulbs", TEXT_BULBS, variants=[T.variant(TEXT_BULBS_CUT, [T.cond("WORLD_FLAG", id=POWER_CUT)])])
    tag(bulbs)
    hum = T.conditional_audio("Landing_BulbHum", FOLDER, (1272.0, -978.0, DECK_TOP + 160.0), HUM, 0.08,
                              conditions=unpowered, attenuation="SA_DC_Hum")
    tag(hum)


def build_skiff(timber_dark, rope, rust, tarp):
    sx, sy, sz = SKIFF_AT
    skiff = T.box("Landing_Skiff", FOLDER, (sx, sy, sz), (380, 130, 40), material=timber_dark,
                  actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(skiff, "Skiff", TEXT_SKIFF, variants=[
        T.variant(TEXT_SKIFF_COMBAT, combat()),
        T.variant(TEXT_SKIFF_COIL, coil()),
    ])
    tag(skiff)
    cleat = dressing_box("Landing_Cleat", CLEAT_AT, (18, 6, 6), rust)
    line = dressing_box("Landing_MooringLine", (1192.0, -1133.0, DECK_TOP - 4.0), (3, 38, 3), rope, rot=(0.0, 18.0, 20.0))
    stub = T.box("Landing_CutLine", FOLDER, (CLEAT_AT[0], CLEAT_AT[1] - 4.0, DECK_TOP - 10.0), (4, 4, 22),
                 material=rope, actor_class=unreal.DCInspectableActor)
    T.setup_inspectable(stub, "Cut line", TEXT_CUT_LINE)
    tag(stub)
    cargo = [
        dressing_box("Landing_SkiffBundle", (sx - 30.0, sy, sz + 26.0), (95, 70, 34), tarp),
        dressing_box("Landing_SkiffWaterCan", (sx + 120.0, sy + 10.0, sz + 26.0), (20, 32, 34), rust),
    ]
    return skiff, line, stub, cargo, cleat


def build_tackle(rust):
    tackle = T.box("Landing_Tackle", FOLDER, (905.0, -1082.0, DECK_TOP + 11.0), (46, 26, 22), material=rust,
                   actor_class=unreal.DCLootContainer)
    T.setup_container(tackle, "Tackle box", TACKLE_ID, [(T.ITEM_AMMO, 6), (T.ITEM_WIRING, 1)])
    return tag(tackle)


def build_pack(cloth_dark):
    return dressing_box("Landing_MaraPack", (PACK_AT_LOOKOUT[0], PACK_AT_LOOKOUT[1], PACK_AT_LOOKOUT[2] + 22.0),
                        (40, 28, 44), cloth_dark)


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

    timber = flat("MI_DC_Timber", (0.19, 0.15, 0.11, 1.0))
    timber_dark = flat("MI_DC_TimberDark", (0.11, 0.09, 0.07, 1.0))
    pile = flat("MI_DC_Pile", (0.08, 0.07, 0.06, 1.0))
    tarp = flat("MI_DC_Tarp", (0.13, 0.15, 0.12, 1.0))
    rope = flat("MI_DC_Rope", (0.36, 0.30, 0.21, 1.0))
    cloth = flat("MI_DC_Blanket", (0.24, 0.19, 0.15, 1.0))
    cloth_dark = flat("MI_DC_PackCanvas", (0.16, 0.17, 0.12, 1.0))
    cable = flat("MI_DC_Cable", (0.02, 0.02, 0.02, 1.0))
    rust = T.surface("MI_DC_RustPaint")

    build_structure(timber, pile, tarp)
    lantern_light, card = build_lantern_and_card()
    lid, closed, thrown, contents, lashing = build_crate(cloth, rope)
    build_bulbs(cable)
    skiff, line, stub, cargo, _cleat = build_skiff(timber_dark, rope, rust, tarp)
    build_tackle(rust)
    pack = build_pack(cloth_dark)
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
    pack_rule = presence("MaraPack", [pack], [
        state("combat", combat()),
        state("coil", coil(), place=placement((PACK_ON_STAGE[0], PACK_ON_STAGE[1], PACK_ON_STAGE[2] + 22.0), (0.0, 15.0, 0.0))),
    ])
    log(f"built {LOCATION_ID}: pack pivot {pack_rule.get_actor_location()}")
