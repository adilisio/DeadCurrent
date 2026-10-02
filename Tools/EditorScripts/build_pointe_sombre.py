"""Build /Game/Maps/Lvl_PointeSombre, the Phase 6 slice map (VerticalSlicePhasePlan.txt §5). The core script.

Integrator-owned. It regenerates the persistent map from scratch on every run:
  1. the terrain tiles from Tools/PointeSombre/island.json (pointe_sombre/island.py, pointe_sombre/terrain_mesh.py)
  2. the water, the walls along the walkable outline, the nav bounds
  3. the three atmospheres (dusk storm, calm evening, night storm), each one presence rule (see ATMOSPHERES)
  4. the player start on the crossing deck and the respawn rule that moves it as the story moves on
  5. the shared anchors (ANCHORS): named, tagged points that portals owned by different cells meet at
  6. one build(tk) hook per cell script in Tools/EditorScripts/pointe_sombre/ (CELLS), in order
  7. Tools/PointeSombre/out/terrain_probe.json: seeded points and their expected ground height, for the map test

The map frame, the cell-placement rule, and why the terrain is a generated mesh are recorded in
Design/technical_architecture.md, "Pointe Sombre: map architecture". Hand edits to the map are lost on the next run.

Run with Tools\\RebuildContent.bat build_pointe_sombre (also part of a full rebuild). Requires import_art.py,
import_audio.py, and the cell scripts. Units are cm here (the island data is in metres); X is north, Y is east.
"""
import importlib
import json
import os
import random
import sys

import unreal

SCRIPTS = os.path.dirname(os.path.abspath(__file__))
SOMBRE_SCRIPTS = os.path.join(SCRIPTS, "pointe_sombre")
if SOMBRE_SCRIPTS not in sys.path:
    sys.path.insert(0, SOMBRE_SCRIPTS)

import island as island_mod  # noqa: E402
import layout as layout_mod  # noqa: E402
import terrain_mesh  # noqa: E402
import toolkit as tk  # noqa: E402

MAP_PATH = "/Game/Maps/Lvl_PointeSombre"
TERRAIN_FOLDER = "/Game/World/PointeSombre/Terrain"
AUDIO_FOLDER = "/Game/World/PointeSombre/Audio"
PROBE_FILE = os.path.normpath(os.path.join(SCRIPTS, "..", "PointeSombre", "out", "terrain_probe.json"))
CORE = "Core"   # the outliner folder (Cells/Core) and tag (Cell:Core) of everything this script makes itself

# Cell scripts, called in this order with the toolkit. Each owns only its own actors (Cells/<cell>, Cell:<cell>).
# arch_test is the Integrator's architecture fixture (VS-04); greybox is the Integrator's exterior greybox (VS-08,
# Checkpoint A), which each cell replaces piece by piece (see retire_greybox). VS-10: crossing (the deck), harbor (the
# quay's people), headland (so far only the false light seen from the deck).
CELLS = ["crossing", "harbor", "headland", "greybox", "arch_test"]

# Interior cells are built far from the island (2.5 km east, 400 m up), 150 m apart: out of every exterior sightline,
# out of the exterior fog, ambience, and lightning, and drawn only from inside (toolkit.make_interior).
INTERIOR_SLOTS = {
    "arch_test": (0.0, 250000.0, 40000.0),
    "vault": (15000.0, 250000.0, 40000.0),
    "net_loft": (30000.0, 250000.0, 40000.0),
}

# Inactive atmosphere sets are parked here, hidden: nowhere a camera goes.
PARKING = (0.0, -400000.0, -200000.0)

# Story flags the core reads. They are the slice documents' own ids (Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md,
# VerticalSlicePhasePlan.txt §5.7, §8.2), recorded in the id ledger. Never rename one once shipped.
REEF_STRUCK = "sombre.reef_struck"     # the crossing is over: the player is on the island
VAULT_OPENED = "sombre.vault_opened"   # the vault has been opened by any route
HALE_ARRIVED = "sombre.hale_arrived"   # the storm has passed (set with knows)
MEETING_DONE = "sombre.meeting_done"   # the net loft has decided: the second storm, at night


