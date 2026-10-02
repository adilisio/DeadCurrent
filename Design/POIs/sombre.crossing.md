# POI Specification — The Crossing (`sombre.crossing`)

Copied from `TEMPLATE.md`. Contract: standard (`VerticalSlicePhasePlan.txt` §11). Ids: `Design/POIs/sombre_ids.md` (the ledger wins over this file). Story source: `Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md` B0, `SLICE_DIALOGUE.md` §1. Written in VS-07 (2026-10-01).

**Frame (VS-04):** `Lvl_PointeSombre`. X is north, Y is east, Z is up, sea level Z 0. Metres unless marked cm. The deck is offshore south-west of the island, centred (−330 m, −360 m), deck top Z 3 m, heading 25° (bow toward the west headland and, beyond it, the tower). Positions marked *(planned)* are set when the cell is built; VS-08 may move the island under them, and Checkpoint A freezes the layout.

---

## Identity

- **Stable POI id:** `sombre.crossing` (cell id). **Not discoverable** (plan §8.4): no location id, no banner.
- **Working name:** the crossing (the *Ida*'s deck). No display name.
- **Region:** Pointe Sombre, offshore south-west, in the storm fog.
- **Biome:** `N/A — an offshore deck; no shore, no recipe zone.`
- **Content status:** `BUILT` (VS-10, 2026-10-02): playable and tested (`Map.Sombre.Crossing`); critics and Anthony at Checkpoint B. Not yet: the storm's new sounds (rain, surf, creak, the strike's hit; they need a Freesound authorization) and the `ida_deck` kit composition (the rails are greybox stanchions).
- **Lore status:** `PROVISIONAL`. Claims listed under Environmental Story. Nothing explains the Current; Mara's backstory is not decided here.
- **Owner:** Claude, VS-10 (with the harbor). Script `pointe_sombre/crossing.py` (grows the stub); dressing `dress_sombre_crossing.py` → `Lvl_PointeSombre_Art_Crossing`; content `Content/World/PointeSombre/Crossing/`; review `Tools/Review/Lvl_PointeSombre/crossing.json`; test `Source/DeadCurrent/Save/DCSombreCrossingMapTest.cpp`.

## Player Promise

> The slice starts with me standing in the storm, seeing the wrong light before anyone else does, and I am too late to stop the strike.

## Discovery

- **What draws attention:** a light burning low on a headland ahead (the false light), and beyond it a tower with no light at all, shown only in lightning. Mara at the rail. A letter on the hatch cover in the lee of the wheelhouse.
- **Intended approach:** the player starts here (new game). The deck is about 24 m by 7 m; everything is within a few steps.
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** none (not a discoverable place). Leaving is the wheelhouse door.

## Spatial Role

