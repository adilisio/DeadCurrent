# POI Specification — The Net Loft (`sombre.net_loft`)

Copied from `TEMPLATE.md`. Contract: standard, small (`VerticalSlicePhasePlan.txt` §11). Ids: `Design/POIs/sombre_ids.md` (the ledger wins). Story: beat script §5 (B8); `SLICE_DIALOGUE.md` §4 (the meeting is Marthe's). Written in VS-07 (2026-10-01).

**Frame:** an abstracted interior in the reserved slot `net_loft` at (300 m, 2500 m, 400 m) (cm (30000, 250000, 40000)). Enclosed, no view of the island, its own light and grade (the VS-04 cell placement rule).

---

## Identity

- **Stable POI id:** `sombre.net_loft` (cell id). **No location id**: the meeting is not a discovery, and the approved list has no loft id; none is minted.
- **Working name:** the net loft (over Marthe's store).
- **Region:** an interior cell; reached by the store's outside stair.
- **Biome:** `N/A — abstracted interior.`
- **Content status:** `SPEC`.
- **Lore status:** `PROVISIONAL`.
- **Owner:** Claude, VS-18. Files: `pointe_sombre/net_loft.py`, `dress_sombre_net_loft.py` → `Lvl_PointeSombre_Art_NetLoft`, `Content/World/PointeSombre/NetLoft/`, `Tools/Review/Lvl_PointeSombre/net_loft.json`, `Source/DeadCurrent/Save/DCSombreMeetingMapTest.cpp`. The up-stair actor is the settlement's; this cell owns the room, the way down, and nothing about the people except their seats' surroundings.

## Player Promise

> The whole island in one room, and what I carry up the stair decides who they turn on.

## Discovery

- **What draws attention:** Marthe: "Net loft, over this store. When there's something to decide." After the call, the island goes up there.
- **Intended approach:** the store's outside stair, "Go up to the loft" (settlement portal `LoftStair_Up` → `Anchor_LoftStair_Loft`).
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** none.

## Spatial Role

- **Approximate footprint:** the `kit_net_loft_shell` room (6 × 8 m) in the slot, the stair head at `Anchor_LoftStair_Loft`, nine places at `Anchor_LoftSeat_{Marthe, Odette, Jonas, Tem, Dell, Sigrun, Hale, Varga, Mara}` around a cleared floor of nets and floats, Marthe's chair at the head.
- **Nearby routes:** `N/A — an interior; the only ways are the stair up (settlement) and down (this cell).`
- **Important sightlines:** from the stair head, every attendee at once; Marthe faces the stair.
- **Exclusion zones:** `N/A — no recipe zone.` Hand dressing keeps clear: the stair head (2 m), each seat's standing spot (1 m), the walk line between them.
- **Navigation requirements:** player only; a flat floor; no AI walks (people stand by presence).

## Gameplay

- **Core interaction:** the meeting, Marthe's `loft` entry (available at `knows` once `sombre.meeting_called`): "Is the light burning tonight, or isn't it?"
  - "Not yet." ends it with nothing set
  - "It's done." needs `sombre.light_decided`
  - "Leave it as she left it." shows only without `light_decided`
  
  Then the evidence: each choice is shown only while the player carries the object, and giving it removes it and sets a flag. Then "That's all." → `sombre.meeting_done`. Then down the stair.
- **Possible danger:** none.
- **Combat route:** `N/A.`
- **Non-combat route:** the only route.
- **RPG / build checks:** none of its own (the evidence is the "check"; the tie line needs a tie flag and `exposed_liv`).
- **Reward:** the outcome: exposures, forgiveness, the tie named; `sombre.characteristic` completes into `done_line` / `done_hand` / `done_dark` from the light flags.
- **Exit state:** `sombre.meeting_done`. Attendees return to their places on the scene cut down the stair. The first "Go down" after the meeting lands on the night quay facing the tower and sets `sombre.night_quay_seen`; later trips land at the store.

## Environmental Story

1. **Clue:** nets hung from the beams, floats in piles, chairs pulled from the store below: the island's meeting room, not a hall.
2. **Clue:** lanterns lit (the loft's own lights); no windows onto the island (the shell's window openings are shuttered from inside).
3. **Clue:** where each person stands says whose side they start on (the seats' arrangement; VS-18 stages it).

- **What happened here (author-only):** the island decides things here, and Marthe chairs because the store is where the money is. PROVISIONAL.
- **What the player can infer:** who is afraid of whom.
- **What deliberately remains unresolved:** what the island does next, beyond the end card.
- **PROVISIONAL claims:** every reaction line; Odette's "Then she owes me a son"; Hale taking Sigrun.
- **Links to existing lore:** `Design/Narrative/QUEST_ARCS.md` §2 (factions and the island, PROVISIONAL).

## State / Persistence

- **Persistent ids:** none spawned here (the people are the settlement's and the harbor's; this cell owns the room and portals).
- **Discovered-location id:** `N/A — no loft location id.`
- **World flags:** the meeting (dialogue, `sombre_marthe`) sets `sombre.exposed_pruitts`, `exposed_odette`, `exposed_liv`, `exposed_sigrun`, `player_named_tie`, `odette_forgiven` (board **and** note given), `meeting_done`. The night portal sets `sombre.night_quay_seen`. Reads `meeting_called`, `light_decided`, the tie flags.
- **Quest stages:** `sombre.characteristic` `knows` → `done_line` (`meeting_done`, `light_line`), `done_hand` (`meeting_done`, `light_hand`), `done_dark` (`meeting_done`); each `done_*` on enter sets `sombre.storm` (ledger §3.1). `sombre.false_light` `named` → `done` on `exposed_pruitts`.
- **Conditions:** `HasItem` (`false_lantern`, `vault_access_log`, `liv_note`, `cut_cable_end`), `WorldFlag`, `QuestStage`.
- **Consequences:** `RemoveItem` (each evidence item given), `SetWorldFlag`.
- **Visible world-state variants:** attendees present (their owners' presence states, deferred, committing on the stair's scene cut) while `meeting_called` and `!meeting_done`; Sigrun absent once she is taken (settlement rule); Hale present only if `hale_arrived` (always true by `knows`). After `meeting_done` the room empties on the next cut. The way down: `LoftStair_DownNight` (night quay, sets `night_quay_seen`) while `meeting_done` and `!night_quay_seen`; `LoftStair_Down` (the store) otherwise — one presence rule in this script, exactly one present (ledger §8).
- **Save impact:** new ids only.

## Asset Plan

### Tier A — Focal

| Asset | Role | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| The attendees | the meeting | their owners' bodies (settlement, harbor) | 0 |
| Marthe's chair | the chair | library chair (`Scene_UnfinishedBuilding` / `Smugglers_cove` props) | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| `kit_net_loft_shell` | the room | yes (its windows get a shutter skin or dark backing; VS-18) |
| nets, floats, crates | dressing | library (`Smugglers_cove` crates and barrels migrated; nets as decals or cloth cards `[P]`) |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| `N/A — abstracted interior; hand-placed dressing only.` | — | NoCollision | stair head, seats, walk line |

## Audio

Room tone with rain on the roof (the night storm is coming), lanterns' faint hiss, floorboard creak. CC0. Gap: no voices.

## Existing Systems Used

Dialogue with evidence (`HasItem` conditions, `RemoveItem`), quests (outcome stages), world flags, conditional presence (attendees by their owners; the two down portals here), the cell portal and the scene cut (shipped VS-03; this is the pattern the scene cut was built for), interior cell rules (lighting channel 1, own grade).

## Missing Reusable Capability

None.

## Tests

- **Content validation:** `Content.Sombre.Dialogue`: evidence choices need the item and remove it; the tie line needs `exposed_liv` and one tie flag; "It's done." / "Leave it as she left it." are exclusive on `light_decided`.
- **Interaction:** `Map.Sombre.Meeting` (not "NetLoft"): evidence only while carried, consumed when given; each exposure; forgiveness needs the board and the note; the tie line; the completion stage matches the light; attendees present after the call on the cut, gone after the meeting. The prelude assertion is `Map.Sombre.SecondStorm`'s (it consumes `night_quay_seen`).
- **Persistence:** F5 in the loft before "That's all."; diverge; F9 back in the loft, attendees present.
- **Alternate routes:** bring nothing; bring everything; "Leave it as she left it."
- **Integration:** the night portal lands on the night quay facing the tower once; the day portal at the store.
- **Regression:** full suite; baseline 52 at VS-07.

## Review Views

`Tools/Review/Lvl_PointeSombre/net_loft.json`. No `tower_site` landmark (no windows).

| Id | Camera (x, y, z) cm, pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `loft_meeting` | from the stair head (`Anchor_LoftStair_Loft` + eye height), 0 / into the room *(planned)* | the gathering | Setup `meeting_called` (and `knows`). The island in one room; Marthe in the chair; lanterns; no island outside. |

## Human Acceptance Checklist

1. (Checkpoint D) Call the island; climb the stair. *Did everyone you expected come?*
2. Bring what you carry, or don't. *Whose side did you end up on, and did it feel like yours?* (plan §22 step 10)
3. Go down. *Did stepping out onto the night quay land at the right moment?*

## Open Creative Decisions

- **Seat arrangement.** Default: Marthe at the head facing the stair; the Pruitts together; Odette apart; Hale and Varga by the stair.
- **Windows in the shell.** Default: shuttered from inside.
- **Varga at the meeting.** Default: present (the beat script's list), standing by the stair.

## Pilot Record

- Spec committed: VS-07. First green map test: `Map.Sombre.Meeting` (VS-18). Existing classes only; new C++ 0; Meshy 0; kit `kit_net_loft_shell`; zone `N/A`; view `loft_meeting`; critic files at Checkpoint D.
