"""Read-only: parameters of the Survival_Character jacket, jeans, backpack instances. Log prefix [DCSURV]."""
import unreal


def log(msg):
    unreal.log_warning("[DCSURV] " + msg)


mel = unreal.MaterialEditingLibrary
for name in ("Jacket", "Jeans", "Backpack", "Skin", "Gloves", "Shoes", "Hair_Brown"):
    mi = unreal.load_asset(f"/Game/Survival_Character/Materials/MI_Survival_Character_{name}")
    log(f"== {name}: {mi.get_class().get_name()}")
    for fn in ("get_material_instance_vector_parameter_names", "get_material_instance_scalar_parameter_names",
               "get_material_instance_texture_parameter_names"):
        try:
            names = getattr(mel, fn)(mi)
            log(f"  {fn}: {[str(n) for n in names]}")
        except Exception as exc:
            log(f"  {fn}: {exc}")
    try:
        log(f"  static switches: {[str(p) for p in mi.get_editor_property('static_parameters')] if hasattr(mi, 'get_editor_property') else ''}")
    except Exception as exc:
        pass
    for n in ("Tint", "Color", "BaseColor", "Base Color Tint", "Base Color Multiplier"):
        try:
            log(f"  vec {n} = {mel.get_material_instance_vector_parameter_value(mi, n)}")
        except Exception:
            pass
log("done")
