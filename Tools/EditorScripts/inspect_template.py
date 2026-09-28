"""Dump template character, input and level settings to the log (read-only)."""
import unreal

def log(msg):
    unreal.log_warning("[DCINSPECT] " + str(msg))

bp_class = unreal.EditorAssetLibrary.load_blueprint_class("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter")
cdo = unreal.get_default_object(bp_class)
move = cdo.get_editor_property("character_movement")
for prop in ["max_walk_speed", "max_walk_speed_crouched", "jump_z_velocity", "air_control",
             "gravity_scale", "braking_deceleration_walking", "ground_friction",
             "max_acceleration", "braking_deceleration_falling"]:
    log(f"move.{prop} = {move.get_editor_property(prop)}")
nav = move.get_editor_property("nav_agent_props")
log(f"can_crouch = {nav.get_editor_property('can_crouch')}")
cap = cdo.get_editor_property("capsule_component")
log(f"capsule half height = {cap.get_unscaled_capsule_half_height()} radius = {cap.get_unscaled_capsule_radius()}")
log(f"crouched half height = {move.get_editor_property('crouched_half_height')}")
log(f"mesh rel loc = {cdo.get_editor_property('mesh').get_editor_property('relative_location')}")

for imc_path in ["/Game/Input/IMC_Default", "/Game/Input/IMC_MouseLook"]:
    imc = unreal.load_asset(imc_path)
    for m in imc.get_editor_property("default_key_mappings").get_editor_property("mappings"):
        action = m.get_editor_property("action")
        mods = [type(x).__name__ for x in m.get_editor_property("modifiers")]
        trig = [type(x).__name__ for x in m.get_editor_property("triggers")]
        log(f"{imc_path}: {action.get_name() if action else None} <- {m.get_editor_property('key').get_editor_property('key_name')} mods={mods} triggers={trig}")

for ia in ["IA_Jump", "IA_Move", "IA_Look", "IA_MouseLook"]:
    a = unreal.load_asset("/Game/Input/Actions/" + ia)
    log(f"{ia}: value_type={a.get_editor_property('value_type')}")

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/FirstPerson/Lvl_FirstPerson")
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
for a in actors:
    log(f"actor {a.get_class().get_name()} {a.get_actor_label()} at {a.get_actor_location()}")
