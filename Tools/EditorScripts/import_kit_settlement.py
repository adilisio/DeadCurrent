"""Import the Great Lakes Working Settlement Kit (Phase 6 WP-KIT, VS-05): its CC0 textures, its skins, and its modules.

Run with Tools\\RebuildContent.bat import_kit_settlement (after import_art.py, whose surface instances some skins
extend). Safe to re-run: textures are re-imported in place, skins rewritten, and a module is re-imported only when its
geometry changed (its stored hash differs).

  Textures  C:\\FO5_AssetLibrary\\CC0\\ambientcg\\<id>\\  ->  /Game/World/Kits/GreatLakesSettlement/Textures/<id>/
            (CC0 1.0, ambientCG; source.json beside each; provenance in Design/art_pipeline.md)
  Skins     /Game/World/Kits/GreatLakesSettlement/Materials/MI_DC_Kit_*  (triplanar M_DC_Surface instances, or
            children of the shore's surface instances, or flat M_FlatCol colours)
  Modules   /Game/World/Kits/GreatLakesSettlement/Meshes/SM_Kit_<Id>  (Tools/EditorScripts/kit/modules.py, written as
            OBJ to Saved/KitModules/ and imported with flat normals and complex-as-simple collision)

No Meshy: Tier B spends no credits.
"""
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "kit"))
import geometry  # noqa: E402
import modules  # noqa: E402

ROOT = "/Game/World/Kits/GreatLakesSettlement"
TEX = ROOT + "/Textures"
MATS = ROOT + "/Materials"
MESHES = ROOT + "/Meshes"
MASTER = "/Game/Environment/Materials/M_DC_Surface"
FLAT = ROOT + "/Materials/M_DC_Kit_Flat"
LIBRARY = r"C:\FO5_AssetLibrary\CC0\ambientcg"
OBJ_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "Saved", "KitModules"))
HASH_TAG = "DCKitModuleHash"
MAX_SIZE = 2048

# id -> maps. WoodSiding011 ships no roughness map; it borrows Planks012's (weathered board, same character).
TEXTURES = {
    "WoodSiding011": {"color": "WoodSiding011_2K-JPG_Color.jpg", "normal": "WoodSiding011_2K-JPG_NormalDX.jpg",
                      "rough": ("Planks012", "Planks012_2K-JPG_Roughness.jpg")},
    "Planks012": {"color": "Planks012_2K-JPG_Color.jpg", "normal": "Planks012_2K-JPG_NormalDX.jpg",
                  "rough": "Planks012_2K-JPG_Roughness.jpg"},
    "WoodSiding005": {"color": "WoodSiding005_2K-JPG_Color.jpg", "normal": "WoodSiding005_2K-JPG_NormalDX.jpg",
                      "rough": "WoodSiding005_2K-JPG_Roughness.jpg"},
}

# Skins. "tex": a triplanar instance of M_DC_Surface on the kit's own textures; "child": a tinted child of a shore
# surface instance; "flat": a plain colour (for faces the triplanar surfaces light wrongly, and glass).
SKINS = {
    "MI_DC_Kit_GreyShingle": {"tex": "WoodSiding011", "tile": 220.0, "tint": (0.92, 0.92, 0.92)},
    "MI_DC_Kit_ShingleRoof": {"tex": "WoodSiding011", "tile": 150.0, "tint": (0.46, 0.46, 0.48)},
    "MI_DC_Kit_BoardGrey": {"tex": "Planks012", "tile": 200.0, "tint": (0.80, 0.80, 0.80)},
    "MI_DC_Kit_Tarred": {"tex": "Planks012", "tile": 200.0, "tint": (0.17, 0.16, 0.15)},
    "MI_DC_Kit_RedBoard": {"tex": "WoodSiding005", "tile": 220.0, "tint": (0.78, 0.70, 0.68)},
    "MI_DC_Kit_Corrugated": {"child": "MI_DC_Steel", "tint": (0.68, 0.72, 0.76)},
    "MI_DC_Kit_RustTrim": {"child": "MI_DC_RustPaint"},
    "MI_DC_Kit_Whitewash": {"child": "MI_DC_Plaster", "tint": (1.25, 1.25, 1.18)},
    "MI_DC_Kit_Concrete": {"child": "MI_DC_Concrete"},
    "MI_DC_Kit_Underside": {"flat": (0.035, 0.030, 0.026), "rough": 0.9},
    "MI_DC_Kit_Pane": {"flat": (0.012, 0.016, 0.022), "rough": 0.25},
}

