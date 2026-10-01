"""kit.compose(): a building from data. The Great Lakes Working Settlement Kit's composition API.

    import compose
    kit = compose.load_kit()                                   # Tools/Kits/great_lakes_settlement/kit.json
    spec = compose.load_structure("store")                     # Tools/Kits/great_lakes_settlement/structures/store.json
    placements = compose.plan(spec, kit)                       # pure Python: what goes where (no engine)
    actor = compose.place(spec, kit, location=(x, y, z), yaw=0.0)   # in the editor: one actor for the structure

plan() is deterministic: the same spec gives the same placements, in the same order, on every machine. It has no
randomness at all; variety comes from the data (footprint, storeys, skins, openings, roof, porch, stair, attachments).

place() spawns ONE actor per structure (label and tag "Structure:<id>") holding one InstancedStaticMeshComponent per
(piece, skin set), each tagged "Kit:<PieceId>". Instancing keeps a building to a few dozen draw calls instead of one
per wall panel. Library attachments (barrels, crates, a lantern) are separate StaticMeshActors attached to it, tagged
"Kit:<PieceId>" and "Structure:<id>" (their pack materials are not flagged for instancing). report_kit_usage.py counts
instances and attachment actors.

The structure's local frame (cm): origin at the front-left corner at ground level, X along the front, Y toward the back,
Z up; the front faces -Y. place() turns it by yaw about the origin and moves it to location.

The composition schema is documented in Design/Kits/great_lakes_settlement_kit.md.
"""
import json
import math
import os

import modules

HERE = os.path.dirname(os.path.abspath(__file__))
KIT_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "Kits", "great_lakes_settlement"))
STOREY = modules.STOREY
T = modules.T
GABLE_PITCH = 35.0
LEAN_PITCH = 16.0
PORCH_LOW = 230.0     # a porch roof's eave above the porch floor


def load_kit(path=None):
    with open(path or os.path.join(KIT_DIR, "kit.json"), encoding="utf-8") as handle:
        return json.load(handle)


def load_structure(name):
    with open(os.path.join(KIT_DIR, "structures", name + ".json"), encoding="utf-8") as handle:
        return json.load(handle)


def structure_names():
    folder = os.path.join(KIT_DIR, "structures")
    return sorted(f[:-5] for f in os.listdir(folder) if f.endswith(".json"))


class Placement:
    """One piece: id, local location (cm), rotation (pitch, yaw, roll), scale, and slot -> skin id."""

    def __init__(self, piece, loc, rot=(0.0, 0.0, 0.0), scale=(1.0, 1.0, 1.0), skins=None):
        self.piece, self.loc, self.rot, self.scale, self.skins = piece, tuple(loc), tuple(rot), tuple(scale), dict(skins or {})

    def key(self):
        return (self.piece, tuple(sorted(self.skins.items())))

    def as_tuple(self):
        r = lambda v: tuple(round(x, 3) for x in v)
        return (self.piece, r(self.loc), r(self.rot), r(self.scale), tuple(sorted(self.skins.items())))


def _sides(w, d):
    """name -> (start (x, y) in cm, direction (dx, dy), length in m, yaw), for a w x d m footprint: the wall runs along
    +X of its local frame and its inside is local +Y."""
    W, D = w * 100.0, d * 100.0
    return {
        "front": ((0.0, 0.0), (1.0, 0.0), w, 0.0),
        "right": ((W, 0.0), (0.0, 1.0), d, 90.0),
        "back": ((W, D), (-1.0, 0.0), w, 180.0),
        "left": ((0.0, D), (0.0, -1.0), d, 270.0),
    }


def _fill(length_m, taken):
    """Greedy 4/2/1 m solid panels over [0, length) minus the 2 m opening slots in taken (start metres)."""
    out, x = [], 0.0
    blocked = sorted(taken)
    while x < length_m - 1e-6:
        if blocked and abs(blocked[0] - x) < 1e-6:
            x += 2.0
            blocked.pop(0)
            continue
        limit = blocked[0] if blocked else length_m
        room = limit - x
        size = 4.0 if room >= 4.0 - 1e-6 else 2.0 if room >= 2.0 - 1e-6 else 1.0
        if room < 1.0 - 1e-6:
            raise ValueError(f"a {room:.2f} m gap is smaller than the 1 m panel")
        out.append((x, size))
        x += size
    return out


