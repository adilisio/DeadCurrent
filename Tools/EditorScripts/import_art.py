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
         tint=(0.62, 0.68, 0.74),
         maps=dict(color="coast_rocks_01_diff_2k.jpg", normal="coast_rocks_01_nor_dx_2k.jpg",
                   orm="coast_rocks_01_arm_2k.jpg")),
    # Ridge and bluffs use the same cold shore rock, tinted down. The warm land-rock photo stayed brown.
    dict(source="PolyHaven", asset_id="coast_rocks_01",
         root=os.path.join(LIBRARY, "polyhaven", "coast_rocks_01", "textures"),
         author="Rob Tuytel, Rico Cilliers",
         url="https://polyhaven.com/a/coast_rocks_01",
         instance="MI_DC_LandRock", tile_cm=640.0, metallic=0.0,
         tint=(0.38, 0.44, 0.50),
         maps=dict(color="coast_rocks_01_diff_2k.jpg", normal="coast_rocks_01_nor_dx_2k.jpg",
                   orm="coast_rocks_01_arm_2k.jpg")),
    dict(source="PolyHaven", asset_id="coast_sand_02",
         root=os.path.join(LIBRARY, "polyhaven", "coast_sand_02"),
         author="Rob Tuytel",
         url="https://polyhaven.com/a/coast_sand_02",
         instance="MI_DC_CoastSand", tile_cm=140.0, metallic=0.0,
         tint=(0.52, 0.58, 0.64),
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
         instance="MI_DC_Steel", tile_cm=80.0, metallic=0.35,
         tint=(0.72, 0.78, 0.84),
         maps=dict(color="CorrugatedSteel009_2K-JPG_Color.jpg",
                   normal="CorrugatedSteel009_2K-JPG_NormalDX.jpg",
                   rough="CorrugatedSteel009_2K-JPG_Roughness.jpg",
                   ao="CorrugatedSteel009_2K-JPG_AmbientOcclusion.jpg",
                   metal="CorrugatedSteel009_2K-JPG_Metalness.jpg")),
    dict(source="PolyHaven", asset_id="rusty_painted_metal",
         root=os.path.join(LIBRARY, "polyhaven", "rusty_painted_metal"),
         author="Amal Kumar",
         url="https://polyhaven.com/a/rusty_painted_metal",
         instance="MI_DC_RustPaint", tile_cm=90.0, metallic=0.25,
         tint=(0.70, 0.66, 0.62),
         maps=dict(color="rusty_painted_metal_diff_2k.jpg", normal="rusty_painted_metal_nor_dx_2k.jpg",
                   rough="rusty_painted_metal_rough_2k.jpg", ao="rusty_painted_metal_ao_2k.jpg")),
    dict(source="AmbientCG", asset_id="Gravel008",
         root=os.path.join(LIBRARY, "ambientcg", "Gravel008"),
         author="ambientCG",
         url="https://ambientcg.com/a/Gravel008",
         instance="MI_DC_Gravel", tile_cm=70.0, metallic=0.0,
         tint=(0.58, 0.62, 0.66),
         maps=dict(color="Gravel008_2K-JPG_Color.jpg", normal="Gravel008_2K-JPG_NormalDX.jpg",
                   rough="Gravel008_2K-JPG_Roughness.jpg", ao="Gravel008_2K-JPG_AmbientOcclusion.jpg")),
]

