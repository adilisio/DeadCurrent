# POI Specification — The Harbor (`sombre.harbor`)

Copied from `TEMPLATE.md`. Contract: standard (`VerticalSlicePhasePlan.txt` §11). Ids: `Design/POIs/sombre_ids.md` (the ledger wins). Story: beat script B1, B5, B9; `SLICE_DIALOGUE.md` §2 (Varga), §8 (Hale). Written in VS-07 (2026-10-01).

**Frame (VS-04):** X north, Y east, Z up, sea level 0; metres unless marked cm. *(planned)* positions are set by VS-08's greybox and frozen at Checkpoint A.

---

## Identity

- **Stable POI id:** `sombre.harbor` (cell id and location id).
- **Working name:** Pointe Sombre Harbor (discovery banner text; Anthony may rename the display text).
- **Region:** the island's south shore: the quay, the sheltered water behind the south reef, the harbor mouth.
- **Biome:** `great_lakes_rocky_shore`, zone file `Tools/Biomes/zones/harbor.json` (one of the three required zones).
- **Content status:** `SPEC`.
- **Lore status:** `PROVISIONAL`. Claims under Environmental Story.
- **Owner:** Claude. VS-10 builds B1 (with the crossing); VS-15 adds Hale and the cutter (B5); VS-19 the night quay picture and the end-card trigger (B9). Files: `pointe_sombre/harbor.py`, `dress_sombre_harbor.py` → `Lvl_PointeSombre_Art_Harbor`, `Content/World/PointeSombre/Harbor/`, `Tools/Review/Lvl_PointeSombre/harbor.json`, `Tools/Biomes/zones/harbor.json`. Tests live in other files (see Tests).

## Player Promise

> I step off a wrecked boat into a working harbor that won't let me leave, and everything I need to look at is already in view.

## Discovery

- **What draws attention:** the *Ida* listing at the quay with her pumps going; the dark tower above the town to the north-east; the false-light post across the water to the west; the *Ashland Grey*'s hull on its reef to the south-west.
- **Intended approach:** the crossing's wheelhouse door lands the player on the quay at `Anchor_CrossingExit_Quay` (−84, 10) facing up the island.
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** `ADCLocationVolume` `sombre.harbor` covering the quay apron; the banner once on arrival; silent on load.

## Spatial Role

