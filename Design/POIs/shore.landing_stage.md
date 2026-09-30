# POI Specification — Landing Stage

Copied from `TEMPLATE.md`. Source contract: `Design/content_production_strategy.md` §6. Ownership rules: `Design/POIs/README.md`. Pilot rules: `Design/POIs/PRODUCTION_PILOT.md`. Phase scope: `WorldStatePhasePlan.txt`.

Coordinates are cm in `Lvl_Boathouse`'s frame: +X is out of the boathouse door, the lake is −Y, Z is up, the beach is Z 0, the lake surface is Z −6.

---

## Identity

- **Stable POI id:** `shore.landing_stage`. Immutable once accepted. It is the `LocationId`.
- **Working name:** Landing Stage (displayed in the discovery banner and under PLACES). Working name; Anthony may rename the display text at any time (the id stays).
- **Region:** the Authority Shore, `Lvl_Boathouse`, in the shallows just east of the boathouse door, south of the path.
- **Biome:** hand-dressed (no biome recipe exists).
- **Content status:** `READY FOR ANTHONY` (2026-09-30): built, reviewed by two independent critics, revised on their findings (`Design/POIs/reviews/shore.landing_stage_revision.md`), re-verified. Only Anthony accepts.
- **Lore status:** `PROVISIONAL`. Every claim is listed under Environmental Story. Nothing explains the Current, decides a faction, or decides Mara's history or where she is going.

## Player Promise

> I did something on this shore, and when I come back to the landing, it shows.

## Discovery

- **What draws attention:** a lit storm lantern on a post and a string of bare bulbs over the water, right of the door as the player steps out; a skiff's shape alongside a timber stage. The bulbs hum.
- **Intended approach:** from the boathouse door, 3 m east and then south down a short gangway from the beach onto the stage. It is on the way the player walks every time; it is not hidden.
- **Marked / unmarked:** unmarked, like the Survey Launch.
- **Discovery behavior:** `ADCLocationVolume` `shore.landing_stage` / "Landing Stage", box X 830..1290, Y −1160..−590, Z −50..400. Walking onto the gangway triggers the banner once. The PLACES list gains it. Silent on load (existing behavior).

## Spatial Role

- **Approximate footprint:** gangway X 950..1070, Y −575..−820; deck X 850..1270, Y −820..−1120; skiff moored along the deck's west side (the side facing the boathouse door), centred about (765, −985), 4 m long; total about 5.5 m × 6.5 m. Adrift placement of the skiff: about (1720, −2080), yaw 35°, out in the channel.
- **Nearby routes:** the boathouse door (X 700..720, Y −50..50); the lake-side walk round the back of the boathouse to the Survey Launch (Y −560..−320 from X 700 west); the review route from the scavenger camp back to the boathouse through (2000, −400) → (700, −450); the beached hull inspectable (X 1090..1310, Y −315..−245); the scavenger's patrol square (X 2000..2700, Y −350..350), about 10 m east.
- **Important sightlines:** the lantern and the bulbs must be visible from the door and from the path west of the scavenger camp. The stage is south of the ridge, so it is **not** visible from Mara's lookout, and her lookout is not visible from the stage: a change at one is never seen happening from the other.
- **Exclusion zones (no collision, no dressing with collision):** the beach strip Y −580..−300 between X 700 and 2000 (the lake-side walk and the review route); the door apron X 700..900, Y −150..150; the hull inspectable's interaction side; the scavenger's patrol square; Mara's lookout shed interior (X 3040..3220, Y 1180..1500) except her pack's authored spot.
- **Navigation requirements:** the gangway and deck are walkable (collision, step heights under 45 cm: beach 0 → curb 18 → gangway 22 → deck 30). The scavenger's patrol and chase are unchanged; nav rebuilds at runtime as before. The lake sheet is already walkable; the stage adds nothing to block it.

## Gameplay

