# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- Local `main` = `origin/main` after the final push (see `git log -1`). Session started at local `822aa16`; `origin/main` was `7a3fc58`, merged as `7d2a3d9`.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers).

## Current Milestone / Task

**Presentation Pass (PP-00..PP-10): READY FOR ANTHONY ACCEPTANCE. Not accepted.** Work stopped here on purpose. Phase 5 has not been started.

## Completed Since Last Update

- PP-10: docs updated, clean rebuild (twice), 38/38 tests, final captures, Development Win64 cook, smoke launch of `Lvl_Boathouse`, frame times measured, `CLAUDE_SESSION_REPORT.md` replaced.
- Fixed a packaging-only compile error in the review capture code (`GetActorLabel` outside `#if WITH_EDITOR`).
- Added `Tools\Package.bat`.
- Wrote `Design/POIs/PRODUCTION_PILOT.md` (plan only).
- Earlier this session: PP-06 to PP-09 (see the session report), `Design/POIs/` templates.

## READY FOR ANTHONY TO CHECK

Launch `Tools\PlayTest.bat` (delete `Saved\SaveGames\DeadCurrent.sav` first for a clean run).

1. **Boathouse.** Where: you wake on the floor. What to do: take the pistol from the bench, fire once, dry-fire, take the ammo and dressing. Should happen: it reads as a cold steel shed; the shot and the dry click sound like a pistol; each pickup clicks. Ask: is the pistol the loudest thing? Do the click and the **Tab** switch flick suit the game?
2. **The door.** Step outside. Should happen: wind and water are already there, before anything glows. Ask: does the wind sit under everything? Does the lap ever startle you?
3. **Coil route.** Walk east past the ridge to the relay. Should happen: it hums, louder as you close in. Take the coil: the hum stops within about half a second. Bring it to Mara. Ask: does Mara read as a person watching the path (teal jacket)? Can you hear her lines over anything?
4. **Combat route.** New game (F10 / delete the save). Kill the scavenger. Should happen: he reads as a rust-brown jacketed figure, clearly not Mara; the hum stops when he dies.
5. **Survey Launch.** Walk west past the back of the boathouse. Should happen: the *Tern* from the beach reads as faded white paint over grime, not pink; the name board reads `T_RN`; the live water hums and now and then snaps; the water is dark and rippled. Pull the leads at the battery bank: one clunk, the hum and snaps stop, the glow goes, the water stays. Loot the locker and the tender. Ask: is the cyan glow too loud? Is the hull right?
6. **One RPG build** (**B** panel, Engineering). Should happen: the breaker panel's extra reading, then the chart from the tender and the sounder. The meshes changed; the words did not.
7. **Save and reload.** F5, quit, relaunch, F9. Should happen: build, cut power, quest as saved; the shore looks and sounds the same.
8. **Overall.** Ask: is the whole shore too blue and grey? Do you stop and look at things even when they don't glow?

## Decisions Needed From Anthony

- Accept the Presentation Pass, or list what to change.
- Grade: too blue or grey? Hull tint: `Tint`, `PaintCast`, `GrimeAmount` on `MI_DC_TernU1/U2`; grade in `build_lighting()`.
- Mara's face: keep the pack's single head as a placeholder, or swap later?
- Wind bed has very faint birds (your choice) versus the "No birds" window line.
- Approve, change, or drop the production pilot and its conditional presence capability (`Design/POIs/PRODUCTION_PILOT.md` §12).
- Chase the frame-time shortfall now or after acceptance?

## Known Issues

- **Frame time 18.4 ms average (about 54 FPS)** vs the 16.7 ms target, uniform across all views; 30% screen percentage did not change it, so it is CPU-side or a fixed cost. Not bisected. Suspects: two heavier animated characters, added actors, five audio components. A `stat unit` session would settle it.
- Basin water slab has a faint dotted edge. Cosmetic.
- Live-water glow is still the Phase 3 flat cyan sheet.
- Both characters share one head. One eye texture (`T_EyeMidPlaneDisplacement`) is still at source size (small).
- Breaker throw is a stand-in sound. Not verified by ear: balance, hum seams, UI cues.
- The packaged game's audio is untested (smoke launch has no audio device).
- Live-water hum is an `ADCConditionalAudio`, not code inside `ADCDamageVolume` as the plan worded it.
- Content rebuilds rewrite imported binaries differently each run; I discarded that churn rather than commit it.

## Future Tier-C Candidates (for the first biome recipe; not automated)

Also in `Design/content_production_strategy.md` §5.

- shore stones (waterline band, under the ridge)
- driftwood and dead branches (nine hand placements in `dress_shore.py`: the cleanest first recipe test)
- minor debris and camp scrap
- mud and wet-sand variation (one texture per band, so the beach reads flat)
- shoreline grass (none yet)
- generic maritime scrap (rope, floats, tires)
- an ambience zone owning the wind and lap beds

## Automated Status

- Build: `DeadCurrentEditor` builds; the Development Win64 game target also builds (packaging).
- Tests: **38 of 38** (31 editor, 7 map). Baseline 36.
- Rebuild: `Tools\RebuildContent.bat` clean, exit 0; art level keeps the sentinel and audio beds.
- Review captures: final run `Saved/Review/2026-09-29_1758`; no defaults, missing textures, warnings, or errors.
- Package: Development Win64 cook to `Saved\Packaged\Windows` succeeded; null-RHI smoke launch loaded `Lvl_Boathouse` and its art level.
- Meshy spend: 330 of 500 credits, ten props.

## Next Autonomous Task

None. Stop and wait for Anthony's acceptance decision and his answer on the production pilot. Do not start Phase 5.
