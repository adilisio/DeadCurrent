"""Import the CC0 surfaces the shore uses, and build M_DC_Surface.

Safe to re-run: textures keep their asset names (replace, never duplicate) and the
material master is created once. Textures are capped at 2K. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi

Sources live outside the repo, under C:\\FO5_AssetLibrary\\CC0. DirectX normal maps only.
"""
import os
import unreal

LIBRARY = r"C:\FO5_AssetLibrary\CC0"
ENV_MATERIALS = "/Game/Environment/Materials"
MASTER = ENV_MATERIALS + "/M_DC_Surface"
MAX_SIZE = 2048

# kind: color (sRGB), normal, data (linear masks: roughness, ao, arm, metal)
SURFACES = [
    dict(source="PolyHaven", asset_id="coast_rocks_01",
         root=os.path.join(LIBRARY, "polyhaven", "coast_rocks_01", "textures"),
         author="Rob Tuytel, Rico Cilliers",
         url="https://polyhaven.com/a/coast_rocks_01",
         instance="MI_DC_CoastRock", tile_cm=180.0, metallic=0.0,
         maps=dict(color="coast_rocks_01_diff_2k.jpg", normal="coast_rocks_01_nor_dx_2k.jpg",
                   orm="coast_rocks_01_arm_2k.jpg")),
    dict(source="PolyHaven", asset_id="coast_land_rocks_02",
         root=os.path.join(LIBRARY, "polyhaven", "coast_land_rocks_02", "textures"),
         author="Rob Tuytel, Rico Cilliers",
         url="https://polyhaven.com/a/coast_land_rocks_02",
         instance="MI_DC_LandRock", tile_cm=200.0, metallic=0.0,
         maps=dict(color="coast_land_rocks_02_diff_2k.jpg", normal="coast_land_rocks_02_nor_dx_2k.jpg",
                   orm="coast_land_rocks_02_arm_2k.jpg")),
    dict(source="PolyHaven", asset_id="coast_sand_02",
         root=os.path.join(LIBRARY, "polyhaven", "coast_sand_02"),
         author="Rob Tuytel",
         url="https://polyhaven.com/a/coast_sand_02",
         instance="MI_DC_CoastSand", tile_cm=140.0, metallic=0.0,
         maps=dict(color="coast_sand_02_diff_2k.jpg", normal="coast_sand_02_nor_dx_2k.jpg",
                   rough="coast_sand_02_rough_2k.jpg", ao="coast_sand_02_ao_2k.jpg")),
    dict(source="PolyHaven", asset_id="brown_mud_02",
         root=os.path.join(LIBRARY, "polyhaven", "brown_mud_02"),
         author="Rob Tuytel",
         url="https://polyhaven.com/a/brown_mud_02",
         instance="MI_DC_Mud", tile_cm=160.0, metallic=0.0,
         maps=dict(color="brown_mud_02_diff_2k.jpg", normal="brown_mud_02_nor_dx_2k.jpg",
                   rough="brown_mud_02_rough_2k.jpg", ao="brown_mud_02_ao_2k.jpg")),
    dict(source="PolyHaven", asset_id="chipped_concrete",
         root=os.path.join(LIBRARY, "polyhaven", "chipped_concrete"),
         author="Amal Kumar",
         url="https://polyhaven.com/a/chipped_concrete",
         instance="MI_DC_Concrete", tile_cm=120.0, metallic=0.0,
         maps=dict(color="chipped_concrete_diff_2k.jpg", normal="chipped_concrete_nor_dx_2k.jpg",
                   rough="chipped_concrete_rough_2k.jpg", ao="chipped_concrete_ao_2k.jpg")),
    dict(source="PolyHaven", asset_id="blue_plaster_weathered",
         root=os.path.join(LIBRARY, "polyhaven", "blue_plaster_weathered"),
         author="Amal Kumar",
         url="https://polyhaven.com/a/blue_plaster_weathered",
         instance="MI_DC_Plaster", tile_cm=100.0, metallic=0.0,
         maps=dict(color="blue_plaster_weathered_diff_2k.jpg", normal="blue_plaster_weathered_nor_dx_2k.jpg",
                   rough="blue_plaster_weathered_rough_2k.jpg", ao="blue_plaster_weathered_ao_2k.jpg")),
    dict(source="AmbientCG", asset_id="CorrugatedSteel009",
         root=os.path.join(LIBRARY, "ambientcg", "CorrugatedSteel009"),
         author="ambientCG",
         url="https://ambientcg.com/a/CorrugatedSteel009",
         instance="MI_DC_Steel", tile_cm=80.0, metallic=1.0,
         maps=dict(color="CorrugatedSteel009_2K-JPG_Color.jpg",
                   normal="CorrugatedSteel009_2K-JPG_NormalDX.jpg",
                   rough="CorrugatedSteel009_2K-JPG_Roughness.jpg",
                   ao="CorrugatedSteel009_2K-JPG_AmbientOcclusion.jpg",
                   metal="CorrugatedSteel009_2K-JPG_Metalness.jpg")),
]

