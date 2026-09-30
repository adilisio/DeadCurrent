"""Read-only: bounds of the name board, the life jackets, the Tern mesh, and the hull blockout. Log prefix [DCFLOAT]."""
import unreal

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels.load_level("/Game/Maps/Lvl_Boathouse")


def log(msg):
    unreal.log_warning("[DCFLOAT] " + msg)


def show(actor):
    o, e = actor.get_actor_bounds(False)
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    mesh = comp.get_editor_property("static_mesh").get_name() if comp and comp.get_editor_property("static_mesh") else "-"
    log(f"{actor.get_actor_label():24s} mesh={mesh:22s} x {o.x-e.x:8.1f}..{o.x+e.x:8.1f}  y {o.y-e.y:8.1f}..{o.y+e.y:8.1f}  z {o.z-e.z:7.1f}..{o.z+e.z:7.1f}")


wanted = ("NameBoard", "NameBoardLetters", "LifeJackets", "Hull", "Bow", "Transom", "Wheelhouse_Roof")
for actor in actors.get_all_level_actors():
    label = actor.get_actor_label()
    tags = [str(t) for t in actor.tags]
    if label in wanted or "StructureDress" in tags and "tern" in label.lower() or "motorboat" in label.lower():
        show(actor)
for actor in actors.get_all_level_actors():
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    if comp and comp.get_editor_property("static_mesh") and "motorboat" in comp.get_editor_property("static_mesh").get_name().lower():
        show(actor)
log("done")