def plan(spec, kit):
    """The placements for one structure spec, in a fixed order."""
    sid = spec["id"]
    w, d = float(spec["footprint"][0]), float(spec["footprint"][1])
    W, D = w * 100.0, d * 100.0
    storeys = int(spec.get("storeys", 1))
    base = float(spec.get("base", 0.0))
    skins = dict(kit["default_skins"])
    skins.update(spec.get("skins", {}))
    side_skin = spec.get("side_skins", {})
    open_sides = set(spec.get("open_sides", []))
    out = []

    def wall_skins(side):
        return {"skin": side_skin.get(side, skins["walls"]), "trim": skins["trim"], "door": skins["door"],
                "pane": skins["pane"]}

    sides = _sides(w, d)
    openings = spec.get("openings", {})
    for name, (start, direction, length, yaw) in sides.items():
        for storey in range(storeys):
            z = base + storey * STOREY
            if name in open_sides and storey == 0:
                continue
            here = [o for o in openings.get(name, []) if int(o.get("storey", 1)) == storey + 1]
            taken = []
            for o in here:
                at = float(o["at"])
                if at < 0 or at + 2.0 > length + 1e-6:
                    raise ValueError(f"{sid}: {name} opening at {at} m does not fit a {length} m side")
                if any(abs(at - t) < 2.0 - 1e-6 for t in taken):
                    raise ValueError(f"{sid}: {name} openings overlap at {at} m")
                taken.append(at)
                piece = {"door": "Wall_200_Door", "window": "Wall_200_Window"}[o["type"]]
                loc = (start[0] + direction[0] * at * 100.0, start[1] + direction[1] * at * 100.0, z)
                out.append(Placement(piece, loc, (0.0, yaw, 0.0), skins=wall_skins(name)))
            for at, size in _fill(length, taken):
                loc = (start[0] + direction[0] * at * 100.0, start[1] + direction[1] * at * 100.0, z)
                out.append(Placement(f"Wall_{int(size * 100)}", loc, (0.0, yaw, 0.0), skins={"skin": side_skin.get(name, skins["walls"])}))

    top = base + storeys * STOREY
    # Corner boards hide the panel joints; on an open side they are the posts.
    for x, y in ((0.0, 0.0), (W, 0.0), (W, D), (0.0, D)):
        out.append(Placement("Corner_280", (x, y, base), scale=(1.0, 1.0, (top - base) / STOREY), skins={"trim": skins["trim"]}))
    if "front" in open_sides:
        for x in [m * 100.0 for m in range(2, int(w), 2)]:
            out.append(Placement("Post_100", (x, 0.0, base), scale=(1.0, 1.0, STOREY / 100.0), skins={"trim": skins["trim"]}))
        out.append(Placement("Beam_100", (0.0, 0.0, base + STOREY), scale=(W / 100.0, 1.0, 1.0), skins={"trim": skins["trim"]}))

    # Foundation: a floor deck at the base, on pilings when raised.
    if base > 0.0 or spec.get("floor", True):
        # On the ground, the floor stands 2 cm proud so its top never shares a plane with the ground.
        out.append(Placement("Deck_100", (0.0, 0.0, base if base > 0.0 else 2.0), scale=(W / 100.0, D / 100.0, 1.0),
                             skins={"deck": skins["deck"]}))
    if base > 0.0:
        for x in [i * 200.0 for i in range(int(w // 2) + 1)] + ([W] if w % 2 else []):
            for y in (0.0, D):
                out.append(Placement("Post_100", (x, y, 0.0), scale=(1.0, 1.0, base / 100.0), skins={"trim": skins["piling"]}))

    out += _roof(spec, skins, W, D, top)
    out += _porch(spec, skins, W, base)
    out += _stair(spec, skins, W, D, base, openings)
    if "chimney" in spec:
        cx, cy = spec["chimney"]
        out.append(Placement("Chimney_200", (cx * 100.0, cy * 100.0, top), skins={"masonry": skins["masonry"]}))
    for a in spec.get("attachments", []):
        if a["piece"] not in kit["attachments"]:
            raise ValueError(f"{sid}: unknown attachment {a['piece']}")
        x, y, z = a["at"]
        out.append(Placement(a["piece"], (x * 100.0, y * 100.0, z * 100.0), (0.0, float(a.get("yaw", 0.0)), 0.0)))
    return out


def _roof(spec, skins, W, D, top):
    roof = spec.get("roof", {"type": "gable", "ridge": "x"})
    kind = roof["type"]
    o = float(roof.get("overhang", 30.0))
    rs = {"roof": skins["roof"], "underside": skins["underside"]}
    out = []
    if kind == "gable":
        pitch = math.radians(float(roof.get("pitch", GABLE_PITCH)))
        along_x = roof.get("ridge", "x") == "x"
        span = D if along_x else W
        rise = span / 2.0 * math.tan(pitch)
        slope = (span / 2.0 + o) / math.cos(pitch)
        length = (W if along_x else D) + 2.0 * o
        gable = {"skin": spec.get("gable_skin", skins["walls"])}
        deg = math.degrees(pitch)
        if along_x:
            out.append(Placement("Gable_100", (W, 0.0, top), (0.0, 90.0, 0.0), (D / 100.0, 1.0, rise / 100.0), gable))
            out.append(Placement("Gable_100", (0.0, D, top), (0.0, 270.0, 0.0), (D / 100.0, 1.0, rise / 100.0), gable))
            out.append(Placement("Roof_100", (W + o, D / 2.0, top + rise), (0.0, 180.0, deg), (length / 100.0, slope / 100.0, 1.0), rs))
            out.append(Placement("Roof_100", (-o, D / 2.0, top + rise), (0.0, 0.0, deg), (length / 100.0, slope / 100.0, 1.0), rs))
            out.append(Placement("Ridge_100", (-o, D / 2.0, top + rise + 8.0), scale=(length / 100.0, 1.0, 1.0), skins={"trim": skins["trim"], "underside": skins["underside"]}))
        else:
            out.append(Placement("Gable_100", (0.0, 0.0, top), (0.0, 0.0, 0.0), (W / 100.0, 1.0, rise / 100.0), gable))
            out.append(Placement("Gable_100", (W, D, top), (0.0, 180.0, 0.0), (W / 100.0, 1.0, rise / 100.0), gable))
            out.append(Placement("Roof_100", (W / 2.0, -o, top + rise), (0.0, 90.0, deg), (length / 100.0, slope / 100.0, 1.0), rs))
            out.append(Placement("Roof_100", (W / 2.0, D + o, top + rise), (0.0, 270.0, deg), (length / 100.0, slope / 100.0, 1.0), rs))
            out.append(Placement("Ridge_100", (W / 2.0, -o, top + rise + 8.0), (0.0, 90.0, 0.0), (length / 100.0, 1.0, 1.0), {"trim": skins["trim"], "underside": skins["underside"]}))
    elif kind == "lean_to":
        # High along the back, falling to the front.
        pitch = math.radians(float(roof.get("pitch", LEAN_PITCH)))
        rise = D * math.tan(pitch)
        slope = (D + 2.0 * o) / math.cos(pitch)
        rake = {"skin": spec.get("gable_skin", skins["walls"])}
        out.append(Placement("Rake_100", (0.0, D, top), (0.0, 270.0, 0.0), (D / 100.0, 1.0, rise / 100.0), rake))
        out.append(Placement("Rake_100", (W - T, D, top), (0.0, 270.0, 0.0), (D / 100.0, 1.0, rise / 100.0), rake))
        out.append(Placement("Roof_100", (W + o, D + o, top + rise + o * math.tan(pitch)), (0.0, 180.0, math.degrees(pitch)),
                             ((W + 2.0 * o) / 100.0, slope / 100.0, 1.0), rs))
    elif kind == "flat":
        out.append(Placement("Roof_100", (-o, -o, top), scale=((W + 2.0 * o) / 100.0, (D + 2.0 * o) / 100.0, 1.0), skins=rs))
    else:
        raise ValueError(f"{spec['id']}: unknown roof type {kind}")
    return out


def _porch(spec, skins, W, base):
    porch = spec.get("porch")
    if not porch:
        return []
    if porch.get("side", "front") != "front":
        raise ValueError(f"{spec['id']}: porches are front-only in this kit version")
    depth = float(porch["depth"]) * 100.0
    x0 = float(porch.get("from", 0.0)) * 100.0
    x1 = float(porch.get("to", W / 100.0)) * 100.0
    high = base + STOREY - 10.0
    low = base + PORCH_LOW
    pitch = math.degrees(math.atan2(high - low, depth))
    slope = math.hypot(depth + 20.0, high - low)
    trim = {"trim": skins["trim"]}
    out = [Placement("Deck_100", (x0, -depth, base), scale=((x1 - x0) / 100.0, depth / 100.0, 1.0), skins={"deck": skins["deck"]})]
    for x in (x0 + 8.0, x1 - 8.0):
        out.append(Placement("Post_100", (x, -depth + 8.0, base), scale=(1.0, 1.0, (low - base) / 100.0), skins=trim))
    out.append(Placement("Beam_100", (x0, -depth + 8.0, low), scale=((x1 - x0) / 100.0, 1.0, 1.0), skins=trim))
    out.append(Placement("Roof_100", (x1 + 20.0, 0.0, high), (0.0, 180.0, pitch), ((x1 - x0 + 40.0) / 100.0, slope / 100.0, 1.0),
                         {"roof": skins["porch_roof"], "underside": skins["underside"]}))
    if base > 0.0:
        out.append(Placement("Deck_100", (x0 + (x1 - x0) / 2.0 - 60.0, -depth - 60.0, base / 2.0), scale=(1.2, 0.6, 1.0),
                             skins={"deck": skins["deck"]}))
    return out


def _stair(spec, skins, W, D, base, openings):
    stair = spec.get("stair")
    if not stair:
        return []
    side = stair["side"]
    door = [o for o in openings.get(side, []) if o["type"] == "door" and int(o.get("storey", 1)) == int(stair.get("to_storey", 2))]
    if not door or side not in ("left", "right"):
        raise ValueError(f"{spec['id']}: an outside stair needs a left or right side with an upper-storey door")
    at = float(door[0]["at"]) * 100.0
    # left side runs from the back toward the front; right side from the front toward the back.
    y0, y1 = (D - at - 200.0, D - at) if side == "left" else (at, at + 200.0)
    x = -120.0 if side == "left" else W + 20.0
    rise_z = base + (int(stair.get("to_storey", 2)) - 1) * STOREY
    run = modules.STEP_RUN * modules.STEPS
    deck = {"deck": skins["deck"]}
    return [
        Placement("Deck_100", (x, y0, rise_z), scale=(1.0, (y1 - y0) / 100.0, 1.0), skins=deck),
        Placement("Stair_280", (x, y0 - run, rise_z - STOREY), skins=deck),
        Placement("Railing_200", (x if side == "left" else x + 100.0, y0, rise_z), (0.0, 90.0, 0.0), skins={"trim": skins["trim"]}),
        Placement("Post_100", (x + 50.0, y0 + 100.0, base), scale=(1.0, 1.0, (rise_z - base - modules.DECK_T) / 100.0), skins={"trim": skins["piling"]}),
    ]


def footprint_cm(spec):
    """The structure's local bounding rectangle including porch and stair, for overlap checks: (x0, y0, x1, y1)."""
    w, d = float(spec["footprint"][0]) * 100.0, float(spec["footprint"][1]) * 100.0
    x0, y0, x1, y1 = -40.0, -40.0, w + 40.0, d + 40.0
    if spec.get("porch"):
        y0 = min(y0, -float(spec["porch"]["depth"]) * 100.0 - 120.0)
    if spec.get("stair"):
        if spec["stair"]["side"] == "left":
            x0 = -160.0
        else:
            x1 = w + 160.0
        y0 = min(y0, -modules.STEP_RUN * modules.STEPS)
    return x0, y0, x1, y1


# --- In the editor -------------------------------------------------------------------------------------------------

def _rotate_yaw(x, y, yaw):
    r = math.radians(yaw)
    return x * math.cos(r) - y * math.sin(r), x * math.sin(r) + y * math.cos(r)


def place(spec, kit, location, yaw=0.0, folder=None, extra_tags=()):
    """Spawn the structure in the current editor level. Returns the actor."""
    import unreal
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    sds = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    lib = unreal.SubobjectDataBlueprintFunctionLibrary
    sid = spec["id"]
    actor = actors.spawn_actor_from_class(unreal.Actor, unreal.Vector(*location), unreal.Rotator(pitch=0.0, yaw=yaw, roll=0.0))
    actor.set_actor_label(f"Structure_{sid}")
    if folder:
        actor.set_folder_path(folder)
    actor.set_editor_property("tags", [unreal.Name(f"Structure:{sid}")] + [unreal.Name(t) for t in extra_tags])
    root_handle = sds.k2_gather_subobject_data_for_instance(actor)[0]   # the actor itself: components attach to its root

    groups = {}
    for p in plan(spec, kit):
        if p.piece in kit["attachments"]:
            _place_attachment(actors, actor, p, kit, sid, folder)
            continue
        groups.setdefault(p.key(), []).append(p)
    for (piece, skin_items), placements in groups.items():
        mesh_path = kit["attachments"][piece]["mesh"] if piece in kit["attachments"] else f"{kit['module_folder']}/SM_Kit_{piece}"
        mesh = unreal.load_asset(mesh_path)
        if not mesh:
            raise RuntimeError(f"{sid}: missing kit piece {piece} ({mesh_path})")
        handle, fail = sds.add_new_subobject(unreal.AddNewSubobjectParams(
            parent_handle=root_handle, new_class=unreal.InstancedStaticMeshComponent, blueprint_context=None))
        if fail and str(fail):
            raise RuntimeError(f"{sid}: could not add a component for {piece}: {fail}")
        comp = lib.get_object(lib.get_data(handle))
        comp.set_static_mesh(mesh)
        comp.set_editor_property("component_tags", [unreal.Name(f"Kit:{piece}"), unreal.Name(f"Structure:{sid}")])
        for slot, skin in dict(skin_items).items():
            comp.set_material_by_name(unreal.Name(slot), unreal.load_asset(kit["skins"][skin]))
        for p in placements:
            comp.add_instance(unreal.Transform(location=unreal.Vector(*p.loc),
                                               rotation=unreal.Rotator(pitch=p.rot[0], yaw=p.rot[1], roll=p.rot[2]),
                                               scale=unreal.Vector(*p.scale)), False)
    return actor


def _place_attachment(actors, structure, p, kit, sid, folder):
    """A library prop as its own actor, attached to the structure: pack materials are not flagged for instancing,
    and a building has only a handful of props. Its bounds' bottom is seated on the given height (a pack's pivot is
    wherever the pack put it)."""
    import unreal
    spec = kit["attachments"][p.piece]
    mesh = unreal.load_asset(spec["mesh"])
    if not mesh:
        raise RuntimeError(f"{sid}: missing attachment {p.piece} ({spec['mesh']})")
    scale = float(spec.get("scale", 1.0))
    yaw = structure.get_actor_rotation().yaw
    x, y = _rotate_yaw(p.loc[0], p.loc[1], yaw)
    base = structure.get_actor_location()
    lift = -mesh.get_bounding_box().min.z * scale
    prop = actors.spawn_actor_from_class(unreal.StaticMeshActor,
                                         unreal.Vector(base.x + x, base.y + y, base.z + p.loc[2] + lift),
                                         unreal.Rotator(pitch=0.0, yaw=yaw + p.rot[1], roll=0.0))
    prop.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    prop.get_component_by_class(unreal.StaticMeshComponent).set_static_mesh(mesh)
    prop.set_actor_label(f"Structure_{sid}_{p.piece}")
    if folder:
        prop.set_folder_path(folder)
    prop.set_editor_property("tags", [unreal.Name(f"Kit:{p.piece}"), unreal.Name(f"Structure:{sid}")])
    prop.attach_to_actor(structure, unreal.Name(), unreal.AttachmentRule.KEEP_WORLD, unreal.AttachmentRule.KEEP_WORLD,
                         unreal.AttachmentRule.KEEP_WORLD, False)
    return prop
