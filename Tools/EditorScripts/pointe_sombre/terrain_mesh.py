"""Pointe Sombre's terrain as static mesh tiles, generated from the island heightfield (island.py).

The method chosen by the VS-04 terrain spike (Design/technical_architecture.md, "Pointe Sombre: map architecture"):
a generated mesh, not a Landscape, because it regenerates headless from versioned data and its collision is the
rendered surface. Each tile is its own StaticMesh asset under /Game/World/PointeSombre/Terrain/, with up to four
material sections chosen per triangle (rock on steep faces, shore near the waterline, seabed under water, turf
elsewhere) and complex-as-simple collision, so traces, bullets, and the player all meet the surface they see.
Triangles wholly below the grid's drop_below level are left out (the water is opaque and the walls keep the player in).
Each tile is written as an OBJ (Saved/PointeSombreTerrain/, with the heightfield's own normals) and imported.

A tile whose stored hash matches the island data's hash is not rebuilt, so an unchanged island leaves the assets
byte-identical and a rebuild stays fast.
"""
import math
import os

import unreal

HASH_TAG = "DCIslandHash"
KINDS = ("turf", "rock", "shore", "seabed", "path")
ROCK_SLOPE_DEG = 38.0
SHORE_TOP_M = 1.4
SEABED_TOP_M = -0.4


def _log(msg):
    unreal.log_warning("[DCSOMBRE] " + msg)


def _triangle_kind(p0, p1, p2, island=None, origin=(0.0, 0.0)):
    ax, ay, az = p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]
    bx, by, bz = p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]
    nx, ny, nz = ay * bz - az * by, az * bx - ax * bz, ax * by - ay * bx
    length = math.sqrt(nx * nx + ny * ny + nz * nz) or 1.0
    slope = math.degrees(math.acos(min(1.0, abs(nz) / length)))
    # Vertices are in cm (tile-local); the height thresholds are metres. (VS-08 fix: until FORMULA_VERSION 7 the mean
    # was compared in cm, so the shingle band was under 1.4 cm instead of 1.4 m and the shore had almost no shingle.)
    mean = (p0[2] + p1[2] + p2[2]) / 300.0
    if mean < SEABED_TOP_M:
        return "seabed"
    if mean < SHORE_TOP_M:
        # The splash band is wave-washed stone even under a trail (VS-08 critic pass: the trail painted along the reef
        # causeway made it read as a paved bridge).
        return "shore"
    if island is not None and island.paths:
        # A trail (island.json "paths", VS-08): the path surface under its walking width, whatever the slope.
        cx = (p0[0] + p1[0] + p2[0]) / 300.0 + origin[0] / 100.0
        cy = (p0[1] + p1[1] + p2[1]) / 300.0 + origin[1] / 100.0
        d, path = island.path_distance(cx, cy)
        if path is not None and d <= path.half_width:
            return "path"
    if slope > ROCK_SLOPE_DEG:
        return "rock"
    return "turf"


def tile_ranges(island):
    """[(row, col, i0, i1, j0, j1)]: grid index ranges (inclusive) of each tile."""
    g = island.spec["grid"]
    step, tile = float(g["step"]), float(g["tile"])
    per = int(round(tile / step))
    nx = int(round((g["x_max"] - g["x_min"]) / step))
    ny = int(round((g["y_max"] - g["y_min"]) / step))
    out = []
    for row, i0 in enumerate(range(0, nx, per)):
        for col, j0 in enumerate(range(0, ny, per)):
            out.append((row, col, i0, min(i0 + per, nx), j0, min(j0 + per, ny)))
    return out