def place(name):
    x, y = ISLAND.places[name]
    return x * 100.0, y * 100.0


def on_ground(name, up=100.0):
    x, y = place(name)
    return x, y, tk.ground_z(x, y) + up


# Shared anchors: (name, location cm, yaw, what meets there). Portals owned by different cells find these by tag
# ("Anchor:<name>"); a cell never references another cell's actors. The full list is in Design/POIs/sombre_ids.md.
def anchors_table():
    qx, qy, qz = on_ground("quay_arrival")
    deck = DECK_TOP
    shipped = [
        ("Anchor_NewGame_Deck", (DECK_CENTER[0], DECK_CENTER[1], deck + 100.0), 25.0,
         "the player start on the Ida's deck (crossing); the respawn rule's default place"),
        ("Anchor_CrossingExit_Quay", (qx, qy, qz), 40.0,
         "where the crossing's wheelhouse door lands the player: the quay, facing up the island"),
        ("Anchor_Respawn_TowerBase", on_ground("tower_base"), 200.0,
         "respawn once the vault is open (lighthouse cell builds the base around it)"),
    ]
    # VS-08: every anchor reserved in the ledger (Design/POIs/sombre_ids.md §9), placed from Tools/PointeSombre/greybox.json.
    lay = layout_mod.Layout.load(ISLAND)
    reserved = [(name, loc, yaw, "reserved in the ledger" + (" (held for VS-20)" if held else ""))
                for name, loc, yaw, held in lay.anchors(INTERIOR_SLOTS, deck=(DECK_CENTER, DECK_TOP, 25.0))]
    return shipped + reserved


# The Ida's deck: the crossing cell builds on it, the core needs it for the player start.
DECK_CENTER = (-33000.0, -36000.0)
DECK_TOP = 300.0


def editor_world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def package_name(obj):
    outer = obj.get_outermost() if obj else None
    return outer.get_name() if outer else ""


def destroy_persistent_actors():
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    persistent = levels.get_current_level()
    if package_name(persistent) != MAP_PATH:
        raise RuntimeError(f"Refusing to rebuild: current level package is '{package_name(persistent)}'")
    doomed = []
    for actor in tk.actors.get_all_level_actors():
        if package_name(actor.get_outer()) != MAP_PATH:
            continue
        if isinstance(actor, unreal.WorldSettings) or (
                isinstance(actor, unreal.Brush) and not isinstance(actor, unreal.Volume)):
            continue
        doomed.append(actor)
    tk.actors.destroy_actors(doomed)
    unreal.SystemLibrary.collect_garbage()
    tk.log(f"cleared {len(doomed)} persistent actors")


# --- Terrain, water, walls

def build_terrain():
    materials = {
        "turf": tk.tinted_surface("MI_DC_Sombre_Turf", "MI_DC_LandRock", (0.78, 0.84, 0.66), tile_cm=520.0),
        "rock": tk.tinted_surface("MI_DC_Sombre_Rock", "MI_DC_CoastRock", (0.70, 0.72, 0.75), tile_cm=420.0),
        # Wet dark stone: the VS-04 tint (0.95) was never seen (the band was 1.4 cm deep until the VS-08 unit fix) and read
        # as white sand once it showed. On the gravel surface, even darkened, the band mirrored the sky and read as a
        # second sheet of water (and as the white fringe along the causeway), so it is the coast rock, rough and dark.
        "shore": tk.tinted_surface("MI_DC_Sombre_Shingle", "MI_DC_CoastRock", (0.40, 0.39, 0.37), tile_cm=140.0),
        "seabed": tk.tinted_surface("MI_DC_Sombre_Seabed", "MI_DC_Mud", (0.35, 0.36, 0.34)),
        # VS-08: the trails (island.json "paths"), trodden earth and grit, so the ground changes underfoot.
        "path": tk.tinted_surface("MI_DC_Sombre_Path", "MI_DC_Mud", (0.46, 0.43, 0.39), tile_cm=260.0),
    }
    tiles = terrain_mesh.ensure_tiles(ISLAND, TERRAIN_FOLDER, materials)
    for path, origin in tiles:
        actor = tk.actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(origin[0], origin[1], 0.0))
        actor.set_actor_label(path.rsplit("/", 1)[-1])
        actor.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(unreal.load_asset(path))
        tk.own(actor, CORE, "SombreTerrain")
    tk.log(f"placed {len(tiles)} terrain tiles")


