"""The architecture fixture (VS-04): a test door by the quay into an abstracted interior cell, built by the Integrator.

Not slice content. It exists so DeadCurrent.Map.Sombre.Architecture can prove the map's architecture on the real
map, independent of any content cell: walk up to a portal, use it, arrive facing the right way, a presence change
that was waiting snaps on the scene cut, save inside the cell, load back into it, a locked portal refuses with its
text, and the way back out. The flags it uses are dev.* ids, which no content reads (Design/POIs/sombre_ids.md).
When the real interior cells and their portals exist (the vault and the loft), the test can move onto them and this
fixture can go; until then it stays, labelled TEST.

Called by build_pointe_sombre.py as build(tk). Owns only actors tagged Cell:arch_test.
"""
CELL = "arch_test"
MOVED = "dev.arch_moved"          # the test crate belongs inside the cell
UNLOCKED = "dev.arch_unlocked"    # the cell's back door opens
LOCKED_TEXT = "The back door is locked. (Architecture test door: dev.arch_unlocked opens it.)"

HUT = (-7400.0, -2400.0)          # by the west end of the quay apron, clear of the arrival point
HALF = 150.0
ROOM_X, ROOM_Y, ROOM_H = 400.0, 300.0, 350.0


def build(tk):
    concrete = tk.surface("MI_DC_Concrete")
    ground = max(tk.ground_z(HUT[0] + dx, HUT[1] + dy) for dx in (-HALF, HALF) for dy in (-HALF, HALF))
    low = min(tk.ground_z(HUT[0] + dx, HUT[1] + dy) for dx in (-HALF, HALF) for dy in (-HALF, HALF))
    floor = ground + 5.0
    hx, hy = HUT

    # The exterior: a solid concrete hut on a footing, its door on the east face.
    tk.block("Test_Footing", CELL, hx - HALF - 40, hx + HALF + 40, hy - HALF - 40, hy + HALF + 40, low - 60.0, floor,
             material=concrete)
    tk.block("Test_Hut", CELL, hx - HALF, hx + HALF, hy - HALF, hy + HALF, floor, floor + 260.0, material=concrete)
    outside = tk.marker("Test_Outside", CELL, (hx, hy + HALF + 160.0, floor + 100.0), 90.0, "ArchTest:Outside")
    behind = tk.marker("Test_Behind", CELL, (hx - HALF - 160.0, hy, floor + 100.0), 180.0, "ArchTest:Behind")

    # The interior cell, in its slot far from the island.
    o = tk.interior_origin(CELL)
    ox, oy, oz = o.x, o.y, o.z
    parts = [
        tk.block("Test_Room_Floor", CELL, ox - ROOM_X, ox + ROOM_X, oy - ROOM_Y, oy + ROOM_Y, oz - 20.0, oz,
                 material=concrete),
        tk.block("Test_Room_Ceiling", CELL, ox - ROOM_X, ox + ROOM_X, oy - ROOM_Y, oy + ROOM_Y, oz + ROOM_H,
                 oz + ROOM_H + 20.0, material=tk.flat("MI_DC_Sombre_Ceiling", (0.16, 0.16, 0.15, 1.0))),
        tk.block("Test_Room_WallW", CELL, ox - ROOM_X - 20.0, ox - ROOM_X, oy - ROOM_Y, oy + ROOM_Y, oz, oz + ROOM_H,
                 material=concrete),
        tk.block("Test_Room_WallE", CELL, ox + ROOM_X, ox + ROOM_X + 20.0, oy - ROOM_Y, oy + ROOM_Y, oz, oz + ROOM_H,
                 material=concrete),
        tk.block("Test_Room_WallS", CELL, ox - ROOM_X, ox + ROOM_X, oy - ROOM_Y - 20.0, oy - ROOM_Y, oz, oz + ROOM_H,
                 material=concrete),
        tk.block("Test_Room_WallN", CELL, ox - ROOM_X, ox + ROOM_X, oy + ROOM_Y, oy + ROOM_Y + 20.0, oz, oz + ROOM_H,
                 material=concrete),
        tk.point_light("Test_Room_Light", CELL, (ox, oy, oz + ROOM_H - 60.0), (255, 226, 190), 400.0, 1400.0),
    ]
    for part in parts:
        tk.make_interior(part)
    inside = tk.marker("Test_Inside", CELL, (ox, oy - ROOM_Y + 120.0, oz + 100.0), 90.0, "ArchTest:Inside")

    def room_grade(settings):
        settings.set_editor_property("override_auto_exposure_bias", True)
        settings.set_editor_property("auto_exposure_bias", -0.6)
        settings.set_editor_property("override_auto_exposure_min_brightness", True)
        settings.set_editor_property("auto_exposure_min_brightness", 0.3)
        settings.set_editor_property("override_auto_exposure_max_brightness", True)
        settings.set_editor_property("auto_exposure_max_brightness", 0.3)
    tk.post_process_box("Test_Room_Grade", CELL, (ox, oy, oz + ROOM_H / 2), (ROOM_X + 40.0, ROOM_Y + 40.0, ROOM_H),
                        10.0, room_grade)

    # Portals: in (open), out (open), and the back door (locked unless dev.arch_unlocked).
    tk.own(tk.portal("Test_DoorIn", CELL, (hx, hy + HALF + 4.0, floor + 105.0), (100.0, 8.0, 210.0), "Test door",
                     [tk.portal_variant("open", "Go in")], inside), CELL, "ArchTest:DoorIn")
    tk.own(tk.make_interior(tk.portal("Test_DoorOut", CELL, (ox, oy - ROOM_Y + 4.0, oz + 105.0), (100.0, 8.0, 210.0),
                                      "Test door", [tk.portal_variant("open", "Go out")], outside)),
           CELL, "ArchTest:DoorOut")
    tk.own(tk.make_interior(tk.portal("Test_BackDoor", CELL, (ox, oy + ROOM_Y - 4.0, oz + 105.0), (100.0, 8.0, 210.0),
                                      "Back door", [tk.portal_variant("unlocked", "Go through", [tk.flag(UNLOCKED)])],
                                      behind, locked_text=LOCKED_TEXT)),
           CELL, "ArchTest:BackDoor")

    # A presence change that waits while the player stands beside it, and snaps on the scene cut when they go in:
    # the net loft's pattern (attendees move up while the player climbs the stair).
    crate_z = tk.ground_z(hx + 200.0, hy + HALF + 260.0)
    crate = tk.box("Test_Crate", CELL, (hx + 200.0, hy + HALF + 260.0, crate_z + 40.0), (80.0, 80.0, 80.0),
                   material=tk.interactable_material())
    tk.own(crate, CELL, "ArchTest:Crate")
    tk.presence("TestCrate", CELL, [crate], [
        tk.state("inside", [tk.flag(MOVED)], place=tk.placement((ox + 200.0, oy - 60.0, oz + 40.0))),
    ])
    tk.log(f"arch_test: hut at {HUT} floor {floor:.0f}, room at {(ox, oy, oz)}")