def _tile_geometry(island, xs, ys, heights, i0, i1, j0, j1):
    """Local-space vertices (cm, relative to the tile's corner) and triangles grouped by kind."""
    drop = float(island.spec["grid"]["drop_below"])
    ox, oy = xs[i0], ys[j0]
    verts = {}
    tris = {kind: [] for kind in KINDS}

    def vid(i, j):
        key = (i, j)
        if key not in verts:
            verts[key] = ((xs[i] - ox) * 100.0, (ys[j] - oy) * 100.0, heights[i][j] * 100.0)
        return key

    for i in range(i0, i1):
        for j in range(j0, j1):
            quad = ((i, j), (i + 1, j), (i + 1, j + 1), (i, j + 1))
            # Split along the diagonal that follows the terrain better (the shorter height difference).
            if abs(heights[i][j] - heights[i + 1][j + 1]) <= abs(heights[i + 1][j] - heights[i][j + 1]):
                pairs = ((quad[0], quad[1], quad[2]), (quad[0], quad[2], quad[3]))
            else:
                pairs = ((quad[0], quad[1], quad[3]), (quad[1], quad[2], quad[3]))
            for a, b, c in pairs:
                ha, hb, hc = heights[a[0]][a[1]], heights[b[0]][b[1]], heights[c[0]][c[1]]
                if ha < drop and hb < drop and hc < drop:
                    continue
                ka, kb, kc = vid(*a), vid(*b), vid(*c)
                tris[_triangle_kind(verts[ka], verts[kb], verts[kc], island, (ox * 100.0, oy * 100.0))].append(
                    (ka, kb, kc))
    return (ox * 100.0, oy * 100.0), verts, tris


OBJ_DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "Saved",
                                       "PointeSombreTerrain"))


def _vertex_normal(heights, xs, ys, i, j):
    """Unit normal (Unreal axes) of the heightfield at grid vertex (i, j), from central differences."""
    i0, i1 = max(i - 1, 0), min(i + 1, len(xs) - 1)
    j0, j1 = max(j - 1, 0), min(j + 1, len(ys) - 1)
    dhdx = (heights[i1][j] - heights[i0][j]) / (xs[i1] - xs[i0])
    dhdy = (heights[i][j1] - heights[i][j0]) / (ys[j1] - ys[j0])
    length = math.sqrt(dhdx * dhdx + dhdy * dhdy + 1.0)
    return -dhdx / length, -dhdy / length, 1.0 / length