- **Approximate footprint:** the quay apron (pad `quay apron`, centre (−78, 10), radius 20 m, Z 1.8 m) and the quay wall along X −88; the shore from Y −150 (the bight) to Y +75 (the harbor mouth side); the water behind the south reef to X −130. *(planned, VS-08)*
- **Nearby routes:** quay → store (10–15 s, plan §5.5), quay → tower base (35–50 s), quay → west headland (45–60 s, past the Pruitts' shed). All three legs start here.
- **Important sightlines:** from the quay, at once: the tower (NE), the false-light post (W), the *Grey*'s hull (SW) — plan §5.5 "landmarks overlap" (Gemini's VS-08 brief prefers the *Grey* as a later reveal; VS-08 judges whether a small distant silhouette helps; the view expectation does not require the hull). From the night quay landing (`Anchor_LoftStair_NightQuay`), the tower straight ahead.
- **Exclusion zones** (in `zones/harbor.json`, landed VS-06 schema, metres; *(planned)*, adjusted with the greybox):
  - `exclude_pads`: `quay apron` (margin 4), `settlement terrace` (margin 6)
  - `ida_berth`: box, centre (−92, 12), half (4, 14), yaw 0
  - `hale_berth`: box, centre (−92, −18), half (4, 12), yaw 0
  - `route_quay_store`: box, centre (−60, 2), half (22, 3), yaw −21
  - `route_quay_tower`: box, centre (−70, 35), half (12, 3), yaw 55
  - `remy_holdback`: box, centre (−82, 58), half (8, 8), yaw 0. The shore at the harbor mouth stays clear so VS-20 can place Remy's marker and look out at his boat on the reef. **Named, empty, not built here** (`sombre_ids.md` §13).
  - Requested by the settlement spec, if they fall inside this zone: `store_door`, `net_racks`, `smokehouse_front`, `loft_stair_foot`. The settlement owns the names and positions; this zone file carries the boxes.
  - Automatic radii (the tool): Varga, Mara, Hale, the portals and anchors' actors, the location volume.
  - Sightline to `tower_site`: `N/A — Tier C instances are under 3 m tall and cannot block the view of a tower on a 26 m rock; no box.`
  - **Landed in VS-08** (`zones/harbor.json`, written by the Integrator; the harbor's file from VS-10): the two pads; `ida_berth` (half (5, 14)), `hale_berth`, `remy_holdback` as planned; the two route boxes became `exclude_paths` `quay_store`, `quay_tower`, `quay_head` (margin 1 each), which follow the trails as built. The west harbor shed stands at (−82, −20), off the line from the quay arrival to the false-light post, and the TEST fixture moved to (−79, −29) for the same reason; from the quay the post and the *Grey* both pass the landmark trace (`greybox_quay_west`), the *Grey* far and small.
- **Navigation requirements:** the quay apron and the shore path are walkable terrain (the 2 m collision mesh). Varga and Hale stand on vessel decks (hidden collision, the Landing Stage pattern). No AI patrols.

## Gameplay

- **Core interaction:** talk to Varga (starts both quests); look at the *Ida*'s damage; later Hale (B5), the night quay (B9), and the end card.
- **Possible danger:** none.
- **Combat route:** `N/A — no combat in the harbor.`
- **Non-combat route:** the only route.
- **RPG / build checks:** none of the harbor's own (Hale's offers are dialogue checks in `sombre_hale`).
- **Reward:** the quests and their objectives (B1); Hale's card and oil (B5); the end card (B9).
- **Exit state:** after B9 the night quay shows the outcome (§ Visible variants) and the end card has been shown once.

## Environmental Story

1. **Clue:** the *Ida* at her berth, listing, pumps thumping, a fresh scrape along her hull — she hit the reef following the wrong light.
2. **Clue:** Varga on her deck: "Somebody lit it on purpose." The harbor works around a dark tower.
3. **Clue (B5):** Hale's Compact cutter in the calm water, a clean modern hull beside the patched *Ida* — the outside world has come for the light.
4. **Clue (B9):** the night quay's picture of the player's decision (the tower, the windows, the headland).
5. **Clue `[P]`:** a Compact Loss Book page on Hale's gangway (B5). Default: cut unless VS-15 has time.

- **What happened here (author-only):** the harbor lost its traffic when the light went dark; Varga's run is its last link. PROVISIONAL.
- **What the player can infer:** the island needs the light; outsiders want it lit for their own reasons.
- **What deliberately remains unresolved:** what the Compact uses the light for.
- **PROVISIONAL claims:** Varga's lines and run; the Compact's Lights Office and Hale's card; the Loss Book.
- **Links to existing lore:** `Design/world_bible.md` (the Compact, PROVISIONAL), `Design/Narrative/QUEST_ARCS.md` §2.

## State / Persistence

