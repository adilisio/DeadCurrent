"""Write the rocky-shoreline recipe into Lvl_PointeSombre_Biome as NoCollision HISM, and the manifest (plan §10).

    Tools\\RebuildContent.bat biome\\scatter        (also the last step of a full rebuild, after the map is built)

1. Loads /Game/Maps/Lvl_PointeSombre and makes sure the always-loaded sublevel /Game/Maps/Lvl_PointeSombre_Biome is
   linked (created once). The core map script never touches it; this script never touches the persistent level.
2. Gathers the automatic exclusions from the persistent level: every interactable, cell portal, player start,
   character, location volume, and actor tagged BiomeExclude (positions in whole cm).
3. Asks plan.generate() for the placements. Nothing here jitters, traces, or re-seeds.
4. Clears the sublevel and spawns one actor per zone (label Biome_<zone>, tags Biome and Biome:<zone>) with one
   HierarchicalInstancedStaticMeshComponent per family mesh (tags Biome:<family>, BiomeMesh:<index>): static,
   NoCollision, no navigation, the family's cull distance and material override.
5. Writes Tools/Biomes/out/Lvl_PointeSombre.json (sorted keys; the hash covers only integers and strings).

Set DC_BIOME_EMPTY=1 to write the sublevel with no instances (and no manifest change): a diagnostic, not a gate.
Owner: the shoreline recipe (WP-BIOME); the sublevel and the manifest are build products the Integrator commits.
"""
import json
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import plan  # noqa: E402

MAP = "/Game/Maps/Lvl_PointeSombre"
BIOME_MAP = "/Game/Maps/Lvl_PointeSombre_Biome"
BIOME_LEVEL = "Lvl_PointeSombre_Biome"
MANIFEST = os.path.join(plan.BIOMES, "out", "Lvl_PointeSombre.json")
BOUNDS_TOLERANCE_CM = 0.15   # the recipe records each mesh's bounds to 0.1 cm

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCBIOME] " + msg)


def package_name(obj):
    outer = obj.get_outermost() if obj else None
    return outer.get_name() if outer else ""


def editor_world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def biome_streaming():
    found = [s for s in unreal.EditorLevelUtils.get_levels(editor_world())
             if package_name(s) == BIOME_MAP]
    return found


def focus(package, names):
    """Make a loaded level current (every ULevel is named PersistentLevel, so the package is the id)."""
    for name in names:
        levels.set_current_level_by_name(name)
        if package_name(levels.get_current_level()) == package:
            return
    raise RuntimeError(f"Could not make {package} the current level")


def ensure_biome_level():
    """Link the always-loaded biome sublevel, creating it once. Returns (its loaded level, whether it was linked now)."""
    world = editor_world()
    linked = biome_streaming()
    if len(linked) > 1:
        raise RuntimeError(f"{BIOME_MAP} is linked {len(linked)} times")
    if linked:
        return linked[0], False
    if unreal.EditorAssetLibrary.does_asset_exist(BIOME_MAP):
        streaming = unreal.EditorLevelUtils.add_level_to_world(world, BIOME_MAP, unreal.LevelStreamingAlwaysLoaded)
        log(f"re-linked {BIOME_MAP}")
    else:
        streaming = unreal.EditorLevelUtils.create_new_streaming_level(unreal.LevelStreamingAlwaysLoaded, BIOME_MAP,
                                                                       False)
        log(f"created {BIOME_MAP}")
    if not streaming:
        raise RuntimeError(f"Could not link {BIOME_MAP}")
    linked = biome_streaming()
    if len(linked) != 1:
        raise RuntimeError(f"{BIOME_MAP} did not load after linking")
    return linked[0], True


def _cm(v):
    return int(round(v))


