# Agent Handoff — <POI id or task> — <role>

Copy this file into the task prompt or to `Design/POIs/handoffs/<poi_id>_<role>.md`. It is a contract between people and agents, not software. Fill every section. An agent that receives a handoff with a blank section stops and asks for it instead of guessing.

Read first: `CLAUDE.md`, `Design/POIs/README.md`, the POI's spec (`Design/POIs/<poi_id>.md`).

---

## Assigned Role

Exactly one of:

- World Architect
- POI Builder
- Narrative Builder
- Presentation Builder
- Systems Engineer
- Visual Critic
- Gameplay Critic
- Verifier / Integrator

A builder never fills a Critic or Verifier role on its own work.

## Allowed Scope

What this handoff may do, in one short list. Anything not listed is out of scope. If the work seems to need something not listed, stop and write it down (see Stop Conditions).

- 

## Files / Content Owned

Only these may be created or edited. Use exact paths, folders, sublevels, and data-asset names.

- 

## Files That Must Not Be Modified

Always include the shared, integration-sensitive files unless this role owns them:

- `Source/**` (unless the role is Systems Engineer and the handoff says which files)
- `Tools/EditorScripts/build_boathouse.py` and any other shared generated-map script that another owner is editing
- `Tools/EditorScripts/create_items.py`, `create_quest.py`, `create_dialogue.py` shared sections
- `Source/**/DCSaveGame*` and the save version
- shipped persistent ids, flags, quest stages, and saved asset names
- `Design/world_bible.md` canon (provisional additions go in the POI spec, marked PROVISIONAL)
- `CLAUDE.md`, `LongTermPlan.txt`, and phase plans
- any file listed as owned by another active handoff

## Input Specification

- **POI spec:** `Design/POIs/<poi_id>.md`, at commit `<sha>`
- **Upstream handoffs / artifacts:** (captures, critiques, asset lists this role consumes)
- **Baseline:** commit `<sha>`, test count `<n>`, review capture run `<path>`
- **Approved decisions from Anthony:** (quote them; link the checklist entry)

## Expected Deliverables

Concrete and checkable. "Done when" for each.

- 

## Required Tests

Which suites must be green before handoff, with the baseline count. New tests that must exist.

- `Tools\RunTests.bat` (add `-build` after C++ changes): at least `<n>` tests passing
- 

## Required Review Artifacts

- Fixed review captures for the views in the POI spec, written to `Saved/Review/<run>/`
- Written findings against each view's expectation (pass / defect, with the file and view id)
- Provenance rows for every imported or generated asset, and Meshy credits spent
- Updated `Design/ANTHONY_CHECKLIST.md` entry

## Known Dependencies

- Content or ids this work needs that another owner is creating
- Order constraints (which handoff must land first)
- Assets that must exist in `C:\FO5_AssetLibrary` or `Content/`

## Stop Conditions

Stop, write the reason at the top of Handoff Notes, and return control when any of these is true:

- the work needs a new C++ system, condition or consequence type, or save field the spec does not list
- a file outside Files / Content Owned would have to change
- a persistent id, flag, or quest stage would have to be renamed
- a lore claim beyond the spec is needed (mark PROVISIONAL in the spec and ask; do not decide it)
- a test that was passing at baseline fails and the cause is not this work
- the Meshy budget in the spec would be exceeded
- a visual change seems to need a gameplay change
- the tree contains uncommitted edits in files this handoff does not own
- usage is running low: finish or revert the smallest unit, verify, record the exact next step, stop

## Handoff Notes

Filled in by the receiving agent when it stops or finishes.

- **Status:** not started / in progress / blocked / complete
- **Commits:** 
- **What changed:** 
- **Findings (critics):** 
- **Tests run and results:** 
- **Open issues:** 
- **Exact next step:** 
- **Return to:** (role or Anthony)
