"""Place the Landing Stage's NoCollision dressing in Lvl_Boathouse_Art (Phase 5 pilot, Tier C, hand-placed).

Owns only actors tagged LandingDress. Safe to re-run: those actors are replaced. Nothing here has collision, and
nothing here changes with state: the stage's state-driven pieces are gameplay actors in build_landing_stage.py.
Kept off the lake-side walk (beach Y -580..-300 between X 700 and 2000 stays clear of anything that blocks), the
door apron, the crate's interaction side, and Mara's coil-route spot on the deck (930, -960).

Spec: Design/POIs/shore.landing_stage.md (Asset Plan, Tier C). Like the other dress_* scripts it is not part of
RebuildContent.bat, and a content rebuild leaves its actors.

Run after build_boathouse.py (and after Tools\\ImportPackAssets.ps1 -Name landing, for the old piles):
UnrealEditor-Cmd.exe DeadCurrent.uproject -run=pythonscript -script=<this file> -unattended -nullrhi
"""
import unreal

MAP_PATH = "/Game/Maps/Lvl_Boathouse"
ART_MAP = "/Game/Maps/Lvl_Boathouse_Art"
TAG = "LandingDress"

TRUNK = "/Game/Art/PolyHaven/dead_tree_trunk"
CAN = "/Game/Art/PolyHaven/can_rusted"
PILE = "/Game/LevelPrototyping/Meshes/SM_Cylinder"
PILE_MATERIAL = "/Game/Environment/Materials/MI_DC_Pile"

# mesh, x, y, bottom z, (pitch, yaw, roll), longest-axis cm, optional (sx, sy, sz) stretch after the uniform scale
PLACEMENTS = [
    # A drowned trunk half in the water beside the gangway root. (The stage's lying branch was removed after the
    # Visual Critic's V-02: its fork hung in the air over the curb.)
    (TRUNK, 1215.0, -650.0, -8.0, (0.0, -15.0, 0.0), 190.0),
    # A spare tin on the deck by the lean-to post, a second one fallen off the gangway.
    (CAN, 1245.0, -1060.0, 30.0, (0.0, 30.0, 0.0), 15.0),
    (CAN, 1085.0, -760.0, -6.0, (0.0, 10.0, 80.0), 15.0),
    # Three older piles standing up out of the lake bed east of the stage: an earlier landing that went. They start
    # well under the surface so nothing shows a gap at the waterline (V-03).
    (PILE, 1360.0, -1010.0, -90.0, (3.0, 0.0, -4.0), 190.0, (0.1, 0.1, 1.0)),
    (PILE, 1430.0, -1190.0, -90.0, (-6.0, 0.0, 5.0), 160.0, (0.12, 0.12, 1.0)),
    (PILE, 1395.0, -1110.0, -90.0, (0.0, 0.0, 9.0), 130.0, (0.11, 0.11, 1.0)),
]

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCLANDRESS] " + msg)


def package_name(obj):
    return str(obj.get_outermost().get_name()) if obj else ""


def find_mesh(path):
    asset = unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else None
    if isinstance(asset, unreal.StaticMesh):
        return asset
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        return None
    best, best_size = None, -1.0
    for asset_path in unreal.EditorAssetLibrary.list_assets(path, recursive=True, include_folder=False):
        candidate = unreal.load_asset(asset_path)
        if isinstance(candidate, unreal.StaticMesh):
            box = candidate.get_bounding_box()
            size = (box.max - box.min).length()
            if size > best_size:
                best, best_size = candidate, size
    return best


def clear_dressing():
    doomed = [a for a in actors.get_all_level_actors()
              if package_name(a.get_outer()) == ART_MAP and TAG in [str(t) for t in a.tags]]
    if doomed:
        actors.destroy_actors(doomed)
    log(f"cleared {len(doomed)} previous landing dressing actors")


def place(mesh, x, y, bottom_z, rotation, longest_cm, stretch=None):
    box = mesh.get_bounding_box()
    extent = box.max - box.min
    scale = longest_cm / max(extent.x, extent.y, extent.z, 1.0)
    sx, sy, sz = stretch or (1.0, 1.0, 1.0)
    actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, 0.0),
                                          unreal.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]))
    actor.set_actor_scale3d(unreal.Vector(scale * sx, scale * sy, scale * sz))
    actor.set_actor_label(f"LandingDress_{actor.get_name()}")
    actor.set_editor_property("tags", [unreal.Name(TAG)])
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(mesh)
    if mesh.get_path_name().startswith(PILE):
        comp.set_material(0, unreal.load_asset(PILE_MATERIAL))
    comp.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    actor.set_actor_enable_collision(False)
    origin, ext = actor.get_actor_bounds(False)
    loc = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(loc.x + x - origin.x, loc.y + y - origin.y,
                                           loc.z - (origin.z - ext.z) + bottom_z), False, True)
    if package_name(actor.get_outer()) != ART_MAP:
        raise RuntimeError(f"{actor.get_actor_label()} landed in {package_name(actor.get_outer())}")
    return actor


def main():
    if not unreal.EditorAssetLibrary.does_asset_exist(MAP_PATH):
        raise RuntimeError(f"Missing {MAP_PATH}")
    levels.load_level(MAP_PATH)
    if not levels.set_current_level_by_name("Lvl_Boathouse_Art"):
        raise RuntimeError("Could not edit Lvl_Boathouse_Art. Run build_boathouse.py first.")
    clear_dressing()
    placed = []
    for path, x, y, z, rot, longest, *stretch in PLACEMENTS:
        mesh = find_mesh(path)
        if not mesh:
            log(f"skip {path}: not in the project (run Tools\\ImportPackAssets.ps1 -Name landing)")
            continue
        placed.append(place(mesh, x, y, z, rot, longest, stretch[0] if stretch else None))
    if not placed:
        raise RuntimeError("Nothing placed")
    package = placed[0].get_outer().get_outermost()
    if not unreal.EditorLoadingAndSavingUtils.save_packages([package], False):
        raise RuntimeError(f"Could not save {ART_MAP}")
    log(f"saved {len(placed)} landing dressing actors in {ART_MAP}")


main()
