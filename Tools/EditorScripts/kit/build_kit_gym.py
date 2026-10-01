"""Build /Game/Maps/Lvl_KitGym: the settlement kit's test ground (a development map, Phase 6 WP-KIT, VS-05).

A flat yard under a neutral overcast (it reads materials honestly; it is not Pointe Sombre's grade), a player start,
and the three gym structures from Tools/Kits/great_lakes_settlement/structures/ ("gym": true), each composed by
kit.compose() from data alone. Regenerated from scratch on every run; nothing else depends on it.

Run with Tools\\RebuildContent.bat kit\\build_kit_gym (after import_kit_settlement). Review: Tools\\ReviewCapture.bat
Lvl_KitGym. Play: Tools\\PlayTest.bat Lvl_KitGym. Registered in Tools/Maps.bat as a development map: never cooked.
"""
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import compose  # noqa: E402

MAP_PATH = "/Game/Maps/Lvl_KitGym"
CUBE = "/Game/LevelPrototyping/Meshes/SM_Cube"
FOLDER = "KitGym"
# Gym positions (cm): structures stand side by side along X with their fronts toward -Y.
POSITIONS = {"kit_store": (0.0, 0.0), "kit_salvage_shed": (1600.0, 0.0), "kit_cottage": (3000.0, 0.0)}

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCKIT] " + msg)


def spawn(cls, location, rotation=(0.0, 0.0, 0.0), label=None):
    actor = actors.spawn_actor_from_class(cls, unreal.Vector(*location),
                                          unreal.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]))
    if label:
        actor.set_actor_label(label)
    actor.set_folder_path(FOLDER)
    return actor


def ground():
    cube = unreal.load_asset(CUBE)
    bounds = cube.get_bounding_box()
    size = bounds.max - bounds.min
    yard = spawn(unreal.StaticMeshActor, (1700.0 - 6000.0, -1500.0 - 4000.0, -50.0), label="Yard")
    yard.set_actor_scale3d(unreal.Vector(12000.0 / size.x, 8000.0 / size.y, 50.0 / size.z))
    comp = yard.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(cube)
    # A darker ground than the shore gravel: under the gym's neutral exposure the gravel read as snow (capture 0940).
    comp.set_material(0, unreal.load_asset("/Game/Environment/Materials/MI_DC_Mud"))


def lighting():
    sun = spawn(unreal.DirectionalLight, (0.0, 0.0, 3000.0), (-42.0, 125.0, 0.0), "Sun")
    s = sun.get_component_by_class(unreal.DirectionalLightComponent)
    s.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    s.set_editor_property("atmosphere_sun_light", True)
    s.set_editor_property("intensity", 75.0)
    s.set_editor_property("forward_shading_priority", 1)
    s.set_editor_property("light_color", unreal.Color(r=230, g=228, b=222, a=255))
    fill = spawn(unreal.DirectionalLight, (0.0, 0.0, 3100.0), (-26.0, 305.0, 0.0), "Fill")
    f = fill.get_component_by_class(unreal.DirectionalLightComponent)
    f.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    f.set_editor_property("atmosphere_sun_light", False)
    f.set_editor_property("cast_shadows", False)
    f.set_editor_property("forward_shading_priority", 0)
    f.set_editor_property("intensity", 24.0)
    sky = spawn(unreal.SkyLight, (0.0, 0.0, 3000.0), label="SkyLight")
    k = sky.get_component_by_class(unreal.SkyLightComponent)
    k.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    k.set_editor_property("real_time_capture", True)
    k.set_editor_property("lower_hemisphere_is_black", False)
    k.set_editor_property("lower_hemisphere_color", unreal.LinearColor(0.10, 0.10, 0.10, 1.0))
    spawn(unreal.SkyAtmosphere, (0.0, 0.0, 0.0), label="SkyAtmosphere")
    fog = spawn(unreal.ExponentialHeightFog, (0.0, 0.0, 0.0), label="HeightFog")
    fog.get_component_by_class(unreal.ExponentialHeightFogComponent).set_editor_property("fog_density", 0.008)
    pp = spawn(unreal.PostProcessVolume, (0.0, 0.0, 0.0), label="PostProcess")
    pp.set_editor_property("unbound", True)
    st = pp.get_editor_property("settings")
    st.set_editor_property("override_auto_exposure_method", True)
    st.set_editor_property("auto_exposure_method", unreal.AutoExposureMethod.AEM_HISTOGRAM)
    st.set_editor_property("override_auto_exposure_min_brightness", True)
    st.set_editor_property("auto_exposure_min_brightness", 0.2)
    st.set_editor_property("override_auto_exposure_max_brightness", True)
    st.set_editor_property("auto_exposure_max_brightness", 1.2)
    st.set_editor_property("override_auto_exposure_bias", True)
    st.set_editor_property("auto_exposure_bias", -0.3)
    pp.set_editor_property("settings", st)


def main():
    if unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        levels.load_level(MAP_PATH)
        doomed = [a for a in actors.get_all_level_actors()
                  if not isinstance(a, unreal.WorldSettings) and not (isinstance(a, unreal.Brush) and not isinstance(a, unreal.Volume))]
        actors.destroy_actors(doomed)
    elif not levels.new_level(MAP_PATH, False):
        raise RuntimeError(f"Could not create {MAP_PATH}")

    ground()
    lighting()
    spawn(unreal.PlayerStart, (1700.0, -2200.0, 100.0), (0.0, 90.0, 0.0), "PlayerStart")

    kit = compose.load_kit()
    placed = 0
    for name in compose.structure_names():
        spec = compose.load_structure(name)
        if not spec.get("gym"):
            continue
        x, y = POSITIONS[spec["id"]]
        compose.place(spec, kit, (x, y, 0.0), 0.0, folder=FOLDER)
        placed += 1
    if placed != 3:
        raise RuntimeError(f"Expected 3 gym structures, placed {placed}")
    if not levels.save_current_level():
        raise RuntimeError(f"Could not save {MAP_PATH}")
    log(f"saved {MAP_PATH}: {placed} structures")


main()