# Paths build_boathouse.py is allowed to resolve. Keep this list and the script's check in step.
SURFACE_INSTANCES = [f"{ENV_MATERIALS}/{spec['instance']}" for spec in SURFACES]


def log(msg):
    unreal.log_warning("[DCART] " + msg)


def connect(src, src_pin, dst, dst_pin):
    if not unreal.MaterialEditingLibrary.connect_material_expressions(src, src_pin, dst, dst_pin):
        raise RuntimeError(f"Material link failed: {src_pin or 'out'} -> {dst_pin or 'in'}")


def make(material, cls, x, y):
    node = unreal.MaterialEditingLibrary.create_material_expression(material, cls, x, y)
    if not node:
        raise RuntimeError(f"Could not create {cls}")
    return node


def mask(material, src, channels, x, y):
    node = make(material, unreal.MaterialExpressionComponentMask, x, y)
    node.set_editor_property("r", "r" in channels)
    node.set_editor_property("g", "g" in channels)
    node.set_editor_property("b", "b" in channels)
    node.set_editor_property("a", "a" in channels)
    connect(src, "", node, "")
    return node


def append(material, a, b, x, y):
    node = make(material, unreal.MaterialExpressionAppendVector, x, y)
    connect(a, "", node, "A")
    connect(b, "", node, "B")
    return node


def add(material, a, b, x, y):
    node = make(material, unreal.MaterialExpressionAdd, x, y)
    connect(a, "", node, "A")
    connect(b, "", node, "B")
    return node


def mul(material, a, b, x, y):
    node = make(material, unreal.MaterialExpressionMultiply, x, y)
    connect(a, "", node, "A")
    connect(b, "", node, "B")
    return node


def div(material, a, b, x, y):
    node = make(material, unreal.MaterialExpressionDivide, x, y)
    connect(a, "", node, "A")
    connect(b, "", node, "B")
    return node


def import_texture(filename, dest_path, dest_name, kind):
    path = f"{dest_path}/{dest_name}"
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", dest_path)
    task.set_editor_property("destination_name", dest_name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        raise RuntimeError(f"Import failed: {filename}")
    tex = unreal.load_asset(path)
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


def import_maps(spec):
    dest = f"/Game/Art/{spec['source']}/{spec['asset_id']}"
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)
    imported = {}
    kinds = dict(color="color", normal="normal", orm="data", rough="data", ao="data", metal="data")
    suffixes = dict(color="BC", normal="N", orm="ORM", rough="R", ao="AO", metal="M")
    for key, filename in spec["maps"].items():
        full = os.path.join(spec["root"], filename)
        if not os.path.isfile(full):
            raise RuntimeError(f"Missing source file {full}")
        name = f"T_{spec['asset_id']}_{suffixes[key]}"
        imported[key] = import_texture(full, dest, name, kinds[key])
        log(f"{spec['asset_id']} {key} -> {dest}/{name}")
    return imported