def gather_automatic():
    """Exclusions from the live persistent level, recorded in whole cm so the manifest replays them exactly."""
    interactable = unreal.DCInteractable.static_class()
    out = []
    for actor in actors.get_all_level_actors():
        if package_name(actor.get_outer()) != MAP:
            continue
        loc = actor.get_actor_location()
        if loc.y > plan.INTERIOR_Y_MIN_M * 100.0:
            continue   # interior cell slots: no shore there
        label = actor.get_actor_label()
        tags = [str(t) for t in actor.get_editor_property("tags")]
        circle = {"shape": "circle", "center_cm": [_cm(loc.x), _cm(loc.y)], "radius_cm": 0}
        if isinstance(actor, unreal.DCCellPortal):
            out.append({"kind": "portal", "id": label, "shape": circle})
        elif unreal.SystemLibrary.does_implement_interface(actor, interactable):
            out.append({"kind": "interactable", "id": label, "shape": circle})
        elif isinstance(actor, unreal.PlayerStart):
            out.append({"kind": "player_start", "id": label, "shape": circle})
        elif isinstance(actor, unreal.Character):
            out.append({"kind": "npc", "id": label, "shape": circle})
        elif isinstance(actor, unreal.DCLocationVolume):
            box = actor.get_editor_property("bounds")
            extent = box.get_scaled_box_extent()
            centre = box.get_world_location()
            out.append({"kind": "location_volume", "id": label, "shape": {
                "shape": "box", "center_cm": [_cm(centre.x), _cm(centre.y)],
                "half_cm": [_cm(extent.x), _cm(extent.y)], "yaw_cdeg": _cm(actor.get_actor_rotation().yaw * 100.0)}})
        if "BiomeExclude" in tags:
            origin, extent = actor.get_actor_bounds(False)
            out.append({"kind": "biome_exclude", "id": label, "shape": {
                "shape": "box", "center_cm": [_cm(origin.x), _cm(origin.y)],
                "half_cm": [_cm(extent.x), _cm(extent.y)], "yaw_cdeg": 0}})
    ids = [(a["kind"], a["id"]) for a in out]
    if len(set(ids)) != len(ids):
        raise RuntimeError(f"two automatic exclusions share a label: {sorted(ids)}")
    return sorted(out, key=lambda a: (a["kind"], a["id"]))


def check_meshes(recipe):
    """Every mesh exists, is not Nanite, and has the bounds the planner seated it with."""
    for fam in recipe["families"]:
        if not fam.get("enabled", True):
            continue
        for m in fam["meshes"]:
            mesh = unreal.load_asset(m["path"])
            if not mesh:
                raise RuntimeError(f"{fam['id']}: missing mesh {m['path']}")
            if mesh.get_editor_property("nanite_settings").get_editor_property("enabled"):
                raise RuntimeError(f"{m['path']} has Nanite on. Run biome\\prepare_biome_meshes.py first.")
            b = mesh.get_bounding_box()
            got = [b.min.x, b.min.y, b.min.z, b.max.x, b.max.y, b.max.z]
            want = m["bounds_cm"]["min"] + m["bounds_cm"]["max"]
            log(f"bounds {m['path'].rsplit('/', 1)[-1]}: min {[round(g, 1) for g in got[:3]]} max {[round(g, 1) for g in got[3:]]}")
            if any(abs(g - w) > BOUNDS_TOLERANCE_CM for g, w in zip(got, want)):
                raise RuntimeError(f"{m['path']}: bounds {[round(g, 2) for g in got]} differ from the recipe's "
                                   f"{want}; update bounds_cm in the recipe")
        override = fam.get("material_override")
        if override and not unreal.load_asset(override):
            raise RuntimeError(f"{fam['id']}: missing material {override}")


def clear_biome_level(level):
    doomed = [a for a in actors.get_all_level_actors() if package_name(a.get_outer()) == BIOME_MAP]
    actors.destroy_actors(doomed)
    unreal.SystemLibrary.collect_garbage()
    log(f"cleared {len(doomed)} biome actors")


