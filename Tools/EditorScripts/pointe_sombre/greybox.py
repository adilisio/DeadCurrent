"""The exterior greybox (VS-08): Pointe Sombre's shape before anything is dressed, for Checkpoint A.

Integrator-owned. Called by build_pointe_sombre.py as build(tk), after the core's anchors. Data:
Tools/PointeSombre/greybox.json (via layout.py). Every actor is tagged Cell:greybox and Greybox (the perf toggle hides
them all at once), plus Greybox:<cell> for the cell that will replace it.

It places:
  - landmarks: the tower (base room with a door, shaft, gallery, lamp room), the west-headland false-light post and its
    hide, the Ashland Grey's stern on its reef, the Authority mast, the vault's lower door in the tower rock
  - the settlement's and the harbor's kit shells (accepted VS-05 kit) on pilings over the real ground, never flattened
  - occluding rock outcrops (placeholder occlusion: the tower slides out of view and is found again)
  - stub interiors in their slots (the vault, the net loft) and open stub portals to them and up the tower, so every
    cell can be walked into at Checkpoint A. Real portals, with their conditions, are the cells' (VS-12, VS-13, VS-14,
    VS-18); each cell's builder removes this script's stub for that cell.
  - the six location volumes (their ids are final; each cell takes its volume over when it is built)
  - the Ida's berth: a hidden fender filling the gap between the quay wall and the moored hull
Nothing here is narrative content: no inspectables, no people, no flags.
"""
import math
import os
import sys

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
KIT = os.path.normpath(os.path.join(HERE, "..", "kit"))
for p in (HERE, KIT):
    if p not in sys.path:
        sys.path.insert(0, p)

import compose  # noqa: E402
import layout as layout_mod  # noqa: E402

CELL = "greybox"
ROCK_MESH = "/Game/Scene_Junkyard/Assets/MS/3D/For_Sca_Rock_Set_01/SM_For_Sca_Rock_Set_01_C"
STUB = " (greybox)"


def build(tk):
    lay = layout_mod.Layout.load(tk.island)
    g = _Greybox(tk, lay)
    g.tower()
    g.post()
    g.grey()
    g.mast()
    g.lower_door()
    g.structures()
    g.rocks()
    g.berth()
    g.interiors()
    g.portals()
    g.location_volumes()
    tk.log(f"greybox: {g.count} actors")


