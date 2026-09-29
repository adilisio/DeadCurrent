"""Export every Survival_Character texture to PNG so the migration can cut it to 1K.
Driven by Tools\\ImportSurvivalCharacter.ps1. Log prefix [DCSURV]. Also records each texture's settings."""
import json
import os
import unreal

ROOT = "/Game/Survival_Character/Textures"
OUT = os.path.join(unreal.SystemLibrary.get_project_directory(), "Saved", "SurvivalExport")
FULL = os.path.join(OUT, "full")


def log(msg):
    unreal.log_warning("[DCSURV] " + msg)


settings = {}
paths = []
for asset_path in unreal.EditorAssetLibrary.list_assets(ROOT, recursive=True, include_folder=False):
    asset = unreal.load_asset(asset_path)
    if not isinstance(asset, unreal.Texture2D):
        continue
    package = asset_path.split(".")[0]
    paths.append(package)
    settings[package] = {
        "compression": asset.get_editor_property("compression_settings").name,
        "srgb": bool(asset.get_editor_property("srgb")),
        "lod_group": asset.get_editor_property("lod_group").name,
        "width": asset.blueprint_get_size_x(),
        "height": asset.blueprint_get_size_y(),
    }

tasks = []
for package in paths:
    task = unreal.AssetExportTask()
    task.object = unreal.load_asset(package)
    task.exporter = None
    task.filename = os.path.join(FULL, package.split("/")[-1] + ".png")
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    tasks.append(task)

ok = unreal.Exporter.run_asset_export_tasks(tasks)
log(f"exported ok={ok} textures={len(paths)}")
missing = [t.filename for t in tasks if not os.path.exists(t.filename)]
if missing:
    # Some textures export as TGA or another extension. List what landed.
    landed = sorted(os.listdir(FULL))
    log(f"files not found at the expected .png name: {missing}")
    log(f"files present: {landed}")
with open(os.path.join(OUT, "settings.json"), "w") as handle:
    json.dump(settings, handle, indent=1)
log("done")