def sample_param(material, name, uv, x, y, sampler=None):
    node = make(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    node.set_editor_property("parameter_name", name)
    if sampler:
        node.set_editor_property("sampler_type", sampler)
    for pin in ("UVs", "Coordinates"):
        if unreal.MaterialEditingLibrary.connect_material_expressions(uv, "", node, pin):
            return node
    names = []
    for inp in node.get_editor_property("inputs"):
        names.append(str(inp.get_editor_property("input_name")))
    raise RuntimeError(f"Could not connect UVs. Texture inputs: {names}")


def triplanar(material, name, scaled_world, weights, x, sampler=None):
    """Blend three samples of one texture parameter. weights is (wx, wy, wz), already normalized."""
    wx, wy, wz = weights
    sw = scaled_world
    uv_x = append(material, mask(material, sw, "b", x, -40), mask(material, sw, "g", x, 40), x + 160, 0)
    uv_y = append(material, mask(material, sw, "r", x, 80), mask(material, sw, "b", x, 160), x + 160, 120)
    uv_z = append(material, mask(material, sw, "r", x, 200), mask(material, sw, "g", x, 280), x + 160, 240)
    sx = sample_param(material, name, uv_x, x + 360, -80, sampler)
    sy = sample_param(material, name, uv_y, x + 360, 80, sampler)
    sz = sample_param(material, name, uv_z, x + 360, 240, sampler)
    return add(
        material,
        add(material, mul(material, sx, wx, x + 620, -40), mul(material, sy, wy, x + 620, 80), x + 820, 0),
        mul(material, sz, wz, x + 820, 160),
        x + 1040, 40)


def static_switch(material, name, default, true_expr, false_expr, x, y):
    node = make(material, unreal.MaterialExpressionStaticSwitchParameter, x, y)
    node.set_editor_property("parameter_name", name)
    node.set_editor_property("default_value", default)
    connect(true_expr, "", node, "True")
    connect(false_expr, "", node, "False")
    return node


def ensure_master():
    if unreal.EditorAssetLibrary.does_asset_exist(MASTER):
        if not unreal.EditorAssetLibrary.delete_asset(MASTER):
            raise RuntimeError(f"Could not replace {MASTER}")
    if not unreal.EditorAssetLibrary.does_directory_exist(ENV_MATERIALS):
        unreal.EditorAssetLibrary.make_directory(ENV_MATERIALS)
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_DC_Surface", ENV_MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Could not create {MASTER}")
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)

    world = make(material, unreal.MaterialExpressionWorldPosition, -1800, 0)
    tile = make(material, unreal.MaterialExpressionScalarParameter, -1800, 200)
    tile.set_editor_property("parameter_name", "TileSizeCm")
    tile.set_editor_property("default_value", 200.0)
    scaled = div(material, world, tile, -1500, 40)

    normal_ws = make(material, unreal.MaterialExpressionVertexNormalWS, -1800, 420)
    abs_n = make(material, unreal.MaterialExpressionAbs, -1600, 420)
    connect(normal_ws, "", abs_n, "")
    wx = mask(material, abs_n, "r", -1400, 340)
    wy = mask(material, abs_n, "g", -1400, 420)
    wz = mask(material, abs_n, "b", -1400, 500)
    weight_sum = add(material, add(material, wx, wy, -1180, 380), wz, -1000, 420)
    weights = (
        div(material, wx, weight_sum, -820, 300),
        div(material, wy, weight_sum, -820, 420),
        div(material, wz, weight_sum, -820, 540),
    )

    # Shared triplanar UVs would be ideal; each call rebuilds them so the graph stays local.
    color = triplanar(material, "BaseColor", scaled, weights, -600)
    rough_separate = triplanar(material, "Roughness", scaled, weights, -600)
    orm = triplanar(material, "ORM", scaled, weights, -600)
    rough = static_switch(material, "PackedORM", False, mask(material, orm, "g", 1200, 200), rough_separate, 1400, 80)
    metal_packed = mask(material, orm, "b", 1200, 360)
    metal_scalar = make(material, unreal.MaterialExpressionScalarParameter, 1200, 480)
    metal_scalar.set_editor_property("parameter_name", "Metallic")
    metal_scalar.set_editor_property("default_value", 0.0)
    metal = static_switch(material, "PackedORM", False, metal_packed, metal_scalar, 1500, 360)
    ao = mask(material, orm, "r", 1200, 40)
    # Separate maps have no packed AO. Darken the albedo with ORM.r only when PackedORM is on.
    ao_lift = make(material, unreal.MaterialExpressionConstant, 1200, -40)
    ao_lift.set_editor_property("r", 1.0)
    ao_factor = static_switch(material, "PackedORM", False, ao, ao_lift, 1500, 0)
    shaded = mul(material, color, ao_factor, 1700, -80)

    normal_sampler = unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    # One normal sample, world-XY, so floors (the walkable ground) keep detail. Walls use the mesh normal
    # where the up-facing weight is low: blend is in the normal pin via a flatten toward (0,0,1).
    uv_ground = append(material, mask(material, scaled, "r", 1200, 640), mask(material, scaled, "g", 1200, 720), 1400, 680)
    normal_sample = sample_param(material, "Normal", uv_ground, 1600, 640, normal_sampler)
    flat = make(material, unreal.MaterialExpressionConstant3Vector, 1600, 860)
    flat.set_editor_property("constant", unreal.LinearColor(0.0, 0.0, 1.0, 1.0))
    normal_lerp = make(material, unreal.MaterialExpressionLinearInterpolate, 1900, 700)
    connect(flat, "", normal_lerp, "A")
    connect(normal_sample, "", normal_lerp, "B")
    connect(weights[2], "", normal_lerp, "Alpha")

    mel = unreal.MaterialEditingLibrary
    mel.connect_material_property(shaded, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)
    mel.connect_material_property(normal_lerp, "", unreal.MaterialProperty.MP_NORMAL)
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {MASTER}")
    log(f"created {MASTER}")
    return material


