"""Export every texture in a migrated pack closure to PNG so it can be cut down, and record each texture's settings.
Driven by Tools\\ImportPackAssets.ps1, which sets DC_PACK_WORK and writes packages.txt there. Log prefix [DCPACK]."""
import json
import os
import unreal

WORK = os.environ["DC_PACK_WORK"]
FULL = os.path.join(WORK, "full")


def log(msg):
    unreal.log_warning("[DCPACK] " + msg)


with open(os.path.join(WORK, "packages.txt"), encoding="utf-8-sig") as handle:
    packages = [line.strip() for line in handle if line.strip()]

settings = {}
tasks = []
for package in packages:
    asset = unreal.load_asset(package)
    if not isinstance(asset, unreal.Texture2D):
        continue
    settings[package] = {
        "compression": asset.get_editor_property("compression_settings").name,
        "srgb": bool(asset.get_editor_property("srgb")),
        "lod_group": asset.get_editor_property("lod_group").name,
        "width": asset.blueprint_get_size_x(),
        "height": asset.blueprint_get_size_y(),
    }
    task = unreal.AssetExportTask()
    task.object = asset
    task.exporter = None
    task.filename = os.path.join(FULL, package.split("/")[-1] + ".png")
    task.automated = True
    task.prompt = False
    task.replace_identical = True
    tasks.append(task)

ok = unreal.Exporter.run_asset_export_tasks(tasks) if tasks else True
log(f"exported ok={ok} textures={len(tasks)} of {len(packages)} packages")
missing = [t.filename for t in tasks if not os.path.exists(t.filename)]
if missing:
    log(f"not exported as .png: {missing}; present: {sorted(os.listdir(FULL))}")
with open(os.path.join(WORK, "settings.json"), "w") as handle:
    json.dump(settings, handle, indent=1)
log("done")
