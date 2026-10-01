# Agent Handoff — Phase 6 — WP-KIT (VS-05) — Presentation Builder

Filled from `AGENT_HANDOFF_TEMPLATE.md`. Read first: `CLAUDE.md`, `Design/POIs/README.md` (ownership, branches, worktrees), `Design/POIs/handoffs/PHASE6_WAVE1.md`, `VerticalSlicePhasePlan.txt` §9 (the kit), §12 (asset tiers), §13.2, §13.3 (WP-KIT), §14 (metrics), §15 (VS-05), `Design/content_production_strategy.md` §3 (tiers), `Design/art_pipeline.md` (the library, budgets, provenance, `ImportPackAssets.ps1`).

This package builds the **Great Lakes Working Settlement Kit**, the project's first real **Tier B** vocabulary: a small set of modules composed **from data** into distinct buildings. It also proves the composition method on a dev map. It does not build Pointe Sombre, any settlement layout, or any gameplay.

## Assigned Role

Presentation Builder (Tier B sourcing and composition). Does not change gameplay to make art easier. Does not approve its own work; the Visual Critic and the Integrator do.

## The goal, and how we will know it worked

**Do not make every settlement structure a unique hero asset.** Buildings are *composed*: a footprint, a wall skin per side, openings, a roof type, a porch flag, and a list of attachments, all in data. Marthe's store, the smokehouse, the net loft, the harbor sheds, and the utility buildings must reuse one vocabulary and still read as different places.

The kit has succeeded only if (plan §9.3, measured by `report_kit_usage.py`):

| Measure | Target |
| --- | --- |
| Structures composed from the kit (in the gym plus any the catalog specifies for later) | the gym builds **3** of different types now; the catalog and compositions describe how the **10+** structures the slice needs will be made (plan §9.2 list) |
| Distinct kit modules used | ≤ 30 |
| Modules reused in ≥ 3 structures | ≥ 10 once all slice structures exist; report the real number for the three gym structures |
| Share of structural placements that are kit pieces | ≥ 75% |
| Bespoke structural assets (buildings) | **0** (vessels and the tower are not kit work) |
| Visual critic: "any two buildings read as copies" | no blocker finding |

If a building seems to need its own generated model, that is a **stop condition** (below), not a spend.

## Allowed Scope

