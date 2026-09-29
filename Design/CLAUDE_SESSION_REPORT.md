# Development Session Report

## Executive Summary

The Presentation Pass (PP-00 to PP-10) is implemented and **READY FOR ANTHONY ACCEPTANCE**. It is not accepted. This session finished PP-06 to PP-10: every clue prop and pickup now wears a Meshy mesh, the water is a dark rippled freshwater material, Mara and the scavenger wear the `Survival_Character` pack in different jackets on the existing animations, and the shore has sound (wind and lap beds, the relay and live-water hums, spark snaps, a breaker clunk, CC0 pistol shots, pickup and inventory cues). No quest, NPC, item, dialogue line, flag, or save field was added.

38 automated tests pass (36 baseline). `DeadCurrentEditor` builds. `Tools\RebuildContent.bat` is clean and leaves the art layer and its audio beds in place. A Development Win64 cook and a null-RHI smoke launch of `Lvl_Boathouse` succeeded. Average frame time in the review captures is **18.4 ms (about 54 FPS)**, short of the 16.7 ms target; see Known Issues.

The session also added the smallest production infrastructure the new strategy calls for (a POI spec template, an agent handoff template, ownership conventions) and a plan-only production pilot for Anthony to approve. Phase 5 was not started.

## Starting SHA

Local `main` `822aa16` (PP-06 greybox overrides cleared). `origin/main` was `7a3fc58` (the strategy doc); they had diverged by one commit each and were merged as `7d2a3d9`. Local HEAD was authoritative. Pushed as a fast-forward to `3dfe142` after checking the remote. The later doc and pilot commits are listed under Git.

## Presentation Pass Progress

| Task | State | Commit |
| --- | --- | --- |
| PP-00..PP-05 | done in earlier sessions | see git log |
| PP-06 Clue props | done. All ten Meshy props (330 of 500 credits) and the library meshes are on the existing actors. The *Tern* hull was salmon pink (warm photo diffuse plus rust grime); `M_DC_Wreck` now desaturates and casts it cool, so it reads as faded white paint over grime. | `90a6436` |
| PP-07 Water | done. `M_DC_Lake` on the open lake and the basin slab, same look so there is no seam. The slab moved down to sit 0.25 cm above the lake. `LiveWater` and sparks untouched. New assertions: the slab is still there after the leads are pulled and after an F9 load. | `59e9d83` |
| PP-08 Bodies | done. `SK_Survival_Character` for both, on `ABP_Unarmed` (the pack skeleton lists the mannequin as compatible). Mara has a teal jacket and light jeans; the scavenger a rust-brown jacket and charcoal jeans. Textures were cut from 773 MB to 50 MB by `Tools\ImportSurvivalCharacter.ps1` before entering the repo. | `873f15c`, `0f88ed8` |
| PP-09 Audio | done. See Audio. | `bf9e8be` |
| PP-10 Stabilize | done: docs, rebuild, suite, captures, cook, smoke, frame times. Also fixed a game-target compile error in the review capture code (`GetActorLabel` outside `#if WITH_EDITOR`) that only the packaging build hits. | this commit |

Deviations from `PresentationPassPlan.txt`, all recorded in its status line: the live-water hum is an `ADCConditionalAudio` on the same `!wreck.power_cut` condition, not code inside `ADCDamageVolume`; sparks and the breaker throw have sound too (their sources were approved with the pass); `Tools\Package.bat` and `Tools\ImportSurvivalCharacter.ps1` are new tools.

## Production Strategy Infrastructure Added

- `Design/POIs/TEMPLATE.md`: the POI specification (identity, player promise, discovery, spatial role, gameplay, environmental story, state, tiered asset plan, audio, systems used, missing capability, tests, review views, acceptance, open decisions), with a definition of "ready to build".
- `Design/POIs/AGENT_HANDOFF_TEMPLATE.md`: role, scope, owned files, forbidden files, inputs, deliverables, tests, review artifacts, dependencies, stop conditions, handoff notes.
- `Design/POIs/README.md`: twelve short ownership rules (one spec per POI, isolated content, explicit ownership, shared files are integration-sensitive, one editor per generated file, immutable ids, provisional lore, no global system changes from a POI builder, builders do not approve their own work).
- `Design/content_production_strategy.md` §5: the first biome-recipe candidates the pass observed. Nothing was automated.
- `CLAUDE.md` now indexes the strategy, the POI docs, and the checklist.
- No PCG, no orchestration software, no new gameplay system.

## Assets Added / Changed