def ensure_instance(spec, textures):
    name = spec["instance"]
    path = f"{ENV_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, ENV_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, unreal.load_asset(MASTER))
    mel.set_material_instance_texture_parameter_value(mi, "BaseColor", textures["color"])
    mel.set_material_instance_texture_parameter_value(mi, "Normal", textures["normal"])
    packed = "orm" in textures
    mel.set_material_instance_static_switch_parameter_value(mi, "PackedORM", packed)
    if packed:
        mel.set_material_instance_texture_parameter_value(mi, "ORM", textures["orm"])
    else:
        mel.set_material_instance_texture_parameter_value(mi, "Roughness", textures["rough"])
    mel.set_material_instance_scalar_parameter_value(mi, "TileSizeCm", spec["tile_cm"])
    mel.set_material_instance_scalar_parameter_value(mi, "Metallic", spec["metallic"])
    if "metal" in textures:
        # The master uses the scalar for unpacked maps. Steel's metalness map is stored for a later
        # switch; the scalar stays at 1 so the sheet reads as metal until that sample is wired.
        pass
    mel.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    log(f"instance {path} packed={packed}")
    return path


def main():
    before = 0
    for spec in SURFACES:
        dest = f"/Game/Art/{spec['source']}/{spec['asset_id']}"
        if unreal.EditorAssetLibrary.does_directory_exist(dest):
            before += len(unreal.EditorAssetLibrary.list_assets(dest, recursive=False, include_folder=False))
    ensure_master()
    made = []
    for spec in SURFACES:
        textures = import_maps(spec)
        made.append(ensure_instance(spec, textures))
    after = 0
    for spec in SURFACES:
        dest = f"/Game/Art/{spec['source']}/{spec['asset_id']}"
        after += len(unreal.EditorAssetLibrary.list_assets(dest, recursive=False, include_folder=False))
    for path in made:
        if not unreal.load_asset(path):
            raise RuntimeError(f"Could not resolve {path}")
    log(f"surface textures before={before} after={after}; instances={len(made)}")


main()
