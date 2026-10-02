"""The Wrong Characteristic (sombre.characteristic): why the Pointe Sombre light failed, and what the player makes of it.

Stages and transitions are the beat script's §6 (`Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md`), with the ids the
ledger fixes (`Design/POIs/sombre_ids.md` §3, §5). Quest and stage ids are stored in saves: never rename one.

  arrived --(sombre.light discovered)--> lamp_room --(cable_cut_found)--> vault --(vault_opened)--> vault_inside
          --(panel_read AND liv_note_found)--> knows --(meeting_done + light_line)--> done_line
                                                     --(meeting_done + light_hand)--> done_hand
                                                     --(meeting_done)-------------> done_dark

- Started only by Varga's first conversation (`sombre_varga` `first`, every choice). Flags set before then chain the
  quest forward when it starts (existing behavior).
- `sombre.storm` is gameplay state, not the atmosphere selector (ledger §3.1): the crossing's strike sets it, `knows`
  clears it (and sets `sombre.hale_arrived`, so the calm look follows by the core's shipped rule), and each `done_*`
  sets it again. Nothing here keys the atmosphere looks to `sombre.storm`.
- Power to the settlement and a destroyed node are flags, not stages: `done_dark` covers settlement power, destruction,
  and leaving it untouched. The stage records the light; the flags record the node.
- `knows`'s objective sends the player to decide, then to the net loft. Marthe's "Call the island to the loft." is what
  gathers the island (plan §8.2 #4, `sombre_marthe`).
Owner: WP-NARR (VS-09). Spec shape: see Tools/ContentSpecs/README.md.
"""


def flag(name, negate=False):
    return cond("WORLD_FLAG", id=name, negate=negate)


STORM = "sombre.storm"
MEETING_DONE = flag("sombre.meeting_done")

SOMBRE_CHARACTERISTIC = dict(
    asset="DA_Quest_SombreCharacteristic",
    quest_id="sombre.characteristic",
    name="The Wrong Characteristic",
    start="arrived",
    stages=[
        dict(
            id="arrived",
            objective="The Ida won't sail while Pointe Sombre is dark. Find out why the light failed.",
            transitions=[
                dict(next="lamp_room", conditions=[cond("LOCATION_DISCOVERED", id="sombre.light")]),
            ],
        ),
        dict(
            id="lamp_room",
            objective="Look over the lamp room.",
            transitions=[
                dict(next="vault", conditions=[flag("sombre.cable_cut_found")]),
            ],
        ),
        dict(
            id="vault",
            objective="The lamp's feed runs down into the rock under the tower. Find a way in.",
            transitions=[
                dict(next="vault_inside", conditions=[flag("sombre.vault_opened")]),
            ],
        ),
        dict(
            id="vault_inside",
            objective="Find out what the vault under the light is for.",
            transitions=[
                # Both, in either order. The panel's destroyed variant also sets panel_read (plan §8.2 #5), and the
                # note stands above the flood line (§8.2 #6), so flooding the vault never strands this stage.
                dict(next="knows", conditions=[flag("sombre.panel_read"), flag("sombre.liv_note_found")]),
            ],
        ),
        dict(
            id="knows",
            objective="Liv pulled the card that tied the light to Section 14. It is trying to come back. "
                      "Decide what happens to the light, then tell the settlement at the net loft.",
            on_enter=[
                cons("CLEAR_WORLD_FLAG", id=STORM),
                cons("SET_WORLD_FLAG", id="sombre.hale_arrived"),
            ],
            transitions=[
                # First match wins. light_line and light_hand exclude each other at the acts themselves.
                dict(next="done_line", conditions=[MEETING_DONE, flag("sombre.light_line")]),
                dict(next="done_hand", conditions=[MEETING_DONE, flag("sombre.light_hand")]),
                dict(next="done_dark", conditions=[MEETING_DONE]),
            ],
        ),
        dict(
            id="done_line",
            completes=True,
            objective="The light burns on the Line again. For three seconds it showed the pattern.",
            on_enter=[cons("SET_WORLD_FLAG", id=STORM)],
        ),
        dict(
            id="done_hand",
            completes=True,
            objective="The light burns by hand. Someone has to wind it every four hours.",
            on_enter=[cons("SET_WORLD_FLAG", id=STORM)],
        ),
        dict(
            id="done_dark",
            completes=True,
            objective="The point is dark. What powers the vault, if anything, is your doing.",
            on_enter=[cons("SET_WORLD_FLAG", id=STORM)],
        ),
    ],
)

QUESTS = [SOMBRE_CHARACTERISTIC]