# The Tern's diffuse maps. The FBX import only wrote material stubs; these JPEGs are the paint.
BOAT_TEXTURES = r"C:\FO5_AssetLibrary\Fab\motorboat_wreck\textures"
BOAT_DEST = "/Game/Art/Fab/motorboat_wreck"
BOAT_MAPS = (
    ("biely_cln_u1_v1_diffuse-denoise.jpeg", "T_motorboat_u1_BC"),
    ("biely_cln_u2_v1_diffuse-sharpen.jpeg", "T_motorboat_u2_BC"),
)
WRECK_MASTER = ENV_MATERIALS + "/M_DC_Wreck"

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
    mips = tex.get_num_mips() if hasattr(tex, "get_num_mips") else "n/a"
    log(f"{dest_name} {tex.blueprint_get_size_x()}x{tex.blueprint_get_size_y()} mips={mips} srgb={tex.get_editor_property('srgb')}")
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
        path = f"{dest}/{name}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            imported[key] = unreal.load_asset(path)
            log(f"{spec['asset_id']} {key} reuse {path}")
            continue
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
    """Blend three samples. weights is (wx, wy, wz), already normalized.

    Each projection is its own parameter (Name_X/Y/Z). Three TextureSampleParameter2D
    nodes that share one name do not all receive the instance texture: the floor uses
    the Z sample, which stayed on the default and shaded as a mirror of the normal grain.
    """
    wx, wy, wz = weights
    sw = scaled_world
    uv_x = append(material, mask(material, sw, "b", x, -40), mask(material, sw, "g", x, 40), x + 160, 0)
    uv_y = append(material, mask(material, sw, "r", x, 80), mask(material, sw, "b", x, 160), x + 160, 120)
    uv_z = append(material, mask(material, sw, "r", x, 200), mask(material, sw, "g", x, 280), x + 160, 240)
    sx = sample_param(material, f"{name}_X", uv_x, x + 360, -80, sampler)
    sy = sample_param(material, f"{name}_Y", uv_y, x + 360, 80, sampler)
    sz = sample_param(material, f"{name}_Z", uv_z, x + 360, 240, sampler)
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
    # The normal pin is a world-space vector. A scaled cube's UV0 tangents turn a
    # world-UV normal sample into black-and-white lighting.
    material.set_editor_property("tangent_space_normal", False)

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
    tint = make(material, unreal.MaterialExpressionVectorParameter, 1900, -200)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))
    shaded = mul(material, shaded, tint, 2100, -80)
    # Bluff and breakwater faces are seen from both sides. A culled backface reads as a black wall.
    material.set_editor_property("two_sided", True)

    normal_sampler = unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
    # One sample, world XY. On an up-facing face this unpacked vector is already a world normal.
    # Walls keep VertexNormalWS. A single Normal parameter binds; the triplanar maps use _X/_Y/_Z.
    uv_ground = append(material, mask(material, scaled, "r", 1200, 640), mask(material, scaled, "g", 1200, 720), 1400, 680)
    normal_sample = sample_param(material, "Normal", uv_ground, 1600, 640, normal_sampler)
    # The master compiles against the parameter's default texture. A color default fails a Normal sampler.
    default_normal = unreal.load_asset("/Engine/EngineMaterials/DefaultNormal")
    if not default_normal:
        raise RuntimeError("Missing /Engine/EngineMaterials/DefaultNormal")
    normal_sample.set_editor_property("texture", default_normal)
    normal_lerp = make(material, unreal.MaterialExpressionLinearInterpolate, 1900, 700)
    connect(normal_ws, "", normal_lerp, "A")
    connect(normal_sample, "", normal_lerp, "B")
    connect(weights[2], "", normal_lerp, "Alpha")
    # A backface stays two-sided, and its normal flips so the basin side of a breakwater is not unlit black.
    facing = make(material, unreal.MaterialExpressionTwoSidedSign, 2100, 820)
    normal_out = mul(material, normal_lerp, facing, 2300, 720)

    mel = unreal.MaterialEditingLibrary
    mel.connect_material_property(shaded, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)
    mel.connect_material_property(normal_out, "", unreal.MaterialProperty.MP_NORMAL)
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

    def set_triplanar(param, texture):
        for suffix in ("_X", "_Y", "_Z"):
            mel.set_material_instance_texture_parameter_value(mi, param + suffix, texture)

    set_triplanar("BaseColor", textures["color"])
    mel.set_material_instance_texture_parameter_value(mi, "Normal", textures["normal"])
    packed = "orm" in textures
    mel.set_material_instance_static_switch_parameter_value(mi, "PackedORM", packed)
    if packed:
        set_triplanar("ORM", textures["orm"])
    else:
        set_triplanar("Roughness", textures["rough"])
    mel.set_material_instance_scalar_parameter_value(mi, "TileSizeCm", spec["tile_cm"])
    mel.set_material_instance_scalar_parameter_value(mi, "Metallic", spec["metallic"])
    tint = spec.get("tint", (1.0, 1.0, 1.0))
    mel.set_material_instance_vector_parameter_value(
        mi, "Tint", unreal.LinearColor(tint[0], tint[1], tint[2], 1.0))
    mel.update_material_instance(mi)
    for suffix in ("_X", "_Y", "_Z"):
        bound = mel.get_material_instance_texture_parameter_value(mi, "BaseColor" + suffix)
        log(f"{spec['instance']} BaseColor{suffix}={bound.get_name() if bound else 'MISSING'}")
        if not bound:
            raise RuntimeError(f"{spec['instance']} has no BaseColor{suffix}")
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    log(f"instance {path} packed={packed}")
    return path


