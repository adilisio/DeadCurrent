# POI Specification — The Lighthouse and the Vault (`sombre.lighthouse`, integrated)

Copied from `TEMPLATE.md`. Contract: **integrated** (`VerticalSlicePhasePlan.txt` §11, §13.3): the tower exterior, the base room, the lamp room and gallery, the vault, the node's physical acts, and the far ends of the three vault entrances. One design, because the dungeon and the decision are one system. **This spec is Integrator-owned and frozen before any build branch starts;** builders request changes, they do not edit it. Ids: `Design/POIs/sombre_ids.md` (the ledger wins). Story: beat script B3, B4, B7, §4, §6; `SLICE_DIALOGUE.md`. Written in VS-07 (2026-10-01).

**Frame (VS-04):** X north, Y east, Z up, sea level 0; metres unless marked cm. *(planned)* positions are VS-08's greybox, frozen at Checkpoint A. The vault is an abstracted cell in the reserved slot `vault` at (150 m, 2500 m, 400 m).

**Cell counting.** Plan §8.4's cell sentence lists the *Ashland Grey* and omits the net loft; §11 (followed here) has seven specs, with the vault inside this one. There is no `sombre.vault.md`.

---

## Implementation boundary (VS-07 decision)

**Decision: serial by default — lighthouse (VS-12), then cable hut (VS-13), then the vault (VS-14), one owner (Claude).** WP-VAULT (a parallel vault package for Grok) runs only if Anthony enables it (plan §13.1). **Update (2026-10-02):** Anthony asked for WP-VAULT to be prepared for Grok once the VS-09 data was real. The Integrator staged the boundary on `main` in VS-10: the `vault` hook with a placeholder `vault.py`, greybox retirement by declaration, the toolkit's damage-volume and container helpers, `art.py`, and the greybox test's ledger labels. The full handoff is `handoffs/sombre_WP-VAULT.md`. It launches when Anthony relays its prompt; until then the serial default stands.

**The boundary below is nevertheless checked against the real scripts and frozen here**, so that if Anthony enables WP-VAULT (at Checkpoint B, before VS-14), the vault can be built in parallel without a redesign. It is clean because:
- one `build(tk)` per script
- hooks run after the core's anchors
- the vault slot is reserved
- `tk.portal(..., destination)` takes an anchor
- no presence rule needs targets on both sides (every shared fact is a flag, with one rule per side)

| Piece | File / actor | Owner |
| --- | --- | --- |
| Tower exterior, base room and its real door (`ADCDoor` `sombre.tower_door`), lamp room, gallery, the stair portals, the tower's light actors, the keeper silhouette, the sea-mouth bubble, the hatch portal `VaultHatch_Out`, the lower-door portal `VaultLower_Out` | `pointe_sombre/lighthouse.py`; `dress_sombre_lighthouse.py` → `Lvl_PointeSombre_Art_Lighthouse`; `Content/World/PointeSombre/Lighthouse/**`; `Tools/Review/Lvl_PointeSombre/lighthouse.json`; `DCSombreLighthouseMapTest.cpp` | Claude (VS-12) |
| Every gameplay actor entirely inside the vault: geometry (six spaces), `VaultHatch_In`, `VaultLower_In`, `VaultConduit_In`, log, sign-in board, panel, DF loop, chart table and Liv's note, sea cock, breaker, dead eels, locker `sombre.vault_locker`, live-water volume, the `sombre.vault` location volume, the vault's raised-water presence, interior lights and ambience | `pointe_sombre/vault.py`; `dress_sombre_vault.py` → `Lvl_PointeSombre_Art_Vault`; `Content/World/PointeSombre/Vault/**`; `Tools/Review/Lvl_PointeSombre/vault.json`; `DCSombreVaultMapTest.cpp` (inside the vault only) | Claude serially (VS-14); Grok only if WP-VAULT is enabled |
| The conduit's outside end `VaultConduit_Out` | `pointe_sombre/cable_hut.py` | Claude (VS-13) |
| This spec, the ledger, every portal anchor (`Anchor_TowerStair_*`, `Anchor_VaultHatch_{Top,Bottom}`, `Anchor_VaultLower_{Out,In}`, `Anchor_VaultConduit_{Out,In}`), `build_pointe_sombre.py`, the generated map, `Map.Sombre.VaultRoutes` (`DCSombreVaultRoutesMapTest.cpp`, written at the vault merge) | core | Integrator |

