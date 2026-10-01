"""Convex solids and their Wavefront OBJ text, for the kit's authored modules. Pure Python (no engine).

A module is a list of parts; a part is a convex solid (a box, or a prism from a convex outline) whose faces each carry
a material slot name. write_obj() turns the parts into an OBJ whose import gives Unreal the module in its own local
frame (cm), with flat face normals and one material slot per slot name.

The axis and winding convention is the one VS-04 found for Pointe Sombre's terrain (Design/technical_architecture.md,
"Terrain"): the importer reads OBJ as Z-up and only mirrors Y, so a point is written (X, -Y, Z), and a face whose
vertices run so that the plain cross product points outward is written in reverse order.
"""


def _sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def _cross(a, b):
    return (a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0])


def _dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def _normalize(v):
    length = (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) ** 0.5 or 1.0
    return (v[0] / length, v[1] / length, v[2] / length)


class Solid:
    """A convex solid: faces as (vertex list, slot). Face order is fixed outward on construction."""

    def __init__(self, faces):
        points = [p for face, _slot in faces for p in face]
        center = tuple(sum(p[i] for p in points) / len(points) for i in range(3))
        self.faces = []
        for face, slot in faces:
            normal = _cross(_sub(face[1], face[0]), _sub(face[2], face[0]))
            face_center = tuple(sum(p[i] for p in face) / len(face) for i in range(3))
            if _dot(normal, _sub(face_center, center)) < 0.0:
                face = list(reversed(face))
            self.faces.append((list(face), slot))


def box(x0, x1, y0, y1, z0, z1, slot, top=None, bottom=None):
    """An axis-aligned box. top/bottom name a different slot for the +Z / -Z face (a roof's underside)."""
    c = [(x, y, z) for z in (z0, z1) for y in (y0, y1) for x in (x0, x1)]
    # c index: x + 2*y + 4*z
    faces = [
        ([c[0], c[1], c[3], c[2]], bottom or slot),   # -Z
        ([c[4], c[5], c[7], c[6]], top or slot),      # +Z
        ([c[0], c[1], c[5], c[4]], slot),             # -Y
        ([c[2], c[3], c[7], c[6]], slot),             # +Y
        ([c[0], c[2], c[6], c[4]], slot),             # -X
        ([c[1], c[3], c[7], c[5]], slot),             # +X
    ]
    return Solid(faces)


def prism_xz(outline, y0, y1, slot):
    """A prism: a convex outline in the XZ plane [(x, z), ...] extruded from y0 to y1."""
    front = [(x, y0, z) for x, z in outline]
    back = [(x, y1, z) for x, z in outline]
    faces = [(front, slot), (back, slot)]
    n = len(outline)
    for i in range(n):
        j = (i + 1) % n
        faces.append(([front[i], front[j], back[j], back[i]], slot))
    return Solid(faces)


def slots_of(parts):
    seen = []
    for part in parts:
        for _face, slot in part.faces:
            if slot not in seen:
                seen.append(slot)
    return seen


def write_obj(parts, header):
    """OBJ text for the parts. Deterministic: the same parts always give the same text."""
    verts, normals, faces_by_slot = [], [], {}
    for part in parts:
        for face, slot in part.faces:
            normal = _normalize(_cross(_sub(face[1], face[0]), _sub(face[2], face[0])))
            normals.append(normal)
            n_index = len(normals)
            indices = []
            for p in face:
                verts.append(p)
                indices.append(len(verts))
            faces_by_slot.setdefault(slot, []).append((indices, n_index))
    lines = [f"# {header}"]
    for x, y, z in verts:
        lines.append(f"v {x:.3f} {-y:.3f} {z:.3f}")
    for x, y, z in verts:
        # Planar UVs in metres; the kit's surfaces are triplanar and do not read them.
        lines.append(f"vt {(x + y) / 100.0:.4f} {z / 100.0:.4f}")
    for nx, ny, nz in normals:
        lines.append(f"vn {nx:.5f} {-ny:.5f} {nz:.5f}")
    for slot in sorted(faces_by_slot):
        lines.append(f"g {slot}")
        lines.append(f"usemtl {slot}")
        for indices, n in faces_by_slot[slot]:
            # Outward-ordered in Unreal axes; reversed for the importer's mirror (see the module docstring).
            for k in range(1, len(indices) - 1):
                a, b, c = indices[0], indices[k], indices[k + 1]
                lines.append(f"f {a}/{a}/{n} {c}/{c}/{n} {b}/{b}/{n}")
    return "\n".join(lines) + "\n"
