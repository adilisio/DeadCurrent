"""Create or update dialogue Data Assets in /Game/Dialogue.

Safe to re-run. Run with:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

DIALOGUE_PATH = "/Game/Dialogue"

# Each node: id, speaker, line, choices=[(text, next_id_or_None), ...]
MARA_INTRO = dict(
    asset="DA_Dialogue_MaraIntro",
    dialogue_id="mara_intro",
    entry="greeting",
    nodes=[
        dict(
            id="greeting",
            speaker="Mara",
            line="Keep your voice down. There's a scavenger past those plates.",
            choices=[
                ("Who are you?", "who"),
                ("What is this place?", "place"),
                ("Goodbye", None),
            ],
        ),
        dict(
            id="who",
            speaker="Mara",
            line="Name's Mara. I watch the shore for people who still listen before they shoot.",
            choices=[
                ("What is this place?", "place"),
                ("Goodbye", None),
            ],
        ),
        dict(
            id="place",
            speaker="Mara",
            line="Old Great Lakes Maritime ground. Boathouse behind you, scavengers ahead. Pick a side carefully.",
            choices=[
                ("Who are you?", "who"),
                ("Goodbye", None),
            ],
        ),
    ],
)


def log(msg):
    unreal.log_warning("[DCDIALOGUE] " + msg)


def get_or_create(asset_name):
    path = f"{DIALOGUE_PATH}/{asset_name}"
    if not unreal.EditorAssetLibrary.does_directory_exist(DIALOGUE_PATH):
        unreal.EditorAssetLibrary.make_directory(DIALOGUE_PATH)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        return unreal.load_asset(path)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.DCDialogueAsset)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, DIALOGUE_PATH, unreal.DCDialogueAsset, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    log(f"created {path}")
    return asset


def make_node(spec):
    node = unreal.DCDialogueNode()
    node.set_editor_property("node_id", spec["id"])
    node.set_editor_property("speaker", unreal.Text(spec["speaker"]))
    node.set_editor_property("line", unreal.Text(spec["line"]))
    choices = []
    for text, next_id in spec["choices"]:
        choice = unreal.DCDialogueChoice()
        choice.set_editor_property("text", unreal.Text(text))
        choice.set_editor_property("next_node_id", unreal.Name(next_id) if next_id else unreal.Name())
        choices.append(choice)
    node.set_editor_property("choices", choices)
    return node


def write_dialogue(spec):
    asset = get_or_create(spec["asset"])
    asset.set_editor_property("dialogue_id", spec["dialogue_id"])
    asset.set_editor_property("entry_node_id", spec["entry"])
    asset.set_editor_property("nodes", [make_node(n) for n in spec["nodes"]])
    unreal.EditorAssetLibrary.save_asset(f"{DIALOGUE_PATH}/{spec['asset']}")
    log(f"saved {DIALOGUE_PATH}/{spec['asset']} ({len(spec['nodes'])} nodes)")


def main():
    write_dialogue(MARA_INTRO)


main()