def build_water():
    # A thin opaque sheet (M_DC_Lake), like the shore's. 4 km square, centred on the island; the interior slots are
    # beyond its edge. NoCollision: the walls along the walkable outline keep the player out of deep water.
    tk.box("Water", CORE, (-2000.0, -2000.0, -11.0), (400000.0, 400000.0, 10.0),
           material=tk.surface("MI_DC_OpenLake"), collision=False)


def build_walls():
    points = [(x * 100.0, y * 100.0) for x, y in ISLAND.bounds]
    height = ISLAND.spec["bounds"]["height"] * 100.0
    for i, a in enumerate(points):
        b = points[(i + 1) % len(points)]
        wall = tk.own(tk.wall_segment(f"Edge_{i:02d}", CORE, a, b, -300.0, height, hidden=True), CORE, "SombreEdge")
        # Stops the player, not sight lines (InvisibleWall ignores the Visibility channel): an invisible fence must not
        # block a landmark trace or the view from the crossing (VS-08).
        wall.get_component_by_class(unreal.StaticMeshComponent).set_collision_profile_name("InvisibleWall")
    # VS-08 containment: a hidden floor just under the water across the grid. Where the shore is a cliff the sea floor
    # drops past drop_below within a few metres and has no triangles, so a player who stepped off the Pointe fell
    # through the world (FellOutOfWorld, not the respawn rule; Codex VS-04 review). Now they land in the shallows,
    # inside the fence, and walk out. The walls rise above the tower's gallery (island.json bounds.height).
    g = ISLAND.spec["grid"]
    floor_top = (float(g["drop_below"]) - 0.05) * 100.0
    tk.own(tk.block("SafetyFloor", CORE, g["x_min"] * 100.0, g["x_max"] * 100.0, g["y_min"] * 100.0, g["y_max"] * 100.0,
                    floor_top - 50.0, floor_top, hidden=True), CORE, "SombreSafetyFloor")
    tk.log(f"walls along {len(points)} outline points, {height / 100.0:.0f} m tall; safety floor at {floor_top / 100.0:.2f} m")


def build_nav():
    g = ISLAND.spec["grid"]
    cx, cy = (g["x_min"] + g["x_max"]) * 50.0, (g["y_min"] + g["y_max"]) * 50.0
    vol = tk.actors.spawn_actor_from_class(unreal.NavMeshBoundsVolume, unreal.Vector(cx, cy, 2000.0))
    vol.set_actor_label("NavBounds")
    vol.set_actor_scale3d(unreal.Vector((g["x_max"] - g["x_min"]) / 2.0, (g["y_max"] - g["y_min"]) / 2.0, 40.0))
    tk.own(vol, CORE)
    unreal.SystemLibrary.execute_console_command(editor_world(), "RebuildNavigation")


# --- Atmosphere

def _sun(label, rot, lux, color, temperature):
    sun = tk.actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 5000.0),
                                           unreal.Rotator(pitch=rot[0], yaw=rot[1], roll=0.0))
    sun.set_actor_label(label)
    comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    comp.set_editor_property("atmosphere_sun_light", True)
    comp.set_editor_property("forward_shading_priority", 1)
    comp.set_editor_property("intensity", lux)
    comp.set_editor_property("light_color", unreal.Color(r=color[0], g=color[1], b=color[2], a=255))
    comp.set_editor_property("use_temperature", True)
    comp.set_editor_property("temperature", temperature)
    return tk.own(sun, CORE)