- **Persistent ids:** `sombre.varga`, `sombre.hale` (VS-15), `sombre.mara` (one actor; this script spawns her and owns every placement: `Anchor_Mara_Rail`, the quay, `Anchor_Mara_Porch`, `Anchor_LoftSeat_Mara`, the night quay).
- **Discovered-location id:** `sombre.harbor`.
- **World flags:** reads `sombre.reef_struck`, `sombre.hale_arrived`, `sombre.meeting_called`, `sombre.meeting_done`, the outcome flags; Varga's dialogue sets `sombre.varga_told` and (default end-card trigger) `sombre.slice_end`.
- **Quest stages:** Varga's `first` starts `sombre.characteristic` (`arrived`) and `sombre.false_light` (`asked`).
- **Conditions:** `WorldFlag`, `QuestActive`, `HasItem` (Varga's and Hale's dialogue).
- **Consequences:** `StartQuest` ×2, `SetWorldFlag`, `GiveItem` (Hale: `section_key_compact`, `lamp_oil`).
- **Visible world-state variants:**
  - Mara on the quay from `sombre.reef_struck`; on the store porch at `Anchor_Mara_Porch` from quest stage `lamp_room` (default; VS-10 may pick another stage boundary); in the loft at `Anchor_LoftSeat_Mara` while `sombre.meeting_called` and `!sombre.meeting_done`; on the night quay after `sombre.meeting_done`.
  - Hale and the cutter absent until `sombre.hale_arrived` (VS-15), on the scene cut out of the vault.
  - Hale and Varga in the loft (`Anchor_LoftSeat_Hale`, `Anchor_LoftSeat_Varga`) while `sombre.meeting_called` and `!sombre.meeting_done`.
  - Night quay (VS-19), by outcome. On the Line: the tower's characteristic, and the one-time prelude on `sombre.night_quay_seen` (the light sequence; lighthouse-owned actors). By hand: slow and steady, with the keeper silhouette. Settlement power: windows lit along the harbor (window lights are this cell's, on `sombre.power_settlement`) and the point dark. Destroyed: dark, the vault mouth bubbling (lighthouse actor). Untouched: dark.
- **Save impact:** new ids only.

## Asset Plan

### Tier A — Focal