class _Greybox:
    def __init__(self, tk, lay):
        self.tk = tk
        self.lay = lay
        self.count = 0
        self.white = tk.flat("MI_DC_Sombre_TowerWhite", (0.50, 0.50, 0.47, 1.0), roughness=0.8)
        self.glass = tk.flat("MI_DC_Sombre_LampGlass", (0.03, 0.035, 0.04, 1.0), roughness=0.15)
        self.steel = tk.surface("MI_DC_Steel")
        self.wreck = tk.tinted_surface("MI_DC_Sombre_GreyHull", "MI_DC_RustPaint", (0.38, 0.36, 0.35))
        self.concrete = tk.surface("MI_DC_Concrete")
        self.timber = tk.surface("MI_DC_TimberDark")
        self.rock = tk.surface("MI_DC_LandRock")
        cyl = unreal.load_asset(tk.CYLINDER)
        b = cyl.get_bounding_box()
        self.cyl_mesh, self.cyl_size, self.cyl_center = cyl, b.max - b.min, (b.max + b.min) * 0.5

    def own(self, actor, cell_for, exclude_biome=False):
        """Tag a greybox actor. exclude_biome: its bounds keep the shoreline recipe clear (the tool's BiomeExclude)."""
        self.count += 1
        tags = ("Greybox", f"Greybox:{cell_for}") + (("BiomeExclude",) if exclude_biome else ())
        return self.tk.own(actor, CELL, *tags)

    def box(self, label, cell_for, center, size, rot=(0.0, 0.0, 0.0), material=None, hidden=False, collision=True):
        return self.own(self.tk.box(label, CELL, center, size, rot=rot, material=material, hidden=hidden,
                                    collision=collision), cell_for)

    def cylinder(self, label, cell_for, base_center, radius, height, material, collision=True):
        """A cylinder standing on base_center (cm), radius and height in cm."""
        s = unreal.Vector(2.0 * radius / self.cyl_size.x, 2.0 * radius / self.cyl_size.y, height / self.cyl_size.z)
        offset = unreal.Vector(self.cyl_center.x * s.x, self.cyl_center.y * s.y, self.cyl_center.z * s.z)
        center = unreal.Vector(base_center[0], base_center[1], base_center[2] + height / 2.0)
        actor = self.tk.actors.spawn_actor_from_class(unreal.StaticMeshActor, center - offset, unreal.Rotator(0, 0, 0))
        actor.set_actor_scale3d(s)
        actor.set_actor_label(label)
        comp = actor.get_component_by_class(unreal.StaticMeshComponent)
        comp.set_static_mesh(self.cyl_mesh)
        for slot in range(comp.get_num_materials()):
            comp.set_material(slot, material)
        if not collision:
            comp.set_collision_profile_name("NoCollision")
        return self.own(actor, cell_for)

    # --- Landmarks

    def tower(self):
        t = self.lay.data["tower"]
        cx, cy = t["center"][0] * 100.0, t["center"][1] * 100.0
        pad = t["pad_z"] * 100.0
        r = t["radius"] * 100.0
        room = t["base_room_height"] * 100.0
        top = pad + t["shaft_top"] * 100.0
        n = int(t["segments"])
        half_door = math.degrees((t["door_width"] * 100.0 / 2.0) / r)
        seg_len = 2.0 * math.pi * r / n + 30.0
        for k in range(n):
            ang = 360.0 * (k + 0.5) / n
            diff = abs((ang - t["door_yaw"] + 180.0) % 360.0 - 180.0)
            if diff < half_door + 360.0 / n / 2.0 - 1.0:
                continue   # the door
            a = math.radians(ang)
            self.box(f"Tower_BaseWall_{k:02d}", "lighthouse", (cx + r * math.cos(a), cy + r * math.sin(a), pad + room / 2.0 - 30.0),
                     (40.0, seg_len, room + 60.0), rot=(0.0, ang, 0.0), material=self.white)
        # Lintel over the door, the base room's ceiling, the shaft, the gallery, the lamp room, the cap.
        a = math.radians(t["door_yaw"])
        self.box("Tower_DoorLintel", "lighthouse", (cx + r * math.cos(a), cy + r * math.sin(a), pad + room - 40.0),
                 (40.0, t["door_width"] * 100.0 + 40.0, 80.0), rot=(0.0, t["door_yaw"], 0.0), material=self.white)
        self.box("Tower_RoomCeiling", "lighthouse", (cx, cy, pad + room + 10.0), (2.0 * r, 2.0 * r, 20.0),
                 material=self.tk.flat("MI_DC_Sombre_Ceiling", (0.16, 0.16, 0.15, 1.0)))
        self.own(self.cylinder("Tower_Shaft", "lighthouse", (cx, cy, pad + room), r, top - pad - room, self.white),
                 "lighthouse", exclude_biome=True)
        self.cylinder("Tower_Gallery", "lighthouse", (cx, cy, top - 30.0), t["gallery_radius"] * 100.0, 30.0, self.steel)
        self.cylinder("Tower_LampRoom", "lighthouse", (cx, cy, top), t["lamp_radius"] * 100.0, t["lamp_height"] * 100.0,
                      self.glass)
        self.cylinder("Tower_Cap", "lighthouse", (cx, cy, top + t["lamp_height"] * 100.0), t["lamp_radius"] * 100.0 + 30.0,
                      50.0, self.steel)
        self.cylinder("Tower_Vent", "lighthouse", (cx, cy, top + t["lamp_height"] * 100.0 + 50.0), 60.0, 120.0, self.steel)
        # The gallery's railing: hidden blockers in a ring, and a thin visible rail.
        gr = t["gallery_radius"] * 100.0 - 15.0
        for k in range(24):
            ang = 360.0 * (k + 0.5) / 24
            a = math.radians(ang)
            pos = (cx + gr * math.cos(a), cy + gr * math.sin(a))
            length = 2.0 * math.pi * gr / 24 + 10.0
            block = self.box(f"Tower_RailBlock_{k:02d}", "lighthouse", (pos[0], pos[1], top + 100.0), (20.0, length, 200.0),
                             rot=(0.0, ang, 0.0), hidden=True)
            # Stops the player, not sight lines: a hidden blocker must not hide the lamp room from a landmark trace.
            block.get_component_by_class(unreal.StaticMeshComponent).set_collision_profile_name("InvisibleWall")
            self.box(f"Tower_Rail_{k:02d}", "lighthouse", (pos[0], pos[1], top + 105.0), (6.0, length, 6.0),
                     rot=(0.0, ang, 0.0), material=self.steel, collision=False)

    def post(self):
        p = self.lay.data["landmarks"]["false_light_post"]
        x, y = p["center"]
        z = self.lay.ground(x, y) * 100.0
        h = p["post_height"] * 100.0
        self.own(self.box("FalseLight_Post", "headland", (x * 100.0, y * 100.0, z + h / 2.0), (25.0, 25.0, h),
                          material=self.timber), "headland", exclude_biome=True)
        self.box("FalseLight_Arm", "headland", (x * 100.0, y * 100.0 - 40.0, z + h - 20.0), (12.0, 90.0, 12.0),
                 material=self.timber)
        self.box("FalseLight_Lantern", "headland", (x * 100.0, y * 100.0 - 80.0, z + h - 70.0), (30.0, 30.0, 45.0),
                 material=self.tk.surface("MI_DC_GlowLantern"), collision=False)
        # The hide: a ring of low stone walls round the post, open to the landward side.
        rr = p["hide_radius"] * 100.0
        hh = p["hide_height"] * 100.0
        for k in range(12):
            ang = 360.0 * (k + 0.5) / 12
            if abs((ang - p["hide_gap_yaw"] + 180.0) % 360.0 - 180.0) < 32.0:
                continue
            a = math.radians(ang)
            px, py = x * 100.0 + rr * math.cos(a), y * 100.0 + rr * math.sin(a)
            gz = self.lay.ground(px / 100.0, py / 100.0) * 100.0
            self.box(f"FalseLight_Hide_{k:02d}", "headland", (px, py, gz + hh / 2.0 - 20.0), (70.0, 2.0 * math.pi * rr / 12 + 20.0, hh + 40.0),
                     rot=(0.0, ang, 0.0), material=self.rock)

    def grey(self):
        g = self.lay.data["landmarks"]["ashland_grey"]
        x, y = g["center"][0] * 100.0, g["center"][1] * 100.0
        L, B, H = g["length"] * 100.0, g["beam"] * 100.0, g["height"] * 100.0
        deck = g["deck_z"] * 100.0
        yaw = g["yaw"]
        fx, fy = math.cos(math.radians(yaw)), math.sin(math.radians(yaw))
        rx, ry = -fy, fx

        def at(f, r, z):
            return (x + fx * f + rx * r, y + fy * f + ry * r, z)

        rot = (g["pitch"], yaw, g["roll"])
        # The broken stern: a hull box jammed on the reef, tilted, its deck walkable by hidden collision.
        self.own(self.box("Grey_Hull", "headland", at(0.0, 0.0, deck - H / 2.0 + 60.0), (L, B, H), rot=rot,
                          material=self.wreck, collision=False), "headland", exclude_biome=True)
        self.box("Grey_DeckCollision", "headland", at(0.0, 0.0, deck - 10.0), (L - 60.0, B - 60.0, 20.0), rot=(0.0, yaw, 0.0),
                 hidden=True)
        self.box("Grey_HullCollision", "headland", at(0.0, 0.0, deck - 220.0), (L - 60.0, B - 60.0, 420.0), rot=(0.0, yaw, 0.0),
                 hidden=True)
        self.box("Grey_Wheelhouse", "headland", at(-L * 0.25, 0.0, deck + 160.0), (380.0, 520.0, 320.0), rot=(0.0, yaw, 0.0),
                 material=self.steel)
        self.box("Grey_Winch", "headland", at(L * 0.2, -B * 0.2, deck + 60.0), (120.0, 160.0, 120.0), rot=(0.0, yaw, 0.0),
                 material=self.tk.surface("MI_DC_RustPaint"))
        self.box("Grey_Break", "headland", at(L / 2.0 - 40.0, 0.0, deck - H / 2.0 + 150.0), (80.0, B + 40.0, H - 100.0),
                 rot=rot, material=self.tk.surface("MI_DC_WreckRust"), collision=False)
        # A ramp from the reef up onto the stern deck (where the hull broke).
        rfx, rfy = g["ramp_from"]
        sx, sy = rfx * 100.0, rfy * 100.0
        ground = self.lay.ground(rfx, rfy) * 100.0
        ex, ey, _ = at(L / 2.0 - 120.0, 0.0, deck)
        run = math.hypot(ex - sx, ey - sy)
        pitch = math.degrees(math.atan2(deck - ground, run))
        self.box("Grey_Ramp", "headland", ((sx + ex) / 2.0, (sy + ey) / 2.0, (ground + deck) / 2.0 - 10.0),
                 (run + 60.0, 240.0, 20.0), rot=(pitch, math.degrees(math.atan2(ey - sy, ex - sx)), 0.0),
                 material=self.tk.surface("MI_DC_WreckRust"))

    def mast(self):
        m = self.lay.data["landmarks"]["mast"]
        x, y = m["center"]
        z = self.lay.ground(x, y) * 100.0
        h = m["height"] * 100.0
        self.cylinder("Mast_Pole", "cable_hut", (x * 100.0, y * 100.0, z), 20.0, h, self.steel)
        for k, frac in enumerate((0.55, 0.8, 0.97)):
            self.box(f"Mast_Arm_{k}", "cable_hut", (x * 100.0, y * 100.0, z + h * frac), (12.0, 240.0 - 60.0 * k, 12.0),
                     material=self.steel, collision=False)
        self.own(self.box("Mast_Footing", "cable_hut", (x * 100.0, y * 100.0, z + 20.0), (140.0, 140.0, 60.0),
                          material=self.concrete), "cable_hut", exclude_biome=True)

    def lower_door(self):
        d = self.lay.data["landmarks"]["lower_door"]
        x, y = d["center"]
        z = self.lay.ground(x, y) * 100.0
        yaw = d["yaw"]
        self.own(self.box("LowerDoor_Frame", "lighthouse", (x * 100.0, y * 100.0, z + 150.0), (120.0, 260.0, 340.0),
                          rot=(0.0, yaw, 0.0), material=self.concrete), "lighthouse", exclude_biome=True)

    # --- The settlement's and harbor's kit shells

    def structures(self):
        kit = compose.load_kit()
        for s in self.lay.data["structures"]:
            spec = self.lay.kit_spec(s["id"])
            x, y, z, yaw, relief = self.lay.structure_origin(s["id"])
            spec["base"] = float(spec.get("base", 0.0)) + relief * 100.0
            actor = compose.place(spec, kit, (x, y, z), yaw, folder=f"Cells/{CELL}")
            self.own(actor, s["cell"], exclude_biome=True)
            tag = f"Structure:{spec['id']}"
            for other in self.tk.actors.get_all_level_actors():
                if other is not actor and tag in [str(t) for t in other.get_editor_property("tags")]:
                    self.own(other, s["cell"])

    def rocks(self):
        mesh = unreal.load_asset(ROCK_MESH)
        b = mesh.get_bounding_box()
        size = b.max - b.min
        for r in self.lay.data["rocks"]:
            x, y = r["center"]
            d, path = self.tk.island.path_distance(x, y)
            clear = max(r["size"][0], r["size"][1]) / 2.0 + 3.0
            if d < clear:
                raise RuntimeError(f"rock {r['id']} is {d:.1f} m from the {path.id} trail (needs {clear:.1f} m): move it")
            sx, sy, sz = (v * 100.0 for v in r["size"])
            z = self.lay.ground(x, y) * 100.0
            scale = unreal.Vector(sx / size.x, sy / size.y, sz / size.z)
            actor = self.tk.actors.spawn_actor_from_class(
                unreal.StaticMeshActor, unreal.Vector(x * 100.0, y * 100.0, z - b.min.z * scale.z - 60.0),
                unreal.Rotator(pitch=0.0, yaw=r["yaw"], roll=0.0))
            actor.set_actor_scale3d(scale)
            actor.set_actor_label(f"Rock_{r['id']}")
            comp = actor.get_component_by_class(unreal.StaticMeshComponent)
            comp.set_static_mesh(mesh)
            comp.set_material(0, self.rock)
            comp.set_collision_profile_name("NoCollision")
            self.own(actor, "harbor", exclude_biome=True)
            # Tier B rock: it blocks, by a hidden box inside its shape.
            self.box(f"Rock_{r['id']}_Block", "harbor", (x * 100.0, y * 100.0, z + sz * 0.35), (sx * 0.75, sy * 0.75, sz * 0.7),
                     rot=(0.0, r["yaw"], 0.0), hidden=True)

    def berth(self):
        f = self.lay.data["landmarks"]["berth_fender"]
        self.own(self.box("Berth_Fender", "harbor", ((f["x0"] + f["x1"]) * 50.0, (f["y0"] + f["y1"]) * 50.0, (f["top_z"] + f["bottom_z"]) * 50.0),
                 ((f["x1"] - f["x0"]) * 100.0, (f["y1"] - f["y0"]) * 100.0, (f["top_z"] - f["bottom_z"]) * 100.0),
                 material=self.timber), "harbor", exclude_biome=True)

    # --- Interiors (stubs in their slots)

    def interiors(self):
        tk = self.tk
        o = tk.interior_origin("vault")
        sx, sy, sh = (v * 100.0 for v in self.lay.data["interiors"]["vault"]["size"])
        ox, oy, oz = o.x, o.y, o.z
        parts = [
            self.box("Vault_Floor", "vault", (ox, oy, oz - 10.0), (sx, sy, 20.0), material=self.concrete),
            self.box("Vault_Ceiling", "vault", (ox, oy, oz + sh + 10.0), (sx, sy, 20.0),
                     material=tk.flat("MI_DC_Sombre_Ceiling", (0.16, 0.16, 0.15, 1.0))),
            self.box("Vault_WallN", "vault", (ox + sx / 2.0 + 10.0, oy, oz + sh / 2.0), (20.0, sy, sh), material=self.concrete),
            self.box("Vault_WallS", "vault", (ox - sx / 2.0 - 10.0, oy, oz + sh / 2.0), (20.0, sy, sh), material=self.concrete),
            self.box("Vault_WallE", "vault", (ox, oy + sy / 2.0 + 10.0, oz + sh / 2.0), (sx, 20.0, sh), material=self.concrete),
            self.box("Vault_WallW", "vault", (ox, oy - sy / 2.0 - 10.0, oz + sh / 2.0), (sx, 20.0, sh), material=self.concrete),
            self.own(tk.point_light("Vault_Light", CELL, (ox, oy, oz + sh - 60.0), (190, 205, 225), 500.0, 2400.0), "vault"),
        ]
        for p in parts:
            tk.make_interior(p)
        self.own(tk.post_process_box("Vault_Grade", CELL, (ox, oy, oz + sh / 2.0), (sx / 2.0 + 60.0, sy / 2.0 + 60.0, sh),
                                     10.0, _room_grade(-0.8)), "vault")

        o = tk.interior_origin("net_loft")
        kit = compose.load_kit()
        spec = compose.load_structure("net_loft_shell")
        w, d = float(spec["footprint"][0]) * 100.0, float(spec["footprint"][1]) * 100.0
        loft = compose.place(spec, kit, (o.x - w / 2.0, o.y - d / 2.0, o.z), 0.0, folder=f"Cells/{CELL}")
        self.own(loft, "net_loft")
        tk.make_interior(loft)
        for other in tk.actors.get_all_level_actors():
            if f"Structure:{spec['id']}" in [str(t) for t in other.get_editor_property("tags")] and other is not loft:
                tk.make_interior(self.own(other, "net_loft"))
        tk.make_interior(self.box("Loft_Ceiling", "net_loft", (o.x, o.y, o.z + 520.0), (w + 200.0, d + 200.0, 20.0),
                                  material=tk.flat("MI_DC_Sombre_Ceiling", (0.16, 0.16, 0.15, 1.0))))
        tk.make_interior(self.own(tk.point_light("Loft_Light", CELL, (o.x, o.y, o.z + 330.0), (255, 214, 160), 380.0, 1600.0),
                                  "net_loft"))
        self.own(tk.post_process_box("Loft_Grade", CELL, (o.x, o.y, o.z + 260.0), (w / 2.0 + 150.0, d / 2.0 + 150.0, 300.0),
                                     10.0, _room_grade(-0.6)), "net_loft")

    # --- Stub portals (open; the cells replace them with the real ones)

    def portals(self):
        tk = self.tk
        t = self.lay.data["tower"]
        cx, cy = t["center"][0] * 100.0, t["center"][1] * 100.0
        pad = t["pad_z"] * 100.0
        top = pad + t["shaft_top"] * 100.0

        def stub(label, cell_for, center, size, name, verb, anchor, yaw=0.0, interior=False):
            actor = tk.portal(f"Greybox_{label}", CELL, center, size, name + STUB, [tk.portal_variant("open", verb)],
                              tk.anchor(anchor), yaw=yaw)
            self.own(actor, cell_for)
            if interior:
                tk.make_interior(actor)
            return actor

        a = math.radians(45.0)
        stub("TowerStair_Up", "lighthouse", (cx + 230.0 * math.cos(a), cy + 230.0 * math.sin(a), pad + 105.0), (60.0, 100.0, 210.0),
             "Tower stair", "Climb the stair", "Anchor_TowerStair_Lamp", yaw=45.0)
        a = math.radians(t["door_yaw"])
        lr = t["lamp_radius"] * 100.0 + 10.0
        stub("TowerStair_Down", "lighthouse", (cx + lr * math.cos(a), cy + lr * math.sin(a), top + 105.0), (20.0, 100.0, 210.0),
             "Tower stair", "Go down", "Anchor_TowerStair_Base", yaw=t["door_yaw"])
        stub("VaultHatch_Out", "lighthouse", (cx - 40.0, cy + 40.0, pad + 4.0), (110.0, 110.0, 8.0),
             "Hatch", "Go down", "Anchor_VaultHatch_Bottom")
        d = self.lay.data["landmarks"]["lower_door"]
        dx, dy = d["center"]
        dz = self.lay.ground(dx, dy) * 100.0
        ya = math.radians(d["yaw"])
        stub("VaultLower_Out", "lighthouse", (dx * 100.0 - 62.0 * math.cos(ya), dy * 100.0 - 62.0 * math.sin(ya), dz + 105.0),
             (8.0, 120.0, 210.0), "Iron door", "Go in", "Anchor_VaultLower_In", yaw=d["yaw"])
        hut = tk.anchor("Anchor_VaultConduit_Out").get_actor_location()
        stub("VaultConduit_Out", "cable_hut", (hut.x + 100.0, hut.y + 100.0, hut.z - 60.0), (90.0, 90.0, 60.0),
             "Conduit", "Crawl in", "Anchor_VaultConduit_In", yaw=45.0)
        o = tk.interior_origin("vault")
        stub("VaultHatch_In", "vault", (o.x - 700.0, o.y - 450.0, o.z + 105.0), (100.0, 8.0, 210.0), "Ladder", "Go up",
             "Anchor_VaultHatch_Top", interior=True)
        stub("VaultLower_In", "vault", (o.x + 700.0, o.y - 590.0, o.z + 105.0), (100.0, 8.0, 210.0), "Iron door", "Go out",
             "Anchor_VaultLower_Out", interior=True)
        stub("VaultConduit_In", "vault", (o.x, o.y + 590.0, o.z + 60.0), (100.0, 8.0, 120.0), "Conduit", "Crawl out",
             "Anchor_VaultConduit_Out", interior=True)
        foot = tk.anchor("Anchor_LoftStair_Store").get_actor_location()
        sx, sy, _z, syaw, _r = self.lay.structure_origin("store")
        fx, fy = layout_mod._rot(-0.7, -0.2, syaw)
        stub("LoftStair_Up", "settlement", (sx + fx * 100.0, sy + fy * 100.0, foot.z + 5.0), (100.0, 40.0, 210.0),
             "Loft stair", "Go up to the loft", "Anchor_LoftStair_Loft", yaw=syaw)
        lo = tk.interior_origin("net_loft")
        stub("LoftStair_Down", "net_loft", (lo.x - 240.0, lo.y - 400.0 + 30.0, lo.z + 105.0), (100.0, 8.0, 210.0),
             "Loft stair", "Go down", "Anchor_LoftStair_Store", interior=True)

    # --- Location volumes (final ids; each cell takes its volume over)

    def location_volumes(self):
        tk = self.tk
        t = self.lay.data["tower"]
        lv = [
            ("harbor", "sombre.harbor", "Pointe Sombre Harbor", (-7800.0, 1000.0, 400.0), (2200.0, 2600.0, 600.0)),
            ("lighthouse", "sombre.light", "Pointe Sombre Light",
             (t["center"][0] * 100.0, t["center"][1] * 100.0, t["pad_z"] * 100.0 + 1100.0), (1300.0, 1300.0, 1400.0)),
            ("headland", "sombre.headland", "The West Head", (-1800.0, -21400.0, 1100.0), (1100.0, 1100.0, 700.0)),
            ("cable_hut", "sombre.cable_hut", "The Cable Hut", (8300.0, 9600.0, 500.0), (1100.0, 1100.0, 600.0)),
            ("headland", "sombre.ashland_grey", "The Ashland Grey", (-11900.0, -22900.0, 400.0), (1000.0, 900.0, 700.0)),
        ]
        o = tk.interior_origin("vault")
        sx, sy, sh = (v * 100.0 for v in self.lay.data["interiors"]["vault"]["size"])
        lv.append(("vault", "sombre.vault", "The Vault", (o.x, o.y, o.z + sh / 2.0), (sx / 2.0, sy / 2.0, sh / 2.0)))
        for cell_for, lid, name, center, extent in lv:
            self.own(tk.location_volume(f"Location_{lid.split('.')[1]}", CELL, lid, name, center, extent), cell_for)


def _room_grade(bias):
    def apply(settings):
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", bias)
        settings.set_editor_property("override_auto_exposure_min_brightness", True)
        settings.set_editor_property("auto_exposure_min_brightness", 0.3)
        settings.set_editor_property("override_auto_exposure_max_brightness", True)
        settings.set_editor_property("auto_exposure_max_brightness", 0.3)
    return apply