# The slot a module's mesh shows by default (compose() overrides per structure).
DEFAULT_SLOT_SKIN = {
    "skin": "MI_DC_Kit_GreyShingle", "trim": "MI_DC_Kit_Tarred", "door": "MI_DC_Kit_BoardGrey", "pane": "MI_DC_Kit_Pane",
    "roof": "MI_DC_Kit_ShingleRoof", "underside": "MI_DC_Kit_Underside", "deck": "MI_DC_Kit_BoardGrey",
    "masonry": "MI_DC_Kit_Concrete",
}


def log(msg):
    unreal.log_warning("[DCKIT] " + msg)


def ensure_dir(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_texture(filename, dest, name, kind):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    path = f"{dest}/{name}"
    tex = unreal.load_asset(path)
    if not tex:
        raise RuntimeError(f"Import failed: {filename}")
    tex.set_editor_property("max_texture_size", MAX_SIZE)
    if kind == "normal":
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
        tex.set_editor_property("srgb", False)
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD_NORMAL_MAP)
    elif kind == "data":
        tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
        tex.set_editor_property("srgb", False)
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
    else:
        tex.set_editor_property("srgb", True)
        tex.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_WORLD)
    if not unreal.EditorAssetLibrary.save_loaded_asset(tex, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    return tex


def import_textures():
    out = {}
    for tid, maps in TEXTURES.items():
        dest = f"{TEX}/{tid}"
        ensure_dir(dest)
        got = {}
        for key, source in maps.items():
            folder, filename = (source if isinstance(source, tuple) else (tid, source))
            full = os.path.join(LIBRARY, folder, filename)
            if not os.path.isfile(full):
                raise RuntimeError(f"Missing {full}. The kit's CC0 sources live in the asset library (see source.json).")
            kind = {"color": "color", "normal": "normal", "rough": "data"}[key]
            name = "T_{}_{}".format(tid, {"color": "BC", "normal": "N", "rough": "R"}[key])
            if unreal.EditorAssetLibrary.does_asset_exist(f"{dest}/{name}"):
                # Imported once and reused, like import_art.py: a re-run leaves the texture byte-identical.
                got[key] = unreal.load_asset(f"{dest}/{name}")
                continue
            got[key] = import_texture(full, dest, name, kind)
        out[tid] = got
        log(f"textures {tid}")
    return out


def _instance(name):
    path = f"{MATS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, MATS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    if not mi:
        raise RuntimeError(f"Could not create {path}")
    return mi


def ensure_flat_master():
    """The kit's own flat-colour master (parameters Base Color, Roughness, Metallic), flagged for instanced meshes:
    the kit draws on instanced components, and the prototype M_FlatCol is not flagged (an unflagged material renders
    as the engine's default checker on an instanced mesh)."""
    if unreal.EditorAssetLibrary.does_asset_exist(FLAT):
        return
    mel = unreal.MaterialEditingLibrary
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_DC_Kit_Flat", MATS, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Could not create {FLAT}")
    material.set_editor_property("used_with_instanced_static_meshes", True)
    color = mel.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 0)
    color.set_editor_property("parameter_name", "Base Color")
    color.set_editor_property("default_value", unreal.LinearColor(0.2, 0.2, 0.2, 1.0))
    rough = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -400, 200)
    rough.set_editor_property("parameter_name", "Roughness")
    rough.set_editor_property("default_value", 0.9)
    metal = mel.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -400, 300)
    metal.set_editor_property("parameter_name", "Metallic")
    metal.set_editor_property("default_value", 0.0)
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {FLAT}")
    log("created M_DC_Kit_Flat")