def capped_jpeg(filename, dest_name):
    """The Fab JPEGs are 8192. Write a 2048 copy and import that, so the asset size is the cap."""
    src = os.path.join(BOAT_TEXTURES, filename)
    if not os.path.isfile(src):
        raise RuntimeError(f"Missing {src}")
    out_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", "Saved", "TernImport"))
    os.makedirs(out_dir, exist_ok=True)
    dest = os.path.join(out_dir, dest_name + ".jpg")
    script = (
        "from PIL import Image\n"
        "import sys\n"
        "im = Image.open(sys.argv[1])\n"
        "im.thumbnail((int(sys.argv[3]), int(sys.argv[3])), Image.Resampling.LANCZOS)\n"
        "im.convert('RGB').save(sys.argv[2], 'JPEG', quality=90)\n"
        "print('%dx%d' % im.size)\n"
    )
    import subprocess
    result = subprocess.run(
        ["py", "-3", "-c", script, src, dest, str(MAX_SIZE)],
        capture_output=True, text=True, check=False)
    if result.returncode != 0 or not os.path.isfile(dest):
        raise RuntimeError(f"Could not downscale {filename}: {result.stderr.strip()}")
    log(f"{dest_name} downscaled {result.stdout.strip()} from {filename}")
    return dest


def import_boat_maps():
    if not unreal.EditorAssetLibrary.does_directory_exist(BOAT_DEST):
        unreal.EditorAssetLibrary.make_directory(BOAT_DEST)
    textures = []
    for filename, dest_name in BOAT_MAPS:
        path = f"{BOAT_DEST}/{dest_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(path):
            existing = unreal.load_asset(path)
            if existing and existing.blueprint_get_size_x() <= MAX_SIZE and existing.blueprint_get_size_y() <= MAX_SIZE:
                log(f"{dest_name} in-editor {existing.blueprint_get_size_x()}x{existing.blueprint_get_size_y()} "
                    f"max_texture_size={existing.get_editor_property('max_texture_size')}")
                textures.append(existing)
                continue
            if not unreal.EditorAssetLibrary.delete_asset(path):
                raise RuntimeError(f"Could not replace {path}")
        capped = capped_jpeg(filename, dest_name)
        tex = import_texture(capped, BOAT_DEST, dest_name, "color")
        log(f"{dest_name} in-editor {tex.blueprint_get_size_x()}x{tex.blueprint_get_size_y()} "
            f"max_texture_size={tex.get_editor_property('max_texture_size')}")
        textures.append(tex)
    return textures


