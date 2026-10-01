# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- `main` carries VS-05 (the settlement kit) on the VS-04 baseline. Integration commits: `ff611d1` (shared surface flag and rebuild log check), merge `54aa7b5`, generated gym `6776cda`. This checklist commit is the tip. See the VS-05 Record.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone

**PHASE 6: STARTED.** Working autonomously toward **Checkpoint A** (the island's shape), and stopping there for you.

| Task | State |
| --- | --- |
| VS-00 Plan | complete, approved |
| **VS-01 Baseline** | **COMPLETE** (`00d8d22`) |
| **VS-02 Production foundations** | **COMPLETE** (`e833811`) |
| Kit research (Gemini) | **DONE and merged** (input, with the Integrator's corrections) |
| **VS-03 Cell portal + scene cut** | **COMPLETE** (merged `2c1d2b6`) |
| **VS-04 Map architecture proof** | **COMPLETE** (see the VS-04 Record) |
| **VS-05 Settlement kit (WP-KIT)** | **COMPLETE** (merge `54aa7b5`; see the VS-05 Record) |
| VS-06 Shoreline recipe, VS-07 Specs and ledger, VS-08 Greybox | VS-06 is next; then VS-07, then VS-08, which ends at **Checkpoint A** |

Plan: `VerticalSlicePhasePlan.txt`. Phase 5 (World State) is accepted and unchanged.

## Inspect When You Return

Things only you can judge, kept current as work lands (newest first):

1. **VS-05, the kit gym.** `Tools\PlayTest.bat Lvl_KitGym` (a development map; it is not part of the slice). Three buildings from one vocabulary: a two-storey store, an open corrugated shed, a shingled cottage. Review frames: `Saved/Review/2026-10-01_1025_Lvl_KitGym/contact_sheet.png`.
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

**Next task:** VS-06, the rocky shoreline recipe. Not started.

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
- Tests: **51 of 51** (34 editor, 13 Boathouse map, 4 Sombre map) after VS-05, same count as VS-04. Phase 6 expects about 70 by the end; that is an estimate, not a target.
- Package: the Development Win64 cook of **both** production maps (0 errors, 0 warnings), and both smoke-load. Re-run after the VS-05 merge (2026-10-01). `Lvl_KitGym` is not cooked.
- Frame time: about 54 FPS at low spec on the shore, deferred. Phase 6 uses same-session A/B gates only. Gate 0 is recorded in the VS-04 Record.
- Meshy: 460 of the earlier 500 spent; balance 437. Phase 6 stop ceiling: 350. Phase 6 spend so far: **0**.

## Next

Continuing autonomously toward Checkpoint A: **VS-06** (the shoreline recipe) is next, then VS-07 (specs and the id ledger), then VS-08 (the greybox). Then I stop and hand you the Checkpoint A package. Nothing after Checkpoint A will be started. This integration does not start VS-06.

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