def make_skins(textures):
    mel = unreal.MaterialEditingLibrary
    ensure_dir(MATS)
    ensure_flat_master()
    surface = unreal.load_asset(MASTER)
    if not surface.get_editor_property("used_with_instanced_static_meshes"):
        raise RuntimeError(f"{MASTER} is not flagged for instanced meshes: run import_art.py (Phase 6 sets the flag)")
    for name, spec in SKINS.items():
        mi = _instance(name)
        if "tex" in spec:
            mel.set_material_instance_parent(mi, unreal.load_asset(MASTER))
            maps = textures[spec["tex"]]
            for suffix in ("_X", "_Y", "_Z"):
                mel.set_material_instance_texture_parameter_value(mi, "BaseColor" + suffix, maps["color"])
                mel.set_material_instance_texture_parameter_value(mi, "Roughness" + suffix, maps["rough"])
            mel.set_material_instance_texture_parameter_value(mi, "Normal", maps["normal"])
            mel.set_material_instance_static_switch_parameter_value(mi, "PackedORM", False)
            mel.set_material_instance_scalar_parameter_value(mi, "TileSizeCm", spec["tile"])
            mel.set_material_instance_scalar_parameter_value(mi, "Metallic", 0.0)
            mel.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(*spec["tint"], 1.0))
        elif "child" in spec:
            mel.set_material_instance_parent(mi, unreal.load_asset(f"/Game/Environment/Materials/{spec['child']}"))
            if "tint" in spec:
                mel.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(*spec["tint"], 1.0))
        else:
            mel.set_material_instance_parent(mi, unreal.load_asset(FLAT))
            mel.set_material_instance_vector_parameter_value(mi, "Base Color", unreal.LinearColor(*spec["flat"], 1.0))
            mel.set_material_instance_scalar_parameter_value(mi, "Roughness", spec["rough"])
            mel.set_material_instance_scalar_parameter_value(mi, "Metallic", 0.0)
        mel.update_material_instance(mi)
        if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save {MATS}/{name}")
    log(f"{len(SKINS)} skins")


def _import_obj(obj_path, name):
    options = unreal.FbxImportUI()
    options.set_editor_property("import_mesh", True)
    options.set_editor_property("import_as_skeletal", False)
    options.set_editor_property("import_animations", False)
    options.set_editor_property("import_materials", False)
    options.set_editor_property("import_textures", False)
    data = options.static_mesh_import_data
    data.set_editor_property("combine_meshes", True)
    data.set_editor_property("auto_generate_collision", False)
    data.set_editor_property("generate_lightmap_u_vs", False)
    data.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", obj_path)
    task.set_editor_property("destination_path", MESHES)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    task.set_editor_property("options", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(f"{MESHES}/{name}")
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Importing {obj_path} did not make a static mesh")
    return mesh


def import_modules():
    ensure_dir(MESHES)
    os.makedirs(OBJ_DIR, exist_ok=True)
    built = skipped = 0
    for module_id in sorted(modules.MODULES):
        name = f"SM_Kit_{module_id}"
        path = f"{MESHES}/{name}"
        want = modules.module_hash(module_id)
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            if unreal.EditorAssetLibrary.get_metadata_tag(unreal.load_asset(path), HASH_TAG) == want:
                skipped += 1
                continue
            if not unreal.EditorAssetLibrary.delete_asset(path):
                raise RuntimeError(f"Could not replace {path}")
        obj_path = os.path.join(OBJ_DIR, module_id + ".obj")
        with open(obj_path, "w", encoding="ascii", newline="\n") as handle:
            handle.write(geometry.write_obj(modules.parts(module_id),
                                            f"Great Lakes settlement kit module {module_id}. Generated; do not edit."))
        mesh = _import_obj(obj_path, name)
        slots = list(mesh.get_editor_property("static_materials"))
        for slot in slots:
            slot_name = str(slot.get_editor_property("material_slot_name"))
            if slot_name not in DEFAULT_SLOT_SKIN:
                raise RuntimeError(f"{name}: unknown material slot {slot_name}")
            slot.set_editor_property("material_interface", unreal.load_asset(f"{MATS}/{DEFAULT_SLOT_SKIN[slot_name]}"))
        mesh.set_editor_property("static_materials", slots)
        body = mesh.get_editor_property("body_setup")
        body.set_editor_property("agg_geom", unreal.KAggregateGeom())
        body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        unreal.EditorAssetLibrary.set_metadata_tag(mesh, HASH_TAG, want)
        if not unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False):
            raise RuntimeError(f"Could not save {path}")
        built += 1
    log(f"modules: {built} imported, {skipped} unchanged, {len(modules.MODULES)} in the kit")


def main():
    if not unreal.load_asset(MASTER):
        raise RuntimeError(f"Missing {MASTER}. Run import_art.py first.")
    make_skins(import_textures())
    import_modules()


main()