def _fill(label, sun_rot, lux, color):
    """The shore's answer to no GI at low spec (build_boathouse.py SkyFill): a dim, shadowless directional light
    from opposite the sun, standing in for sky bounce on the faces the sun does not reach. Not an atmosphere sun."""
    fill = tk.actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0.0, 0.0, 5200.0),
                                            unreal.Rotator(pitch=-26.0, yaw=sun_rot[1] + 180.0, roll=0.0))
    fill.set_actor_label(label)
    comp = fill.get_component_by_class(unreal.DirectionalLightComponent)
    comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    comp.set_editor_property("atmosphere_sun_light", False)
    comp.set_editor_property("forward_shading_priority", 0)
    comp.set_editor_property("cast_shadows", False)
    comp.set_editor_property("intensity", lux)
    comp.set_editor_property("light_color", unreal.Color(r=color[0], g=color[1], b=color[2], a=255))
    return tk.own(fill, CORE)


def _sky(label, rayleigh, rayleigh_scale, mie_scale, luminance):
    atmo = tk.actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0.0, 0.0, 0.0))
    atmo.set_actor_label(label)
    comp = atmo.get_component_by_class(unreal.SkyAtmosphereComponent)
    comp.set_editor_property("rayleigh_scattering", unreal.LinearColor(*rayleigh, 1.0))
    comp.set_editor_property("rayleigh_scattering_scale", rayleigh_scale)
    comp.set_editor_property("mie_scattering_scale", mie_scale)
    comp.set_editor_property("sky_luminance_factor", unreal.LinearColor(*luminance, 1.0))
    return tk.own(atmo, CORE)


def _fog(label, density, max_opacity, inscatter, falloff=0.2):
    fog = tk.actors.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0.0, 0.0, 0.0))
    fog.set_actor_label(label)
    comp = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    comp.set_editor_property("fog_density", density)
    comp.set_editor_property("fog_max_opacity", max_opacity)
    comp.set_editor_property("fog_height_falloff", falloff)
    comp.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(*inscatter, 1.0))
    return tk.own(fog, CORE)


def _grade(bias, min_b, max_b, saturation, contrast, gain=(1.0, 1.0, 1.0), white_temp=6500.0):
    def apply(settings):
        settings.set_editor_property("override_auto_exposure_method", True)
        settings.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
        settings.set_editor_property("override_auto_exposure_min_brightness", True)
        settings.set_editor_property("auto_exposure_min_brightness", min_b)
        settings.set_editor_property("override_auto_exposure_max_brightness", True)
        settings.set_editor_property("auto_exposure_max_brightness", max_b)
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", bias)
        settings.set_editor_property("override_white_temp", True)
        settings.set_editor_property("white_temp", white_temp)
        settings.set_editor_property("override_color_saturation", True)
        settings.set_editor_property("color_saturation", unreal.Vector4(*saturation, 1.0))
        settings.set_editor_property("override_color_contrast", True)
        settings.set_editor_property("color_contrast", unreal.Vector4(contrast, contrast, contrast, 1.0))
        settings.set_editor_property("override_color_gain", True)
        settings.set_editor_property("color_gain", unreal.Vector4(*gain, 1.0))
    return apply


def _lightning(label, look, conditions):
    """A huge, mostly-dark point light high over the island: ADCFlickerLight with world ActiveConditions."""
    light = tk.actors.spawn_actor_from_class(unreal.DCFlickerLight, unreal.Vector(-4000.0, -3000.0, 25000.0),
                                             unreal.Rotator(0.0, 0.0, 0.0))
    light.set_actor_label(label)
    comp = light.get_editor_property("light")
    comp.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
    comp.set_editor_property("intensity", 4.0e7)
    comp.set_editor_property("attenuation_radius", 90000.0)
    comp.set_editor_property("light_color", unreal.Color(r=205, g=215, b=255, a=255))
    light.set_editor_property("min_brightness", 0.55)
    light.set_editor_property("dropout_chance", 0.94)
    light.set_editor_property("min_interval", 0.05)
    light.set_editor_property("max_interval", 0.45)
    light.set_editor_property("active_conditions", list(conditions))
    return tk.own(light, CORE, "Lightning", f"Lightning:{look}")


