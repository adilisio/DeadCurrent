# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- `main` = `origin/main` after the VS-03 merge (see `git log -3`). VS-03 was built on branch `vs/sys-portal` (`224457d`) and merged with `--no-ff`. Earlier: VS-02 `e833811`, VS-01 `00d8d22`, the approved plan `24d3e0d` + `034cbc7`.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone

**PHASE 6: STARTED** (Anthony approved the plan, 2026-09-30).

**VS-01, VS-02, VS-03: COMPLETE.** The kit research is merged. Next: **VS-04** (the map architecture proof, now unblocked) and **VS-05** (the kit), both waiting for your go-ahead.

| Task | State |
| --- | --- |
| VS-00 Plan | complete, approved |
| **VS-01 Baseline** | **COMPLETE** (`00d8d22`) |
| **VS-02 Production foundations** | **COMPLETE** (`e833811`) |
| First parallel wave: kit research (Gemini) | **DONE and merged** (accepted as input after one revision; see below) |
| **VS-03 Cell portal + scene cut** | **COMPLETE** (built by Claude at your request, merged; see below) |
| VS-05 Settlement kit (Claude) | ready, not started |
| VS-04 Map architecture proof | unblocked, not started |
| VS-06 onward | not started |

Plan: `VerticalSlicePhasePlan.txt`. Phase 5 (World State) is accepted and unchanged.

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
- Tests: **47 of 47** (34 editor, 13 map). Phase 6 expects about 70 by the end; that is an estimate, not a target.
- Package: the Development Win64 cook (0 errors, 0 warnings) and the smoke launch of `Lvl_Boathouse` succeeded after the VS-03 merge.
- Frame time: about 54 FPS at low spec, deferred. Phase 6 uses same-session A/B gates only. VS-03 adds no actors to any map, so no capture was needed.
- Meshy: 460 of the earlier 500 spent; balance 437. Phase 6 stop ceiling: 350.

## Next (your call)

- **VS-04, the map architecture proof**, is now unblocked. It is the Integrator's task (plan §15):
  - the `Lvl_PointeSombre` core script and per-cell hooks
  - the terrain-method spike
  - a test cell with a two-way portal
  - the storm, calm, and night atmosphere spike
  - respawn after the crossing
  - cooking and smoke-loading both maps
  - `Map.Sombre.Architecture`
  - It needs no new decision from you, only the go-ahead.
- **VS-05, the settlement kit** (`Design/POIs/handoffs/sombre_WP-KIT.md`), is still ready and can run alongside VS-04: the files don't overlap. Its research input is merged (`Design/Kits/research/`, with the Integrator's corrections at the top).
- I have not started either.

## First Parallel Wave: Record (research merged; portal built by Claude and merged; kit not started)

As prepared: three packages, three agents, no overlapping files. The research and the portal are done (above); WP-KIT remains. The wave plan, ownership table, merge order, launch prompt, and worktree commands are in **`Design/POIs/handoffs/PHASE6_WAVE1.md`**.

| Package | Plan task | Agent | Handoff |
| --- | --- | --- | --- |
| WP-SYS-PORTAL: the cell portal and scene cut (the one capability VS-04 cannot start without) | VS-03 | Grok / Cursor | `Design/POIs/handoffs/sombre_WP-SYS-PORTAL.md` |
| WP-KIT: the Great Lakes Working Settlement Kit and its dev gym map | VS-05 | Claude | `Design/POIs/handoffs/sombre_WP-KIT.md` |
| WP-KIT-RESEARCH: library and CC0 survey, visual reference analysis (docs only). **DONE, merged.** | feeds VS-05 | Gemini | `Design/POIs/handoffs/sombre_WP-KIT-RESEARCH.md` |

**Kit research result:** `Design/Kits/research/kit_library_survey.md` (+ `reference_links.md`). It was run headless by Claude through Google's Antigravity CLI (Gemini 3.1 Pro High) in its own worktree.
- **Permissions:** temporary domain-scoped web reads and three read-only PowerShell cmdlets. Your `agy` settings were restored byte-identical afterwards, and `C:\FO5_AssetLibrary` was verified unchanged.
- **Review:** the first draft claimed a CC0 "Bitumen" roofing texture was verified, but it does not exist. The revision fixed most review points, but its reference links are mislabelled (boats, not buildings) or 404.
- **Verdict:** accepted as **input**. The Integrator's verification and corrections sit at the top of the survey.
- **Still open for WP-KIT:**
  - a weathered grey wood skin
  - a tar-paper or shingle roofing source
  - building reference photos

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