- **Approximate footprint:** the deck, 2400 × 720 cm, centred (−33000, −36000) cm, heading 25°. Wheelhouse at the stern (aft 500 cm of the deck). Walkable via hidden collision under NoCollision art (the Landing Stage pattern, shipped in the stub).
- **Nearby routes:** none; the only exit is the wheelhouse door portal to the quay.
- **Important sightlines:** from the bow: the false-light post on the west headland (−18, −215) m and, beyond and to its right, the tower rock (22, 160) m. Both must be in the frame of `crossing_bow`. The island stays low on the horizon (fog), never a clear postcard.
- **Exclusion zones:** keep-clear pads (not recipe boxes; there is no zone here): the wheelhouse door approach (2 m in front of the door), the letter's inspect side, Mara's rail mark, the player start (`Anchor_NewGame_Deck`). No dressing with collision on the deck's walk lines.
- **Navigation requirements:** player only (no AI walks here). Deck collision is the hidden box; rails are hidden blockers. No nav mesh needed (outside the island's nav bounds).

## Gameplay

- **Core interaction:** read Liv's letter, look at the recap chart, talk to Mara (the tie question), look ahead at the lights, then "Tell Varga about the light".
- **Possible danger:** none.
- **Combat route:** `N/A — no combat.`
- **Non-combat route:** the only route.
- **RPG / build checks:** none. Nothing here depends on the build.
- **Reward:** `liv_letter`; the recap (where the player is and why); the tie (if chosen).
- **Exit state:** `sombre.reef_struck` and `sombre.storm` set by the wheelhouse door; the player on the quay; Mara and the *Ida* already there (presence snaps on the scene cut). The deck is never seen again (the *Ida* at the quay is the same art in its second placement).

## Environmental Story

1. **Clue:** Liv's letter on the hatch cover — "Read": tie-neutral text from the prologue doc (P1). Gives `liv_letter`, sets `watch.mara_travelling`.
2. **Clue:** the chart pinned in the lee of the wheelhouse — a one-screen recap: the Authority Shore, the Narrows, Pointe Sombre.
3. **Clue:** ahead, a light low on a headland, and a dark tower beyond it that shows only when the lightning comes — the wrong light is burning and the right one is not.
4. **Clue:** Mara at the rail (`sombre_mara` `crossing_tie`, `crossing`, and its `watch` and `third` nodes).

- **What happened here (author-only):** the Pruitts show a lantern on the west head in storms; the *Ida* reads it as the point. PROVISIONAL.
- **What the player can infer:** someone is showing a light where the lighthouse should be; the lighthouse is dark.
- **What deliberately remains unresolved:** why the tower is dark; who lit the false light.
- **PROVISIONAL claims:** the letter's wording (prologue P1); the recap chart's places; Mara's watch reveal ("That wasn't me. That was the watch."); the strike card text (VS-04 stub: "The *Ida* turns for the burning light, then shudders on stone." — VS-10 writes the final).
- **Links to existing lore:** `Design/world_bible.md` (the Authority Shore, the lake); `Design/Narrative/PROLOGUE_POSTED_FROM_THE_SHORE.md` (P1 letter, P2 tie question).

## State / Persistence

- **Persistent ids:** `sombre.mara` is spawned by `harbor.py` (one actor; the deck rail is one of her presence placements — `sombre_ids.md` §7). The crossing spawns no persistent actor.
- **Discovered-location id:** `N/A — not discoverable.`
- **World flags:** sets `watch.mara_travelling` (letter), one of `player.tie.sister` / `player.tie.partner` / `player.tie.took_in` or none, and `sombre.mara_tie_asked` (Mara), `watch.revealed` (Mara's `watch`), `sombre.reef_struck` and `sombre.storm` (door). Reads `wreck.mara_pressed`, `shore.liv_asked`.
- **Quest stages:** none set here. `sombre.characteristic` is not started on the deck; Varga starts both quests on the quay.
- **Conditions:** `WorldFlag` (negated tie flags = NO_TIE), `QuestNotStarted sombre.characteristic` (Mara's entries).
- **Consequences:** `GiveItem liv_letter`, `SetWorldFlag` (letter, tie, Mara's flags, the door's two flags).
- **Visible world-state variants:** the *Ida* (hull, deck collision, rails, wheelhouse, and the door) is one set of `crossing.py` actors with one presence rule: offshore as authored before the strike; at `Anchor_IdaBerth` on the quay after `sombre.reef_struck` (snaps on the scene cut). Not two vessels. Mara at `Anchor_Mara_Rail` before the strike, on the quay after (her rule is in `harbor.py`). The respawn rule (core, shipped) moves the player start to the quay. `Lantern_Crossing` (headland script) burns at the post only while `!sombre.reef_struck`. The wheelhouse door portal is not in the hull's rule: it has its own rule, present before the strike and absent after, so the strike cannot be replayed from the quay.
- **Save impact:** new ids only. No `SaveVersion` change. A save on the deck reloads on the deck.

## Asset Plan

### Tier A — Focal

| Asset | Role in the clues | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| Liv's letter (prop + inspect text) | the recap and the tie-key condition | CC0 paper from `SM_Notes` (library, owned pack) | 0 |
| Recap chart (pinned) | the one-screen recap | `SM_Notes` sheet + a chart texture made from project art | 0 |
| The *Ida*'s hull and wheelhouse | the vessel (also at the quay) | bespoke greybox from boxes (VS-04 stub); dressing pass in VS-21 from library hull parts if they fit (plan §18 fallback: one hull reused). Library searched first | 0 |
| Mara | the companion | the accepted placeholder body (Phase 5) | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| *Ida* deck furniture and wheelhouse front: `Deck_100`, `Railing_200`, `Wall_200_Door`, `Post_100` | deck edge, wheelhouse face | modules yes; **composition no** — the owner adds `Tools/Kits/great_lakes_settlement/structures/ida_deck.json` (reuse, no new module) |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| `N/A — offshore deck; no recipe zone. Deck clutter (rope, a bucket) is hand-placed NoCollision dressing in the art sublevel.` | — | NoCollision | the keep-clear pads above |

## Audio

- The exterior wind bed (shipped, `SA_DC_SombreExterior`), heavy surf, rain, thunder, the *Ida*'s bilge pump and rope/timber creak: CC0, sourced the 2026-09-29 way (plan §8.10) in VS-10; recorded in `art_pipeline.md`.
- The strike: a hull-on-stone hit under the fade (one-shot on `sombre.reef_struck`'s rising edge, `ADCConditionalAudio`).
- Gap, written down: no voice.

## Existing Systems Used

Interaction (inspectables with "Read"), inventory (`liv_letter`), dialogue (`sombre_mara`), conditions and consequences, world flags, conditional presence (Mara, the *Ida*'s two placements), the cell portal (shipped, VS-03), atmosphere by presence (shipped, VS-04: the dusk storm is the new-game look), lightning (`ADCFlickerLight`), `ADCConditionalAudio`, player start and the respawn rule (shipped).

## Missing Reusable Capability

None. The portal (VS-03) is shipped; everything else is data and layout.

## Tests

- **Content validation:** `Content.Sombre.Items` / `Content.Sombre.Dialogue` (WP-NARR) resolve `liv_letter` and `sombre_mara`'s entries; no tie leak.
- **Interaction:** `Map.Sombre.Crossing`: a new game starts on the deck in the storm look; the letter gives `liv_letter` once and sets `watch.mara_travelling`; Mara's tie question sets exactly one tie flag or none, and `sombre.mara_tie_asked`; she does not ask again.
- **Persistence:** F5 on the deck, diverge, F9: back on the deck, letter state restored.
- **Alternate routes:** read nothing and talk to no one, then the door: the strike still happens (zero investment).
- **Integration:** the door sets `sombre.reef_struck` and `sombre.storm`; the player arrives on the quay facing up the island; Mara and the *Ida* are on the quay (scene cut). The harbor arrival is asserted in the same test (there is no `Map.Sombre.Harbor`): `sombre.harbor` discovered once; Varga's `first` starts `sombre.characteristic` (`arrived`) and `sombre.false_light` (`asked`).
- **Regression:** the full suite, baseline **52** at VS-07 (34 editor, 13 Boathouse, 5 Sombre); `Map.Sombre.Architecture`, `Respawn`, `Atmosphere` unchanged.

## Review Views

`Tools/Review/Lvl_PointeSombre/crossing.json`. The core view `crossing_deck` stays the core's. Cameras are *(planned)*; the expectation is the contract.

| Id | Camera (x, y, z) cm, pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `crossing_bow` | (−32000, −35500, 470), 2 / 30 | the false light, the tower beyond | The false light low on a headland; the dark tower beyond it, readable against the storm sky (lightning helps; landmark `tower_site` traced). |
| `crossing_rail_mara` | (−33300, −36100, 470), 0 / 60 | Mara at the rail; the letter's spot | Mara at the rail, the letter on the hatch cover in the lee; the island low on the horizon. |

## Human Acceptance Checklist

1. New game. Look around the deck. *Does it read as a vessel in a storm, not a box on water?*
2. Read the letter; look at the chart. *Do you know where you are and why?*
3. Talk to Mara; answer "Who is she to you?" however you like. *Did the question feel like yours to answer?*
4. Look ahead from the bow. *Which light is wrong?*
5. "Tell Varga about the light." *Did the crossing feel played, not watched?* (Checkpoint B question, plan §22 step 1.)

## Open Creative Decisions

- **Strike card text.** Default: the VS-04 stub's line until VS-10 writes the final from B0's "Sees" sentence; Anthony may replace it at Checkpoint B. **VS-10:** kept as the final default (it is B0's own sentence): "The *Ida* turns for the burning light, then shudders on stone."
- **The recap chart's words** (VS-10, PROVISIONAL): "Varga's chart of the run, pinned in the lee of the wheelhouse. Her pencil line runs from the Authority Shore, east through the Narrows, to Pointe Sombre. She has circled the light on the Pointe." Only the run's places; it explains nothing.
- **Crossing fallback** (a story card over a fixed view, plan §5.4). Default: not used; the deck is played. The fallback needs Anthony's sign-off.
- **Where the letter sits.** Default: the hatch cover in the lee of the wheelhouse.

## Pilot Record

- Spec committed: VS-07. First green map test: `Map.Sombre.Crossing` (VS-10, 2026-10-02, first run). Built in VS-10 by Claude; found while building: Mara's crossing entries leaked onto the quay (fixed in the data, change list #17); the false light was invisible from the deck (`Lantern_Crossing` built for distance); the solid rails read as water (now stanchions); the hidden rail blocks would have blocked the interaction trace at the berth (`InvisibleWall`; the test walks to Varga). Captures `Saved/Review/2026-10-02_1005_Lvl_PointeSombre_vs10_final`. Existing classes only; new C++ 0; Meshy 0; kit composition `ida_deck` (new, from existing modules); zone `N/A`; views `crossing_bow`, `crossing_rail_mara`; critic files `Design/POIs/reviews/sombre_crossing_<critic>.md` at Checkpoint B.
