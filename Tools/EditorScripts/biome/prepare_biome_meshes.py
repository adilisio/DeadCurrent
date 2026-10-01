"""One-time preparation of the library meshes the shoreline recipe instances (run after Tools\\ImportPackAssets.ps1
-Name biome; idempotent; not part of RebuildContent.bat, like the pack migration itself).

The Megascans rock set (Scene_Junkyard For_Sca_Rock_Set_01) came with Nanite on and a virtual-texture material.
This project runs neither (Nanite is off on the low-spec DX11 path; virtual texturing is not enabled), and the recipe
draws the rocks with the island's own triplanar rock surface anyway. So: Nanite off (the meshes' own LODs draw),
the default material becomes MI_DC_Sombre_Rock (flagged for instancing through M_DC_Surface), and the rock set's
own material, its virtual textures, and the MSPresets master they needed are deleted once nothing references them.
The driftwood and weed materials are kept: their masters are already flagged for instanced meshes (checked here).
DriftWood_MasterMaterial ships with two texture parameters (Normal_Medium, Normal_Low) that have no default texture,
so it does not compile and every log would draw as the engine default; those two get the engine's flat normal map.
Each log's material instance also leaves its distance versions (*_Medium, *_Low) and Specular on the engine's
checker texture, so a log seen from further away would turn into a checker: they get that log's own 512 maps (its AO
map stands in for Specular: dry wood is dull, darker in the cracks).

    Tools\\RebuildContent.bat biome\\prepare_biome_meshes
"""
import unreal

ROCKS = [
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/SM_For_Sca_Rock_Set_01_A",
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/SM_For_Sca_Rock_Set_01_C",
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/SM_For_Sca_Rock_Set_01_E",
]
ROCK_MATERIAL = "/Game/World/PointeSombre/Materials/MI_DC_Sombre_Rock"
UNUSED = [
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/MI_For_Sca_Rock_Set_01",
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/T_For_Sca_Rock_Set_01_D",
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/T_For_Sca_Rock_Set_01_DpR",
    "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/T_For_Sca_Rock_Set_01_N",
    "/Game/Scene_Junkyard/Materials/MSPresets/M_MS_Default_Material_VT/M_MS_Default_Material_VT",
    "/Game/Scene_Junkyard/Materials/MSPresets/M_MS_Default_Material/Functions/MF_ObjAdjustments",
    "/Game/Scene_Junkyard/Materials/MSPresets/MSVTTextures/BlackPlaceholder",
    "/Game/Scene_Junkyard/Materials/MSPresets/MSVTTextures/DefaultDiffuse",
    "/Game/Scene_Junkyard/Materials/MSPresets/MSVTTextures/Placeholder_Normal",
    "/Game/Scene_Junkyard/Materials/MSPresets/MSVTTextures/WhitePlaceholder",
]
DRIFTWOOD_MASTER = "/Game/DriftWoodPack/Materials/MasterMaterial/DriftWood_MasterMaterial"
FLAT_NORMAL = "/Engine/EngineMaterials/DefaultNormal"
DRIFTWOOD_INSTANCES = {
    "/Game/DriftWoodPack/Materials/Driftwood7/DrifftWood7_LowPoly_Inst": "/Game/DriftWoodPack/Textures/Driftwood7",
    "/Game/DriftWoodPack/Materials/Driftwood11/DrifftWood11_LowPoly_Inst": "/Game/DriftWoodPack/Textures/Driftwood11",
}
KEPT_MASTERS = [
    "/Game/DriftWoodPack/Materials/MasterMaterial/DriftWood_MasterMaterial",
    "/Game/Smugglers_cove/materials/master_materials/M_foliage_master",
]


def log(msg):
    unreal.log_warning("[DCBIOME] " + msg)


