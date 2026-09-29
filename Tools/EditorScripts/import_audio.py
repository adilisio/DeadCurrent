"""Import the Presentation Pass audio and build the two attenuation assets.

Sources are the game-ready CC0 files in C:\\FO5_AssetLibrary\\Audio (Ambience\\game, SFX\\game) and the finished
Gunshots folder. Provenance and licenses are in each source.json and in Design/art_pipeline.md. Idempotent:
a second run replaces the waves in place. Registered in RebuildContent.bat before create_items.py (the pistol's
FireSound and DryFireSound resolve these paths) and before build_boathouse.py (the hums and sparks resolve them).
Log prefix [DCAUDIO].
"""
import os
import unreal

LIBRARY = r"C:\FO5_AssetLibrary\Audio"
DEST = "/Game/Audio"

# id in the library, relative file, destination folder, asset name, loops
SOUNDS = [
    ("amb_lake_wind", r"Ambience\game\amb_lake_wind.wav", "Ambience", "S_DC_LakeWind", True),
    ("amb_water_lap", r"Ambience\game\amb_water_lap.wav", "Ambience", "S_DC_WaterLap", True),
    ("hum_live_water", r"Ambience\game\hum_live_water.wav", "Ambience", "S_DC_HumLiveWater", True),
    ("hum_relay", r"Ambience\game\hum_relay.wav", "Ambience", "S_DC_HumRelay", True),
    ("sfx_breaker_pull", r"SFX\game\sfx_breaker_pull.wav", "SFX", "S_DC_BreakerPull", False),
    ("sfx_spark_01", r"SFX\game\sfx_spark_01.wav", "SFX", "S_DC_Spark01", False),
    ("sfx_spark_02", r"SFX\game\sfx_spark_02.wav", "SFX", "S_DC_Spark02", False),
    ("sfx_spark_03", r"SFX\game\sfx_spark_03.wav", "SFX", "S_DC_Spark03", False),
    ("sfx_spark_04", r"SFX\game\sfx_spark_04.wav", "SFX", "S_DC_Spark04", False),
    ("sfx_spark_05", r"SFX\game\sfx_spark_05.wav", "SFX", "S_DC_Spark05", False),
    ("sfx_spark_06", r"SFX\game\sfx_spark_06.wav", "SFX", "S_DC_Spark06", False),
    ("gun_revolver_38_shot", r"Gunshots\gun_revolver_38_shot.wav", "Weapons", "S_DC_PistolShot", False),
    ("gun_dryfire_striker", r"Gunshots\gun_dryfire_striker.wav", "Weapons", "S_DC_PistolDry", False),
]

# name, inner radius cm, falloff cm
ATTENUATIONS = [
    ("SA_DC_Hum", 250.0, 2200.0),     # relay and live water: a hum you find by walking toward it
    ("SA_DC_Snap", 200.0, 1800.0),    # sparks over the water
    ("SA_DC_Clunk", 300.0, 3000.0),   # the breaker throw
]


def log(msg):
    unreal.log_warning("[DCAUDIO] " + msg)


def import_sound(entry):
    lib_id, relative, folder, name, loops = entry
    source = os.path.join(LIBRARY, relative)
    if not os.path.isfile(source):
        raise RuntimeError(f"Missing {source} ({lib_id})")
    dest = f"{DEST}/{folder}"
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)
    task = unreal.AssetImportTask()
    task.filename = source
    task.destination_path = dest
    task.destination_name = name
    task.replace_existing = True
    task.automated = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    wave = unreal.load_asset(f"{dest}/{name}")
    if not isinstance(wave, unreal.SoundWave):
        raise RuntimeError(f"Could not import {lib_id} as {dest}/{name}")
    wave.set_editor_property("looping", loops)
    if not unreal.EditorAssetLibrary.save_loaded_asset(wave, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {dest}/{name}")
    log(f"{dest}/{name} loops={loops} from {lib_id}")


def ensure_attenuation(name, inner, falloff):
    path = f"{DEST}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        asset = unreal.load_asset(path)
    else:
        asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, DEST, unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
        if not asset:
            raise RuntimeError(f"Could not create {path}")
    settings = asset.get_editor_property("attenuation")
    settings.set_editor_property("attenuate", True)
    settings.set_editor_property("spatialize", True)
    natural = [n for n in dir(unreal.SoundDistanceModel) if "NATURAL" in n.upper()]
    if not natural:
        raise RuntimeError(f"No natural-sound distance model in {dir(unreal.SoundDistanceModel)}")
    settings.set_editor_property("distance_algorithm", getattr(unreal.SoundDistanceModel, natural[0]))
    settings.set_editor_property("attenuation_shape", unreal.AttenuationShape.SPHERE)
    settings.set_editor_property("attenuation_shape_extents", unreal.Vector(inner, 0.0, 0.0))
    settings.set_editor_property("falloff_distance", falloff)
    asset.set_editor_property("attenuation", settings)
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    log(f"{path} inner={inner} falloff={falloff}")


def main():
    for entry in SOUNDS:
        import_sound(entry)
    for name, inner, falloff in ATTENUATIONS:
        ensure_attenuation(name, inner, falloff)
    log(f"imported {len(SOUNDS)} sounds and {len(ATTENUATIONS)} attenuation assets")


main()
