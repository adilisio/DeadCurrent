"""Reimport the cut-down PNGs over a migrated pack's textures: same asset paths, same compression, sRGB, and LOD group.
Driven by Tools\\ImportPackAssets.ps1 (DC_PACK_WORK). Log prefix [DCPACK]."""
import json
import os
import unreal

WORK = os.environ["DC_PACK_WORK"]
SMALL = os.path.join(WORK, "small")


def log(msg):
    unreal.log_warning("[DCPACK] " + msg)


with open(os.path.join(WORK, "settings.json")) as handle:
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
    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)
    done += 1
    log(f"{package}: {saved['width']}x{saved['height']} -> {texture.blueprint_get_size_x()}x{texture.blueprint_get_size_y()}")
log(f"reimported {done} of {len(settings)}")