# The three looks (plan §6 #8, §8.8). Exactly one is active for any combination of flags:
#   storm (dusk, the first storm)  neither hale_arrived nor meeting_done
#   calm (evening, after knows)    hale_arrived, not meeting_done
#   night (the second storm)       meeting_done
# Each look is one presence rule over its sun, fill light, sky, fog, and post-process volume: the active state leaves them as
# authored; otherwise they are hidden and parked (a post-process volume honours its bounds, not hidden, so parking
# it is what switches it off). Changes are not deferred: every change happens while the player is in an interior
# (the vault at knows, the loft at the meeting), and the portal back out is a scene cut.
ATMOSPHERES = {
    "storm": dict(
        conditions=lambda: [tk.flag(HALE_ARRIVED, negate=True), tk.flag(MEETING_DONE, negate=True)],
        sun=((-38.0, 50.0), 50.0, (182, 190, 200), 6800.0),
        fill=(22.0, (150, 165, 185)),
        sky=((0.20, 0.23, 0.26), 0.016, 0.30, (0.30, 0.32, 0.35)),
        fog=(0.040, 0.86, (0.15, 0.17, 0.19)),
        grade=_grade(-0.35, 0.12, 1.0, (0.70, 0.74, 0.80), 1.12, gain=(0.95, 0.98, 1.0)),
        lightning=True),
    "calm": dict(
        conditions=lambda: [tk.flag(HALE_ARRIVED), tk.flag(MEETING_DONE, negate=True)],
        sun=((-17.0, 75.0), 95.0, (232, 210, 178), 4900.0),
        fill=(26.0, (160, 172, 200)),
        sky=((0.20, 0.25, 0.30), 0.018, 0.12, (0.38, 0.38, 0.40)),
        fog=(0.016, 0.62, (0.27, 0.26, 0.25)),
        grade=_grade(-0.25, 0.12, 1.2, (0.84, 0.86, 0.90), 1.08),
        lightning=False),
    "night": dict(
        conditions=lambda: [tk.flag(MEETING_DONE)],
        sun=((-42.0, 205.0), 9.0, (130, 150, 205), 9000.0),
        fill=(3.5, (90, 110, 160)),
        sky=((0.05, 0.07, 0.10), 0.008, 0.25, (0.05, 0.06, 0.09)),
        fog=(0.035, 0.85, (0.020, 0.026, 0.040)),
        grade=_grade(-0.6, 0.04, 0.22, (0.55, 0.62, 0.75), 1.10, gain=(0.85, 0.92, 1.0), white_temp=7600.0),
        lightning=True),
}
# The exterior grade's box: the island, the reef, and the crossing deck, with room above the tower.
EXTERIOR_PP_CENTER = (-14000.0, -9000.0, 12500.0)
EXTERIOR_PP_EXTENT = (28000.0, 33000.0, 17500.0)


def build_atmospheres():
    for name, look in ATMOSPHERES.items():
        rot, lux, color, temp = look["sun"]
        targets = [
            _sun(f"Sun_{name}", rot, lux, color, temp),
            _fill(f"Fill_{name}", rot, *look["fill"]),
            _sky(f"Sky_{name}", *look["sky"]),
            _fog(f"Fog_{name}", *look["fog"]),
            tk.post_process_box(f"Grade_{name}", CORE, EXTERIOR_PP_CENTER, EXTERIOR_PP_EXTENT, 1.0, look["grade"]),
        ]
        for target in targets:
            tk.own(target, CORE, f"Atmosphere:{name}")
        tk.presence(f"Atmosphere_{name}", CORE, targets, [
            tk.state("active", look["conditions"]()),
            tk.state("off", [], present=False, place=tk.placement(PARKING)),
        ], pivot=(0.0, 0.0, 0.0), defer=False)
        if look["lightning"]:
            _lightning(f"Lightning_{name}", name, look["conditions"]())

    # One shared sky light. It captures whichever sky is present, so each look's ambient follows its own sky.
    sky = tk.actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0.0, 0.0, 5000.0))
    sky.set_actor_label("SkyLight")
    comp = sky.get_component_by_class(unreal.SkyLightComponent)
    comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    comp.set_editor_property("real_time_capture", True)
    comp.set_editor_property("intensity", 1.3)
    comp.set_editor_property("lower_hemisphere_is_black", False)
    comp.set_editor_property("lower_hemisphere_color", unreal.LinearColor(0.08, 0.085, 0.09, 1.0))
    tk.own(sky, CORE)