| Asset | Role | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| The *Ida* at the quay | landmark, Varga's deck | the crossing's art, second placement (no second vessel) | 0 |
| Hale's cutter (VS-15) | B5's visible change | re-trim of the *Ida*'s hull parts (plan §18 fallback) unless the library has a fitting hull; library searched first | 0 by default |
| Varga, Hale | people | `Survival_Character` body pipeline (Checkpoint B decides NPC bodies, §12.3) | 0 |
| Quay wall and bollards | the berth | Fab `harbor_props` (bollard, cleat, chain) + pier planks/poles (migrated, Phase 5) | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| `kit_harbor_shed` ×2 | gear sheds on the quay, gable to the water, turned differently | yes |
| Hale's cutter deck furniture: `Deck_100`, `Railing_200`, `Post_100` | the cutter's gangway | modules yes; **composition no** — VS-15 adds `structures/cutter_deck.json` |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| Shore rock, cobble, driftwood, scrub | `great_lakes_rocky_shore`, zone `harbor`, polygons `harbor_shore` (Y −60..+75, X −98..−70) and `harbor_bight` (Y −150..−60), exposure `auto` (fetch reads the reef shelter), seed chosen by the owner (not the test zone's 7), density scale 1.0 | NoCollision | the list under Spatial Role |

## Audio

The shipped exterior wind bed; water lap; the *Ida*'s bilge pump (loop while at the berth, `ADCConditionalAudio` on `sombre.reef_struck`); rope and timber creak; at B9 the night storm's rain and thunder (VS-19). CC0, sourced and recorded as in plan §8.10. Gap: no voice, no music (default §12.4).

## Existing Systems Used

Dialogue (Varga, Hale, Mara), quests (start), conditions/consequences, world flags, conditional presence (Mara, Hale, the cutter, loft seats), location discovery, `ADCConditionalAudio`, point lights for windows (presence on `power_settlement`), the cell portal's arrival anchor, the shoreline recipe.

## Missing Reusable Capability

None in the harbor. The night-quay pattern prelude uses the light sequence (built at the start of VS-17, lighthouse-owned actors); the end card uses the story card (start of VS-19). Both are approved capabilities (plan §6), listed here as dependencies.

## Tests

- **Content validation:** `Content.Sombre.Dialogue` covers `sombre_varga` and `sombre_hale` (Varga's `first` starts both quests; no tie leak).
- **Interaction:** B1 in `Map.Sombre.Crossing` (arrival, `sombre.harbor` once, Varga starts both quests). B5 in `Map.Sombre.Midpoint` (Hale and the cutter appear on the scene cut out of the vault; his offers). B9 in `Map.Sombre.SecondStorm` (each outcome's night picture; the end card once, not on load). There is **no** `Map.Sombre.Harbor`.
- **Persistence:** `Map.Sombre.Saves` (VS-24): F5 at `arrived` and at each `done_*` on the quay; diverge; F9.
- **Alternate routes:** skip Varga and explore: quests start when she is met and chain forward (existing behavior).
- **Integration:** `Map.Sombre.BiomeExclusions` stays green with `harbor.json` (nothing in `remy_holdback`, the berths, or the corridors). `Map.Sombre.Greybox` reaches every cell from the quay.
- **Regression:** full suite; baseline 52 at VS-07.

## Review Views

`Tools/Review/Lvl_PointeSombre/harbor.json`. The core's `quay_storm`, `quay_calm`, `quay_night` stay the core's (the atmosphere trio). Every exterior view traces the landmark `tower_site` [2200, 16000, 2700] cm. Cameras *(planned)*.

| Id | Camera (x, y, z) cm, pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `arrival_quay` | (−8300, 900, 360), 4 / 50 | the *Ida* and the tower | Setup `sombre.reef_struck`. The *Ida* listing and pumping at her berth; the dark tower above the town. |
| `harbor_overview` | (−9000, −2000, 900), −4 / 40 | three landmarks | Setup `arrived`. Tower (NE) and false-light post (W) in one frame; the *Grey* may show as a small distant silhouette or not at all (VS-08's call). |
| `harbor_mouth_remy` | (−7600, 5000, 450), −6 / 200 | the harbor mouth | Default. **Clear rocks where Remy's boat will be; not a boat** (until VS-20). |
| `hale_arrival` | (−8300, −900, 360), 2 / 160 | the cutter | Setup `sombre.hale_arrived`. The cutter in the calm harbor beside the *Ida*. |
| `night_quay_line`, `night_quay_hand`, `night_quay_power`, `night_quay_destroyed`, `night_quay_untouched` | from `Anchor_LoftStair_NightQuay`, 6 / toward the tower | the tower | Setup each outcome, `sombre.meeting_done`, `sombre.night_quay_seen`. The tower's steady state per outcome (plan §21); the prelude is not captured. |

## Human Acceptance Checklist

1. Arrive from the crossing. *Did you notice the Ida and the tower before anything else?*
2. Talk to Varga. *Do you know what she wants and why she won't leave?*
3. Look around the quay before going anywhere; F5. *What did you want to go and look at first, and why?* (plan §22 step 2)
4. (Checkpoint C) Come out of the vault. *Did you notice the harbor had changed before anyone told you?*
5. (Checkpoint D) Come down from the loft onto the night quay. *Did the settlement visibly react?*

## Open Creative Decisions

- **End-card trigger (§8.2 #11).** Default: **Varga's "Where next?" sets `sombre.slice_end`.** Alternative kept: a "Watch the light" interact at the quay's end. Chosen in VS-19, judged at Checkpoint D.
- **When Mara moves to the store porch.** Default: quest stage `lamp_room`.
- **Loss Book page on the gangway `[P]`.** Default: cut.
- **The *Grey* from the quay** (plan §5.5 vs Gemini's reveal preference). Default: VS-08 judges by capture; a small, partly occluded silhouette at most.

## Pilot Record

- Spec committed: VS-07. First green map test: `Map.Sombre.Crossing` (VS-10). Existing classes only; new C++ 0 (the sequence and card are approved capabilities owned elsewhere); Meshy 0; kit `kit_harbor_shed` ×2, `cutter_deck` (new composition, existing modules); zone `harbor`; views as above; critic files at Checkpoints B, C, D.