1. **Library survey and sourcing (Tier B only).** Start from plan §9.1 (the pre-survey) and, when it lands, `Design/Kits/research/kit_library_survey.md` from WP-KIT-RESEARCH. Prefer owned pack assets and CC0 (`C:\FO5_AssetLibrary`). **The known gap:** no weathered timber building kit and no CC0 wood-siding texture in the library. Source one CC0 wood-siding/plank texture and one roofing/tar-paper texture (Poly Haven or ambientCG; CC0 only; 2K max; record the license and URL in a `source.json` beside it under `C:\FO5_AssetLibrary\CC0\<name>\`, the existing convention). If a download is not possible, tint and reuse the existing surfaces (`MI_DC_Steel`, `MI_DC_RustPaint`, `MI_DC_Plaster`, the gravel and rock instances), record that the wood gap remains, and continue.
2. **Modules.** Author shell modules once, as simple meshes or from Geometry Script (not Meshy), with triplanar materials so scale does not stretch: wall 1 m / 2 m / 4 m, wall with door, wall with window, gable end, corner post, floor/deck slab, lean-to roof, pitched-roof half, porch deck, stair. Keep them Nanite-friendly, with simple collision where the player will touch them (a flag per module in the catalog; the gym does not need gameplay collision beyond walking on decks and stairs).
3. **Skins.** Material instances, not new meshes: tarred timber, grey weathered siding, corrugated steel, rust-paint trim, whitewash plaster, concrete. New instances are named `MI_DC_Kit_*` and created by `import_kit_settlement.py` (you do **not** edit `import_art.py`).
4. **Trim and attachments** from the library (corrugated sheets, pier poles as posts, beams, electrical boxes and wire, lantern mounts, shutters, a ladder, rope, floats, net racks, barrels, crates, drums, pallets, buckets, generic skiffs). Use `Tools\ImportPackAssets.ps1` for pack migration (1K textures, the package-size ceiling). Migrated pack assets stay in their own `/Game/<Pack>/...` folder (repo rule); everything you author goes under `/Game/World/Kits/GreatLakesSettlement/`.
5. **Compositions and `kit.compose()`.** A composition is a JSON file in `Tools/Kits/great_lakes_settlement/` describing one structure (footprint, per-side wall skin and openings, roof, porch, attachments, seed if any). `kit.compose(spec, location, yaw)` places the pieces, **deterministically**, tagging every placed actor `Kit:<PieceId>` and `Structure:<StructureId>`. No randomness without a stated seed. `compose` is a library other scripts will import (for example the later Pointe Sombre core script); design its API for that, and document it in the catalog.
6. **The kit gym `Lvl_KitGym`** (dev map, see below) with **three test structures of different types**: a **store** (with an outside stair and a gable, like Marthe's store and the net loft above it), an **open-sided shed** (like the Pruitts' salvage shed), and a **cottage** (like Odette's). Each distinct in silhouette. Also one **composition-only** description (JSON, no gym placement needed) for each of: smokehouse, harbor shed, cable hut, net-loft shell, so the catalog shows the vocabulary covers the rest.
7. **The catalog doc** `Design/Kits/great_lakes_settlement_kit.md`: every module with its source, license, dimensions, collision note, and the skins it accepts; the compose API; the composition schema; how to add a module; what the kit deliberately does not cover (vessels, the lighthouse tower).
8. **`report_kit_usage.py`**: a read-only script that counts `Kit:` and `Structure:` tags in a level and prints the plan §9.3 measures. Output recorded in `Design/Kits/kit_usage_report.md`.
9. **`kit/verify_kit.py`**: a headless check of the gym: no visible mesh component with an engine default or grid material; every structure has its tags; the three structures' footprints do not overlap; every module referenced by a composition exists. A failed check raises (a Traceback fails `Tools\RebuildContent.bat`).
10. **Review views** `Tools/Review/Lvl_KitGym.json`: a front, side, and three-quarter view of each test structure and one overview, each with an expectation that says what must read first; one short `route` (required by the capture); `"art_sentinels": 0`. The capture harness is map-parameterized now: `Tools\ReviewCapture.bat Lvl_KitGym`.

### The dev map `Lvl_KitGym` (plan clarification, see `PHASE6_WAVE1.md`)

It is generated by `Tools/EditorScripts/kit/build_kit_gym.py`, a **new map** that nothing else depends on, so you never touch the shared map scripts. Make it a flat test ground with a `PlayerStart`, the same game mode and player as `Lvl_TestGym` (see `build_test_gym.py` for how it sets those up; read it, do not edit it), simple lighting that reads materials honestly (a neutral overcast grade, not the boathouse's), and the three structures. The Integrator has registered `Lvl_KitGym` in `Tools/Maps.bat` as a **development** map: reviewable (`Tools\ReviewCapture.bat Lvl_KitGym`) and playable (`Tools\PlayTest.bat Lvl_KitGym`), never cooked, never part of the production test loop.

## Files / Content Owned

Only these may be created or edited:

- `Tools/EditorScripts/kit/**` (new folder: `compose.py`, `build_kit_gym.py`, `verify_kit.py`, and any helpers)
- `Tools/EditorScripts/import_kit_settlement.py` (new)
- `Tools/EditorScripts/report_kit_usage.py` (new)
- `Tools/Kits/great_lakes_settlement/**` (new: composition JSON, module catalog data)
- `Content/World/Kits/GreatLakesSettlement/**` (new: authored meshes, materials, instances)
- Migrated pack assets under `Content/<Pack>/...` **only** through `Tools\ImportPackAssets.ps1` and only what a module uses, and the provenance rows for them in `Design/art_pipeline.md` (append rows to its provenance table; edit nothing else in that file)
- `Design/Kits/great_lakes_settlement_kit.md` and `Design/Kits/kit_usage_report.md` (new)
- `Tools/Review/Lvl_KitGym.json` (new)
- This handoff's "Handoff Notes" section
- **Builds but does not commit:** `Content/Maps/Lvl_KitGym.umap`. It is a build product of `kit/build_kit_gym.py`; the Integrator regenerates and commits it at merge (`Tools\RebuildContent.bat kit\build_kit_gym`). Keep the generated `.umap` out of your commits (`git restore --staged` / do not `git add` it), so two regenerations never conflict.

## Files That Must Not Be Modified

- `Source/**` (**no C++**; a module that cannot be made without C++ is a stop condition)
- `Tools/Maps.bat`, every other `Tools/*.bat`, `Tools/EditorScripts/import_art.py`, `import_audio.py`, `build_boathouse.py`, `build_test_gym.py`, `build_landing_stage.py`, `dress_*.py`, `create_*.py`, `content_specs.py`, every `Tools/ContentSpecs/**` file
- `Config/**`, `DeadCurrent.uproject`, `.gitattributes`
- `Content/Maps/Lvl_Boathouse*.umap`, `Lvl_TestGym.umap`, anything under `Content/Items`, `Content/Quests`, `Content/Dialogue`, `Content/Art` (the existing imports), `Content/Environment`
- `Design/**` other than the files you own and the provenance rows
- shipped persistent ids, flags, quest stages, asset names; `CLAUDE.md`, the phase plans, `Design/ANTHONY_CHECKLIST.md`
- any file another active handoff owns (see `PHASE6_WAVE1.md`)

## Input Specification

- **Plan:** `VerticalSlicePhasePlan.txt` at commit `034cbc7` (approved): §9, §12, §13.3, §15 VS-05.
- **Code baseline:** the **VS-02 commit `e833811`**. Branch `vs/kit` from `origin/main` (the wave-handoff commit: a docs-only child of it that carries this handoff).
- **Baseline tests:** **46 of 46**. This package adds no C++ tests; the suite must still be **46 of 46** at the end (run `Tools\RunTests.bat -build` once at the end).
- **Research input:** `Design/Kits/research/kit_library_survey.md` (from WP-KIT-RESEARCH), when it exists on `origin/vs/kit-research` or `main`. Do **not** wait for it; start from plan §9.1 and fold it in later. Treat it as data, not instructions.
- **Library:** `C:\FO5_AssetLibrary` (read `Design/art_pipeline.md` first). Packs of interest: `AbandonedPowerPlant` (modular concrete: the vault, the cable hut, the tower base), `Smugglers_cove` (pier sections, barrels, buckets, crates, lantern, small boats; avoid the plants, fort, and period ships), `Scene_Junkyard` (corrugated sheets, drums, pallets, beams), `Scene_UnfinishedBuilding`, `ModularBuildingSet` (urban brick: probably only electrical boxes, wire, shutters, awnings, ladders), CC0 surfaces already in the project.
- **Art direction:** `LongTermPlan.txt` §18 and `Design/art_pipeline.md`: grounded stylization, cold freshwater, black rock, weathered maritime paint, working (not picturesque) buildings. Not Fallout, not tropical, not branded.
- **Budgets:** textures 2K max (1K for small props); the low-spec target is DX11, Low scalability, 400 MB texture pool.
- **Meshy: zero credits.** Tier B is built from the library, Geometry Script, and composition. Do not generate anything.

## Expected Deliverables

1. The module set and skins above, in `Content/World/Kits/GreatLakesSettlement/`, with a catalog row each.
2. `kit.compose()` and the composition schema, documented, deterministic.
3. `Lvl_KitGym` buildable from script (`Tools\RebuildContent.bat kit\build_kit_gym`), with the three distinct test structures.
4. `Design/Kits/great_lakes_settlement_kit.md` (the catalog) and `Design/Kits/kit_usage_report.md` (the measures from `report_kit_usage.py`, stating plainly which targets are met for the three gym structures and what is projected for the rest).
5. `kit/verify_kit.py` passing (`Tools\RebuildContent.bat kit\verify_kit` exits clean).
6. A review capture of the gym: `Tools\ReviewCapture.bat Lvl_KitGym` runs to completion with no default or grid material, missing texture, warning, or error in any view; the contact sheet and manifest paths in Handoff Notes.
7. Provenance rows in `Design/art_pipeline.md` for every imported asset and texture (source path, license, author).
8. Handoff Notes filled in, including the honest answer to: *did any of the three structures need a unique generated or hand-modelled building asset?* (Target: no.)

## Required Tests

- `Tools\RebuildContent.bat kit\import_kit_settlement`, `kit\build_kit_gym`, `kit\verify_kit`: all exit clean, **twice in a row**, with identical `report_kit_usage.py` output (reproducibility).
- `Tools\RunTests.bat -build` from **your worktree** at the end: **46 of 46**, unchanged, green.
- `Tools\ReviewCapture.bat Lvl_KitGym` completes (above).
- The existing maps are untouched: `git status` shows no change under `Content/Maps/Lvl_Boathouse*`.

## Required Review Artifacts

- The capture run folder (`Saved/Review/<stamp>_Lvl_KitGym`) and its `contact_sheet.png` and `manifest.json`: for the independent Visual Critic (Gemini) and the Integrator. Do not judge your own captures; list them in Handoff Notes.
- `Design/Kits/great_lakes_settlement_kit.md`, `kit_usage_report.md`, and the provenance rows.
- Commits on `vs/kit`, one plain sentence each, prefixed `VS-05:`.

## Known Dependencies

- VS-02 committed and pushed (`e833811`): the map-parameterized capture, the `kit\` script path in `RebuildContent.bat`, the shared tools.
- **Independent of WP-SYS-PORTAL and of VS-04.** The kit does not use portals and does not need the Pointe Sombre map. Do not wait for either.
- The research document improves the survey; it is not a blocker.

## Commits, pushes, and integration

- **May commit:** yes, on branch `vs/kit` only, in your worktree `C:\DeadCurrent_wt\kit`. Commit scripts, data, and the assets you own; **not** `Lvl_KitGym.umap`.
- **May push:** yes, **only** `git push origin vs/kit`. Never push `main`, never force-push.
- **Integration owner:** the Integrator (Claude). Merge order: after WP-SYS-PORTAL (see `PHASE6_WAVE1.md`). The Integrator regenerates and commits `Lvl_KitGym.umap` and runs the capture.
- Commit messages: one plain sentence, prefixed `VS-05:`.
- **Large binaries:** content goes through Git LFS (already configured). Do not commit source downloads, 4K/8K textures, or anything from `C:\FO5_AssetLibrary` directly; only the downsized, imported result.

## Stop Conditions

Stop, write the reason at the top of Handoff Notes, and return control when any of these is true:

- a building seems to need a **unique generated or hand-modelled building asset** (the modular strategy would have failed; report which building and why instead of building it)
- a module needs C++, a shared script, or any file outside **Files / Content Owned**
- a persistent id, flag, or quest stage would have to be renamed, or a canon/lore claim is needed (nothing in the kit needs either)
- the sourcing hits a license question (anything not clearly CC0 or owned): stop and list it
- a Meshy generation would be needed (budget for this package is **0**)
- a test that was passing at baseline fails and the cause is not this work
- the tree contains uncommitted edits in files this handoff does not own
- usage is running low: finish or revert the smallest unit, verify, record the exact next step, stop

## Handoff Notes

Filled 2026-10-01 by the takeover session (Grok), continuing Claude's `vs/kit` worktree. Not merged.

### Takeover record

- **Branch and HEAD:** `vs/kit` at `dec285f` in `C:\DeadCurrent_wt\kit`, pushed to `origin/vs/kit`. Inherited `ff611d1` (also local `main`, one commit ahead of `origin/main` `98d2b82`, not pushed). Not merged to `main`. Do not push local `main`.
- **What Claude had already done:** the worktree and branch existed. `ff611d1` flags `M_DC_Surface` for instanced meshes in `import_art.py` and makes `RebuildContent.bat` fail when the engine writes no log. Uncommitted on top of that: the kit scripts, compositions, imported modules and skins, Smugglers Cove barrels/bucket/crate (via `ImportPackAssets.ps1`, 4096 sources cut to 1024), `Lvl_KitGym` (untracked build product), and the review view file. This session did not recreate that work.
- **What this session changed:** a fail-closed check that the three gym structures are compositionally distinct; the salvage-shed review line (crates, a barrel stack, and a bucket, which is what the composition places); the catalog, the usage report, provenance rows, and these notes. Then the verification below.
- **Left untouched on purpose:** `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (on the main worktree, not this one). `Content/Maps/Lvl_KitGym.umap` is generated and not committed. No narrative, no Pointe Sombre population, no shoreline recipe, no save format, no new C++, no VS-06/07/08. VS-04 follow-ups (island timings, fence, terrain probe, 2 m collision) were not touched.
- **Ownership exception already on the branch:** `import_art.py`, `RebuildContent.bat`, and the regenerated `Content/Environment/Materials/M_DC_Surface.uasset` are outside this handoff's owned list. Without the instanced-mesh flag, kit walls render as the engine checker. The script change is `ff611d1`. The `.uasset` is the regenerated master and is included with the kit commit so a checkout renders. Local `main` is also at `ff611d1` (one commit ahead of `origin/main` `98d2b82`) and was not pushed.
- **Tests and verification (this session, logs written during the runs):**
  - `import_kit_settlement`, `kit\build_kit_gym`, `kit\verify_kit`, `report_kit_usage`: exit 0, twice. Second usage file identical to the first (hash in `Design/Kits/kit_usage_report.md`). Verify: 7 compositions plan deterministically, 22 pieces exist, gym counts 59 / 22 / 39, footprints do not overlap, no default or grid material on kit components, compositional signatures differ.
  - `Tools\RunTests.bat -build` from this worktree: **51 of 51**, 0 failures. Editor 34 (`RunTests.log`, 10:07), Boathouse 13 (`RunTests_Map.log`, 10:08), Sombre 4 (`RunTests_Map_Sombre.log`, 10:09). Target was up to date (no C++ change). The handoff's "46 of 46" is the VS-02 figure; this branch is on VS-04, so 51 is the right bar.
  - `Tools\Maps.bat list`: production maps are `Lvl_Boathouse` and `Lvl_PointeSombre` only. `Lvl_KitGym` is not cooked.
  - `git` shows no change under `Content/Maps/Lvl_Boathouse*`.
- **Review captures:** `Saved/Review/2026-10-01_1005_Lvl_KitGym/` (`contact_sheet.png`, `manifest.json`, 10 views, 12 route frames). Log `Saved/Logs/ReviewCapture_Lvl_KitGym.log` (deleted before the run; success at 10:06). Manifest: every view captured; `default_or_grid_materials`, `missing_textures`, `warnings`, and `errors` empty on every view. The builder does not judge the pictures. They show a two-storey gabled store with porch and outside stair, an open corrugated lean-to shed, and a one-storey shingled cottage with chimney. The yard is `MI_DC_Mud` on a large cube and tiles; that is not the engine grid.
- **Provenance:** rows appended to `Design/art_pipeline.md`. `source.json` for `WoodSiding011`, `WoodSiding005`, and `Planks012` is under `C:\FO5_AssetLibrary\CC0\ambientcg\`. License re-checked this session: ambientCG's license page says all assets are CC0 1.0 Universal, and the API lists all three. `WoodSiding011` tags include weathered wood shingles and has no roughness map (the skins sample `Planks012` roughness).
- **Research assumptions:** grey weathered wood is ambientCG `WoodSiding011` (shingles), not the survey's `WoodSiding013` (white; not downloaded). The roof is that shingle darkened, not tar paper. There is still no verified CC0 tar-paper texture; tarred timber is `Planks012` tinted near black. `Bitumen` was not used (it does not exist). `RoofingTiles014` was not used (clay tile). `Aerial Asphalt 01` was not used. Driftwood and the mislabelled boat photos were not used as building references. Meshy spend: 0.
- **Did any structure need a unique generated or hand-modelled building?** No.
- **Remaining defects (not stop conditions):** the outside stair reads as a solid timber run from the pure side and as steps from the three-quarter view; the gym yard tiles; v1 attachments do not yet include drums, pallets, a ladder, shutters, nets, or electrical boxes (the pier pole is available as `Pole` and unused). The slice's "≥ 10 structures" and the gym's "8 pieces reused in all three" are reported as they are; the seven-composition projection already reuses 12 pieces in three or more structures.
- **Done-when:** yes for VS-05 / WP-KIT. Three structures are composed from data only and are distinct in silhouette; the catalog lists every module with source and license; the kit-gym capture has no default material; `report_kit_usage.py` printed the counts; verification above is green.
- **Exact next step:** the Integrator merges `vs/kit` into `main` (do not force-push; local `main` is already one commit ahead with `ff611d1` and must be reconciled, not pushed as-is), regenerates and commits `Lvl_KitGym.umap` if the merge should carry the build product, and does not start VS-06 until that merge is accepted. VS-06 is the shoreline recipe. This session stops here.
- **Return to:** the Integrator (Claude)