def build_ambience():
    """The exterior wind bed, attenuated (not spatialized) so it fills the island and the crossing deck and fades out
    long before the interior slots 2.5 km away. A 2D bed would play inside the vault."""
    path = f"{AUDIO_FOLDER}/SA_DC_SombreExterior"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        att = unreal.load_asset(path)
    else:
        if not unreal.EditorAssetLibrary.does_directory_exist(AUDIO_FOLDER):
            unreal.EditorAssetLibrary.make_directory(AUDIO_FOLDER)
        att = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "SA_DC_SombreExterior", AUDIO_FOLDER, unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
    settings = att.get_editor_property("attenuation")
    settings.set_editor_property("attenuate", True)
    settings.set_editor_property("spatialize", False)
    settings.set_editor_property("attenuation_shape", unreal.AttenuationShape.SPHERE)
    settings.set_editor_property("attenuation_shape_extents", unreal.Vector(55000.0, 0.0, 0.0))
    settings.set_editor_property("falloff_distance", 40000.0)
    att.set_editor_property("attenuation", settings)
    if not unreal.EditorAssetLibrary.save_loaded_asset(att, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    wind = unreal.load_asset("/Game/Audio/Ambience/S_DC_LakeWind")
    if not wind:
        raise RuntimeError("Missing /Game/Audio/Ambience/S_DC_LakeWind. Run import_audio.py first.")
    actor = tk.actors.spawn_actor_from_class(unreal.DCConditionalAudio, unreal.Vector(-6000.0, -8000.0, 1500.0),
                                             unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label("Ambience_Wind")
    actor.set_editor_property("sound", wind)
    actor.set_editor_property("attenuation", att)
    actor.set_editor_property("volume_multiplier", 0.09)
    actor.set_editor_property("mode", unreal.DCConditionalAudioMode.WHILE_TRUE)
    tk.own(actor, CORE, "SombreAmbience")


# --- Player start, respawn, anchors

def build_anchors():
    made = {}
    for name, location, yaw, _why in anchors_table():
        made[name] = tk.marker(name, CORE, location, yaw, f"Anchor:{name}")
    return made


def build_player_start(anchors):
    deck = anchors["Anchor_NewGame_Deck"]
    start = tk.actors.spawn_actor_from_class(unreal.PlayerStart, deck.get_actor_location(), deck.get_actor_rotation())
    start.set_actor_label("PlayerStart")
    tk.own(start, CORE)
    # A dead player respawns at the PlayerStart (ADCPlayerCharacter::Respawn). The start follows the story: the
    # deck for a new game, the quay once the reef is struck, the tower base once the vault is open (plan §5.7).
    # Not deferred: the start is never seen.
    quay = anchors["Anchor_CrossingExit_Quay"]
    tower = anchors["Anchor_Respawn_TowerBase"]
    tk.presence("Respawn", CORE, [start], [
        tk.state("tower_base", [tk.flag(VAULT_OPENED)],
                 place=tk.placement(tower.get_actor_location(), tower.get_actor_rotation())),
        tk.state("harbor", [tk.flag(REEF_STRUCK)],
                 place=tk.placement(quay.get_actor_location(), quay.get_actor_rotation())),
    ], defer=False)


def write_probe():
    """Seeded grid vertices inside the walkable outline with their height, for Map.Sombre.Architecture. Vertices,
    because the tiles interpolate between them: a trace there must meet the surface at exactly that height."""
    rng = random.Random(1004)
    g = ISLAND.spec["grid"]
    step = float(g["step"])
    nx = int(round((g["x_max"] - g["x_min"]) / step))
    ny = int(round((g["y_max"] - g["y_min"]) / step))
    points = []
    while len(points) < 160:
        x = g["x_min"] + rng.randint(0, nx) * step
        y = g["y_min"] + rng.randint(0, ny) * step
        h = ISLAND.height(x, y)
        if ISLAND.inside_bounds(x, y) and h > float(g["drop_below"]):
            points.append([round(x * 100.0, 1), round(y * 100.0, 1), round(h * 100.0, 1)])
    os.makedirs(os.path.dirname(PROBE_FILE), exist_ok=True)
    with open(PROBE_FILE, "w", encoding="utf-8", newline="\n") as handle:
        json.dump({"about": "Generated by build_pointe_sombre.py from island.json. Terrain grid vertices (cm): x, y, "
                            "ground z. Map.Sombre.Architecture traces down at each.",
                   "island_hash": ISLAND.spec_hash(), "points": points}, handle, indent=1)
        handle.write("\n")


def run_cells():
    modules = []
    for cell in CELLS:
        module = importlib.import_module(cell)
        importlib.reload(module)
        module.build(tk)
        tk.log(f"cell {cell} built")
        modules.append(module)
    retire_greybox(modules)


def retire_greybox(modules):
    """A cell takes over its greybox stand-ins without editing greybox.py (Integrator-owned) by declaring them:
    GREYBOX_RETIRE (actor labels, e.g. "FalseLight_Lantern") and GREYBOX_RETIRE_GROUPS (a whole Greybox:<cell> group,
    e.g. "vault"). They are removed after every cell has built. A label that matches nothing stops the build, so a
    renamed stand-in is never silently left in place."""
    labels, groups = set(), set()
    for module in modules:
        labels.update(getattr(module, "GREYBOX_RETIRE", ()))
        groups.update(getattr(module, "GREYBOX_RETIRE_GROUPS", ()))
    if not labels and not groups:
        return
    doomed, found = [], set()
    for actor in tk.actors.get_all_level_actors():
        tags = [str(t) for t in actor.get_editor_property("tags")]
        if "Cell:greybox" not in tags:
            continue
        label = actor.get_actor_label()
        if label in labels or any(f"Greybox:{g}" in tags for g in groups):
            doomed.append(actor)
            found.add(label)
    missing = labels - found
    if missing:
        raise RuntimeError(f"GREYBOX_RETIRE names greybox actors that do not exist: {sorted(missing)}")
    tk.actors.destroy_actors(doomed)
    tk.log(f"greybox: retired {len(doomed)} stand-ins ({sorted(labels)}; groups {sorted(groups)})")


def main():
    global ISLAND
    ISLAND = island_mod.Island.load()
    levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        levels.load_level(MAP_PATH)
        destroy_persistent_actors()
    elif not levels.new_level(MAP_PATH, False):
        raise RuntimeError(f"Could not create {MAP_PATH}")

    slots = {cell: unreal.Vector(*origin) for cell, origin in INTERIOR_SLOTS.items()}
    tk.configure(ISLAND, {}, slots)
    build_terrain()
    build_water()
    build_walls()
    build_atmospheres()
    build_ambience()
    anchors = build_anchors()
    tk.configure(ISLAND, anchors, slots)
    build_player_start(anchors)
    run_cells()
    build_nav()
    write_probe()

    if not levels.save_current_level():
        raise RuntimeError(f"Could not save {MAP_PATH} (is the file read-only?)")
    tk.log(f"saved {MAP_PATH} with {len(tk.actors.get_all_level_actors())} actors (island {ISLAND.spec_hash()})")


ISLAND = None
main()
