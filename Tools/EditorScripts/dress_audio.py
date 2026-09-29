"""Place the always-on shore ambience in Lvl_Boathouse_Art.

Owns only actors tagged ShoreAudio. Safe to re-run: those actors are replaced. build_boathouse.py does not load
the art level for editing and does not save it, so a content rebuild leaves the ambience in place.

Two 2D beds with no conditions: lake wind and water lap, at the starting volumes in
C:\\FO5_AssetLibrary\\Audio\\SOURCING_NOTES.md. Positional sounds that follow game state (the hums, the breaker) are
placed by build_boathouse.py because they belong to gameplay actors' state. Requires import_audio.py.

Run after build_boathouse.py:
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

MAP_PATH = "/Game/Maps/Lvl_Boathouse"
ART_MAP = "/Game/Maps/Lvl_Boathouse_Art"
TAG = "ShoreAudio"

# label, sound, volume multiplier
BEDS = [
    ("ShoreAudio_Wind", "/Game/Audio/Ambience/S_DC_LakeWind", 0.07),
    ("ShoreAudio_Lap", "/Game/Audio/Ambience/S_DC_WaterLap", 0.46),
]

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCAUDIODRESS] " + msg)


def package_name(obj):
    return str(obj.get_outermost().get_name()) if obj else ""


def clear():
    doomed = [actor for actor in actors.get_all_level_actors()
              if package_name(actor.get_outer()) == ART_MAP and TAG in [str(tag) for tag in actor.tags]]
    if doomed:
        actors.destroy_actors(doomed)
    log(f"cleared {len(doomed)} previous audio actors")


def main():
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        raise RuntimeError(f"Missing {MAP_PATH}")
    levels.load_level(MAP_PATH)
    if not levels.set_current_level_by_name("Lvl_Boathouse_Art"):
        raise RuntimeError("Could not edit Lvl_Boathouse_Art. Run build_boathouse.py first.")
    clear()
    placed = []
    for label, sound_path, volume in BEDS:
        sound = unreal.load_asset(sound_path)
        if not sound:
            raise RuntimeError(f"Missing {sound_path}. Run import_audio.py first.")
        actor = actors.spawn_actor_from_class(unreal.DCConditionalAudio, unreal.Vector(0.0, 0.0, 100.0))
        actor.set_actor_label(label)
        actor.set_editor_property("tags", [unreal.Name(TAG)])
        actor.set_editor_property("sound", sound)
        actor.set_editor_property("volume_multiplier", volume)
        if package_name(actor.get_outer()) != ART_MAP:
            raise RuntimeError(f"{label} landed in {package_name(actor.get_outer())}")
        placed.append(actor)
    package = placed[0].get_outer().get_outermost()
    if not unreal.EditorLoadingAndSavingUtils.save_packages([package], False):
        raise RuntimeError(f"Could not save {ART_MAP}")
    log(f"saved {len(placed)} beds in {ART_MAP}")


main()
