"""Read-only: where is Mara's head bone, and how is it oriented? Log prefix [DCHEAD]."""
import unreal

levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels.load_level("/Game/Maps/Lvl_Boathouse")


def log(msg):
    unreal.log_warning("[DCHEAD] " + msg)


for actor in actors.get_all_level_actors():
    if actor.get_actor_label() != "Mara":
        continue
    comp = actor.get_editor_property("mesh")
    log(f"mesh comp relative loc={comp.get_editor_property('relative_location')} rot={comp.get_editor_property('relative_rotation')}")
    for bone in ("pelvis", "spine_05", "neck_01", "neck_02", "head"):
        try:
            t = comp.get_socket_transform(bone, unreal.RelativeTransformSpace.RTS_COMPONENT)
            rot = t.rotation.rotator()
            fwd = unreal.MathLibrary.get_forward_vector(rot)
            right = unreal.MathLibrary.get_right_vector(rot)
            up = unreal.MathLibrary.get_up_vector(rot)
            log(f"{bone}: loc=({t.translation.x:.1f},{t.translation.y:.1f},{t.translation.z:.1f}) "
                f"X=({fwd.x:.2f},{fwd.y:.2f},{fwd.z:.2f}) Y=({right.x:.2f},{right.y:.2f},{right.z:.2f}) Z=({up.x:.2f},{up.y:.2f},{up.z:.2f})")
        except Exception as exc:
            log(f"{bone}: {exc}")
    head = actor.get_editor_property("head_swap")
    log(f"head_swap comp: {head}")
mesh = unreal.load_asset("/Game/Art/Meshy/mara_head/SM_mara_head")
box = mesh.get_bounding_box()
log(f"head mesh bounds min=({box.min.x:.1f},{box.min.y:.1f},{box.min.z:.1f}) max=({box.max.x:.1f},{box.max.y:.1f},{box.max.z:.1f})")
log("done")
