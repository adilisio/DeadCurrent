# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- Local `main` is at `bf9e8be` (PP-09). Session started at local `822aa16`; `origin/main` was `7a3fc58`, merged as `7d2a3d9`. **Nothing pushed yet.**
- Uncommitted on purpose: `Content/Variant_Shooter/` and `Tools/EditorScripts/inspect_assets.py` (earlier leftovers), this file.

## Current Milestone / Task

**Presentation Pass — PP-06 to PP-09 are committed. PP-10 (stabilize: docs, cook, smoke launch, performance, session report) is next.** Not yet marked ready for acceptance.

## Completed Since Last Update

- PP-06 `90a6436`: every clue prop and pickup wears a Meshy mesh (330 of 500 credits). The *Tern* hull was salmon pink (photo diffuse plus rust grime); it now casts cool, faded white-grey.
- PP-07 `59e9d83`: the lake and the basin slab share a dark rippled freshwater material (`M_DC_Lake`). New map assertions: the slab is still there after the leads are pulled and after a load.
- PP-08 `873f15c`, `0f88ed8`: Mara (teal jacket) and the scavenger (rust-brown jacket) use the `Survival_Character` pack on the mannequin animations. Textures were cut from 770 MB to 50 MB before entering the repo.
- PP-09 `bf9e8be`: shore wind and lap beds (art level), relay hum, live-water hum, spark snaps, breaker clunk, pistol shot and dry-fire (CC0), pickup and inventory cues. `ADCConditionalAudio` follows the rule language and sets nothing.
- `Design/POIs/` (`TEMPLATE.md`, `AGENT_HANDOFF_TEMPLATE.md`, `README.md`) added: `547dae7`.

## READY FOR ANTHONY TO CHECK

Nothing to check yet. The Presentation Pass is not ready for acceptance until PP-10 finishes. When it is, the walkthrough lives in `PresentationPassPlan.txt` §9 and the session report.

Things only your ears and eyes can judge, collected as they land:

1. **Audio balance** (`Tools\PlayTest.bat`). Start a new game. Walk out the door, then west to the wreck. Does the wind bed sit under everything? Does the water lap startle you? Can you hear Mara's lines over the relay hum near the camp? Is the pistol still the loudest thing?
2. **Relay hum.** Near the relay it should hum. Take the coil or kill the scavenger: it should stop within about half a second.
3. **Live water.** Near the water at the wreck: a low hum and now and then a spark snap. Pull the leads: one breaker clunk, the hum and snaps stop, the water stays (dark and rippled).
4. **UI cues.** A short click when you take anything. A switch flick when you open the inventory (Tab). These are the pack's `Click_03` and `Flick_Switch_01`, chosen by file size, not by ear. Tell me if either sounds wrong.
5. **Mara and the scavenger.** Do they read as two different people at a distance? Both share the pack's single male head (see decisions).
6. **The *Tern*.** From the beach: does the hull read as faded white paint over grime, not pink or blown out?

## Decisions Needed From Anthony

- **Mara's face.** The pack ships one head (short dark hair, male-presenting). I did not decide who Mara is. Options: keep it as a placeholder, or swap to another head later. Default: keep.
- **Cold grade and hull tint** (open question from the plan): the hull is now pale blue-white. `Tint`, `PaintCast`, and `GrimeAmount` on `MI_DC_TernU1/U2` are the controls.
- **Wind bed has very faint birds.** You chose to keep it; the boathouse window line says "No birds". No action unless you change your mind.

## Known Issues

- The basin water slab shows a faint dotted line at its south edge (near-coplanar with the lake). Cosmetic.
- The live-water glow is still the Phase 3 flat cyan sheet; only the permanent water changed.
- `T_EyeMidPlaneDisplacement` (Survival_Character eye texture) would not export and stays at source size (small).
- Both characters have the same face. See decisions.
- Live-water hum is an `ADCConditionalAudio` placed by `build_boathouse.py`, not code inside `ADCDamageVolume` as `PresentationPassPlan.txt` §4 words it. Same rule (`!wreck.power_cut`), one fewer C++ change.

## Future Tier-C Candidates (for the first biome recipe; not automated)

Noted while dressing the shore. Nothing here is built as a system.

- shore stones (black rock, waterline band)
- driftwood, small and large (`dead_quiver_branch_01`, `dead_tree_trunk`, `tree_stump_01` are already placed by hand in `dress_shore.py`: nine placements, a good first recipe test)
- minor debris and camp scrap (crates, cans)
- mud and wet-sand variation (currently one texture per band)
- shoreline grass (none yet; the beach is bare)
- generic maritime scrap (rope, floats, tires)
- ambient audio zones: the wind and lap beds are two 2D actors; a biome recipe would own these

## Automated Status

- Build: `DeadCurrentEditor` builds (last run with PP-09).
- Tests: **38 of 38 pass** (31 editor, 7 map). Baseline was 36. New: `DeadCurrent.Presentation.ConditionalAudio`, `Map.Boathouse.ArtLayer` (now also the audio actors), plus water-slab, relay-hum assertions in existing map tests.
- Rebuild (`Tools\RebuildContent.bat`): clean, exit 0. Art level keeps the sentinel and both audio beds after a rebuild.
- Review captures: last run `Saved/Review/2026-09-29_1732` (PP-08). Audio has no visual output; no recapture needed for PP-09.
- Package: not attempted yet.
- Meshy spend: **330 of 500 credits** across ten props.

## Next Autonomous Task

PP-10: update docs (`technical_architecture.md`, `game_design.md`, `art_pipeline.md`, `CLAUDE.md`, `PresentationPassPlan.txt` status), full rebuild, full suite, final captures and frame times, Development Win64 cook and smoke launch, replace `CLAUDE_SESSION_REPORT.md`, mark **READY FOR ANTHONY ACCEPTANCE**, push fast-forward after checking origin. Then write `Design/POIs/PRODUCTION_PILOT.md` (plan only). Do not start Phase 5.
