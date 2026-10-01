"""The Great Lakes Working Settlement Kit's authored modules: geometry as code. Pure Python (no engine).

Every module is authored once here and imported once as /Game/World/Kits/GreatLakesSettlement/Meshes/SM_Kit_<Id>.
A module's local frame (cm): X runs along the piece, Y is its thickness or depth (a wall's +Y is the inside),
Z is up, and the origin is the outer, left, bottom corner. Scalable modules are unit pieces stretched by compose()
along the axes listed in SCALABLE; the kit's surfaces are triplanar in world space, so stretching never stretches a
texture. Fixed modules (walls, openings) are never scaled: that is what makes them countable vocabulary.

Material slots (the names compose() assigns skins to):
  skin       the main cladding (siding, boards, corrugated)
  trim       corner boards, frames, posts, beams, rails
  door       a closed door leaf (the slice builds no home interiors)
  pane       window glass
  roof       a roof's top and edges
  underside  any face that looks down (the triplanar surfaces light a downward face wrongly; VS-04)
  deck       floors, porches, stairs
  masonry    a chimney stack
"""
import hashlib

from geometry import box, prism_xz

STOREY = 280.0   # one storey's wall height
T = 15.0         # wall thickness
ROOF_T = 12.0    # roof panel thickness
DECK_T = 20.0    # deck slab thickness
STEP_RISE, STEP_RUN, STEPS = 20.0, 30.0, 14   # a storey of stair: 14 steps of 20 cm over 4.2 m


def _wall(length):
    return [box(0.0, length, 0.0, T, 0.0, STOREY, "skin")]


def _frame(x0, x1, z0, z1):
    """A trim frame around an opening, standing 3 cm proud of both faces."""
    w = 6.0
    return [
        box(x0 - w, x0, -3.0, T + 3.0, z0 - (w if z0 > 0 else 0.0), z1 + w, "trim"),
        box(x1, x1 + w, -3.0, T + 3.0, z0 - (w if z0 > 0 else 0.0), z1 + w, "trim"),
        box(x0, x1, -3.0, T + 3.0, z1, z1 + w, "trim"),
    ] + ([box(x0, x1, -3.0, T + 3.0, z0 - w, z0, "trim")] if z0 > 0 else [])


def _wall_door():
    x0, x1, top = 50.0, 150.0, 210.0
    return [
        box(0.0, x0, 0.0, T, 0.0, STOREY, "skin"),
        box(x1, 200.0, 0.0, T, 0.0, STOREY, "skin"),
        box(x0, x1, 0.0, T, top, STOREY, "skin"),
        box(x0, x1, 5.0, 10.0, 0.0, top, "door"),
    ] + _frame(x0, x1, 0.0, top)


def _wall_window():
    x0, x1, sill, head = 55.0, 145.0, 100.0, 190.0
    return [
        box(0.0, x0, 0.0, T, 0.0, STOREY, "skin"),
        box(x1, 200.0, 0.0, T, 0.0, STOREY, "skin"),
        box(x0, x1, 0.0, T, 0.0, sill, "skin"),
        box(x0, x1, 0.0, T, head, STOREY, "skin"),
        box(x0, x1, 6.0, 9.0, sill, head, "pane"),
    ] + _frame(x0, x1, sill, head)


def _stair():
    # Solid steps ascending along +Y, one storey high: walkable, with simple collision from the steps themselves.
    run = STEP_RUN * STEPS
    return [box(0.0, 100.0, i * STEP_RUN, run, 0.0, (i + 1) * STEP_RISE, "deck") for i in range(STEPS)]


def _railing():
    parts = [box(x - 3.0, x + 3.0, -3.0, 3.0, 0.0, 100.0, "trim") for x in (3.0, 100.0, 197.0)]
    return parts + [box(0.0, 200.0, -4.0, 4.0, 95.0, 105.0, "trim")]


# id -> (description, parts factory)
MODULES = {
    "Wall_100": ("Solid wall, 1 m", lambda: _wall(100.0)),
    "Wall_200": ("Solid wall, 2 m", lambda: _wall(200.0)),
    "Wall_400": ("Solid wall, 4 m", lambda: _wall(400.0)),
    "Wall_200_Door": ("Wall, 2 m, with a closed door (1.0 x 2.1 m) and frame", _wall_door),
    "Wall_200_Window": ("Wall, 2 m, with a window (0.9 x 0.9 m at 1.0 m) and frame", _wall_window),
    "Gable_100": ("Gable end: unit isosceles triangle (span 1 m, rise 1 m), stretched to span and rise",
                  lambda: [prism_xz([(0.0, 0.0), (100.0, 0.0), (50.0, 100.0)], 0.0, T, "skin")]),
    "Rake_100": ("Lean-to end: unit right triangle (run 1 m, rise 1 m at the origin), stretched",
                 lambda: [prism_xz([(0.0, 0.0), (100.0, 0.0), (0.0, 100.0)], 0.0, T, "skin")]),
    "Roof_100": ("Roof panel, 1 x 1 m, 12 cm, underside on its own slot; stretched",
                 lambda: [box(0.0, 100.0, 0.0, 100.0, 0.0, ROOF_T, "roof", bottom="underside")]),
    "Ridge_100": ("Ridge cap, 1 m; stretched along its length",
                  lambda: [box(0.0, 100.0, -14.0, 14.0, 0.0, 10.0, "trim", bottom="underside")]),
    "Deck_100": ("Deck or floor slab, 1 x 1 m, 20 cm, top at Z 0; stretched",
                 lambda: [box(0.0, 100.0, 0.0, 100.0, -DECK_T, 0.0, "deck", bottom="underside")]),
    "Post_100": ("Post or piling, 16 cm square, 1 m; stretched in height",
                 lambda: [box(-8.0, 8.0, -8.0, 8.0, 0.0, 100.0, "trim")]),
    "Beam_100": ("Beam or fascia, 16 cm square, 1 m along X, top at Z 0; stretched",
                 lambda: [box(0.0, 100.0, -8.0, 8.0, -16.0, 0.0, "trim", bottom="underside")]),
    "Corner_280": ("Corner board, 20 cm square, one storey; stretched in height",
                   lambda: [box(-10.0, 10.0, -10.0, 10.0, 0.0, STOREY, "trim")]),
    "Stair_280": ("Outside stair, 1 m wide, one storey (14 steps over 4.2 m), ascending +Y", _stair),
    "Railing_200": ("Railing, 2 m: three posts and a top rail", _railing),
    "Chimney_200": ("Chimney stack, 60 cm square, 2 m", lambda: [box(0.0, 60.0, 0.0, 60.0, 0.0, 200.0, "masonry")]),
}

# Axes compose() may stretch, per module. Everything else is placed at scale 1.
SCALABLE = {
    "Gable_100": "xz", "Rake_100": "xz", "Roof_100": "xy", "Ridge_100": "x", "Deck_100": "xy",
    "Post_100": "z", "Beam_100": "x", "Corner_280": "z",
}

# Collision note per module for the catalog: what the player touches.
COLLISION = {
    "Deck_100": "walkable", "Stair_280": "walkable", "Roof_100": "blocks", "Post_100": "blocks",
    "Railing_200": "blocks", "Chimney_200": "blocks",
}


def parts(module_id):
    return MODULES[module_id][1]()


def module_hash(module_id):
    """Changes when the module's geometry changes (the import skips an unchanged module)."""
    from geometry import write_obj
    return hashlib.sha1(write_obj(parts(module_id), module_id).encode()).hexdigest()[:16]