def ensure_wreck_master():
    if unreal.EditorAssetLibrary.does_asset_exist(WRECK_MASTER):
        if not unreal.EditorAssetLibrary.delete_asset(WRECK_MASTER):
            raise RuntimeError(f"Could not replace {WRECK_MASTER}")
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_DC_Wreck", ENV_MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Could not create {WRECK_MASTER}")
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    material.set_editor_property("two_sided", True)

    uv = make(material, unreal.MaterialExpressionTextureCoordinate, -900, 0)
    diffuse = sample_param(material, "HullDiffuse", uv, -520, -40)
    tint = make(material, unreal.MaterialExpressionVectorParameter, -520, 220)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(0.42, 0.46, 0.48, 1.0))
    tinted = mul(material, diffuse, tint, -160, 80)

    world = make(material, unreal.MaterialExpressionWorldPosition, -900, 420)
    tile = make(material, unreal.MaterialExpressionScalarParameter, -900, 580)
    tile.set_editor_property("parameter_name", "GrimeTileCm")
    tile.set_editor_property("default_value", 80.0)
    scaled = div(material, world, tile, -560, 500)
    grime_uv = append(
        material,
        mask(material, scaled, "r", -320, 440),
        mask(material, scaled, "g", -320, 520),
        -80, 480)
    grime = sample_param(material, "Grime", grime_uv, 180, 420)
    amount = make(material, unreal.MaterialExpressionScalarParameter, 180, 680)
    amount.set_editor_property("parameter_name", "GrimeAmount")
    amount.set_editor_property("default_value", 0.55)
    blend = make(material, unreal.MaterialExpressionLinearInterpolate, 520, 160)
    connect(tinted, "", blend, "A")
    connect(grime, "", blend, "B")
    connect(amount, "", blend, "Alpha")

    rough = make(material, unreal.MaterialExpressionScalarParameter, 520, 420)
    rough.set_editor_property("parameter_name", "Roughness")
    rough.set_editor_property("default_value", 0.78)
    metal = make(material, unreal.MaterialExpressionScalarParameter, 520, 560)
    metal.set_editor_property("parameter_name", "Metallic")
    metal.set_editor_property("default_value", 0.1)

    mel = unreal.MaterialEditingLibrary
    mel.connect_material_property(blend, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(metal, "", unreal.MaterialProperty.MP_METALLIC)
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {WRECK_MASTER}")
    log(f"created {WRECK_MASTER}")
    return material


def ensure_wreck_instance(name, diffuse, grime):
    path = f"{ENV_MATERIALS}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        mi = unreal.load_asset(path)
    else:
        mi = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, ENV_MATERIALS, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        if not mi:
            raise RuntimeError(f"Could not create {path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(mi, unreal.load_asset(WRECK_MASTER))
    mel.set_material_instance_texture_parameter_value(mi, "HullDiffuse", diffuse)
    mel.set_material_instance_texture_parameter_value(mi, "Grime", grime)
    # Faded maritime paint: dark, cool, and more grime than hull. Anthony tunes these.
    mel.set_material_instance_vector_parameter_value(mi, "Tint", unreal.LinearColor(0.34, 0.38, 0.40, 1.0))
    mel.set_material_instance_scalar_parameter_value(mi, "GrimeAmount", 0.62)
    mel.set_material_instance_scalar_parameter_value(mi, "GrimeTileCm", 70.0)
    mel.set_material_instance_scalar_parameter_value(mi, "Roughness", 0.82)
    mel.set_material_instance_scalar_parameter_value(mi, "Metallic", 0.08)
    mel.update_material_instance(mi)
    if not unreal.EditorAssetLibrary.save_loaded_asset(mi, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    log(f"instance {path}")
    return mi


def find_static_mesh(dest):
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        return None
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(asset_path)
        if isinstance(asset, unreal.StaticMesh):
            return asset
    return None


def import_mesh(filename, dest, max_size=MAX_SIZE):
    existing = find_static_mesh(dest)
    if existing:
        log(f"reuse mesh {existing.get_path_name()}")
        return existing
    if not os.path.isfile(filename):
        raise RuntimeError(f"Missing {filename}")
    if not unreal.EditorAssetLibrary.does_directory_exist(dest):
        unreal.EditorAssetLibrary.make_directory(dest)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", filename)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = find_static_mesh(dest)
    if not mesh:
        raise RuntimeError(f"Import produced no static mesh in {dest}")
    for asset_path in unreal.EditorAssetLibrary.list_assets(dest, recursive=True, include_folder=False):
        asset = unreal.load_asset(asset_path)
        if isinstance(asset, unreal.Texture):
            asset.set_editor_property("max_texture_size", max_size)
            unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
            log(f"{asset.get_name()} {asset.blueprint_get_size_x()}x{asset.blueprint_get_size_y()} cap {max_size}")
    log(f"imported {mesh.get_path_name()}")
    return mesh


def import_clue_meshes():
    """Library meshes for the clue actors. Not Meshy. The pistol stays on SM_Pistol."""
    import_mesh(
        r"C:\FO5_AssetLibrary\CC0\polyhaven\life_jacket\life_jacket_2k.gltf",
        "/Game/Art/PolyHaven/life_jacket")
    import_mesh(
        r"C:\FO5_AssetLibrary\Meshy\lighthouse_logbook\model.fbx",
        "/Game/Art/Meshy/lighthouse_logbook")
    for folder in ("/Game/Art/PolyHaven/life_jacket", "/Game/Art/Meshy/lighthouse_logbook"):
        for asset_path in unreal.EditorAssetLibrary.list_assets(folder, recursive=True, include_folder=False):
            asset = unreal.load_asset(asset_path)
            if isinstance(asset, (unreal.Material, unreal.MaterialInstanceConstant)):
                log(f"{asset.get_name()} samples {sampled_texture_names(asset)}")


def sampled_texture_names(material):
    """Texture asset names a material actually samples. Instances report their parameter values."""
    names = []
    if isinstance(material, unreal.Material):
        for expr in material.get_editor_property("expressions") or []:
            try:
                tex = expr.get_editor_property("texture")
            except Exception:
                tex = None
            if tex:
                names.append(tex.get_name())
        return names
    if isinstance(material, unreal.MaterialInstanceConstant):
        for param in material.get_editor_property("texture_parameter_values") or []:
            tex = param.get_editor_property("parameter_value")
            if tex:
                names.append(tex.get_name())
    return names


def ensure_relay_material():
    """Material_001 is the FBX instance. It must sample texture_0 and the three data maps."""
    folder = "/Game/Art/Meshy/relay_housing"
    instance = unreal.load_asset(folder + "/Material_001")
    if not isinstance(instance, unreal.MaterialInstanceConstant):
        raise RuntimeError(f"Material_001 is {type(instance)}, expected a MaterialInstanceConstant")
    wanted = ("texture_0", "texture_0_normal", "texture_0_roughness", "texture_0_metallic")
    textures = {}
    for name in wanted:
        tex = unreal.load_asset(f"{folder}/{name}")
        if not tex:
            raise RuntimeError(f"Missing {folder}/{name}")
        textures[name] = tex
        if name != "texture_0":
            tex.set_editor_property("srgb", False)
            unreal.EditorAssetLibrary.save_loaded_asset(tex, only_if_is_dirty=True)
    before = sampled_texture_names(instance)
    log(f"Material_001 before samples {before}")
    parent_path = folder + "/M_DC_Relay"
    if unreal.EditorAssetLibrary.does_asset_exist(parent_path):
        unreal.EditorAssetLibrary.delete_asset(parent_path)
    parent = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_DC_Relay", folder, unreal.Material, unreal.MaterialFactoryNew())
    if not parent:
        raise RuntimeError(f"Could not create {parent_path}")
    parent.set_editor_property("two_sided", True)
    parent.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    uv = make(parent, unreal.MaterialExpressionTextureCoordinate, -500, 0)
    pins = {
        "texture_0": unreal.MaterialProperty.MP_BASE_COLOR,
        "texture_0_normal": unreal.MaterialProperty.MP_NORMAL,
        "texture_0_roughness": unreal.MaterialProperty.MP_ROUGHNESS,
        "texture_0_metallic": unreal.MaterialProperty.MP_METALLIC,
    }
    mel = unreal.MaterialEditingLibrary
    facing = make(parent, unreal.MaterialExpressionTwoSidedSign, 400, 200)
    for index, (name, prop) in enumerate(pins.items()):
        sampler = None
        if "normal" in name:
            sampler = unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
        elif name != "texture_0":
            sampler = unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR
        node = sample_param(parent, name, uv, -40, index * 200, sampler)
        node.set_editor_property("texture", textures[name])
        if prop == unreal.MaterialProperty.MP_NORMAL:
            flipped = mul(parent, node, facing, 280, index * 200)
            mel.connect_material_property(flipped, "", prop)
        else:
            mel.connect_material_property(node, "", prop)
    mel.recompile_material(parent)
    if not unreal.EditorAssetLibrary.save_loaded_asset(parent, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {parent_path}")
    mel = unreal.MaterialEditingLibrary
    mel.set_material_instance_parent(instance, parent)
    for name, tex in textures.items():
        mel.set_material_instance_texture_parameter_value(instance, name, tex)
    mel.update_material_instance(instance)
    if not unreal.EditorAssetLibrary.save_loaded_asset(instance, only_if_is_dirty=False):
        raise RuntimeError("Could not save Material_001")
    found = sampled_texture_names(instance)
    log(f"Material_001 after samples {found}")
    missing = [name for name in wanted if name not in found]
    if missing:
        raise RuntimeError(f"Material_001 still missing {missing}")
    return instance


def import_relay():
    """The relay housing. 1K: it is a small inspectable, and PlayTest's texture pool is 400 MB."""
    src = r"C:\FO5_AssetLibrary\Meshy\relay_housing\model.fbx"
    if not os.path.isfile(src):
        raise RuntimeError(f"Missing {src}. Run Tools/generate_meshy.py relay_housing first.")
    mesh = import_mesh(src, "/Game/Art/Meshy/relay_housing", max_size=1024)
    ensure_relay_material()
    return mesh


def write_chalk_png():
    out_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", "Saved", "Chalk"))
    os.makedirs(out_dir, exist_ok=True)
    dest = os.path.join(out_dir, "chalk_keep_out.png")
    script = r"""
from PIL import Image, ImageDraw, ImageFont, ImageFilter
import random, sys
w = h = 1024
im = Image.new("RGB", (w, h), (78, 64, 48))
px = im.load()
rng = random.Random(7)
for y in range(h):
    for x in range(w):
        n = rng.randint(-18, 18)
        r, g, b = px[x, y]
        px[x, y] = (max(0, min(255, r + n)), max(0, min(255, g + n - 4)), max(0, min(255, b + n - 8)))
draw = ImageDraw.Draw(im)
font = ImageFont.truetype(r"C:\Windows\Fonts\arialbd.ttf", 86)
chalk = (214, 214, 206)
draw.text((70, 360), "KEEP OUT OF", font=font, fill=chalk)
draw.text((110, 500), "THE WATER", font=font, fill=chalk)
im = im.filter(ImageFilter.GaussianBlur(radius=0.6))
im.save(sys.argv[1], "PNG")
print(sys.argv[1])
"""
    import subprocess
    result = subprocess.run(["py", "-3", "-c", script, dest], capture_output=True, text=True, check=False)
    if result.returncode != 0 or not os.path.isfile(dest):
        raise RuntimeError(f"Could not write the chalk board: {result.stderr.strip()}")
    log(f"chalk board {dest}")
    return dest


def ensure_chalk_board():
    tex = import_texture(write_chalk_png(), "/Game/Art/Decals", "T_ChalkKeepOut", "color")
    path = ENV_MATERIALS + "/M_DC_Chalk"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        unreal.EditorAssetLibrary.delete_asset(path)
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_DC_Chalk", ENV_MATERIALS, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Could not create {path}")
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    material.set_editor_property("two_sided", True)
    # The prototype cube's UV0 is a point, so the words never land on the face.
    # This plank stands in world YZ at about y=-960..-900, z=0..60.
    world = make(material, unreal.MaterialExpressionWorldPosition, -700, 0)
    y0 = make(material, unreal.MaterialExpressionConstant, -700, 160)
    y0.set_editor_property("r", 960.0)
    z0 = make(material, unreal.MaterialExpressionConstant, -700, 240)
    z0.set_editor_property("r", 0.0)
    span = make(material, unreal.MaterialExpressionConstant, -700, 320)
    span.set_editor_property("r", 60.0)
    # Capture 1401 showed the words mirrored and upside down from the interaction side.
    y_raw = div(material, add(material, mask(material, world, "g", -480, 40), y0, -300, 40), span, -80, 0)
    z_raw = div(material, add(material, mask(material, world, "b", -480, 140), z0, -300, 140), span, -80, 120)
    y_uv = make(material, unreal.MaterialExpressionOneMinus, 40, 0)
    connect(y_raw, "", y_uv, "")
    z_uv = make(material, unreal.MaterialExpressionOneMinus, 40, 120)
    connect(z_raw, "", z_uv, "")
    uv = append(material, y_uv, z_uv, 200, 40)
    sample = sample_param(material, "Board", uv, 280, 0)
    sample.set_editor_property("texture", tex)
    rough = make(material, unreal.MaterialExpressionConstant, 200, 200)
    rough.set_editor_property("r", 0.9)
    mel = unreal.MaterialEditingLibrary
    mel.connect_material_property(sample, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    log(f"created {path}")


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
    boat = import_boat_maps()
    ensure_wreck_master()
    grime = unreal.load_asset("/Game/Art/PolyHaven/rusty_painted_metal/T_rusty_painted_metal_BC")
    if not grime:
        raise RuntimeError("Missing rusty_painted_metal base color for the Tern grime blend")
    ensure_wreck_instance("MI_DC_TernU1", boat[0], grime)
    ensure_wreck_instance("MI_DC_TernU2", boat[1], grime)
    import_clue_meshes()
    import_relay()
    ensure_chalk_board()


main()