**Rules that keep it clean** (any break forces the serial order; written here so a builder sees them first):
- Portals reference anchors only.
- A presence rule's targets live in one script. Same flag, two rules, no shared actor:
  - the sea-mouth bubble on `sombre.node_destroyed` is a lighthouse actor with a lighthouse rule
  - the vault's raised water is a vault actor with a vault rule
- The lighthouse test passes with no vault geometry (`Map.Sombre.Lighthouse` asserts the hatch and the lower door, and stops at the portals).
- No builder edits `build_pointe_sombre.py`, the ledger, or the other package's files.
- **Decision acts are split by room.** Splice, clockwork, and the hand light are in the lamp room (`lighthouse.py`). Seat the card, the settlement feed, and the sea cock are in the vault (`vault.py`). VS-17 (one owner, after the vault has merged) may touch both.
- Sigrun's unjam is dialogue setting `sombre.vault_opened`: neither cell's actor.
- After a WP-VAULT merge, the vault files return to Claude; VS-15..VS-19 are serial.

---

## Identity

- **Stable POI id:** cell `sombre.lighthouse`. Location ids: `sombre.light` (the tower; discovery moves the quest `arrived` → `lamp_room`) and `sombre.vault` (inside the vault).
- **Working name:** Pointe Sombre Light; the vault (the Authority's node under the tower). Display names: "Pointe Sombre Light", "The Vault" (working; Anthony may rename).
- **Region:** the Pointe, the east end of the island: the tower rock (pad `tower base`, centre (22, 160), radius 11 m, Z 26.5 m), its sea cliff to the north-east, and the vault cell.
- **Biome:** `N/A — the north waterline below the tower is the cable hut's zone (one polygon, one owner); the cliff by the lower door is an exclusion requested in that file. The vault is an interior.`
- **Content status:** `SPEC`.
- **Lore status:** `PROVISIONAL`. Claims under Environmental Story. Nothing explains the Current; "Section 14", the pattern, and the Authority's purpose stay open.
- **Owner:** see the boundary above. Tasks: VS-12 (tower), VS-14 (vault), VS-17 (decision acts and the light sequence's tower lights).

## Player Promise

> The light did not fail; someone cut it, and under the tower is a machine that wants it back — and I decide, with my own hands, what the light becomes.

## Discovery

- **What draws attention:** the dark tower is the island's primary landmark: visible from every exterior cell and the crossing, never behind the player for long (plan §5.5, Gemini: tower primary, partial views and reacquisition, not a postcard everywhere). The door at its base is open.
- **Intended approach:** quay → tower base, 35–50 s, uphill, the tower in view the whole climb. The lower door: along the shore below the tower's north-east cliff (VS-08 makes it reachable on foot from the cable hut's shore). The conduit: from the cable hut.
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** `sombre.light` volume around the base and the lamp room; `sombre.vault` volume in the vault's upper room and around each entrance's landing (one location; first entry discovers). Silent on load.

## Spatial Role

- **Approximate footprint** *(planned)*:
  - tower base on the pad (22, 160), a shell of about 6 m diameter, 20 m tall; the lamp room and gallery at its top (about Z 46.5 m); the base room inside at pad level
  - lower door at the foot of the north-east sea cliff, about (36, 194), Z 1–2 m
  - vault slot (15000, 250000, 40000) cm: upper room, stair, lower gallery with catwalk (flooded), node room, sea-cock chamber, cable gallery
- **Nearby routes:** the quay → tower leg; tower → cable hut (15–25 s, "a cut stair down the north side"); the shore under the cliff to the lower door.
- **Important sightlines:**
  - from the quay, the settlement, the headland, the cable hut, and the crossing: the tower (the capture's landmark trace on `tower_site`)
  - from the gallery: the whole island (`gallery_view`)
  - from the night quay: the tower's outcome
- **Exclusion zones:**
  - named pads this spec requires: `tower_door_approach` (3 m), `lower_door_approach` (4 m), `conduit_mouth` (the cable hut's own), the tower-path corridor (2 m wide, quay → tower), the gallery sightline (no tall dressing between the gallery and the harbor)
  - on a zoned shore these are boxes in the zone owner's file: the lower door's is requested in `zones/cable_hut.json`
  - vault interior: `N/A — no recipe`
- **Navigation requirements:**
  - the base room, lamp room, and gallery are walkable (gallery by hidden collision)
  - the tower shell is a landmark silhouette with hidden collision only where walked
  - the lower door's shore is reachable on the terrain collision (VS-08 proves it in `Map.Sombre.Greybox`)
  - **Containment:** the high ground here was above the VS-04 fence (12 m vs the 26.5 m pad). Fixed in VS-08: the fence rises to 55 m (above the gallery at 46.5 m), and a hidden floor at −1.25 m catches a fall off the cliffs inside it. `Map.Sombre.Greybox` pushes a pawn at 30 m and 50 m against every fence segment.
  - Vault: walkable floors, the catwalk and pipe route across the lower gallery; no AI.

## Gameplay

- **Core interaction:**
  - **B3, the lamp room:** the lamp, the clockwork, the feed box, the unlit oil burner on the gallery; the feed box gives `cut_cable_end` and sets `sombre.cable_cut_found`
  - **B4, the vault:** get in (four ways), cross the lower gallery, read the log, the sign-in board, and the panel; take the bearing; find Liv's note
  - **B7, the decision:** the physical acts below
- **Possible danger:** live water in the lower gallery (`ADCDamageVolume`, `ActiveConditions = [sombre.storm, !sombre.node_destroyed, !sombre.vault_floor_isolated]`).
  - avoid it by the catwalk and pipe route
  - Survival reads the dead-eel line (the safe edge); Pulse Read on the eels: "One shock, from the middle of the floor, where there is nothing."
  - Engineering 2 isolates the floor grid at the breaker (`sombre.vault_floor_isolated`)
  - waiting for the storm to pass is not possible, because `knows` needs the panel
- **Combat route:** `N/A — no enemies in the tower or the vault (the slice's one fight is the Grey, headland spec).`
- **Non-combat route:** all of it.
- **RPG / build checks:**
  - Engineering 2: the clockwork reading; the feed box's "lineman's shears / the feed runs down" variant; "Pry the jam" (lower door); the panel's five conditions; the bearing without the storm; isolate the floor; splice the feed; jumper the settlement feed; open the sea cock without Sigrun; file a pawl
  - Survival: the eel line
  - Persuasion and the tie: Odette's key (settlement)
  - **Zero investment gets in and reaches every outcome:**
    - **in:** the conduit, which needs no check; or Sigrun with the cable end; or Odette's key, but only if the player answered Mara's tie question and carries the letter (a player who said "Does it matter?" has the other two ways)
    - **outcomes:** Hale's crew splices; the pawl from the *Grey* plus rendered oil gives the hand light; Sigrun's wrench opens the sea cock
- **Reward:** evidence (`cut_cable_end`, `vault_access_log`, `liv_note`), `liv_chart`, the truth (`knows`), loot in `sombre.vault_locker`; the decision itself.
- **Exit state:** the quest at `knows` (the storm clears and Hale arrives, on the scene cut out of the vault). After B7, exactly one node state (Line, Settlement, Destroyed, or Untouched), with or without the hand light; the tower's light and the panel's read-out show it.

## Environmental Story

1. **Clue (lamp room):** an electric lamp in a big lens, dead; beside it on the gallery floor an old oil burner set out and never lit.
2. **Clue (lamp room):** the clockwork's chain rusted to its drum, a pawl sheared.
3. **Clue (feed box):** the feed's cable ends cut and folded back. Clean cuts: shears, not a saw. (Engineering 2: the feed runs *down*, into the rock.)
4. **Clue (vault):** the Authority keeper's log in three hands, "Current event, minor. Shore lamps to reserve," years before the collapse; the last entry: "Section order received 03:12. Link locked. God help Kenning."
5. **Clue (vault):** the panel: "CONTINUITY SECTION 14. SEVERED. RESYNC PENDING: 2 OF 5." An empty card slot. The sign-in board: O.B. and L.K., four months old. Liv's chart and her note to Odette.

- **What happened here (author-only):** Liv pulled the card that tied the light to Section 14; someone else cut the feed so it could not be fixed. PROVISIONAL.
- **What the player can infer:** the light was part of something larger; Liv stopped it on purpose; the island has been living with the cost.
- **What deliberately remains unresolved:** what Section 14 is, what "the pattern" means, what the Current is (never explained), where Liv went beyond "past the Narrows".
- **PROVISIONAL claims:** the log entries; the panel text and its five conditions; Liv's note (approved as written by Anthony, 2026-09-30); the empty oil store `[P]`; the tower's own characteristic (default two flashes every 10 s, PROVISIONAL, Checkpoint D).
- **Links to existing lore:** `Design/Narrative/MYSTERY_LEDGER.md` §5, `QUEST_ARCS.md` §2, `Design/world_bible.md` (the Authority, the Compact: PROVISIONAL).

## State / Persistence

- **Persistent ids:** `sombre.tower_door` (`ADCDoor`), `sombre.vault_locker` (container). Inspectables save through their flags. Portals save nothing.
- **Discovered-location ids:** `sombre.light`, `sombre.vault`.
- **World flags (all in the ledger):**
  - lamp room: `cable_cut_found`, `feed_spliced`, `clockwork_freed`, `light_hand`
  - vault: `vault_opened`, `log_read`, `panel_read`, `bearing_taken`, `liv_note_found`, `vault_floor_isolated`, `light_line`, `power_settlement`, `node_destroyed`
  - `light_decided`, on every act
  - read: `storm`, `hale_crew_up`, `sigrun_helping`, `keeper_odette`, `keeper_dell`, `meeting_done`, `night_quay_seen`
- **Quest stages (`sombre.characteristic`):**
  - `arrived` → `lamp_room` on `LocationDiscovered sombre.light`
  - `lamp_room` → `vault` on `sombre.cable_cut_found`
  - `vault` → `vault_inside` on `sombre.vault_opened`
  - `vault_inside` → `knows` on `sombre.panel_read` and `sombre.liv_note_found`
  - **`knows` on enter:** `ClearWorldFlag sombre.storm`, `SetWorldFlag sombre.hale_arrived` (quest data, WP-NARR; the atmosphere follows by the core's shipped rules). **`knows` objective text:** decide what happens to the light, then tell the settlement at the net loft. Marthe's call (settlement spec) is what gathers them.
- **Conditions:** `WorldFlag`, `HasItem` (`sombre_vault_key`, `section_key_compact`, `clockwork_pawl`, `lamp_oil`), `SkillAtLeast Skill.Engineering 2`, `QuestStage`.
- **Consequences:** `GiveItem` (`cut_cable_end`, `vault_access_log`, `liv_note`, `liv_chart`), `RemoveItem` (`section_key_compact`, `clockwork_pawl`, `lamp_oil`), `SetWorldFlag`.
- **The decision acts** (first match wins within each actor; mutual exclusion by negated conditions):

| Act | Room / actor, verb | Conditions | Consequences |
| --- | --- | --- | --- |
| Splice the feed | lamp room feed box, "Splice the feed" | `SkillAtLeast Skill.Engineering 2`; or (second variant) `sombre.hale_crew_up` | `sombre.feed_spliced` |
| Seat the card | vault panel, "Seat the card" | `HasItem section_key_compact`, `feed_spliced`, `!node_destroyed`, `!power_settlement`, `!light_hand` | `RemoveItem section_key_compact`, `light_line`, `light_decided` |
| Throw the settlement feed | vault panel (card variant; jumper variant) | `!node_destroyed`, `!light_line`, and `HasItem section_key_compact` or `SkillAtLeast Skill.Engineering 2` | `power_settlement` (card variant also `RemoveItem section_key_compact`), `light_decided` |
| Open the sea cock | vault sea cock, "Open it" | `!light_line`, and `sigrun_helping` or `SkillAtLeast Skill.Engineering 2` | `node_destroyed`, `light_decided` |
| Free the clockwork | lamp room clockwork, "Fit the pawl" / "File a pawl" | `HasItem clockwork_pawl`, or `SkillAtLeast Skill.Engineering 2` | `clockwork_freed` (fit: `RemoveItem clockwork_pawl`) |
| Light it by hand | lamp room lamp, "Fill and light the burner" | `clockwork_freed`, `HasItem lamp_oil`, `!light_line` | `RemoveItem lamp_oil`, `light_hand`, `light_decided` |

- **Panel read-outs** (inspect variants, first match wins): destroyed "Dark. Water to the second rung." (this variant **also sets `sombre.panel_read`**, §8.2 #5, so `knows` stays reachable if the sea cock comes first); Line "RESYNC PENDING: 4 OF 5" (+ Engineering 2 "Only confirmation left. It's asking."); Settlement "3 OF 5 …"; default "2 OF 5".
- **Data loss is real:** after `node_destroyed`, an unread log is "pulp" and an untaken bearing is "under water". **Soft-lock guard (§8.2 #6):** the chart table and Liv's note stand **above the flood line** in the node room, so flooding never destroys the note (`liv_note_found` gates `knows`).
- **The bearing (§8.2 #7):** "Take the bearing" needs `sombre.storm` or Engineering 2. The storm clears at `knows`, so taking it is a choice the player can miss. That is intended; the gameplay critic checks it reads as a choice. It depends on `sombre.storm` actually being set (the strike sets it; ledger §3.1).
- **Visible world-state variants:**
  - the tower dark until an outcome
  - one light actor per outcome, from VS-17: the Line (the light sequence's characteristic, plus the one-time prelude on `[sombre.light_line, sombre.night_quay_seen]`); by hand (slow, steady)
  - the keeper silhouette on the gallery: `keeper_odette` or `keeper_dell`, with `meeting_done`
  - the vault flooded after `node_destroyed`: water to the second rung (vault rule); the sea mouth bubbling below the cliff (lighthouse rule)
  - panel read-outs by state
- **Save impact:** new ids only.

## Asset Plan

### Tier A — Focal

| Asset | Role | Source (library searched first; plan §12.2) | Meshy credits (est.) |
| --- | --- | --- | --- |
| Tower shell (silhouette; walkable only at the base and gallery) | primary landmark | procedural revolved profile (Geometry Script or generated mesh like the terrain) → Meshy only if that fails | 0 (worst 50) |
| Lamp and Fresnel lens | B3, the dead lamp | library search → Meshy | 0–40 |
| Rotation clockwork (weight, drum, governor, sheared pawl) | B3, B7 | `Scene_Junkyard` gears and chain composed → Meshy | 0–40 |
| Feed box with cut, folded ends | B3 | library electrical box + a small cut-ends mesh | 0–30 |
| Node panel (Section 14) | B4, B7 | the relay housing / breaker panel language (shipped Meshy props) → Meshy | 0–40 |
| DF loop on its turntable | B4 | Geometry Script ring on a turntable → Meshy | 0–40 |
| Sea cock wheel and pipe | B7 | `AbandonedPowerPlant` / `Scene_Junkyard` → Meshy | 0–30 |
| Keeper's log | B4 | `lighthouse_logbook` (earlier Meshy output, owned) | 0 |
| Chart table + Liv's chart, Liv's note, sign-in board | B4 | kit table + authored chart texture; `SM_Notes` | 0 |
| Compact card, pawl, oil can | B5, B7 | library gear, can, jerrycan → one small Meshy batch if needed | 0–30 |
| Dead eels | B4 Pulse Read | reuse `dead_fish` (shipped Meshy prop), retinted | 0 |

Every Meshy run needs this table's justification plus the failed library searches recorded here first; the phase ceiling is 350 (a stop limit).

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| Tower base room (`Wall_*`, `Deck_100`, concrete skin) | inside the base | modules yes; **composition no** — VS-12 adds `structures/tower_base_room.json` |
| Vault spaces | the vault | `N/A — Authority concrete from AbandonedPowerPlant modules (library meshes, listed as Tier A/B library pieces in vault.py), not the settlement kit` |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| `N/A — the shore below the tower is the cable hut's zone; this spec's pads are requested in zones/cable_hut.json. The vault and the tower have no recipe.` | — | — | `lower_door_approach`, `tower_door_approach`, tower-path corridor |

## Audio

- Tower: wind at height (positional), rope and cable slap, the lamp room's storm light hum.
- Vault: drip and room tone, the sixty-cycle hum at the node (existing `S_DC_HumRelay` family), sparks on the live water (shipped), the sea cock's rush after `node_destroyed`.
- Emergency lamps flicker with no power (`ADCFlickerLight`).
- CC0, sourced per plan §8.10. Gap: no music; no voice.

## Existing Systems Used

Inspectables with variants and per-variant verbs and conditions, inventory, loot container, the damage volume (live water), flicker lights, conditional audio, conditional presence (flooded water, sea-mouth bubble, keeper silhouette, evidence props after pickup), location discovery, quests (transitions, on-enter consequences), world flags, build checks, `ADCDoor`, the cell portal (stair, hatch, lower door, conduit).

## Missing Reusable Capability

None for VS-12 and VS-14. **Dependency:** the tower's lit states use the **authored light sequence** (plan §6 #2), built at the start of VS-17 by WP-SYS-SEQUENCE (an approved capability). Until then the tower is dark and uses `ActiveConditions` only.

## Tests

- **Content validation:** `Content.Sombre.QuestGraph` (every outcome reachable; `knows` needs both the panel and the note; the untouched ending completes).
- **Interaction:**
  - **`Map.Sombre.Lighthouse`** (VS-12): `sombre.light` discovered → `lamp_room`; the feed box → `cable_cut_found` → `vault`; the build readings; `cut_cable_end` given once; the stair both ways (anchors). The hatch refuses without the key (locked text) and opens with it. The lower door refuses without Engineering 2. It does **not** assert the vault interior.
  - **`Map.Sombre.Vault`** (VS-14, inside only): live water by storm and isolation; the evidence; data loss after the sea cock; both soft-lock guards; `knows` in both orders (panel first, note first); the storm clears; Hale arrives out of sight.
  - **`Map.Sombre.VaultRoutes`** (Integrator, at the vault merge): all four entries with their builds; zero investment by the conduit and by Sigrun; build-gated routes hidden without the build.
  - **`Map.Sombre.Resolution`** (VS-17): every act, mutual exclusion, hand-light combinations, keeper choices, panel read-outs, every outcome for zero, Engineering, Survival, Persuasion.
- **Persistence:** `Map.Sombre.Saves`: F5 at `lamp_room`, `vault`, `vault_inside`, `knows`; inside the vault; diverge; F9 (no re-announce).
- **Alternate routes:** as above (zero investment by the conduit and by Sigrun).
- **Integration:** the tower's landmark trace from every exterior view; the gallery view.
- **Regression:** full suite; baseline 52 at VS-07.

## Review Views

Lighthouse file `Tools/Review/Lvl_PointeSombre/lighthouse.json`; vault file `vault.json`. The core view `tower_rock_from_settlement` stays the core's. Exterior views trace `tower_site`; the vault's do not (no windows). Cameras *(planned)*.

| Id | File | Setup | Expected observation |
| --- | --- | --- | --- |
| `lighthouse_base` | lighthouse | default | The tower's base and open door from the path; the tower reads as the destination. |
| `lamp_room` | lighthouse | `lamp_room` | The dead lamp and the unlit burner. |
| `feed_box_close` | lighthouse | `lamp_room` | The clean cuts in the feed box. |
| `gallery_view` | lighthouse | calm (`hale_arrived`) | The whole island from the gallery: quay, settlement, headland, cable hut, the *Grey*. |
| `vault_entry_lower` | lighthouse | `vault_opened` | The lower door in the sea cliff, and the way on (exterior end). |
| `vault_entry_hatch`, `vault_entry_conduit` | vault | `vault_opened` | Where each route lands; a route onward. |
| `vault_lower_gallery` | vault | storm | The live water and the catwalk; the dead eels. |
| `vault_node_room` | vault | storm | The panel, the loop, the chart table above the flood line. |
| `vault_flooded` | vault | `node_destroyed` | Water to the second rung; the chart table still dry. |

## Human Acceptance Checklist

1. (Checkpoint B) Climb the tower; read the lamp room. *Did you know where to go next without being told?* (plan §22 step 4)
2. (Checkpoint C) Get into the vault however your character can. *Was there more than one way you could see?*
3. (Checkpoint C) Cross the lower gallery; read everything; find Liv's note. *Did it feel like a dungeon rather than a room with clues? Where was the danger?*
4. (Checkpoint D) Decide the light with your hands. *Did you understand what you physically changed? Did the decision feel earned?*

## Open Creative Decisions

- **WP-VAULT (parallel vault package).** Default: **not enabled; serial (VS-14, Claude).** Anthony may enable it at Checkpoint B; the frozen boundary above then applies.
- **The tower's own characteristic.** Default: two flashes every 10 s, PROVISIONAL (Checkpoint D).
- **The empty oil store at the base `[P]`.** Default: kept if VS-12 has time.
- **Hale's crew at the base after `hale_crew_up` `[P]`.** Default: no figures; the splice simply works.
- **The lower door's locked text.** Default: "The door is jammed in its frame."
- **Tower shell method.** Default: a generated revolved profile; Meshy only if it fails the look at Checkpoint A/B.

## Pilot Record

- Spec committed: VS-07. First green map tests: `Map.Sombre.Lighthouse` (VS-12), `Map.Sombre.Vault` (VS-14). Existing classes only plus the approved light sequence (VS-17); Meshy 0 so far (Tier A rows may spend with justification); kit `tower_base_room` (new composition); zone `N/A`; views as above; critic files at Checkpoints B, C, D.