def fix_driftwood_master():
    """Give the master's empty texture parameters a default, so it compiles (idempotent)."""
    master = unreal.load_asset(DRIFTWOOD_MASTER)
    flat = unreal.load_asset(FLAT_NORMAL)
    if not master or not flat:
        raise RuntimeError(f"Missing {DRIFTWOOD_MASTER} or {FLAT_NORMAL}")
    fixed = []
    for expression in unreal.ObjectIterator(unreal.MaterialExpressionTextureSampleParameter2D):
        if expression.get_outermost() != master.get_outermost():
            continue
        if expression.get_editor_property("texture") is None:
            expression.set_editor_property("texture", flat)
            fixed.append(str(expression.get_editor_property("parameter_name")))
    if fixed:
        unreal.MaterialEditingLibrary.recompile_material(master)
        if not unreal.EditorAssetLibrary.save_loaded_asset(master, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save {DRIFTWOOD_MASTER}")
        log(f"gave {DRIFTWOOD_MASTER} defaults for {sorted(fixed)}")
    names = unreal.MaterialEditingLibrary.get_texture_parameter_names(master)
    empty = [str(n) for n in names
             if unreal.MaterialEditingLibrary.get_material_default_texture_parameter_value(master, n) is None]
    if empty:
        raise RuntimeError(f"{DRIFTWOOD_MASTER} still has empty texture parameters {empty}")


def fix_driftwood_instances():
    mel = unreal.MaterialEditingLibrary
    for path, folder in DRIFTWOOD_INSTANCES.items():
        mi = unreal.load_asset(path)
        if not mi:
            raise RuntimeError(f"Missing {path}")
        wanted = {}
        for kind, source in (("Diffuse", "Diffuse"), ("Normal", "Normal"), ("AO", "AO"), ("Roughness", "Roughness"),
                             ("Specular", "AO")):
            texture = unreal.load_asset(f"{folder}/{source}_512")
            if not texture:
                raise RuntimeError(f"Missing {folder}/{source}_512")
            for suffix in ("", "_Medium", "_Low"):
                wanted[kind + suffix] = texture
        changed = []
        for name, texture in sorted(wanted.items()):
            current = mel.get_material_instance_texture_parameter_value(mi, name)
            if not current or current.get_path_name() != texture.get_path_name():
                mel.set_material_instance_texture_parameter_value(mi, name, texture)
                changed.append(name)
        if changed:
            mel.update_material_instance(mi)
            if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
                raise RuntimeError(f"Could not save {path}")
            log(f"{path}: set {changed}")


def main():
    eal = unreal.EditorAssetLibrary
    rock = unreal.load_asset(ROCK_MATERIAL)
    if not rock:
        raise RuntimeError(f"Missing {ROCK_MATERIAL}. Run build_pointe_sombre.py first.")
    any_changed = False
    for path in ROCKS:
        mesh = unreal.load_asset(path)
        if not mesh:
            raise RuntimeError(f"Missing {path}. Run Tools\\ImportPackAssets.ps1 -Name biome first.")
        changed = False
        nanite = mesh.get_editor_property("nanite_settings")
        if nanite.get_editor_property("enabled"):
            nanite.set_editor_property("enabled", False)
            mesh.set_editor_property("nanite_settings", nanite)
            changed = True
        # StaticMesh.set_material: editing the static_materials array from Python only edits copies.
        for index, sm in enumerate(mesh.get_editor_property("static_materials")):
            current = sm.get_editor_property("material_interface")
            if not current or current.get_path_name() != rock.get_path_name():
                mesh.set_material(index, rock)
                changed = True
        if changed:
            if not eal.save_loaded_asset(mesh, only_if_is_dirty=False):
                raise RuntimeError(f"Could not save {path}")
            log(f"prepared {path}")
            any_changed = True
        else:
            log(f"already prepared {path}")

    if any_changed:
        # The asset registry in this process still lists the old references; a fresh run sees the saved meshes.
        log("meshes changed: run this script once more to delete the unused rock material and textures")
        return
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    for path in UNUSED:
        if not eal.does_asset_exist(path):
            continue
        package = path.rsplit(".", 1)[0]
        users = [str(u) for u in registry.get_referencers(package, unreal.AssetRegistryDependencyOptions())
                 if not str(u).startswith(package)] if hasattr(registry, "get_referencers") else []
        users = [u for u in users if u not in UNUSED]
        if users:
            raise RuntimeError(f"{path} is still used by {users}; not deleting it")
        if not eal.delete_asset(path):
            raise RuntimeError(f"Could not delete {path}")
        log(f"deleted unused {path}")

    fix_driftwood_master()
    fix_driftwood_instances()
    for path in KEPT_MASTERS:
        master = unreal.load_asset(path)
        if not master or not master.get_editor_property("used_with_instanced_static_meshes"):
            raise RuntimeError(f"{path} is not flagged for instanced static meshes")
    log("biome meshes ready")


main()