def write_obj(obj_path, xs, ys, heights, origin, verts, tris):
    """The tile as Wavefront OBJ, with explicit per-vertex normals and one material group per kind.

    The importer reads the OBJ as right-handed and Z-up and only mirrors Y into Unreal's left-handed axes:
    (X, Y, Z) = (x, -y, z) (found in VS-04: writing it Y-up stood every tile on its edge). So a point is written
    (X, -Y, Z), and a face is listed counter-clockwise seen from above in OBJ space, its front face after the mirror.
    Deterministic text: the same island always writes the same file."""
    order = sorted(verts)
    index = {key: n + 1 for n, key in enumerate(order)}
    lines = ["# Pointe Sombre terrain tile, generated by Tools/EditorScripts/pointe_sombre/terrain_mesh.py. Do not edit."]
    for key in order:
        x, y, z = verts[key]
        lines.append(f"v {x:.2f} {-y:.2f} {z:.2f}")
    for key in order:
        x, y, _z = verts[key]
        lines.append(f"vt {(x + origin[0]) / 400.0:.4f} {(y + origin[1]) / 400.0:.4f}")
    for key in order:
        nx, ny, nz = _vertex_normal(heights, xs, ys, *key)
        lines.append(f"vn {nx:.5f} {-ny:.5f} {nz:.5f}")
    for kind in KINDS:
        if not tris[kind]:
            continue
        lines.append(f"g {kind}")
        lines.append(f"usemtl {kind}")
        for a, b, c in tris[kind]:
            # _tile_geometry lists (a, b, c) clockwise from above in Unreal axes; in OBJ space that is (a, c, b).
            ia, ib, ic = index[a], index[c], index[b]
            lines.append(f"f {ia}/{ia}/{ia} {ib}/{ib}/{ib} {ic}/{ic}/{ic}")
    os.makedirs(os.path.dirname(obj_path), exist_ok=True)
    with open(obj_path, "w", encoding="ascii", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")


def _import_obj(obj_path, folder, name):
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
    task.set_editor_property("destination_path", folder)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", False)
    task.set_editor_property("options", options)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh = unreal.load_asset(f"{folder}/{name}")
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Importing {obj_path} did not make a static mesh at {folder}/{name}")
    return mesh


def build_tile(path, island, xs, ys, heights, bounds, materials):
    """Create or rebuild one tile asset. Returns (origin_cm, triangle count), or (origin_cm, 0) for an empty tile.

    The tile is written as an OBJ with explicit normals and imported. Building the mesh in Python from a mesh
    description gave tiles with no usable normals whatever the build settings said, and M_DC_Surface takes its
    triplanar weights from the vertex normal, so the island rendered black (VS-04 captures 0836-0847; cubes with the
    same materials beside it rendered correctly)."""
    i0, i1, j0, j1 = bounds
    origin, verts, tris = _tile_geometry(island, xs, ys, heights, i0, i1, j0, j1)
    count = sum(len(t) for t in tris.values())
    if unreal.EditorAssetLibrary.does_asset_exist(path) and not unreal.EditorAssetLibrary.delete_asset(path):
        # A fresh asset each time: rebuilding a loaded tile in place asserted in a background worker (VS-04 spike).
        raise RuntimeError(f"Could not replace {path}")
    if count == 0:
        return origin, 0

    folder, name = path.rsplit("/", 1)
    obj_path = os.path.join(OBJ_DIR, name + ".obj")
    write_obj(obj_path, xs, ys, heights, origin, verts, tris)
    mesh = _import_obj(obj_path, folder, name)

    # Material slots come from the OBJ groups; give each its surface by slot name.
    static_materials = list(mesh.get_editor_property("static_materials"))
    for slot in static_materials:
        kind = str(slot.get_editor_property("material_slot_name"))
        if kind not in materials:
            raise RuntimeError(f"{path}: unexpected material slot {kind}")
        slot.set_editor_property("material_interface", materials[kind])
    mesh.set_editor_property("static_materials", static_materials)
    body = mesh.get_editor_property("body_setup")
    body.set_editor_property("agg_geom", unreal.KAggregateGeom())
    body.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    unreal.EditorAssetLibrary.set_metadata_tag(mesh, HASH_TAG, island.spec_hash())
    if not unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {path}")
    return origin, count


def tile_path(folder, row, col):
    return f"{folder}/SM_SombreTerrain_{row:02d}_{col:02d}"


def ensure_tiles(island, folder, materials, force=False):
    """Build every tile whose stored hash differs from the island's. Returns [(path, origin_cm)] of non-empty tiles."""
    want = island.spec_hash()
    xs, ys, heights = None, None, None
    out = []
    built = skipped = 0
    for row, col, i0, i1, j0, j1 in tile_ranges(island):
        path = tile_path(folder, row, col)
        g = island.spec["grid"]
        origin = ((g["x_min"] + i0 * g["step"]) * 100.0, (g["y_min"] + j0 * g["step"]) * 100.0)
        if not force and unreal.EditorAssetLibrary.does_asset_exist(path):
            mesh = unreal.load_asset(path)
            if unreal.EditorAssetLibrary.get_metadata_tag(mesh, HASH_TAG) == want:
                out.append((path, origin))
                skipped += 1
                continue
        if heights is None:
            xs, ys, heights = island.sample_grid()
        origin, count = build_tile(path, island, xs, ys, heights, (i0, i1, j0, j1), materials)
        if count:
            out.append((path, origin))
            built += 1
    # A tile that a smaller grid no longer has would linger; the map only places what this list returns.
    _log(f"terrain tiles: {built} built, {skipped} unchanged, {len(out)} placed (hash {want})")
    return out
