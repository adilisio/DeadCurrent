# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- `main` = `origin/main` = the VS-01 commit (see `git log -1`), on top of the approved plan commits `24d3e0d` and `034cbc7` (pushed 2026-09-30 after `git fetch` showed `origin/main` still at `9640c05`: a clean fast-forward).
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone

**PHASE 6: STARTED** (Anthony approved the plan, 2026-09-30).

| Task | State |
| --- | --- |
| VS-00 Plan | complete, approved |
| **VS-01 Baseline** | **COMPLETE** (this commit) |
| VS-02 Production foundations | in progress |
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
- Tests: **45 of 45** (33 editor, 12 map). Phase 6 expects about 70 by the end; that is an estimate, not a target.
- Package: the Development Win64 cook and the smoke launch of `Lvl_Boathouse` succeeded on the VS-01 baseline.
- Frame time: about 54 FPS at low spec, deferred. Phase 6 uses same-session A/B gates only.
- Meshy: 460 of the earlier 500 spent; balance 437.

## Next

VS-02 (production foundations): per-file content specs, multi-map tooling, shared map-test helpers. Then the first parallel wave (VS-03 portal, VS-05 kit, the kit research), prepared but not launched until VS-02 is green.
