"""Read-only: dump the graph feeding M_DC_Prop's base color. Log prefix [DCPROP]."""
import unreal

mel = unreal.MaterialEditingLibrary


def log(msg):
    unreal.log_warning("[DCPROP] " + msg)


material = unreal.load_asset("/Game/Environment/Materials/M_DC_Prop")
log(f"shading={material.get_editor_property('shading_model')} blend={material.get_editor_property('blend_mode')} two_sided={material.get_editor_property('two_sided')}")
for prop in (unreal.MaterialProperty.MP_BASE_COLOR, unreal.MaterialProperty.MP_ROUGHNESS, unreal.MaterialProperty.MP_METALLIC, unreal.MaterialProperty.MP_NORMAL):
    node = mel.get_material_property_input_node(material, prop)
    log(f"{prop.name}: {node.get_class().get_name() if node else None} out={mel.get_material_property_input_node_output_name(material, prop) if node else ''}")
    if node:
        try:
            for i, inp in enumerate(mel.get_inputs_for_material_expression(material, node)):
                log(f"   input {i}: {inp.get_class().get_name() if inp else None} {inp.get_editor_property('parameter_name') if inp and hasattr(inp, 'parameter_name') else ''}")
                if inp:
                    for j, inp2 in enumerate(mel.get_inputs_for_material_expression(material, inp)):
                        log(f"      input {j}: {inp2.get_class().get_name() if inp2 else None}")
        except Exception as exc:
            log(f"   inputs: {exc}")
log(f"used textures: {[t.get_name() for t in mel.get_used_textures(material)] if hasattr(mel, 'get_used_textures') else 'n/a'}")
log("done")
