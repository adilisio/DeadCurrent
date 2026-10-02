"""Art sublevels for Pointe Sombre: one always-loaded Lvl_PointeSombre_Art_<Cell> per cell (VerticalSlicePhasePlan.txt §5.6).

Integrator-owned helper for the cells' dress_sombre_<cell>.py scripts (VS-10 made the first). A sublevel holds only
stateless dressing: NoCollision props that never change with the story. Anything that follows state, that a player
stands on, or that blocks belongs in the persistent map, built by the cell's pointe_sombre/<cell>.py.

    sub = art.ArtLevel("Harbor")          # loads Lvl_PointeSombre, links the sublevel (created once), clears it
    sub.prop("Quay_Pile_00", MESH, (x_cm, y_cm), height_cm=380.0, sink_cm=150.0)
    sub.save()                             # the sublevel; the persistent map only if the link was just made

The same run twice writes the same sublevel. The core map script never touches an art sublevel, and a dress script
never touches the persistent level's actors (a core rebuild keeps the link, as it does the biome's).
"""
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import island as island_mod  # noqa: E402

MAP = "/Game/Maps/Lvl_PointeSombre"
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def log(msg):
    unreal.log_warning("[DCSOMBREART] " + msg)


def _package(obj):
    outer = obj.get_outermost() if obj else None
    return outer.get_name() if outer else ""


def _world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


class ArtLevel:
    def __init__(self, cell):
        self.cell = cell
        self.name = f"Lvl_PointeSombre_Art_{cell}"
        self.path = f"/Game/Maps/{self.name}"
        self.island = island_mod.Island.load()
        self.count = 0
        if not unreal.EditorAssetLibrary.does_asset_exist(MAP):
            raise RuntimeError(f"{MAP} does not exist. Run build_pointe_sombre first.")
        levels.load_level(MAP)
        self.level, self.newly_linked = self._link()
        doomed = [a for a in actors.get_all_level_actors() if _package(a.get_outer()) == self.path
                  and not isinstance(a, unreal.WorldSettings)
                  and not (isinstance(a, unreal.Brush) and not isinstance(a, unreal.Volume))]
        actors.destroy_actors(doomed)
        unreal.SystemLibrary.collect_garbage()
        self._focus(self.path, (self.name,))
        log(f"{self.name}: cleared {len(doomed)} actors{' (linked now)' if self.newly_linked else ''}")

    def _linked(self):
        return [s for s in unreal.EditorLevelUtils.get_levels(_world()) if _package(s) == self.path]

    def _link(self):
        linked = self._linked()
        if len(linked) > 1:
            raise RuntimeError(f"{self.path} is linked {len(linked)} times")
        if linked:
            return linked[0], False
        if unreal.EditorAssetLibrary.does_asset_exist(self.path):
            streaming = unreal.EditorLevelUtils.add_level_to_world(_world(), self.path, unreal.LevelStreamingAlwaysLoaded)
        else:
            streaming = unreal.EditorLevelUtils.create_new_streaming_level(unreal.LevelStreamingAlwaysLoaded,
                                                                           self.path, False)
        if not streaming:
            raise RuntimeError(f"Could not link {self.path}")
        linked = self._linked()
        if len(linked) != 1:
            raise RuntimeError(f"{self.path} did not load after linking")
        return linked[0], True

    def _focus(self, package, names):
        for name in names:
            levels.set_current_level_by_name(name)
            if _package(levels.get_current_level()) == package:
                return
        raise RuntimeError(f"Could not make {package} the current level")

    def ground_z(self, x_cm, y_cm):
        return self.island.height(x_cm / 100.0, y_cm / 100.0) * 100.0

    def prop(self, label, mesh_path, where, yaw=0.0, height_cm=None, fit_cm=None, sink_cm=0.0, material=None,
             tilt=(0.0, 0.0)):
        """A NoCollision static mesh. where: (x, y) cm stands it on the terrain, or (x, y, z) cm puts its base at z.
        height_cm or fit_cm scales it uniformly (to that height, or that longest side); sink_cm lowers it."""
        mesh = unreal.load_asset(mesh_path)
        if not mesh:
            raise RuntimeError(f"Missing {mesh_path}")
        b = mesh.get_bounding_box()
        size = b.max - b.min
        scale = 1.0
        if height_cm:
            scale = height_cm / max(size.z, 1.0)
        elif fit_cm:
            scale = fit_cm / max(size.x, size.y, size.z, 1.0)
        x, y = where[0], where[1]
        base = where[2] if len(where) > 2 else self.ground_z(x, y)
        z = base - b.min.z * scale - sink_cm
        actor = actors.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(x, y, z),
                                              unreal.Rotator(pitch=tilt[0], yaw=yaw, roll=tilt[1]))
        actor.set_actor_label(label)
        actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
        comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        comp.set_static_mesh(mesh)
        comp.set_collision_profile_name("NoCollision")
        comp.set_editor_property("can_ever_affect_navigation", False)
        if material:
            for slot in range(comp.get_num_materials()):
                comp.set_material(slot, material)
        actor.set_folder_path(f"Art/{self.cell}")
        actor.set_editor_property("tags", [unreal.Name("Art"), unreal.Name(f"Art:{self.cell}")])
        self.count += 1
        return actor

    def save(self):
        stray = [a.get_actor_label() for a in actors.get_all_level_actors()
                 if _package(a.get_outer()) == MAP and f"Art:{self.cell}" in [str(t) for t in a.get_editor_property("tags")]]
        if stray:
            raise RuntimeError(f"art landed in the persistent level: {stray[:5]}")
        self._focus(MAP, (MAP.rsplit("/", 1)[-1], "PersistentLevel"))
        if not unreal.EditorLoadingAndSavingUtils.save_packages([self.level.get_outermost()], False):
            raise RuntimeError(f"Could not save {self.path}")
        if self.newly_linked:
            if not levels.save_current_level():
                raise RuntimeError(f"Could not save {MAP}")
            log(f"saved {MAP} with {self.name} linked")
        log(f"saved {self.path}: {self.count} props")