- **Core interaction:** look. Four inspectables (crate, lantern, bulbs, note on the post), one inspectable that is either the moored skiff or the cut line, one small loot container (tackle box).
- **Possible danger:** the scavenger, on the coil route only (he is alive there). From the south-west corner of his patrol, facing west, the gangway and deck are about 11 m away and inside his sight cone, and nothing blocks the line, so standing on the stage to take in the coil picture can start a chase. The stage does not change his behavior; it sits inside his existing sight. Kept by default as the tension the reward line already names ("He's still out there"). **Parked for Anthony** (Gameplay Critic G-02): keep it, or move the deck out of the cone.
- **Combat route:** Shore Watch by the kill route changes the stage (see State). No fighting at the stage.
- **Non-combat route:** Shore Watch by the coil route changes it differently; Mara comes to the stage.
- **RPG / build checks:** none. A character with nothing spent sees everything. (A build variant would be new writing; not in the pilot.)
- **Reward:** information (the consequence, seen), and the tackle box: 6× 9mm, 1× Salvaged Wiring.
- **Exit state:** the stage in one of four legible states (below) that persists through save/load.

## Environmental Story

In the order a player finds them:

1. **Clue:** the lantern on the post at the head of the gangway, lit, and a card tacked under it: *Don't tie up after dark unless the lamp is lit.* The landing is kept, and a lit lamp means open.
2. **Clue:** the crate under the lean-to, half packed (tins, a coil of line, a rolled blanket), lid off and leaning. Someone is getting ready to go somewhere by water.
3. **Clue:** the skiff, tied off short alongside, oars shipped. A boat someone means to use.
4. **Clue:** bare bulbs on a cable along the stage's edge, humming; the cable runs off the end of the stage into the lake. No generator.

- **What happened here (author-only):** the landing is Mara's way off this shore, kept against the day the watch has to travel. On the coil route she trusts the player enough to get ready; on the kill route she does not, and the landing is closed and stripped. **PROVISIONAL, author-only, never stated in game.** Who cut the skiff and emptied the crate on the kill route is deliberately open.
- **What the player can infer:** before: someone is preparing to leave by water. Coil: Mara is the one leaving, and she is ready. Combat: the landing has been shut down and stripped; the skiff was cut loose on purpose.
- **What deliberately remains unresolved:** where anyone is going; who closed the landing; why the bulbs have power.
- **PROVISIONAL claims:**
  - the card's rule (a lit lamp means the landing is open)
  - the landing belongs to, or is kept by, Mara (implied on the coil route only by her standing there with her pack)
  - the kill-route stripping (lamp pinched out, card torn down, crate emptied, skiff cut loose), by an unnamed hand
  - the bulbs on a cable into the lake that hum while the Survey Launch battery is live and go dark when its leads are pulled (the same unexplained power as the wreck; nothing says how)
- **Links to existing lore:** `Design/world_bible.md` (the Authority Shore, the Current left unexplained); Shore Watch outcomes (`technical_architecture.md`, First playable map). Accepted narrative facts touched: Mara is the companion (her readiness to travel on the coil route). Not touched: Liv, the tie, the watch office, Pointe Sombre, the *Ida*.

## State / Persistence

- **Persistent ids:** `landing.tackle` (the tackle box, `ADCLootContainer`). Reserved by the pilot and not used, because inspectables and presence rules carry no saved state: `landing.crate`, `landing.skiff`, `landing.note`. They stay reserved.
- **Discovered-location id:** `shore.landing_stage`.
- **World flags:** none new. Reads `shore.path_cleared`, `shore.relay_recovered`, `wreck.power_cut`.
- **Quest stages:** none new. Reads none directly (the flags are the outcomes of `shore.watch`).
- **Conditions:** `WorldFlag` on inspect variants, on `ADCConditionalPresence` states, on the bulbs' `ADCFlickerLight.ActiveConditions`, and on the hum's `ADCConditionalAudio`.
- **Consequences:** none.
- **Visible world-state variants:**

