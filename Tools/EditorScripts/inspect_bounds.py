"""Read-only: can the player reach the edge of the world? Grid-samples Lvl_Boathouse for collidable static
surfaces, flood-fills the walkable area from the player start, and reports any reachable cell that borders void.
Log prefix [DCBOUNDS]."""
import unreal

MAP_PATH = "/Game/Maps/Lvl_Boathouse"
X0, X1, Y0, Y1, STEP = -3800, 4200, -4600, 2400, 40


def log(msg):
    unreal.log_warning("[DCBOUNDS] " + msg)


levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
levels.load_level(MAP_PATH)

boxes = []
for actor in actors.get_all_level_actors():
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    if not comp or not comp.get_editor_property("static_mesh"):
        continue
    if comp.get_collision_enabled() == unreal.CollisionEnabled.NO_COLLISION:
        continue
    origin, extent = actor.get_actor_bounds(True)
    boxes.append((origin.x - extent.x, origin.x + extent.x, origin.y - extent.y, origin.y + extent.y,
                  origin.z - extent.z, origin.z + extent.z))
log(f"{len(boxes)} colliding static meshes")

nx = (X1 - X0) // STEP + 1
ny = (Y1 - Y0) // STEP + 1
WALK, VOID, WALL = 0, 1, 2
grid = [[VOID] * nx for _ in range(ny)]
for j in range(ny):
    y = Y0 + j * STEP
    for i in range(nx):
        x = X0 + i * STEP
        floor = None
        wall = False
        for (ax0, ax1, ay0, ay1, zb, zt) in boxes:
            if ax0 <= x <= ax1 and ay0 <= y <= ay1:
                if zt >= 100 and zb <= 100:
                    wall = True
                elif zt <= 60 and (floor is None or zt > floor):
                    floor = zt
        grid[j][i] = WALL if wall else (WALK if floor is not None else VOID)

start = ((220 - X0) // STEP, (0 - Y0) // STEP)
seen = {start}
stack = [start]
leaks = []
while stack:
    i, j = stack.pop()
    for di, dj in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        a, b = i + di, j + dj
        if not (0 <= a < nx and 0 <= b < ny):
            leaks.append((X0 + i * STEP, Y0 + j * STEP))
            continue
        if (a, b) in seen or grid[b][a] == WALL:
            continue
        if grid[b][a] == VOID:
            leaks.append((X0 + i * STEP, Y0 + j * STEP))
            continue
        seen.add((a, b))
        stack.append((a, b))
log(f"reachable cells: {len(seen)}; cells bordering void: {len(leaks)}")
for x, y in leaks[:40]:
    log(f"  leak near x={x} y={y}")
xs = [x for x, _ in leaks]
ys = [y for _, y in leaks]
if leaks:
    log(f"leak extent x {min(xs)}..{max(xs)}  y {min(ys)}..{max(ys)}")
log("done")
