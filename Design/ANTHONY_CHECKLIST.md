# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- `main` (see `git log -3`): the VS-02 commit on top of the VS-01 commit `00d8d22`, on top of the approved plan commits `24d3e0d` and `034cbc7`. The plan and VS-01 are pushed (`origin/main` was a clean fast-forward from `9640c05`).
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone

**PHASE 6: STARTED** (Anthony approved the plan, 2026-09-30).

| Task | State |
| --- | --- |
| VS-00 Plan | complete, approved |
| **VS-01 Baseline** | **COMPLETE** (`00d8d22`) |
| **VS-02 Production foundations** | **COMPLETE** (this commit) |
| VS-03 onward | not started |

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

- Build: `DeadCurrentEditor` builds (with the new `RHI` module dependency for the review capture's draw-call counts).
- Tests: **46 of 46** (33 editor, 13 map). Phase 6 expects about 70 by the end; that is an estimate, not a target.
- Package: the Development Win64 cook (explicit `-map=`) and the smoke launch of `Lvl_Boathouse` succeeded.
- Frame time: about 54 FPS at low spec, deferred. Phase 6 uses same-session A/B gates only (the VS-02 capture was 2 ms faster than the reference).
- Meshy: 460 of the earlier 500 spent; balance 437. Phase 6 stop ceiling: 350.

## Next

The first parallel wave (VS-03 portal, VS-05 kit, the kit research): handoffs are prepared in the commit after this one, and launched only by you.