| State | Rule (first match wins) | Lantern | Card | Crate | Skiff | Mara, her pack | Bulbs |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Combat | `WorldFlag shore.path_cleared` | light hidden (lantern mesh stays, cold) | torn down (hidden), tack left | lid thrown on the deck, contents gone | moved to the adrift placement with its line's cut end; the moored line hidden, a cut stub on the cleat shown | at the lookout | per power |
| Coil | `WorldFlag shore.relay_recovered` | lit | shown | lid on, lashed (rope shown), contents hidden | moored, cargo shown (a tarped bundle, a jerrycan) | **Mara and her pack on the deck** | per power |
| Default | none | lit | shown | lid leaning against it, contents shown | moored, empty | at the lookout | per power |
| Power (additive) | `WorldFlag wreck.power_cut` | — | — | — | — | — | light and glow off, hum stops |

  From `landing_far`, the difference must read without text: warm light or none, a boat alongside or a boat far out, a person on the stage or not.
- **Save impact:** new ids only (`landing.tackle`, `shore.landing_stage`). No `SaveVersion` bump. The presence result is derived from flags, so an older save shows the state its flags imply.

## Asset Plan

### Tier A — Focal

| Asset | Role in the clues | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| Skiff (moored, loaded, adrift) | Clue 3 and the combat route's loudest change | `Smugglers_cove` `SM_boat_dutch_small_02` (a plank rowing boat without a mast; `_01` has one and was dropped), migrated at 1K, scaled to 4 m. Fallback: `motorboat_wreck` scaled (reads as a wreck, weaker). Meshy only if both read wrong in captures, one prop at about 40 credits. | 0 planned |
| Crate with a separate lid | Clue 2; the lid is what moves between states | PolyHaven `wooden_crate_01` (body, lid, latch meshes; CC0, already in `Content/Art/PolyHaven`) | 0 |
| Storm lantern | Clue 1 | PolyHaven `Lantern_01` (CC0, already imported) | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| Timber deck and gangway | the stage | New: `Smugglers_cove` `SM_wooden_pier_planks` (migrated at 1K, WS-05), stretched over hidden walkable blocks. The first timber kit piece; reusable for any later dock. |
| Piles, lean-to posts, lantern post | under and on the deck | Dark boxes (`MI_DC_Pile`). The pack's `SM_wooden_pier_poles` is a multi-pole frame, used only as old piles in the water (Tier C). |
| Lean-to roof | over the crate | A tarp-coloured box (`MI_DC_Tarp`). |
| Rope (mooring line, lashing, cut stub) | skiff, crate | Thin boxes in `MI_DC_Rope`. `harbor_props/rope.fbx` not needed yet. |
| Mooring cleat | deck edge | A rust box (`MI_DC_RustPaint`). |
| Bulbs and cable | deck east edge | `ADCFlickerLight` glow cubes (existing) on a thin dark box cable. |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| Driftwood at the gangway root, tins, two old piles in the water east of the stage | hand-placed in `dress_landing.py`, 6 pieces (`dead_quiver_branch_01`, `dead_tree_trunk`, `can_rusted`, `SM_wooden_pier_poles`) | NoCollision | beach strip Y −580..−300 (nothing with collision); door apron; the skiff's mooring |
| Crate contents (two tins, a rolled blanket) | placed by the POI script (presence targets) | NoCollision | — |
| Mara's pack | a canvas bag and a bedroll (two boxes, `MI_DC_PackCanvas`, `MI_DC_Blanket`), a presence target | NoCollision | — |
| Skiff cargo | a tarped bundle (box, `MI_DC_Tarp`) and `can_rusted` scaled as a water can, a presence target | NoCollision | — |

The Megascans sack, jerrycan, and tarped pallet in the library were rejected: 70 to 136 MB per mesh (high-poly scans), far over what a background prop may cost in the repo. `Tools\ImportPackAssets.ps1` now refuses such packages.

Budget: about 30 new actors was the plan. Built: about 40 small actors plus 8 presence rules (rules render nothing). Frame time is checked in WS-10.

## Audio

- Bulbs' hum: `ADCConditionalAudio`, `/Game/Audio/Ambience/S_DC_HumLiveWater` at 0.08 (half the live-water hum), `SA_DC_Hum`, conditions `[!wreck.power_cut]`. Existing CC0.
- The lantern: none. The skiff: none (a hull knock would be new sourcing; written down as a gap).

