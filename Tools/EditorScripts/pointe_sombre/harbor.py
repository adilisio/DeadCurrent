"""The harbor (sombre.harbor): the quay, and the people the island meets there. Spec: Design/POIs/sombre.harbor.md.

Owner: Claude. VS-10 builds B1 with the crossing; VS-15 adds Hale and his cutter; VS-18 and VS-19 add the loft and
night placements below. Ids are the ledger's (Design/POIs/sombre_ids.md §6, §7, §9).

  Mara (sombre.mara, sombre_mara): ONE actor for the whole slice; every placement is a state of one presence rule
  here (ledger §7). Default: at the Ida's rail on the crossing (Anchor_Mara_Rail). Then, first match wins:
    - porch: once the tower has been found (LocationDiscovered sombre.light, the fact that moves the quest from
      arrived to lamp_room; presence follows world facts, not quest stages), on the store porch (Anchor_Mara_Porch)
    - quay:  after the strike (sombre.reef_struck), beside the arrival, facing the player as the fade lifts
    (VS-18 adds the loft seat, VS-19 the night quay, both before these.)
  Varga (sombre.varga, sombre_varga): on the Ida's deck at her berth, on the side facing the quay. Hidden before the
  strike (she is at the wheel). Her first conversation starts both quests.

The quay's discovery volume (sombre.harbor) is still the greybox's: its id and bounds are final, so the harbor keeps it
until the dressing pass replaces the quay. Called by build_pointe_sombre.py as build(tk). Owns only Cell:harbor actors.
"""
import math

CELL = "harbor"
REEF_STRUCK = "sombre.reef_struck"

# Mara: the accepted Phase 5 placeholder (the one-piece Quinn mannequin in her two matte colours, build_boathouse.py).
MARA_PAINTS = ("MI_DC_MaraBody", "MI_DC_MaraTrim")
# Where Mara waits on the quay, from the arrival point (cm, in its frame: forward, right). Ahead and to the left, so
# she is in view as the fade lifts without standing on the trails up the island.
MARA_QUAY_OFFSET = (320.0, -210.0)
# Varga stands on the Ida's deck this far from the berth's centreline, toward the quay (the Ida's beam is 720 cm).
VARGA_FROM_CENTRE = 260.0
VARGA_ALONG = 300.0              # and this far toward the bow from midships
IDA_HEADING_AT_SEA = 25.0        # crossing.HEADING: the Ida's heading on the crossing, before the berth's turn
VARGA_JACKET = (1.1, 1.3, 2.4)   # a faded navy oilskin
VARGA_JEANS = (1.2, 1.1, 1.0)


def _yaw_to(a, b):
    return math.degrees(math.atan2(b[1] - a[1], b[0] - a[0]))


def build(tk):
    quay = tk.anchor("Anchor_CrossingExit_Quay")
    q = quay.get_actor_location()
    qyaw = quay.get_actor_rotation().yaw
    f = (math.cos(math.radians(qyaw)), math.sin(math.radians(qyaw)))
    r = (-f[1], f[0])

    # --- Mara: one actor, placed by presence.
    rail = tk.anchor("Anchor_Mara_Rail")
    mara = tk.npc("Mara", CELL, rail.get_actor_location(), rail.get_actor_rotation().yaw, "sombre.mara", "Mara",
                  "DA_Dialogue_SombreMara", body="quinn", paints=[tk.surface(n) for n in MARA_PAINTS])
    mx = q.x + f[0] * MARA_QUAY_OFFSET[0] + r[0] * MARA_QUAY_OFFSET[1]
    my = q.y + f[1] * MARA_QUAY_OFFSET[0] + r[1] * MARA_QUAY_OFFSET[1]
    on_quay = (mx, my, tk.ground_z(mx, my) + tk.NPC_HALF_HEIGHT)
    porch = tk.anchor("Anchor_Mara_Porch")
    tk.presence("Mara", CELL, [mara], [
        tk.state("porch", [tk.flag(REEF_STRUCK), tk.cond("LOCATION_DISCOVERED", id="sombre.light")],
                 place=tk.placement(porch.get_actor_location(), porch.get_actor_rotation())),
        tk.state("quay", [tk.flag(REEF_STRUCK)],
                 place=tk.placement(on_quay, (0.0, _yaw_to(on_quay, (q.x, q.y)), 0.0))),
    ])

    # --- Varga: on the moored Ida, on the side toward the quay. The hull's heading at the berth is the crossing's
    # heading plus the berth anchor's turn (ledger §9); the side is whichever beam faces the quay arrival.
    berth = tk.anchor("Anchor_IdaBerth")
    b = berth.get_actor_location()
    h = math.radians(IDA_HEADING_AT_SEA + berth.get_actor_rotation().yaw)
    ahead, beam = (math.cos(h), math.sin(h)), (-math.sin(h), math.cos(h))
    side = 1.0 if (q.x - b.x) * beam[0] + (q.y - b.y) * beam[1] >= 0.0 else -1.0
    vx = b.x + beam[0] * side * VARGA_FROM_CENTRE + ahead[0] * VARGA_ALONG
    vy = b.y + beam[1] * side * VARGA_FROM_CENTRE + ahead[1] * VARGA_ALONG
    varga = tk.npc("Varga", CELL, (vx, vy, b.z + tk.NPC_HALF_HEIGHT), math.degrees(math.atan2(beam[1] * side, beam[0] * side)),
                   "sombre.varga", "Captain Varga", "DA_Dialogue_SombreVarga",
                   jacket=tk.costume("MI_DC_Sombre_VargaJacket", "Jacket", VARGA_JACKET),
                   jeans=tk.costume("MI_DC_Sombre_VargaJeans", "Jeans", VARGA_JEANS))
    tk.presence("Varga", CELL, [varga], [tk.state("at_the_wheel", [tk.flag(REEF_STRUCK, negate=True)], present=False)])
    tk.log(f"harbor: Mara at the rail, quay {tuple(round(v) for v in on_quay)}; Varga on the Ida at {round(vx)}, {round(vy)}")
