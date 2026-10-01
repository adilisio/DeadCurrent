"""Count the settlement kit's use (VerticalSlicePhasePlan.txt §9.3). Read-only: it changes no asset and no level.

    Tools\\RebuildContent.bat report_kit_usage                      the kit gym, Lvl_KitGym
    (set DC_KIT_REPORT_MAP=/Game/Maps/<map> to count another level, e.g. Lvl_PointeSombre once it uses the kit)

Two parts, written to the log ([DCKIT] lines) and to Saved/KitUsage/<map>.txt (deterministic text: two runs on the
same level give identical files, which is the reproducibility check):
  1. the LEVEL: actors tagged Structure:<id> and their components tagged Kit:<PieceId>, counted by instances
  2. the PROJECTION: every composition in Tools/Kits/great_lakes_settlement/structures/, planned by compose.plan()
     without the engine, so the measures for the slice's structures can be read before they are placed
"""
import collections
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "kit"))
import compose  # noqa: E402
import modules  # noqa: E402

TARGETS = [
    ("Structures composed from the kit", ">= 10 (slice)"),
    ("Distinct kit pieces used", "<= 30"),
    ("Pieces reused in >= 3 structures", ">= 10 (slice)"),
    ("Share of structural placements that are kit pieces", ">= 75%"),
    ("Bespoke structural assets (buildings)", "0"),
]


def measures(per_structure, structural_kit, structural_other, bespoke):
    """per_structure: {structure id: Counter(piece -> placements)}."""
    used = collections.Counter()
    for counts in per_structure.values():
        for piece in counts:
            used[piece] += 1
    reused = sorted(p for p, n in used.items() if n >= 3)
    total = structural_kit + structural_other
    share = 100.0 * structural_kit / total if total else 0.0
    return [
        len(per_structure),
        len(used),
        f"{len(reused)} ({', '.join(reused)})",
        f"{share:.1f}% ({structural_kit} of {total})",
        bespoke,
    ]


def projection(kit):
    per, kit_n = {}, 0
    for name in compose.structure_names():
        spec = compose.load_structure(name)
        counts = collections.Counter(p.piece for p in compose.plan(spec, kit))
        per[spec["id"]] = counts
        kit_n += sum(n for piece, n in counts.items() if piece in modules.MODULES)
    return per, kit_n


def level(map_path, kit):
    import unreal
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(map_path)
    known_meshes = {f"{kit['module_folder']}/SM_Kit_{m}" for m in modules.MODULES}
    known_meshes |= {a["mesh"] for a in kit["attachments"].values()}
    per, kit_n, other_n, bespoke = {}, 0, 0, set()
    for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        sids = [str(t).split(":", 1)[1] for t in actor.get_editor_property("tags") if str(t).startswith("Structure:")]
        if not sids:
            continue
        counts = per.setdefault(sids[0], collections.Counter())
        actor_pieces = [str(t)[4:] for t in actor.get_editor_property("tags") if str(t).startswith("Kit:")]
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            mesh = comp.get_editor_property("static_mesh")
            path = mesh.get_path_name().split(".")[0] if mesh else ""
            n = comp.get_instance_count() if isinstance(comp, unreal.InstancedStaticMeshComponent) else 1
            pieces = [str(t)[4:] for t in comp.get_editor_property("component_tags") if str(t).startswith("Kit:")] or actor_pieces
            if pieces and path in known_meshes:
                counts[pieces[0]] += n
                if pieces[0] in modules.MODULES:
                    kit_n += n
            elif n:
                other_n += n
                bespoke.add(path)
    return per, kit_n, other_n, len(bespoke)


def render(title, rows, per):
    lines = [title]
    for (label, target), value in zip(TARGETS, rows):
        lines.append(f"  {label}: {value}   [target {target}]")
    for sid in sorted(per):
        detail = ", ".join(f"{p} x{n}" for p, n in sorted(per[sid].items()))
        lines.append(f"  {sid}: {sum(per[sid].values())} placements: {detail}")
    return lines


def main():
    kit = compose.load_kit()
    map_path = os.environ.get("DC_KIT_REPORT_MAP", "/Game/Maps/Lvl_KitGym")
    out = [f"Great Lakes settlement kit usage, {len(modules.MODULES)} authored modules + {len(kit['attachments'])} library attachments"]
    per, kit_n, other_n, bespoke = level(map_path, kit)
    out += render(f"LEVEL {map_path}", measures(per, kit_n, other_n, bespoke), per)
    pper, pkit = projection(kit)
    out += render("PROJECTION (every composition, planned without the engine)", measures(pper, pkit, 0, 0), pper)
    text = "\n".join(out) + "\n"
    folder = os.path.normpath(os.path.join(HERE, "..", "..", "Saved", "KitUsage"))
    os.makedirs(folder, exist_ok=True)
    with open(os.path.join(folder, map_path.rsplit("/", 1)[-1] + ".txt"), "w", encoding="utf-8", newline="\n") as handle:
        handle.write(text)
    import unreal
    for line in out:
        unreal.log_warning("[DCKIT] " + line)


main()