## Existing Systems Used

Interaction, inspectables with variants, `ADCLootContainer` and its persistence, `ADCLocationVolume` discovery, world flags and `WorldFlag` conditions, `ADCFlickerLight` with `ActiveConditions`, `ADCConditionalAudio`, `ADCFriendlyNPC` (Mara, unchanged), save/load (unchanged), and the Phase 5 capability `ADCConditionalPresence`.

## Missing Reusable Capability

Was: conditional presence (show, hide, or place an actor by conditions). Needed by: Mara's second placement, her pack, the crate lid's three placements, the crate contents, the lashing, the skiff group's two placements, the cargo, the lantern light, the card, and the cut stub. The existing grammar could change text, hazards, lights, and sounds, but not where or whether an actor is. **Built in WS-02** as `ADCConditionalPresence` with its own test before this POI. Nothing else is missing.

## Tests

- **Content validation:** `Content.Validate` unchanged; the POI's conditions are world flags only.
- **Interaction:** each inspectable reads its variant in each state; the tackle box gives its stacks.
- **Persistence:** save in the coil state and in the combat state, diverge, F9: placements and visibility restored at once; no second discovery banner; a hand-written version-5 save that predates the stage loads with the derived state and the stage undiscovered.
- **Alternate routes:** coil route and combat route both played through the real pickup, kill, and Mara's turn-ins; zero-investment build; the kill-then-coil order stays in the combat picture.
- **Integration:** Mara does not move while the player is talking to her; she moves once the player is away; the lake-side walk, the scavenger's patrol, and the hull inspectable are unobstructed.
- **Regression:** the 39 tests at WS-02 stay green; existing map tests unchanged.

## Review Views

| Id | Camera (x, y, z), pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `landing_far` | (780, 120, 175), −6 / −72 | the stage from the door | A timber landing someone uses: a lit lantern, bulbs, a skiff alongside, a half-packed crate. Reads before any text. |
| `landing_coil` | same | same, `shore.watch` at `done_coil` | Same camera: Mara on the stage beside a closed, lashed crate and her pack; the skiff loaded. Different at a glance. |
| `landing_combat` | same | same, `shore.watch` at `done_killed` | Same camera: no warm light, no skiff alongside (a shape far out), crate open with the lid on the boards. Reads as a place shut down, not a texture swap. |
| `landing_power_cut` | same | same, `wreck.power_cut` | The bulbs are dark; everything else as default. |
| `landing_close` | (1010, −790, 170), −22 / −40 | the crate at interaction range | Crate half packed, lid leaning, contents readable as supplies. |
| `landing_close_coil` | same | `done_coil` | Lid on, rope lashing, pack beside it. |
| `landing_close_combat` | same | `done_killed` | Empty crate, lid thrown down, card gone from the post. |
| `landing_adrift` | (1060, −1110, 175), −4 / −57 | the skiff, combat | The skiff far out in the channel, turned, a cut line on the cleat in the foreground. |
| `lookout_coil` | the existing `lookout` camera | `done_coil` | Mara's lookout empty: no Mara, no pack. |

Added to `Tools/Review/Lvl_Boathouse.json` in WS-07.

## Human Acceptance Checklist

The walkthrough is `WorldStatePhasePlan.txt` §7. In short:

1. Step out of the boathouse, look right: the stage, lit. Walk onto it (banner once). Inspect the crate, card, bulbs. *Does it read as a place someone is getting ready to use?*
2. Coil route, then walk back: Mara on the stage, crate lashed, skiff loaded, her lookout empty. *Did you notice before you were told?*
3. F5, diverge, F9: as saved, at once.
4. From the step-1 save, the kill route: lantern out, card gone, crate empty, skiff out in the channel. *Clearly different, and a consequence rather than a texture swap?*
5. Pull the Survey Launch leads: the bulbs go dark.
6. *Does it feel like a place someone made on purpose? Anything floating, fake, or in the way?*