- 43 `Survival_Character` textures at 1K, the mesh, skeleton, physics asset, and 11 material instances under `/Game/Survival_Character` (50 MB). Four costume tint instances in `import_art.py`.
- `M_DC_Lake`, `MI_DC_OpenLake`, `MI_DC_Water` (normal map is engine example content `water_n`).
- `M_DC_Wreck` gained a desaturate and cool cast step.
- 13 sounds and three attenuation assets under `/Game/Audio`; two UI cues migrated to `/Game/Interface_And_Item_Sounds`.
- The ten Meshy meshes from PP-06 and their materials.
- Provenance for all of it is in `Design/art_pipeline.md`.

## Meshy Credits Used

330 of the 500-credit budget, across ten props: `relay_housing` 50 (including one rejected preview), `depth_sounder` 40, and eight others at 30 each. 170 credits unspent. No generation happened this session; the props were generated earlier and are imported and reviewed here.

## Visual Review Findings

Captures: `Saved/Review/2026-09-29_1758` (final; contact sheet and 20 views). Findings against the fixed expectations:

- **Passing:** clue props read as objects, not grey boxes (breaker panel, battery bank, beacon, sounder, fish, chalk board, name board `T_RN`). No default or grid materials, no missing textures, no warnings or errors in any view. Mara and the scavenger stand in animated poses (not T-pose) and read as two different people. The water reads as water, and the live-water sheet still hides with the power.
- **Fixed this session:** salmon-pink *Tern* hull; basin slab as a lighter rectangle with a black edge (the edge is now a faint dotted line); both characters near-black under the pack's dark textures (tints raised above 1.0).
- **Still weak, for Anthony's eye:** the overall palette is very blue-grey. Sand, boathouse steel, the hull, and the sky all sit in one cool band, so the only strongly saturated things are the cyan live-water sheet and the rust wood. The plan asked whether the grade is too blue: my read is yes, a little. The controls are the `Tint` parameters on the surface instances and `MI_DC_TernU1/U2`, and the grade in `build_lighting()`.
- The beach still reads flat from a distance (one texture per band, no stones or grass). Those are Tier C candidates.
- The live-water glow is still the Phase 3 flat cyan sheet. I did not change it.
- Mara and the scavenger have the same head.

## Audio

All sources are CC0 and were approved by Anthony (files under `C:\FO5_AssetLibrary\Audio`, provenance in `art_pipeline.md`).

- Shore bed: wind (0.07) and water lap (0.46), two 2D beds in the art level (`dress_audio.py`, tag `ShoreAudio`).
- Relay hum (0.06, positional): plays while the player does not hold `radio_coil`, `shore.relay_recovered` is unset, and `boat.scavenger` is alive. Inspecting the rig does not stop it.
- Live-water hum (0.02, positional): plays while `!wreck.power_cut`. Six spark snaps (0.22, 30% chance per flash). One breaker clunk (0.56) on the rising edge of `wreck.power_cut`; silent on a load that already has the flag.
- Pistol: CC0 .38 shot and a striker dry-fire. Pickup click and inventory switch flick from the `Interface_And_Item_Sounds` pack.
- `ADCConditionalAudio` sets no flag and saves nothing (test: `DeadCurrent.Presentation.ConditionalAudio`, plus relay-hum assertions in the coil route map test and an art-level ambience assertion).
- **Not verifiable by me:** level balance, whether the water lap startles, whether the hum seams are audible, and whether the two UI cues sound right (chosen by file size). The wind bed keeps its faint birds by Anthony's choice. The breaker throw is a stand-in.

## Automated Tests

**38 of 38 pass** (31 editor, 7 map) via `Tools\RunTests.bat -build`. Baseline 36. Added: `DeadCurrent.Presentation.ConditionalAudio`; the art-layer test now also checks the two shore beds and five audio actors; the survey-launch test checks the water slab exists, survives the power cut, and survives a load; the coil-route test checks the relay hum starts audible and stops when the coil is taken.

## Build / Rebuild

`DeadCurrentEditor` builds with the new C++ (`ADCConditionalAudio`, `DCAudioCues`, flicker-light flash sounds, pickup, container, and inventory hooks). `Tools\RebuildContent.bat` (now `import_art, import_audio, create_items, create_quest, create_dialogue, build_test_gym, build_boathouse`) exits 0, twice in a row, and the art level keeps the sentinel and audio beds.

## Package / Smoke Test

