"""Read-only: the head instance, its master, and the masked graph. Log prefix [DCPROP]."""
import unreal

mel = unreal.MaterialEditingLibrary


def log(msg):
    unreal.log_warning("[DCPROP] " + msg)


mi = unreal.load_asset("/Game/Art/Meshy/mara_head/MI_mara_head")
parent = mi.get_editor_property("parent")
log(f"instance parent={parent.get_path_name() if parent else None} CutZ={mel.get_material_instance_scalar_parameter_value(mi, 'CutZ')}")
mat = unreal.load_asset("/Game/Environment/Materials/M_DC_PropCut")
log(f"cut master blend={mat.get_editor_property('blend_mode')} clip={mat.get_editor_property('opacity_mask_clip_value')}")
node = mel.get_material_property_input_node(mat, unreal.MaterialProperty.MP_OPACITY_MASK)
log(f"opacity mask node={node.get_class().get_name() if node else None}")
mesh = unreal.load_asset("/Game/Art/Meshy/mara_head/SM_mara_head")
log("slots: " + str([s.get_editor_property('material_interface').get_path_name() for s in mesh.get_editor_property('static_materials')]))
b = mesh.get_bounding_box()
log(f"bounds z {b.min.z:.1f}..{b.max.z:.1f}")