## Open Creative Decisions

Each has the default the builder uses unless Anthony says otherwise.

- **The stage's place on the shore.** Default: in the shallows east of the door, south of the path. The pilot said "between the scavenger's camp and Mara's lookout", but there is no water there, and on the coil route the scavenger is alive, so Mara in his patrol square would read wrong.
- **Display name.** Default: "Landing Stage".
- **The card's text.** Default: *Don't tie up after dark unless the lamp is lit.*
- **Combat route, who stripped the landing.** Default: unstated.
- **Kill, then hand over the coil anyway.** Default: stays the combat picture (a drifted skiff does not come back).
- **Mara's lookout on the combat route.** Default: unchanged in the world (she and her pack stay); her existing greeting and the lookout crate already read the outcome. No new line.
- **Mara's lines.** One line changed (Gameplay Critic G-01): her coil payout, `reward_coil`, ended "I'm going to sit up with this coil tonight and listen", which contradicted her packing and waiting at the landing. It now ends "I've got packing to do." PROVISIONAL; it names no destination. Every other line is unchanged.
- **The bulbs' power.** Default: tied to the Survey Launch's battery (`wreck.power_cut`), unexplained.

## Pilot Record (`PRODUCTION_PILOT.md` §10)

Filled by the builder with facts only; the judgment of quality is the critics' and Anthony's.

| Measure | Value |
| --- | --- |
| Approved spec to playable candidate | same session, 2026-09-30 (spec `bfc9a71` → gameplay `53c9e36` → tests `5e44416` → presentation `2538f27` → review packet `937f86c`) |
| Candidate to accepted | critics and revision the same day (Gemini visual, Grok gameplay; revision `82b808a`..`1803d5f`); acceptance open |
| New C++ classes | 1 (`ADCConditionalPresence`, with its `FDCPresenceState`), built before the POI, plus one signal (`UDCWorldStateSubsystem::OnRestored`). Nothing POI-specific in C++. |
| Bespoke assets generated / Meshy credits | 0 / 0 |
| Tier A from the library | skiff (`Smugglers_cove` rowing boat), crate with lid (PolyHaven), lantern (PolyHaven) |
| Tier B reused vs added | reused: surface and flat materials, glow material, `ADCFlickerLight`, `ADCConditionalAudio`, rust paint; added: the timber deck (`SM_wooden_pier_planks`), 10 flat-colour instances |
| Tier C | 6 hand-placed pieces (`dress_landing.py`); a recipe would not have saved anything at this size |
| Shared files touched by the POI | `build_boathouse.py` (one hook), `Tools/Review/Lvl_Boathouse.json` (views), `DCBoathouseMapTest.cpp` (the art-layer audio count skips the stage's hum) |
| Pack migration | new generic `Tools\ImportPackAssets.ps1`; 21 MB committed; the Megascans props were rejected at 70 to 136 MB per mesh, and the tool now refuses packages like them |
| Tests added | 4 (`World.ConditionalPresence`, `Map.Boathouse.LandingStage`, `Map.Boathouse.LandingStageSaves`, `Map.Boathouse.LandingStagePlayerSave`); 38 → 42 |
| Defects found by the builder's own captures before the critics | 5: the moored skiff hidden behind the deck from the door; lantern and bulbs too small to read; the masted boat variant; flat colours rendering pale and glossy; open water showing through the plank gaps |
| Defects found by critics | Visual 8 (V-01..V-08): 7 fixed in full or in part (V-02 and V-06 partly, the rest of each parked); V-07 parked. Gameplay 4 (G-01..G-04): all accepted; G-02's placement parked. The builder had flagged 2 of these 12 as open questions (V-04's bulbs, V-06's flat pack and lashing) and caught none of the other 10, including all three visual blockers and the dialogue contradiction. The critic step paid for itself. |
| Defects found by Anthony | open |
| Would the second cell need new C++? | Not for presence or placement. The narrative drafts' next asks (a talkable hostile, map travel) are different capabilities. |