`Tools\Package.bat` (new): Development Win64 `BuildCookRun` to `Saved\Packaged\Windows` (about 1.2 GB with engine files), then a null-RHI launch of `Lvl_Boathouse`. Both succeeded; the log shows the map and `Lvl_Boathouse_Art` loading. The first attempt failed on a compile error in `DCReviewCaptureTest.cpp` that only the game target hits (`GetActorLabel`); fixed with the file's existing label helper. Null RHI logs six `invalid ShaderMap` errors, which are expected with no renderer. `DirectoriesToAlwaysCook` now includes `/Game/Audio` and `/Game/Interface_And_Item_Sounds` because the UI cues load by path. Not verified: that the packaged game actually makes each sound (the smoke launch has no audio device).

## READY FOR ANTHONY TO TEST

Launch with `Tools\PlayTest.bat` (delete `Saved\SaveGames\DeadCurrent.sav` first for a clean run). The walkthrough is `PresentationPassPlan.txt` §9. In short:

1. Boathouse: it should read as a cold shed. Take the pistol, fire once, dry-fire. Both sound like a pistol. Pickups click; **Tab** flicks a switch.
2. Step out: wind and water are already there. Does the wind sit under everything, and does the lap startle you?
3. Shore Watch coil route: near the relay it hums. Take the coil: the hum stops. Bring it to Mara. Does Mara read as a person watching the path?
4. New game. Combat route: kill the scavenger; the hum stops.
5. Survey Launch: the *Tern* from the beach (faded white, not pink?), the name board, the log, the life jackets, the live water's hum and snaps. Pull the leads: one clunk, the hum and snaps stop, the dark rippled water stays. Loot the locker and the tender.
6. One RPG build (Engineering): breaker reading, then the chart and the sounder. The meshes changed; the words did not.
7. F5, quit, relaunch, F9: build, cut power, quest as saved; the shore looks and sounds the same.

Judgement calls only you can make: is the grade too blue or grey; do the two characters read apart; is any sound too loud or wrong; do the click and the switch flick suit the game.

## Production Strategy Pilot Status

`Design/POIs/PRODUCTION_PILOT.md` is a **plan only** and is not approved. It proposes one small cell, the Landing Stage (`shore.landing_stage`, PROVISIONAL name), whose look and Mara's placement change with the Shore Watch route and the wreck's power cut, with the tier split, role ownership, builder-critic-reviser-verifier workflow, tests, and review views. It identifies one likely missing reusable capability (a generic conditional presence rule) and a cosmetic-only fallback. Nothing was built. Phase 5 was not started.

## Known Issues

- **Frame time 18.4 ms average (about 54 FPS)** against a 16.7 ms target, at the PlayTest settings, uniformly across all 20 views including the empty boathouse interior. The greybox baseline sat at 16.7 ms because it hit the cap, so the true baseline is unknown. Dropping the screen percentage from 70% to 30% did not change it (18.7 ms), so the cost is not pixel work. It is CPU-side or a fixed per-frame cost. Likely contributors, **not bisected**: two heavier animated characters (skeletal meshes with many material slots), the added actors and meshes in the persistent map and art level, and five audio components. A `stat unit` session would settle it. A failed attempt to use a `ShowFlag` to bisect made the capture crawl; that is a harness quirk, not a finding.
- The basin water slab has a faint dotted edge (near-coplanar with the lake). Cosmetic.
- The live-water glow is the Phase 3 flat cyan sheet. It is the loudest colour on the shore.
- Both characters share one head. `T_EyeMidPlaneDisplacement` would not export and stays at source size (small).
- Runtime navigation is rebuilt at launch (pre-existing warning).
- Untracked and left alone: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py`.
- A content rebuild rewrites imported binaries byte-for-byte differently each run. I discarded that churn instead of committing it, so the committed assets are from the last deliberate change.

## Decisions Needed

- Accept the Presentation Pass, or list what to change.
- Is the grade too blue or grey, and is the *Tern*'s pale hull right? (`Tint`, `PaintCast`, `GrimeAmount`.)
- Mara's face: keep the pack's single head as a placeholder, or swap later?
- The wind bed's faint birds (your earlier choice) against the "No birds" window line.
- Approve, change, or drop the production pilot and its conditional presence capability (`PRODUCTION_PILOT.md` §12).
- Chase the frame-time shortfall now, or after acceptance?

## Recommended Next Step

Human acceptance of the Presentation Pass, using the walkthrough above, and review of `Design/POIs/PRODUCTION_PILOT.md`. Do not begin Phase 5 until you choose it.
