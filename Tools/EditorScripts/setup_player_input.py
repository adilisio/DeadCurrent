"""Create the project's player input actions, map them in IMC_Default, and assign them to the player Blueprint.

Add new player actions to ACTION_KEYS and BP_PROPERTIES. Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

ACTIONS_PATH = "/Game/Input/Actions"
IMC_PATH = "/Game/Input/IMC_Default"
PLAYER_BP_PATH = "/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"

ACTION_KEYS = {
    "IA_Sprint": ["LeftShift", "Gamepad_LeftThumbstick"],
    "IA_Crouch": ["LeftControl", "C", "Gamepad_FaceButton_Right"],
    "IA_Interact": ["E", "Gamepad_FaceButton_Left"],
}

BP_PROPERTIES = {
    "IA_Sprint": "sprint_action",
    "IA_Crouch": "crouch_action",
    "IA_Interact": "interact_action",
}


def log(msg):
    unreal.log_warning("[DCINPUT] " + msg)


def save(asset):
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Could not save {asset.get_path_name()} (is the file read-only?)")


def get_or_create_action(name):
    path = f"{ACTIONS_PATH}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    action = tools.create_asset(name, ACTIONS_PATH, unreal.InputAction, unreal.InputAction_Factory())
    action.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
    save(action)
    log(f"created {path}")
    return action


def make_key(name):
    key = unreal.Key()
    key.set_editor_property("key_name", name)
    return key


def main():
    imc = unreal.load_asset(IMC_PATH)
    existing = {
        (m.get_editor_property("action").get_name(), str(m.get_editor_property("key").get_editor_property("key_name")))
        for m in imc.get_editor_property("default_key_mappings").get_editor_property("mappings")
        if m.get_editor_property("action")
    }

    actions = {}
    for name, keys in ACTION_KEYS.items():
        action = get_or_create_action(name)
        actions[name] = action
        for key in keys:
            if (name, key) not in existing:
                imc.map_key(action, make_key(key))
                log(f"mapped {name} <- {key}")
    # map_key does not mark the package dirty.
    save(imc)

    bp = unreal.load_asset(PLAYER_BP_PATH)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(unreal.EditorAssetLibrary.load_blueprint_class(PLAYER_BP_PATH))
    for name, prop in BP_PROPERTIES.items():
        cdo.set_editor_property(prop, actions[name])
        log(f"{PLAYER_BP_PATH}.{prop} = {name}")
    save(bp)
    log("done")


main()
