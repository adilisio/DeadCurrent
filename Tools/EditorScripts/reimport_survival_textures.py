"""Reimport the 1K PNGs over the migrated Survival_Character textures, same asset paths, same
compression and sRGB settings. Driven by Tools\\ImportSurvivalCharacter.ps1. Log prefix [DCSURV]."""
import json
import os
import unreal

OUT = os.path.join(unreal.SystemLibrary.get_project_directory(), "Saved", "SurvivalExport")
SMALL = os.path.join(OUT, "1k")


def log(msg):
    unreal.log_warning("[DCSURV] " + msg)


with open(os.path.join(OUT, "settings.json")) as handle:
    settings = json.load(handle)

done = 0
for package, saved in settings.items():
    name = package.split("/")[-1]
    png = os.path.join(SMALL, name + ".png")
    if not os.path.exists(png):
        log(f"NO PNG for {package}")
        continue
    task = unreal.AssetImportTask()
    task.filename = png
    task.destination_path = package.rsplit("/", 1)[0]
    task.destination_name = name
    task.replace_existing = True
    task.replace_existing_settings = True
    task.automated = True
    task.save = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    texture = unreal.load_asset(package)
    if not isinstance(texture, unreal.Texture2D):
        log(f"FAILED to import {package}")
        continue
    texture.set_editor_property("compression_settings", getattr(unreal.TextureCompressionSettings, saved["compression"]))
    texture.set_editor_property("srgb", saved["srgb"])
    texture.set_editor_property("lod_group", getattr(unreal.TextureGroup, saved["lod_group"]))
    texture.set_editor_property("max_texture_size", 1024)
    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)
    done += 1
    log(f"{package}: {saved['width']}x{saved['height']} -> {texture.blueprint_get_size_x()}x{texture.blueprint_get_size_y()}")
log(f"reimported {done} of {len(settings)}")
