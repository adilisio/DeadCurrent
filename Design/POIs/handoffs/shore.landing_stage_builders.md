# Agent Handoff — shore.landing_stage — POI Builder, Narrative Builder, Presentation Builder

Filled from `AGENT_HANDOFF_TEMPLATE.md`. In this pilot one agent (Claude, the Phase 5 integrator) fills the three builder roles **in sequence**, because they share one layout and one POI script. Each role's files are still listed separately so a later cell can split them across agents. The builder does not fill a Critic role on this work (see `shore.landing_stage_critics.md`).

Read first: `CLAUDE.md`, `Design/POIs/README.md`, `Design/POIs/shore.landing_stage.md`, `WorldStatePhasePlan.txt`.

## Assigned Role

POI Builder, then Narrative Builder, then Presentation Builder.

## Allowed Scope

- POI Builder: the stage's layout, gameplay actors, presence rules, discovery volume, tackle box, the bulbs and their hum, as written in the spec.
- Narrative Builder: the stage's inspect text (all PROVISIONAL), as text constants in the POI script.
- Presentation Builder: art-level dressing, mesh and material choices for the stage's actors, the migration of the library kit pieces the spec names, provenance rows.

## Files / Content Owned

- POI Builder: `Tools/EditorScripts/build_landing_stage.py` (new); the persistent-map actors it spawns (outliner folder `LandingStage`).
- Narrative Builder: the `TEXT_*` constants at the top of `build_landing_stage.py`.
- Presentation Builder: `Tools/EditorScripts/dress_landing.py` (new; art-level actors tagged `LandingDress`); a generic pack-migration tool (`Tools/ImportPackAssets.ps1` and its Python helpers, new); the migrated assets under `Content/Smugglers_cove/` and `Content/Scene_Junkyard/` (only the closure of the pieces the spec names); new provenance rows in `Design/art_pipeline.md`.

## Files That Must Not Be Modified

- `Source/**` (WS-02 is done; any further C++ is a Systems Engineer handoff)
- `Tools/EditorScripts/build_boathouse.py`, except the one hook the Integrator adds at the end of `main()`
- `create_items.py`, `create_quest.py`, `create_dialogue.py`, and every existing dialogue, quest, and item asset
- `Source/**/DCSaveGame*`, the save version, every shipped id, flag, and stage
- `Design/world_bible.md` canon, `CLAUDE.md`, `LongTermPlan.txt`, the phase plan (the Integrator updates status)
- `Tools/Review/*.json` (the Verifier / Integrator owns review entries)

## Input Specification

- **POI spec:** `Design/POIs/shore.landing_stage.md` at the WS-03 commit
- **Upstream:** `ADCConditionalPresence` (WS-02, `ea411a2`)
- **Baseline:** `ea411a2`, 39 tests (32 editor, 7 map); last capture run `Saved/Review/2026-09-29_2246`
- **Approved decisions from Anthony:** `PRODUCTION_PILOT.md` §12 (cell, capability, Mara moves on the coil route, runs in Phase 5); Phase 5 go-ahead 2026-09-30.

## Expected Deliverables

- The stage in all four states, built by script, surviving `Tools\RebuildContent.bat`. Done when setting the flags in play shows each state.
- The inspect text for every variant. Done when no variant is blank and nothing explains the Current or decides Mara.
- Dressing and kit meshes in place of greybox where the review views show greybox. Done when the captures show no default or grid material and nothing floats.

## Required Tests

- `Tools\RunTests.bat` (with `-build` after any C++ change): at least 39 before WS-06, at least 41 after.
- New (written by the Integrator in WS-06): `DeadCurrent.Map.Boathouse.LandingStage`, `DeadCurrent.Map.Boathouse.LandingStageSaves`.

## Required Review Artifacts

Captures of the spec's review views (WS-07), provenance rows for migrated assets, Meshy credits spent (planned: 0), the checklist entry.

## Known Dependencies

`ADCConditionalPresence` (done). `import_art.py` has imported the PolyHaven crate and lantern. The Survey Launch's `wreck.power_cut` drives the bulbs.

## Stop Conditions

As in the template. In particular: any new C++, any shipped id change, any lore beyond the spec's PROVISIONAL list, any Meshy spend.

## Handoff Notes

- **Status:** in progress (WS-04)
- **Commits:** see `git log --grep "WS-0[45]"`
- **What changed:** see the commits
- **Findings (critics):** not the builder's to write
- **Tests run and results:** recorded in `Design/ANTHONY_CHECKLIST.md`
- **Open issues:** recorded in `Design/ANTHONY_CHECKLIST.md`
- **Exact next step:** write `build_landing_stage.py` and its hook, rebuild, run the suite.
- **Return to:** Verifier / Integrator, then the critics.
