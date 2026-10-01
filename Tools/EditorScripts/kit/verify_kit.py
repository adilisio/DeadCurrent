"""Check the settlement kit and its gym headless (Phase 6 WP-KIT, VS-05). A failed check raises, which fails
Tools\\RebuildContent.bat kit\\verify_kit.

  1. every piece any composition uses (modules and attachments) exists as an asset
  2. every composition plans without error, deterministically (twice, identical)
  3. Lvl_KitGym holds exactly the gym structures, each one actor tagged Structure:<id> whose components are each
     tagged Kit:<PieceId>, with as many instances as the plan has placements
  4. no visible kit component shows an engine default, grid, or prototype material
  5. the gym structures' footprints do not overlap
  6. the gym structures are compositionally distinct (footprint, storeys, roof, openings, porch, stair, chimney),
     so three recolors of one box fail
"""
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)
import compose  # noqa: E402
import modules  # noqa: E402

MAP_PATH = "/Game/Maps/Lvl_KitGym"
BAD_MATERIALS = ("WorldGridMaterial", "DefaultMaterial", "MI_PrototypeGrid", "MI_DefaultColorway", "DefaultLitMaterial")


def log(msg):
    unreal.log_warning("[DCKIT] verify: " + msg)


def fail(msg):
    raise RuntimeError("verify_kit: " + msg)


def main():
    kit = compose.load_kit()
    specs = {name: compose.load_structure(name) for name in compose.structure_names()}

    # 1, 2
    pieces = set()
    for name, spec in specs.items():
        a = [p.as_tuple() for p in compose.plan(spec, kit)]
        b = [p.as_tuple() for p in compose.plan(spec, kit)]
        if a != b:
            fail(f"{name} does not plan deterministically")
        pieces |= {p[0] for p in a}
    for piece in sorted(pieces):
        path = kit["attachments"][piece]["mesh"] if piece in kit["attachments"] else f"{kit['module_folder']}/SM_Kit_{piece}"
        if not unreal.EditorAssetLibrary.does_asset_exist(path):
            fail(f"piece {piece} is missing ({path})")
    for module_id in modules.MODULES:
        if not unreal.EditorAssetLibrary.does_asset_exist(f"{kit['module_folder']}/SM_Kit_{module_id}"):
            fail(f"module {module_id} was never imported")
    log(f"{len(specs)} compositions plan deterministically; {len(pieces)} pieces in use all exist")

    # 3, 4, 5
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP_PATH)
    level_actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    structures, attached = {}, {}
    for actor in level_actors:
        tags = [str(t) for t in actor.get_editor_property("tags")]
        sid = next((t.split(":", 1)[1] for t in tags if t.startswith("Structure:")), None)
        if sid is None:
            continue
        if any(t.startswith("Kit:") for t in tags):
            attached[sid] = attached.get(sid, 0) + 1   # a library attachment: its own actor, tagged Kit: and Structure:
            for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
                for i in range(comp.get_num_materials()):
                    mat = comp.get_material(i)
                    name = mat.get_path_name() if mat else "None"
                    if mat is None or any(bad in name for bad in BAD_MATERIALS):
                        fail(f"{sid}: attachment {actor.get_actor_label()} slot {i} shows {name}")
        else:
            structures[sid] = actor
    want = {spec["id"]: spec for spec in specs.values() if spec.get("gym")}
    if set(structures) != set(want):
        fail(f"gym structures {sorted(structures)} != compositions marked gym {sorted(want)}")
    # A recolor of one box shares footprint, storey count, and roof type. Skins are not part of this signature.
    cores = {}
    for sid, spec in want.items():
        roof = spec.get("roof") or {}
        core = (tuple(spec["footprint"]), int(spec.get("storeys", 1)), roof.get("type"),
                tuple(sorted(spec.get("open_sides", []))), spec.get("porch") is not None,
                spec.get("stair") is not None, "chimney" in spec)
        cores[sid] = core
    if len(set(cores.values())) != len(cores):
        fail(f"gym structures are not compositionally distinct: {cores}")
    boxes = {}
    for sid, actor in structures.items():
        expected = len(compose.plan(want[sid], kit))
        count = attached.get(sid, 0)
        for comp in actor.get_components_by_class(unreal.InstancedStaticMeshComponent):
            ctags = [str(t) for t in comp.get_editor_property("component_tags")]
            if not any(t.startswith("Kit:") for t in ctags) or f"Structure:{sid}" not in ctags:
                fail(f"{sid}: component {comp.get_name()} lacks its Kit:/Structure: tags ({ctags})")
            count += comp.get_instance_count()
            for i in range(comp.get_num_materials()):
                mat = comp.get_material(i)
                name = mat.get_path_name() if mat else "None"
                if mat is None or any(bad in name for bad in BAD_MATERIALS):
                    fail(f"{sid}: {comp.get_name()} slot {i} shows {name}")
        if count != expected:
            fail(f"{sid}: {count} instances and attachments, plan has {expected}")
        origin, extent = actor.get_actor_bounds(False, True)
        boxes[sid] = (origin.x - extent.x, origin.y - extent.y, origin.x + extent.x, origin.y + extent.y)
        log(f"{sid}: {count} instances, bounds {boxes[sid]}")
    ids = sorted(boxes)
    for i, a in enumerate(ids):
        for b in ids[i + 1:]:
            A, B = boxes[a], boxes[b]
            if A[0] < B[2] and B[0] < A[2] and A[1] < B[3] and B[1] < A[3]:
                fail(f"footprints of {a} and {b} overlap")
    log("all checks passed")


main()
