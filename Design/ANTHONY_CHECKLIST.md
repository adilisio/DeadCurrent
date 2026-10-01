# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- `main` carries **VS-08, the exterior greybox** (the VS-08 commit; its hash is recorded in the commit after it), on VS-07 (`7126435`) and VS-06 (`ad50261`). See the Checkpoint A section and the VS-08 Record.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone

**PHASE 6: STARTED.** **Stopped at Checkpoint A** (the island's shape). Your playtest and decision come next; nothing after it has been started.

| Task | State |
| --- | --- |
| VS-00 Plan | complete, approved |
| **VS-01 Baseline** | **COMPLETE** (`00d8d22`) |
| **VS-02 Production foundations** | **COMPLETE** (`e833811`) |
| Kit research (Gemini) | **DONE and merged** (input, with the Integrator's corrections) |
| **VS-03 Cell portal + scene cut** | **COMPLETE** (merged `2c1d2b6`) |
| **VS-04 Map architecture proof** | **COMPLETE** (see the VS-04 Record) |
| **VS-05 Settlement kit (WP-KIT)** | **COMPLETE** (merge `54aa7b5`; see the VS-05 Record) |
| **VS-06 Shoreline recipe (WP-BIOME)** | **COMPLETE** (see the VS-06 Record) |
| **VS-07 Slice specs and the id ledger** | **COMPLETE** (docs only; see the VS-07 Record) |
| **VS-08 Exterior greybox** | **COMPLETE** (see the VS-08 Record) |
| **Checkpoint A** | **waiting for you** (see below) |

Plan: `VerticalSlicePhasePlan.txt`. Phase 5 (World State) is accepted and unchanged.

## CHECKPOINT A: the Island's Shape (stopped here for you)

**VS-08 is complete and I have stopped.** Nothing after Checkpoint A has been started: no VS-09, no narrative, no cell content. The island's layout and the cell footprints freeze when you accept it.

**Play it:** `Tools\PlayTest.bat Lvl_PointeSombre` (close the editor first). You start on the *Ida*'s deck. The wheelhouse door ("Tell Varga about the light") fades you to the quay. Walk from there (plan §16: 15–20 min):
- quay → tower door: the trail east along the harbor, then up the tower rock. Inside, "Climb the stair (greybox)" takes you to the lamp room; walk out onto the gallery.
- tower → cable hut: the cut stair down the north side. A spur goes to the iron door at the rock's foot.
- settlement → cable hut: the north path over the ridge's end
- quay → west headland: past the Pruitts' shed to the post and its hide
- headland → *Ashland Grey*: down onto the reef causeway, then the ramp onto the stern deck
- Stub doors are labelled "(greybox)" and are all open: the tower stair, the hatch to the vault, the iron door, the conduit, and the loft stair from the store. They only prove that each cell can be reached; the real doors and their locks come with each cell.

**What only you can judge** (the tests and the critic cannot):
1. Does the island feel like a coherent place rather than a level?
2. Does it feel larger than its physical footprint (about 435 × 245 m)?
3. Are the route lengths right? (The timings are below.) Is any stretch empty just because it is long? The critic thinks the quay → headland walk is.
4. Is the lighthouse a useful dominant landmark without making the island feel miniature?
5. Does the settlement feel organically accumulated rather than grid-built? Its buildings stand 20–35 m apart, at the cell spec's positions. **The critic says too spread out. I left it for you, because it moves the spec's positions.**
6. Do the harbor and the exposed shore feel geographically different (the sheltered cobble bight vs the north and west scarps with talus)?
7. Is the cable hut isolated enough? From the settlement you see only the mast's top over the ridge; the hut appears at the crest.
8. Does the west headland / *Ashland Grey* reveal work? From the quay the *Grey* is a small, far shape (plan §5.5 wants it seen from there); it looms only from the headland crest. Gemini's brief preferred it hidden from the quay. Which do you want?
9. Does any place feel theme-park adjacent?
10. Does anything look obviously procedural or generic? (The turf texture visibly tiles from height. That is art, later.)
11. Do collision and visible terrain ever disagree? (Trails are painted per 2 m triangle, so their edges are saw-toothed; the collision is the same mesh you see.)
12. Can you reach anything you should not (over the fence, off the map, into a void)?

## Inspect When You Return

Things only you can judge, kept current as work lands (newest first):

0. **Checkpoint A, the island's shape** (VS-08): the section above. `Tools\PlayTest.bat Lvl_PointeSombre`.
1. **VS-05, the kit gym — accepted.** Anthony playtested `Lvl_KitGym` (2026-10-01). The store, lean-to, and cottage read as distinct structures; the shared kit looks convincing and not obviously repetitive; the visual quality is good enough to accept the kit concept. The one defect, a crate floating in the lean-to, is fixed (it now sits on the crate below). Recapture: `Saved/Review/2026-10-01_1033_Lvl_KitGym/contact_sheet.png`. The map is still development-only and is not part of the slice.
2. **VS-04, the new map.** `Tools\PlayTest.bat Lvl_PointeSombre`.
   - You start on a stub of the *Ida*'s deck, offshore in the storm.
   - The wheelhouse door ("Tell Varga about the light") fades you to the quay and sets `sombre.reef_struck`.
   - The island is a first terrain frame (VS-08 shapes it). The grey door marked TEST west of the quay is the architecture fixture, not content: it leads to a test room 2.5 km away and back.
   - What to look at: does the portal's fade feel right (0.35 s out, 0.15 s hold, 0.35 s in)? Is the storm look readable?
   - Review frames: `Saved/Review/2026-10-01_0910_Lvl_PointeSombre/contact_sheet.png` (the three atmospheres from the quay, the deck, the island, the test room).
   - Rendered portal proof: `Saved/Screenshots/WindowsEditor/SombreArch_*.png`.
3. **One plan clarification I made in VS-04.** The atmosphere looks are switched by `sombre.hale_arrived` and `sombre.meeting_done`, with the storm as the default (no flag), not by `sombre.storm`. The slice doc says "`sombre.storm` is set on arrival", but nothing in the slice sets it at a new game yet. The dusk storm as the default look keeps a new game correct whatever VS-10 decides. `sombre.storm` still drives hazards and lightning content later.

## VS-01 Record: the Phase 6 Starting Point

- **Tree:** `main` = `origin/main` after the planning push. No tracked file was changed by the verification runs.
- **Tests:** **45 of 45** (33 editor, 12 map), 0 failures, on the approved-plan commit (`Tools\RunTests.bat -build`).
- **Package:** `Tools\Package.bat` on the clean committed tree: the Development Win64 cook succeeded to `Saved\Packaged\Windows`, and the null-RHI smoke launch loaded `Lvl_Boathouse` and evaluated all 8 presence rules.
- **Frame-time reference (same session, this machine):** two back-to-back `Tools\ReviewCapture.bat` runs on the unmodified tooling.
  - `Saved/Review/2026-09-30_1924` is **the reference** (nothing else running). `Saved/Review/2026-09-30_1920` is a noise check (a disk-heavy search ran during it).
  - 36 of 36 views were captured in both, with no default or grid material, missing texture, or error.
  - Average view time **35.67 ms** (1924) and 35.50 ms (1920), so run-to-run noise is about 0.2 ms. Route frames (17): 23.6 and 22.2 ms.
  - This machine is slower today than on the Phase 5 run days. Only same-session A/B comparisons count (plan §20): every Phase 6 gate is measured against a capture taken in the same session as the change it judges. This folder is only the start-of-phase record.
- **Stale status:** `CLAUDE.md`, `Design/game_design.md`, and `Design/technical_architecture.md` now say Phase 6 has started. The Phase 5 session report is left as history (its "not pushed" line carries a dated correction from VS-00).

## VS-02 Record: Production Foundations (the factory is safe for several builders)

- **Content specs are one file per owner.** `create_items.py`, `create_quest.py`, and `create_dialogue.py` now hold no content; they load every file in `Tools/ContentSpecs/{items,quests,dialogue}/` (`content_specs.py`; contract in `Tools/ContentSpecs/README.md`). The accepted Shore content moved unchanged into `items/shore.py`, `quests/shore_watch.py`, and `dialogue/mara_intro.py`.
  - **Proof it is behavior-preserving:** `Tools\DumpContent.bat` (new, read-only) dumps every generated asset's properties. The dump before the move (from the committed assets) and after regenerating through the new loader are **byte-identical**, and regeneration left every `.uasset` byte-identical (no binary churn in `git status`).
  - **Isolation probes:** a second spec file defining an existing asset fails loudly and names both files; a brand-new dialogue file in its own file generated a new asset, resolved an item defined in another folder, and left Mara's dialogue identical (then removed). A future agent adds `dialogue/sombre_varga.py` without touching Mara's.
- **Multi-map tooling.** One registry, `Tools/Maps.bat`, feeds every tool. Adding Pointe Sombre needs no new scripts.
  - `Tools\RunTests.bat [-build] [Map.<Group>[.<Test>]]`: each map's tests run on that map (`Map.Sombre...` on `Lvl_PointeSombre`); no argument runs the editor suite, then every production map that exists. Logs: `RunTests_Map.log` for `Lvl_Boathouse`, `RunTests_Map_<Group>.log` otherwise.
  - `Tools\ReviewCapture.bat [map]`: the test reads `-ReviewMap=` (no map is hard-coded in C++). A map's views are `Tools/Review/<map>.json` plus `Tools/Review/<map>/*.json` (one file per cell; ids unique, exactly one route, optional `art_sentinels`). The manifest now records the map, the view files, and per-view cost numbers: visible primitive components, material slots, instances, and the RHI's peak draw calls and primitives.
  - `Tools\Package.bat`: cooks the registered production maps explicitly (`-map=`) and smoke-loads each (`PackageSmoke.log` for `Lvl_Boathouse`, `PackageSmoke_<map>.log` otherwise). `Tools\PlayTest.bat [map]`. `Tools\RebuildContent.bat` accepts scripts in subfolders (`kit\build_kit_gym`).
  - **Not created:** `Lvl_PointeSombre` (the registry knows it; every tool says "registered but not built yet" until VS-04 builds it). `Lvl_KitGym` is registered as a *development* map for the kit package.
  - **Checked:** a temporary placeholder map file (removed) showed the list, the `-map=` join, and the test routing/log name for a second map all work.
- **Shared map-test helpers.** `Core/DCMapTestHelpers.h` (namespace `DCMapTest`) holds the generic harness extracted from `DCBoathouseMapTest.cpp`: find by persistent id or display name, talk and pick a reply, teleport, F5/F9 with a scratch slot, wait for a condition (new), and more. `DCBoathouseMapTest.cpp` uses it; all 12 of its tests pass with unchanged behavior. `DeadCurrent.Map.Boathouse.TestHelpers` is a new infrastructure test and the smallest worked example of a map test. (`DCLandingStageMapTest.cpp` keeps its own copies on purpose: an accepted file, left unchanged.)
- **Docs:** `Design/POIs/README.md` (how ownership works, branches and worktrees, what is Integrator-owned) and `Design/technical_architecture.md` (helpers, tooling, content specs) describe only what now exists.
- **Verification (all after the last code change):**
  - `Tools\RunTests.bat -build`: **46 of 46** (33 editor, 13 map), 0 failures (the 45 accepted tests plus `TestHelpers`).
  - `DeadCurrent.Content.Validate` is part of that run (green).
  - `Tools\ReviewCapture.bat Lvl_Boathouse` through the new parameterised path: the same 36 views in the same order, 17 route frames, every view captured, no default material, missing texture, or error. Average view time **33.6 ms vs the 35.7 ms same-session reference**: no regression. Draw-call peaks per view: 67 to 487. (`Saved/Review/2026-09-30_1939`; reference `..._1924`.)
  - `Tools\Package.bat` with the new explicit `-map=`: cooked, packaged, `Lvl_Boathouse` smoke-loaded, all 8 presence rules evaluated, 0 errors.
- **Problems found and fixed on the way (recorded, not hidden):** a batch `shift` had broken `%~dp0` in `RunTests.bat` (caught by an error-path test before any run); `PlayTest.bat` originally called the map resolver inside a parenthesised block, so cmd expanded its variables too early and **launched the game with no map while I was testing its error paths**. I killed those two stray game windows, rewrote that section with `goto` labels, and re-tested both the error paths (no launch) and the happy paths (map passed; no-argument form unchanged).

## VS-03 Record: the Cell Portal (2026-10-01)

You asked Claude to build this package instead of launching Grok. It was built exactly to its handoff (`Design/POIs/handoffs/sombre_WP-SYS-PORTAL.md`), on its own branch and worktree, then merged by the Integrator.

- **What exists now:** `ADCCellPortal` (`World/`): a door, hatch, or stair that moves the player within the same map.
  - **Variants:** ordered, first match wins, like inspect variants. Each has conditions, a verb, and consequences.
  - **Locked:** when no variant passes it shows `LockedText` and nothing else happens. This is the slice's locked access, with no separate locked-door class.
  - **Using it:** the screen fades out with input locked; at black the portal applies the variant's consequences, moves the player to `Destination` at rest, facing its yaw, and signals a **scene cut**; then it fades back in.
  - **Instant mode:** all fade durations zero, for tests and tools.
  - **Saves nothing:** no persistent id, no save field, no `SaveVersion` change.
- **Scene cut:** `UDCWorldStateSubsystem::OnSceneCut` / `NotifySceneCut()`. Conditional presence now snaps on it as well as on a restore. That is what lets the net loft fill while the player climbs the stair. A scene cut is deliberately *not* a restore; later work (the tower's one-time pattern) depends on the difference.
- **Test:** `DeadCurrent.World.CellPortal`. It covers:
  - locked text and prompts
  - first-match variants
  - consequences once per use
  - arrival at rest and facing
  - the scene cut snapping a deferred presence change, while a plain flag change still defers
  - the scene cut and the restore never standing in for each other
  - the timed fade
  - a missing destination is refused
  - it saves nothing
- **Verification on `main` after the merge:**
  - `Tools\RunTests.bat -build`: **47 of 47** (34 editor, 13 map).
  - `Tools\Package.bat`: cooked with 0 errors and 0 warnings; the smoke launch loaded `Lvl_Boathouse` with all 8 presence rules.
  - No accepted behavior changed. The only edit to an existing class is presence binding one more signal to its existing `Snap`.
- **Not yet seen in a rendered game:** the camera fade and the input lock are first exercised for real by VS-04's map test, `Map.Sombre.Architecture`.
- **One engine detail found while testing (recorded, harmless):** a timer set during a frame starts at the next tick, so a timed fade-out lasts one frame longer than set.
- **Decisions I made inside the contract** (listed in the handoff notes, all easy to change):
  - the default verbs "Go" and "Try", and the locked line "It won't open."
  - fades of 0.35 / 0.15 / 0.35 s
  - a `TryUse` result enum, so tests can tell locked from passed without a HUD
  - a portal with no destination refuses and logs an error

## VS-04 Record: the Map Architecture Proof (2026-10-01)

**What exists now:** `Lvl_PointeSombre`, generated by `Tools/EditorScripts/build_pointe_sombre.py` (part of a full `Tools\RebuildContent.bat` now). Every detail is in `Design/technical_architecture.md`, "Pointe Sombre: map architecture".
- **The island as data:** `Tools/PointeSombre/island.json` holds the coast, hills, pads, the reef causeway, and the walkable outline. `pointe_sombre/island.py` reads it with no engine, so the route timer and the biome tool use the same island.
- **Core script and hooks:** terrain, water, walls along the walkable outline, nav bounds, the three atmospheres, the exterior wind, anchors, the player start, then one `build(tk)` per cell script (`pointe_sombre/<cell>.py`). Cells get a shared toolkit and meet only at named anchors.
- **Terrain method, chosen by a spike:** a generated mesh from the heightfield.
  - A Landscape cannot be created headless: the engine asserts in the rebuild commandlet.
  - The library's rock and cliff scans are 12–160 MB Nanite meshes, over our 12 MB ceiling.
  - The generated mesh regenerates in 25 s (3 s when unchanged), and traces meet its surface within 0.1 cm.
- **Interior cells** sit 2.5 km east and 400 m up, enclosed, on lighting channel 1, each with its own grade. The fixture room proves it.
- **Atmosphere: presence is enough.** No `ADCConditionalAtmosphere` was needed. Three looks (dusk storm, calm evening, night storm), each a presence rule that hides and parks its sun, fill, sky, fog, and a bounded post-process volume. Lightning is a flicker light.
- **Respawn: the existing code is enough.** A presence rule moves the player start: deck, then the quay after the reef strike, then the tower base once the vault opens.
- **Crossing stub:** the *Ida*'s deck with the wheelhouse door to the quay.

**The first rendered run of the VS-03 portal** (a real window, on the real map):
- The screen fades to black and the player arrives in the test room facing the marker.
- A waiting presence change snaps on the scene cut.
- F5 inside the room, then F9, puts you back inside it.
- The locked back door shows its text, and the way back out works.

**Found and fixed on the way** (recorded, not hidden):
- **Portal (generic, minimal):** a second portal could start mid-fade. The way back out stands at the arrival point, so the return could fire during the incoming transition. Now every portal refuses while any portal's transition runs. Regression case added to `World.CellPortal`, and the in-map test presses E, jumps, and tries the way back mid-transition.
- **`RebuildContent.bat`:** a Python syntax error reported success, and a one-script run always exited 0 (`%errorlevel%` inside a parenthesised block). Both fixed and proved.
- **`RunTests.bat` / `Package.bat`** (Codex's preflight): stale logs could pass for new runs, and a missing production map was silently skipped. Logs are now deleted before each launch, a missing log fails, and a missing registered map fails (proved by moving the map aside).
- **Terrain:** tiles built through a Python mesh description rendered black (no usable normals). They are now written as OBJ with the heightfield's normals and imported. The axis convention and the winding are recorded.
- **Interior ceilings:** the triplanar surface lights a downward face as if it faced up, so ceilings use a flat material.

**Verification after the last change:**
- `Tools\RunTests.bat -build`: **51 of 51** (34 editor, 13 Boathouse map, 4 Sombre map: `Architecture`, `CrossMapLoad`, `Respawn`, `Atmosphere`).
- The rendered `Architecture` run passes too.
- `Tools\Package.bat`: Development Win64 cook of **both maps**, 0 errors and 0 warnings. Both smoke-load (Pointe Sombre in 0.08 s, all presence rules evaluated).
- **Perf gate 0** (same session): Pointe Sombre views all at the 16.7 ms cap (60 FPS), route 21.3 ms. `Lvl_Boathouse` 36.6 ms the same session. Captures `Saved/Review/2026-10-01_0910_Lvl_PointeSombre` and `..._0909`. Watch item: draw-call peaks of 512–662 with few components visible (sky capture, parked sets).

## VS-05 Record: the Settlement Kit (2026-10-01)

**COMPLETE.** Merged `origin/vs/kit` into `main` as `54aa7b5` (no force-push, `vs/kit` left intact). The generated gym map is `6776cda`. No Meshy credits. No bespoke building mesh.

**`ff611d1` verdict: kept.** It is already the parent of the kit branch, and it changes only two shared files:
- `import_art.py` sets `used_with_instanced_static_meshes` on `M_DC_Surface`. Required: without it, instanced kit walls render as the engine checker. Generic: it is the shared triplanar master, and the shoreline recipe will instance it too. Non-instanced meshes keep the same shading.
- `RebuildContent.bat` deletes the script log before each run and fails if the engine writes none. Generic fail-closed tooling, the same idea as the VS-04 test and package logs.

The regenerated `M_DC_Surface.uasset` is in the kit commit, not in `ff611d1`. `ensure_master()` deletes and rewrites that one asset, so the binary is the script's output plus the new flag. No other environment asset changed. A post-merge `import_kit_settlement` resaved the eleven kit material instances (unconditional save, LFS pointer only). Those were restored and not committed.

**What landed:** 16 authored modules, 11 skins, 7 library attachments, `kit.compose()`, seven compositions, three of them on `Lvl_KitGym` (store 59 placements, shed 22, cottage 39). They differ in footprint, storeys, and roof, not only in colour. Catalog: `Design/Kits/great_lakes_settlement_kit.md`. Usage: `Design/Kits/kit_usage_report.md`. Grey wood is ambientCG `WoodSiding011` (CC0). The roof is that shingle darkened. There is still no tar-paper texture.

**Verification on `main` after the merge** (logs from this run, not the branch):
- `import_kit_settlement`, `kit\build_kit_gym`, `kit\verify_kit`, `report_kit_usage`: exit 0. Verify passed. Usage: 3 structures, 22 pieces, 8 reused in all three, 100% of 109 structural placements are kit pieces, 0 bespoke buildings. The seven-composition projection reuses 12 pieces.
- `Tools\RunTests.bat -build`: **51 of 51** (34 editor, 13 Boathouse, 4 Sombre), 0 failures. No new C++ tests.
- `Tools\ReviewCapture.bat Lvl_KitGym`: `Saved/Review/2026-10-01_1025_Lvl_KitGym`. 10 views and 12 route frames. Manifest checks for default or grid materials, missing textures, warnings, and errors were empty.
- `Tools\Package.bat`: Development Win64 cook of `Lvl_Boathouse` and `Lvl_PointeSombre` only (`-Map=` lists those two; `Lvl_KitGym` does not appear in `Package.log`). Success, 0 errors, 0 warnings. Smoke-load: Boathouse 0.12 s, Pointe Sombre 0.09 s, no smoke errors.
- `Tools\Maps.bat list` is still those two production maps. No Boathouse or Pointe Sombre map file changed.

**Remaining visual limits, not blockers:** the gym yard is `MI_DC_Mud` and tiles; the outside stair reads as a solid run from the pure side and as steps from the three-quarter view; drums, pallets, ladders, shutters, and nets are not in v1.

**Anthony's playtest (2026-10-01): the kit concept is accepted.** The three gym structures read as distinct, the shared modules do not feel obviously repetitive, and the visual quality is good enough. One defect before that could be final: a single crate floating in the lean-to. The stacked `Crate` in `salvage_shed.json` was placed at 0.75 m. `wooden_crate_01` at scale 1.3 is 0.42 m tall, so that height left a gap of about 0.31 m. Its bottom is now at 0.439 m, on the crate beneath. Nothing else in the kit changed. Rebuilt `Lvl_KitGym`; `kit\verify_kit` passed (store 59, shed 22, cottage 39). Recapture `Saved/Review/2026-10-01_1033_Lvl_KitGym`: 10 views and 12 route frames, and the manifest lists no default or grid materials, missing textures, warnings, or errors. The shed front shows the two crates stacked on the floor.

**Next task:** VS-06, the rocky shoreline recipe. Not started.

## VS-08 Record: the Exterior Greybox (2026-10-01)

**COMPLETE.** Every done-when item is met. Details: `Design/technical_architecture.md`, "The exterior greybox".

**Recovered state.** This session took over from one that ran out of usage after building most of VS-08 (uncommitted on `7126435`). I re-checked everything against the repository before continuing: the routes, reach, the biome verifier, the map tests, and the captures, image by image. That work was coherent and is kept. On top of it this session:
- found Gate 1 invalid (the toggle had toggled nothing) and fixed the tool
- found the white capture frames were lightning
- fixed the shore band reading as water
- fixed the broken cable-hut reveal view
- opened the quay sightline to the false-light post
- ran the Gemini critic twice and reshaped the terrain in response
- re-measured everything on the final geometry

**The post-merge audit.** `C:\DeadCurrent_reviews\CODEX_VS06_VS07_POSTMERGE.md` did not exist at any point in this session; its scratch files stopped at 16:34. There was nothing to apply, so there were no repair commits. If it lands later, read it before VS-09.

**What exists now:**
- **Landmarks:** the tower (base room with a door, shaft, gallery with a rail, lamp room), the false-light post and its hide, the *Ashland Grey*'s tilted stern on its reef, the Authority mast, and the vault's iron door at the tower rock's foot.
- **Kit shells**, all from the accepted VS-05 kit, seated on pilings over the real ground with nothing flattened: the store, Odette's cottage, a stand-in for the Leclair house (the cottage turned and re-skinned), the Pruitts' shed, the smokehouse, two harbor sheds, and the cable hut.
- **Occluders:** three rock outcrops, so the tower slides out of view on the climb and returns.
- **Stub interiors** (the vault, the net loft) and open stub portals to every interior and the lamp room.
- **The six location volumes** with their final ids, and every ledger anchor. `Anchor_RemyMarker` is placed and held; nothing of Remy's is built, and the `remy_holdback` exclusion keeps his shore clear.
- **The *Ida*:** after the strike, the same vessel lies at her berth on the quay (presence), with a fender.
- **The three real shore zones** (`harbor.json`, `cable_hut.json`, `headland.json`, in the landed VS-06 schema plus `exclude_paths`). `_test.json` is retired from the map and kept as a verifier fixture.
- **Trails** in `island.json`, graded (fill only) and painted.
- **Containment:** the fence now rises to 55 m (it was 12 m, under the 26.5 m tower pad), on the `InvisibleWall` profile so it never blocks a sightline. A hidden floor at −1.25 m catches a fall off a cliff, so there is no `FellOutOfWorld`.
- **New tests and tools:**
  - `Map.Sombre.Greybox`
  - the route-timing report `pointe_sombre/routes.py`
  - the reachable-space flood fill `pointe_sombre/reach.py`
  - 14 `greybox_*` review views

**Frame kept: 435 × 245 m.** The short axis is under the plan's 300–400 m, but every route times inside its window on it, so the compression stays. Nothing was enlarged.

**Route-timing report** (`py -3 Tools\EditorScripts\pointe_sombre\routes.py`). The table walks each trail on the terrain mesh at the player's 450 cm/s, on the final geometry (island hash `9fab64703e4707fc`). `Map.Sombre.Greybox` then walks the real player along the same trails; its times are in the verification below.

| Leg (trail) | Walked | Climb | Steepest 2 m | Time | Target | Result |
| --- | --- | --- | --- | --- | --- | --- |
| Quay → Marthe's store (`quay_store`) | 48.3 m | 3.2 m | 24.0° | 10.7 s | 10–15 s | inside the window |
| Quay → tower door (`quay_tower`) | 184.9 m | 30.0 m | 25.3° | 41.1 s | 35–50 s | inside |
| Settlement → cable hut (`settle_hut`) | 143.1 m | 7.6 m | 31.9° | 31.8 s | 25–40 s | inside |
| Tower → cable hut (`tower_hut`) | 79.5 m | 0.0 m | 32.8° | 17.7 s | 15–25 s | inside |
| Quay → west-headland post (`quay_head`) | 231.2 m | 15.4 m | 25.2° | 51.4 s | 45–60 s | inside |
| Headland → *Ashland Grey* stern (`head_grey`) | 98.2 m | 0.9 m | 26.5° | 21.8 s | 20–30 s | inside |

**No reachable void** (`reach.py`):
- From the quay on foot: 16,458 cells (6.6 ha), 0 void, with 127 of them in the shallows on the safety floor.
- The store, the tower door, the cable hut, the headland post, the reef by the *Grey*, the iron door, and the settlement are all reachable.
- The highest reachable ground is 26.5 m, against a 55 m fence.
- In the engine, `Map.Sombre.Greybox` also sweeps every 5 m inside the fence (0 void) and pushes a pawn at 30 m and 50 m against every fence segment.

**Landmarks** (the capture's trace check, `Saved/Review/2026-10-01_1824_Lvl_PointeSombre_checkpointA`): 13 of 13 pass.
- The tower is in frame with a clear line of sight from every exterior cell's view: the deck, the quay (three looks), the settlement, the tower path, the cable hut, the headland post, and the *Grey*'s stern.
- From the quay, the false-light post and the *Grey*'s hull are also clear (`greybox_quay_west`, plan §5.5's landmark overlap).
- Moved to get there: the west harbor shed (8 m along the shore) and the TEST fixture door (7 m). Both stood on the line from the quay to the post.

**Shore zones:**
- 328 recipe instances in 20 components: talus 71, cobble 220, driftwood 11, scrub 26 (cap 800, not reached). 0 hand-placed Tier C (100% recipe).
- Manifest hash `aaa46ba8…`; `verify_plan.py --manifest` re-plans it exactly.
- Sheltered vs exposed is decided by data. Cobble collects in the reef-sheltered bight; talus is on the north and west scarps.
- Nothing lies inside an authored, pad, trail, or automatic exclusion. The berths, Remy's holdback, the hide, Dell's post, the conduit, and the iron door are all clear.

**Performance gate 1 (against gate 0, same session, final geometry):** `Saved/Review/2026-10-01_1826_Lvl_PointeSombre_gate1_final_ab`.
- **Method:** one uncapped run. At every view, the greybox and the recipe (`Greybox+Biome` tags) are hidden and shown in alternating windows (ABBA, 4 cycles of 2 s), with lightning hidden. This laptop flips between two clock states even inside one view (windows near 4 ms beside windows near 11 ms), so each view compares off and on within the slow state. Separate before and after runs are meaningless here (VS-06 saw 1–9 ms swings).
- **Greybox plus recipe cost:** median **+0.59 ms**, mean +0.67 ms over the 23 exterior views (range −0.38 to +1.75). The two control views, where nothing toggled is in view, read +0.18 and +0.77 ms, so the cost sits about at this machine's noise.
- **Whole map, everything on:** 8–13 ms per view in the slow state and 3.6–6.2 ms in the fast one. Every view is under 16.7 ms (gate 0's views all sat at the 60 FPS cap, so gate 0 only bounds them from above). Nothing is hidden by the cap: this run was uncapped.
- **Route** (the trail from the quay to the tower door, on the surface, everything on): mean 10.4 ms, p90 12.7 ms, max 17.1 ms. Gate 0's 21.3 ms route is not a valid baseline (VS-06 showed its frames were not on the surface).
- **Cost columns:** 13–167 visible components and 0–510 instances per exterior view. Draw-call peaks (418–1542) are unstable on this machine (the same scene read 441 and 1340 in two runs), so they are recorded but not used as a gate signal; the VS-04 watch item stands.
- An earlier valid run on the pre-critic terrain gave median +0.52 ms (`..._1756_..._gate1_ab`). The very first VS-08 run (`..._1553_..._gate1`) toggled nothing; it is the bug fixed below and is not counted.

**Gemini critic (filed: `Design/POIs/reviews/sombre_greybox_gemini.md`):**
- Round 1 said **NOT READY**, with three blockers. I checked each against the images:
  - **The causeway read as a paved bridge.** Confirmed, and fixed: it is now a wandering, broken bedrock spine (crest 0.36–0.9 m), with no trail paint in the splash band.
  - **The shores were gentle beaches everywhere.** Confirmed, and fixed: the exposed north and west now drop in scarps, and broad folds break up the smooth bank.
  - **The settlement is too spread out.** Left for you.
- Round 1 also misread two views, the tower-path outcrop and the cable-hut reveal. I found the pale "second water sheet" along the shore myself; it was also the white waterline slivers, and it is fixed (dark wet stone).
- Round 2 said **READY WITH NAMED FIXES**. Its one blocker, the tower "swallowed" at the cable hut, is a misreading: the tower stands large in that frame, and the trace passes. Still FAIL in its view, and left for you: the settlement's spacing and the long western walk.

**Review captures:**
- `Saved/Review/2026-10-01_1824_Lvl_PointeSombre_checkpointA/contact_sheet.png` is the Checkpoint A set: 25 views and the 41-frame route, with lightning hidden (`-ReviewHideTag=Lightning`). Start with the `greybox_*` views.
- `..._1754_..._checkpointA` is the pre-critic layout, for comparison.

**Fixed on the way:**
- **The capture's toggle A/B toggled nothing without `-ReviewToggleViews`** (`FString::StartsWith("")` is false). The first gate-1 run was therefore invalid; the tool is fixed (one line).
- **Storm-look lightning whitened random capture frames.** Review sets for judging now hide it.
- **The TEST fixture's exterior markers assumed level ground.** After the folds, the ground outside its door fell 0.65 m below the hut floor, and `Map.Sombre.Architecture` failed ("Back outside at the door") in the first final run. The markers now stand on the ground under them (the portal rule).
- **The terrain's height bands compared cm with m** (the shingle band was 1.4 cm). Once it showed, its gravel mirrored the sky. It is now coast rock, and the white slivers along the waterline went with it.

**Known limitations, deferred (none blocks Checkpoint A):**
- Sun-facing slopes wash out pale in the dusk storm (most visible on the tower rock from the south-west). This is the storm look, not the greybox; it is for the storm's look-dev (VS-19/VS-21).
- Trail edges are saw-toothed: they are painted per 2 m triangle.
- The turf texture visibly tiles from height (art).
- The quay is bare terrain. The built quay, the harbor dressing, and the real portals come with their cells.
- The store's porch may sit high on its pilings; VS-11 owns its steps.
- The lightning flash itself may be too strong (it whitens the ground); that is VS-19's storm.
- Draw-call peaks are not a stable signal on this machine; frame time is measured A/B.

- The package logs 2 cook warnings: the VS-06 driftwood meshes `DriftWood_7_LowPoly` and `Driftwood_11_LowPoly` "must be resaved before it will cook deterministically". They predate VS-08; resave them with the next pack-asset touch.

**Verification after the last change (all on the committed state):**
- `py -3 Tools\EditorScripts\pointe_sombre\routes.py --check`: 6 of 6 legs inside their windows.
- `reach.py`: 0 void, every place reachable, fence clear.
- `biome\verify_plan.py --manifest`: all checks pass, and the committed manifest re-plans to its own hash.
- `Tools\RunTests.bat -build`: **53 of 53** (34 editor, 13 Boathouse, 6 Sombre). `Map.Sombre.Greybox` walked the real player: 10.3 s, 39.6 s, 31.0 s, 16.3 s, 50.6 s, 21.0 s, all inside their windows; 2,640 fence points, 0 void; 28 anchors; 6 location volumes.
- `Tools\Package.bat`: the Development Win64 cook succeeded (0 errors, 2 warnings, above), and **both** `Lvl_Boathouse` and `Lvl_PointeSombre` smoke-load with 0 errors.
- Review capture `..._1824_..._checkpointA` (25 of 25 views, 13 of 13 landmarks, no default material, missing texture, or error) and gate 1 `..._1826_..._gate1_final_ab`.
- Meshy spend: **0**. No save-format change, no new condition or consequence type, no new foundational C++ (two test files and a one-line review-tool fix).

## VS-07 Record: the Slice Specs and the Id Ledger (2026-10-01)

**COMPLETE (docs only: no Unreal, no C++, no actors).**

**What exists now:**
- seven cell specs in `Design/POIs/`, every field filled or `N/A — reason`, every open decision with its default: `sombre.crossing.md`, `sombre.harbor.md`, `sombre.settlement.md` (with the NPC appendix), `sombre.lighthouse.md` (integrated: the tower and the vault), `sombre.net_loft.md`, `sombre.cable_hut.md`, `sombre.headland.md` (with the *Ashland Grey*). There is no vault spec and no *Grey* spec.
- the id ledger `Design/POIs/sombre_ids.md`
- the production report `Design/POIs/SLICE_PRODUCTION_REPORT.md`, with what is already measured filled in and "not yet" elsewhere
- handoffs: `sombre_WP-NARR.md` (written, not launched: VS-09 waits until you say so) and `sombre_WP-CELL-lighthouse.md`, plus a "not launched, serial" note for WP-VAULT

Codex's VS-07 preflight was the audit. Its recommendations were re-checked against the landed VS-06 schema, and its discrepancy list is kept in the ledger (§15), not silently merged.

**Decisions I made inside the plan** (each recorded in the ledger and the specs; all easy to change):

1. **`sombre.storm` is gameplay state, not the atmosphere selector.**
   - It means "a storm is on now". It drives the vault's live water, the false light, Dell at the post, and the bearing.
   - **The crossing's wheelhouse door (the strike) sets it,** alongside `sombre.reef_struck`. `knows` clears it, and each ending sets it again.
   - Why the strike: nothing in the existing grammar can set a flag at a new game without new C++ or a player action, and every new game passes through the strike. The preflight's "the core sets it at new game" has no mechanism.
   - The only storm moment before the strike is the false light seen from the deck. It is its own light, conditioned on "not yet struck".
   - The three atmosphere looks stay exactly as shipped: dusk storm, calm on `hale_arrived`, night on `meeting_done`. Nothing in VS-07 implements this; VS-10 adds the door's second consequence.
2. **The lighthouse and the vault: serial.** The lighthouse (VS-12), the cable hut (VS-13), and the vault (VS-14) are built by Claude in that order. A parallel vault package for Grok runs only if you enable it, and you have not. The file and actor boundary was checked against the real scripts and is clean, so it is frozen in the lighthouse spec; enabling it at Checkpoint B costs no redesign.
3. **People who move between cells stay one actor in one script.**
   - Mara, Varga, and Hale belong to the harbor; the six islanders to the settlement.
   - Placements in another cell's space go to Integrator anchors: loft seats, Mara's rail and porch, Dell's post, the *Ida*'s berth.
   - The keeper you see on the gallery at night is a tower-owned silhouette, not Odette or Dell moved there.
4. **The loft's way down is two portals at the stair head,** with one presence rule showing exactly one: "night" onto the night quay once after the meeting, otherwise "day" to the store. A shipped portal has a single destination, and a hidden portal cannot be focused (checked in the code).
5. **The hatch and the lower door are portals,** not inspectables. That settles the dialogue doc's "the hatch is an inspectable" against the plan's portals.
6. **Remy's marker and boat are held back** for VS-20's fresh-agent test:
   - reserved: the ids, the anchor, an empty harbor exclusion `remy_holdback`, and the file names
   - VS-08, VS-10, and VS-11 must not build or depend on them
   - the harbor-mouth view expects clear rocks, not a boat
7. **Zones:** exactly three cell zone files (harbor, cable hut, headland), in the landed VS-06 schema. Every other spec's keep-clear needs are requested by name in those files.

**Recorded discrepancies kept visible (none blocks VS-08; ledger §15):**
- the plan's two cell lists (§11 vs §8.4)
- `sombre_pruitts` and `sombre_meeting` are aliases (split, and folded into Marthe)
- `sombre_vault_key` was missing from the beat script's item list and is minted
- plan §19's Settlement test vs VS-11 on Dell's confession
- the art-sublevel count: eight from seven specs
- "zero investment" means some way into the vault, not always Odette's key
- the `knows` objective vs Marthe's call

**Next:** VS-08, the greybox, then Checkpoint A.

## VS-06 Record: the Rocky-Shoreline Recipe (2026-10-01)

**COMPLETE.** Tier C by recipe: a zone is data, and the recipe fills it. Details: `Design/technical_architecture.md`, "Shoreline recipe".

- **PCG or scripted HISM: scripted HISM.** The plan's spike was fail-fast (about 5 minutes; PCG enabled on the command line only, nothing saved). It failed gate 1: in the headless commandlet `RebuildContent.bat` uses, `generate()` only schedules the graph and never runs (`generated` false, `ExecutionSource cancelled` at shutdown), and the spawner's mesh list cannot be set from Python. No PCG plugin, graph, or level is committed.
- **What exists:**
  - `Tools/Biomes/great_lakes_rocky_shore.json`: the recipe, four families (talus, cobble, driftwood, sparse scrub), every number a tuning value
  - `Tools/Biomes/zones/_test.json`: the test zone, two polygons (the sheltered bight west of the quay, the west headland's tip), one authored exclusion, three pads
  - `Tools/EditorScripts/biome/`: `plan.py` (pure Python, the only placement authority), `scatter.py` (writes the HISM sublevel and the manifest), `verify_plan.py` (pure checks), `prepare_biome_meshes.py` (one-time mesh preparation)
  - `/Game/Maps/Lvl_PointeSombre_Biome` (always loaded, linked once; `Lvl_PointeSombre.umap` changed only to link it) and `Tools/Biomes/out/Lvl_PointeSombre.json`
  - `Map.Sombre.BiomeExclusions` (`Source/DeadCurrent/Save/DCSombreBiomeMapTest.cpp`)
  - `biome\scatter` is now the last step of a full `RebuildContent.bat`
- **Meshes, all from your library, no Meshy:** three Megascans stones from the `Scene_Junkyard` pack (talus and cobble, drawn with our own rock surfaces), two `DriftWoodPack` low-poly logs, and two small `Smugglers_cove` weeds that are common on Great Lakes shores (Canada lettuce, sorrel). Provenance rows are in `art_pipeline.md`.
- **Manifest hash `5c2076d57d954a767cc1711793d0328060fca0d6`.** 262 instances in 8 components (talus 27, cobble 176, driftwood 18, scrub 41; cap 800, not reached).
  - Two clean `biome\scatter` runs on the final recipe wrote byte-identical manifests.
  - `verify_plan.py` re-plans the committed manifest from its recorded inputs, with no Unreal, and gets the same hash.
  - A core map rebuild keeps the sublevel linked and its instances intact.
- **The harbor and the exposed shore differ, decided by data.** Exposure is measured as open-water fetch, so the reef-sheltered south shore reads as protected and the headland as exposed. Cobble: 124 sheltered vs 52 exposed. Talus: 1 vs 22. The test checks that contrast.
  - Codex's suggested coast-turn rule would have read the quay as exposed. The harbor is sheltered by the reef, not by a bay.
- **Exclusions work, authored and automatic.**
  - Nothing lies inside the authored box, the pads, or the automatic radii.
  - The tests also prove each exclusion would otherwise have held instances.
  - The arch-test door stands inside the bight polygon and leaves its hole.
- **Every Tier C component is NoCollision** and out of navigation, with no default material. No instance floats (the map test traces each one).
- **Tests:** `Tools\RunTests.bat -build`: **52 of 52** (34 editor, 13 Boathouse, 5 Sombre).
- **Cost gate (biome on vs off, ≤ 0.5 ms): passed, +0.18 ms and +0.28 ms.**
  - Measured inside one run: the biome is hidden and shown at each biome view, in alternating windows.
  - Why not two separate runs: this laptop flips between two clock states mid-capture (the same view at about 3 ms or about 11 ms). Separate OFF and ON runs differed by 1 to 9 ms even on views the biome cannot touch, so they could not resolve 0.5 ms. Those inconclusive pairs are kept and not counted.
  - Draw-call peaks rose by at most 16 on the biome views; the quay views are unchanged.
  - Talus draws from LOD2 and cobble from LOD3, because the scanned stones carry 5–9k triangles at LOD0.
  - Captures:
    - `Saved/Review/2026-10-01_1427_Lvl_PointeSombre_biome_ab2` and `..._1430_..._biome_ab3`: the gate
    - `..._1320_..._biome_off_a` / `..._1324_..._biome_on_a` and `..._1327_..._biome_off_b` / `..._1331_..._biome_on_b`: full runs with the route
- **Review views:** `Tools/Review/Lvl_PointeSombre/biome.json`, three views:
  - `biome_harbor`: the cobble line at the bight's waterline
  - `biome_exposed`: dark talus on the headland's south face
  - `biome_overview`: the headland from the water, both exposures in one frame
- **Fixed on the way (shared tools):**
  - **The review route now walks on the terrain.** It traces from 200 m down, ignores the player, and fails unless it hits `SombreTerrain`. The last route frame is now at 21.7 m on the tower rock. **The VS-04 gate-0 route figure (21.3 ms) is not a valid comparison basis.**
  - **`ImportPackAssets.ps1` dropped textures named like `AO_512`.** The engine stores such a name as `AO` plus a number. Every texture of each log was missing.
  - **The texture export named PNGs by short name,** so three `Normal_512`s overwrote each other. They are now named by full path; earlier migrations were checked and had no collisions.
  - **The driftwood pack's master material did not compile** (two empty texture defaults), and its instances showed the engine checker at a distance. Both are fixed in our copies.
- **New capture options** for same-session gates: uncapped FPS, longer sampling, no route, hide by tag, and the in-run on/off toggle. They are set through `DC_REVIEW_ARGS`; see Review capture in `technical_architecture.md`.
- **Deferred, not VS-06:**
  - the island's short axis, the fence below the tower, the 2 m collision mesh (VS-08)
  - the real cell zones and the 20-line zone test (VS-08, VS-16)
  - white slivers along the waterline where terrain triangles meet the water sheet. They predate the recipe and are a VS-08 watch item.
  - Litter, decals, trees, and wet-band were left out on purpose.
- **What you judge at Checkpoint A:** whether the shore reads as a Great Lakes place rather than random scatter. The recipe is deliberately conservative (one test zone); VS-08 adds the real zones.

## Decisions

Your approval settled all three pre-VS-01 decisions: the plan, the prologue staying out of Phase 6, and the agent roles (the plan's defaults). Nothing is open. The Meshy stop ceiling for Phase 6 is **350 credits** (balance 437); it is a limit, not a target.

## What the Phase 6 Plan Is (approved: `24d3e0d` + `034cbc7`)

- **The slice:** "The Wrong Characteristic" at Pointe Sombre, built as written in `Design/Narrative/SLICE_*`: the played crossing, the harbor, the settlement, the lighthouse, the vault dungeon, Hale's midway arrival, False Light, the decision made with your hands, Marthe's net loft, the second storm, and the end card. It runs 60–90 minutes.
- **Two proofs:** that it works as a game, and that more of it can now be made cheaply (the kit, the shoreline recipe, seven cells built through the contract, production metrics).
- **Three small new capabilities, nothing else in C++. Each is built just in time, by the first content that needs it:**
  - a cell portal (a fade, a move, a fade back; locked by conditions; a "scene cut" that lets presence snap), in VS-03, because the map architecture needs it
  - an authored light sequence (the tower's characteristic, plus a one-time three-second pattern), at the start of VS-17, when the tower is first lit
  - a story card (the end card), at the start of VS-19
- **No save-format change and no `SaveVersion` bump.**
- **Four playable checkpoints for you before the final run** (§16): A the island's shape; B people and the first clue; C the dungeon; D the decision. The final run (§22) comes after them.
- **One new map, `Lvl_PointeSombre`, not World Partition.** Interiors are cells inside the same map, reached through portals. The lamp room stays in place at the top of the real tower.
- **Mara is placed, not following.** The prologue stays out of Phase 6.
- **Compressed geography, a modular settlement kit (Tier B), and a rocky-shoreline recipe (Tier C)** are the Fallout-NYC-inspired production proofs. Seven content cells are built through the contract; one late fresh-agent cell is real slice content, never filler.
- **Agents:** Claude integrates and builds most cells; Grok builds the capabilities and the biome tool and critiques gameplay; Gemini surveys the kit research (docs only) and critiques visuals. Generated maps are build products that only the Integrator commits; content specs are per file; tests and review views are per cell; one worktree per builder.

## Known Risks

- **Named NPCs may read as twins or mannequins.** Eight people on one pack body. A bounded spike (MetaHuman low-LOD, or a rigged generated body) goes to you at Checkpoint B.
- **Combat is thin by design:** one melee archetype, one fight on the *Grey*. Checkpoint C asks whether that is enough. A ranged enemy would be your scope call.
- **The storm, calm, and night looks may not switch with presence alone** (post-process volumes). A spike happens in VS-04; the fallback is one small actor.
- **The crossing could read as a static set.** Its fallback (a fixed view) needs your sign-off.
- **Vessels and the tower are the bespoke art.** The fallbacks are a revolved-profile tower, one hull reused, and the *Grey* composed from parts.
- **Continuity fixes in the slice docs** (plan §8.2), each the smallest change and shown to you at Checkpoint B:
  - Dell's "hasn't rained" line, in a slice that opens in rain
  - "Catch them at it" placed after the meeting
  - "Force the Pruitts" (cut)
  - the net loft gathering
  - two soft-lock guards in the vault
- **One save slot for both maps:** F5 in the slice overwrites a shore save.

## Automated Baseline

- Build: `DeadCurrentEditor` builds.
- Tests: **53 of 53** (34 editor, 13 Boathouse map, 6 Sombre map) after VS-08 (`Map.Sombre.Greybox` added). 52 after VS-06, 51 after VS-05. Phase 6 expects about 70 by the end; that is an estimate, not a target.
- Package: the Development Win64 cook of **both** production maps, and both smoke-load (re-run after VS-08, 2026-10-01: 0 errors, 2 warnings, the VS-06 driftwood meshes asking to be resaved). `Lvl_KitGym` is not cooked.
- Frame time: about 54 FPS at low spec on the shore, deferred. Phase 6 uses same-session A/B gates only. Gate 0 is recorded in the VS-04 Record.
- Meshy: 460 of the earlier 500 spent; balance 437. Phase 6 stop ceiling: 350. Phase 6 spend so far: **0** (through VS-08).

## Next

**Stopped at Checkpoint A.** Waiting for your playtest and decision on the island's shape. After you accept it (with or without changes), the layout and the cell footprints freeze, and VS-09 (slice data, WP-NARR) and the cells (VS-10 onward) can start when you say so. If `C:\DeadCurrent_reviews\CODEX_VS06_VS07_POSTMERGE.md` lands meanwhile, it is read before any of that.

## First Parallel Wave: Record (research merged; portal merged; kit merged)

As prepared: three packages, three agents, no overlapping files. The research, the portal, and the kit are done (the kit is the VS-05 Record). The wave plan, ownership table, merge order, launch prompt, and worktree commands are in **`Design/POIs/handoffs/PHASE6_WAVE1.md`**.

| Package | Plan task | Agent | Handoff |
| --- | --- | --- | --- |
| WP-SYS-PORTAL: the cell portal and scene cut (the one capability VS-04 cannot start without) | VS-03 | Grok / Cursor | `Design/POIs/handoffs/sombre_WP-SYS-PORTAL.md` |
| WP-KIT: the Great Lakes Working Settlement Kit and its dev gym map | VS-05 | Claude | `Design/POIs/handoffs/sombre_WP-KIT.md` |
| WP-KIT-RESEARCH: library and CC0 survey, visual reference analysis (docs only). **DONE, merged.** | feeds VS-05 | Gemini | `Design/POIs/handoffs/sombre_WP-KIT-RESEARCH.md` |

**Kit research result:** `Design/Kits/research/kit_library_survey.md` (+ `reference_links.md`). It was run headless by Claude through Google's Antigravity CLI (Gemini 3.1 Pro High) in its own worktree.
- **Permissions:** temporary domain-scoped web reads and three read-only PowerShell cmdlets. Your `agy` settings were restored byte-identical afterwards, and `C:\FO5_AssetLibrary` was verified unchanged.
- **Review:** the first draft claimed a CC0 "Bitumen" roofing texture was verified, but it does not exist. The revision fixed most review points, but its reference links are mislabelled (boats, not buildings) or 404.
- **Verdict:** accepted as **input**. The Integrator's verification and corrections sit at the top of the survey.
- **Open at research time, answered by VS-05** (details in the VS-05 Record):
  - a weathered grey wood skin: ambientCG `WoodSiding011`
  - a tar-paper or shingle roofing source: the shingle is `WoodSiding011` darkened; there is still no tar-paper texture
  - building reference photos: not collected; the kit did not depend on them

Each handoff fixes: the role, the starting commit (`e833811`), allowed and forbidden files, dependencies, deliverables, tests (with exact counts), review artifacts, stop conditions, commit and push permissions (own branch `vs/<package>` only; never `main`), and the integration owner (Claude). Merge order: research, then the portal, then the kit.

**Inspect before launching:**

1. The three handoffs' allowed/forbidden file lists and stop conditions (the parts that keep the agents out of each other's way).
2. **Three plan clarifications I made** so the packages did not contradict the plan (details in `PHASE6_WAVE1.md`). None changes scope:
   - the kit's test ground is its own dev map `Lvl_KitGym`, not a corner of the Pointe Sombre blockout (that map does not exist until VS-04, and WP-KIT may not touch shared map scripts)
   - the kit is verified by a headless script (`kit/verify_kit.py`) plus review captures, not by `Map.Sombre.KitGym` (which needs the Sombre map and a `Source/**` file WP-KIT may not create)
   - the Gemini research is its own package with its own folder (`Design/Kits/research/`)
3. That `e833811` is what each agent starts from (`git fetch`, then `git log origin/main -3`).
4. Machine load: the portal agent runs a full C++ build and editor tests, and the kit agent runs the engine headless, each in its own worktree. Expect it to be slow but not to collide.
5. `Lvl_PointeSombre` still does not exist; the tools say "registered but not built yet" until VS-04.

Launch prompt and worktree commands: `PHASE6_WAVE1.md` ("Launching an agent").

## Known Issues Carried Forward

- The ~54 FPS low-spec shortfall is deferred (same-session A/B only).
- Mara is a placeholder mannequin; two unused generated heads remain in Content.
- `DCLandingStageMapTest.cpp` keeps its own helper copies (an accepted file left unchanged on purpose; migrate later if wanted).
- Untracked leftovers, not ours: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py`.
- Two VS-06 driftwood meshes need a resave for a deterministic cook (package warnings).
- Sun-facing slopes wash out pale in the dusk storm, and the lightning flash whitens the ground: storm look-dev, VS-19/VS-21.