def spawn_zone(zone_id, placements, recipe):
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    actor = actors.spawn_actor_from_class(unreal.Actor, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    actor.set_actor_label(f"Biome_{zone_id}")
    actor.set_folder_path("Biome")
    actor.set_editor_property("tags", [unreal.Name("Biome"), unreal.Name(f"Biome:{zone_id}")])
    root = sds.k2_gather_subobject_data_for_instance(actor)[0]
    families = {f["id"]: f for f in recipe["families"] if f.get("enabled", True)}
    groups = {}
    for p in placements:
        groups.setdefault((p["family"], p["mesh"]), []).append(p)
    components = []
    for (fid, index) in sorted(groups):
        fam = families[fid]
        mesh_spec = fam["meshes"][index]
        handle, fail = sds.add_new_subobject(unreal.AddNewSubobjectParams(
            parent_handle=root, new_class=unreal.HierarchicalInstancedStaticMeshComponent, blueprint_context=None))
        if fail and str(fail):
            raise RuntimeError(f"could not add a component for {fid}/{index}: {fail}")
        comp = lib.get_object(lib.get_data(handle))
        comp.set_static_mesh(unreal.load_asset(mesh_spec["path"]))
        if fam.get("material_override"):
            material = unreal.load_asset(fam["material_override"])
            for slot in range(comp.get_num_materials()):
                comp.set_material(slot, material)
        comp.set_editor_property("component_tags", [unreal.Name("Biome"), unreal.Name(f"Biome:{fid}"),
                                                    unreal.Name(f"BiomeMesh:{index}")])
        comp.set_mobility(unreal.ComponentMobility.STATIC)
        comp.set_collision_profile_name("NoCollision")
        comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        comp.set_editor_property("can_ever_affect_navigation", False)
        comp.set_editor_property("instance_end_cull_distance", int(fam["cull_distance_cm"]))
        comp.set_editor_property("instance_start_cull_distance", int(fam["cull_distance_cm"] * 0.8))
        if int(fam.get("min_lod", 0)) > 0:
            comp.set_editor_property("override_min_lod", True)
            comp.set_editor_property("min_lod", int(fam["min_lod"]))
        transforms = []
        for p in groups[(fid, index)]:
            s = p["scale_milli"] / 1000.0
            transforms.append(unreal.Transform(
                location=unreal.Vector(p["x_mm"] / 10.0, p["y_mm"] / 10.0, p["z_mm"] / 10.0),
                rotation=unreal.Rotator(pitch=p["pitch_cdeg"] / 100.0, yaw=p["yaw_cdeg"] / 100.0,
                                        roll=p["roll_cdeg"] / 100.0),
                scale=unreal.Vector(s, s, s)))
        comp.add_instances(transforms, False, True)
        if comp.get_instance_count() != len(transforms):
            raise RuntimeError(f"{fid}/{index}: {comp.get_instance_count()} instances, expected {len(transforms)}")
        components.append({"actor": f"Biome_{zone_id}", "family": fid, "mesh": index, "path": mesh_spec["path"],
                           "instances": len(transforms), "collision": "NoCollision",
                           "cull_distance_cm": int(fam["cull_distance_cm"]), "min_lod": int(fam.get("min_lod", 0))})
    return components


def write_manifest(body, components):
    manifest = dict(body)
    manifest["map"] = "Lvl_PointeSombre"
    manifest["sublevel"] = BIOME_MAP
    manifest["about"] = ("Generated by Tools/EditorScripts/biome/scatter.py from the recipe, the zone files, island.json, "
                         "and the automatic exclusions recorded here. Do not edit. 'hash' is SHA-1 over the canonical "
                         "JSON (sorted keys, no spaces) of planner, island_hash, inputs, counts, and placements. "
                         "Placements: mm, hundredths of a degree, thousandths of scale.")
    manifest["components"] = components
    os.makedirs(os.path.dirname(MANIFEST), exist_ok=True)
    with open(MANIFEST, "w", encoding="utf-8", newline="\n") as handle:
        json.dump(manifest, handle, sort_keys=True, indent=1, ensure_ascii=False)
        handle.write("\n")


def main():
    island = plan.island_mod.Island.load()
    recipe = plan.load_recipe()
    zones = plan.load_zones()
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP):
        raise RuntimeError(f"{MAP} does not exist. Run build_pointe_sombre first.")
    levels.load_level(MAP)
    check_meshes(recipe)
    level, newly_linked = ensure_biome_level()
    automatic = gather_automatic()
    log(f"automatic exclusions: {len(automatic)} ({sorted(set(a['kind'] for a in automatic))})")
    body = plan.generate(island, recipe, zones, automatic)
    empty = os.environ.get("DC_BIOME_EMPTY") == "1"

    clear_biome_level(level)
    focus(BIOME_MAP, (BIOME_LEVEL,))
    components = []
    if not empty:
        by_zone = {}
        for p in body["placements"]:
            by_zone.setdefault(p["zone"], []).append(p)
        for zone_id in sorted(by_zone):
            components.extend(spawn_zone(zone_id, by_zone[zone_id], recipe))
    for actor in actors.get_all_level_actors():
        if package_name(actor.get_outer()) == BIOME_MAP and actor.get_actor_label().startswith("Biome_"):
            continue
        if package_name(actor.get_outer()) == BIOME_MAP and not isinstance(actor, unreal.WorldSettings):
            raise RuntimeError(f"unexpected actor in the biome level: {actor.get_actor_label()}")
    focus(MAP, (MAP.rsplit("/", 1)[-1], "PersistentLevel"))

    if not unreal.EditorLoadingAndSavingUtils.save_packages([level.get_outermost()], False):
        raise RuntimeError(f"Could not save {BIOME_MAP}")
    # The persistent map is saved only when the link was just made (its actors are untouched), so a normal run
    # leaves Lvl_PointeSombre.umap byte-identical.
    if newly_linked:
        if not levels.save_current_level():
            raise RuntimeError(f"Could not save {MAP}")
        log(f"saved {MAP} with the biome sublevel linked")
    if empty:
        log("DC_BIOME_EMPTY=1: wrote an empty biome level; the manifest is unchanged")
        return
    write_manifest(body, components)
    total = sum(body["counts"].values())
    log(f"wrote {total} instances in {len(components)} components; counts {body['counts']}; capped {body['capped']}; "
        f"hash {body['hash']}")


main()
