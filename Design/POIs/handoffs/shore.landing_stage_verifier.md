# Agent Handoff — shore.landing_stage — Verifier / Integrator

Filled from `AGENT_HANDOFF_TEMPLATE.md`. In this pilot the Phase 5 integrator (Claude) is also the Verifier. Verification here is mechanical (rebuild, tests, captures, package, frame times). It does not judge the stage's quality; that is the critics' job and Anthony's.

## Assigned Role

Verifier / Integrator

## Allowed Scope

- The one hook in `build_boathouse.py` that calls `build_landing_stage.build`
- Review entries in `Tools/Review/Lvl_Boathouse.json` for the spec's views
- The stage's map tests (new file under `Source/DeadCurrent/Save/`)
- The review packet, the checklist, the session report, status lines in the plan and the spec, `technical_architecture.md`
- Running `RebuildContent.bat`, `RunTests.bat`, `ReviewCapture.bat`, `Package.bat`

## Files / Content Owned

As in Allowed Scope.

## Files That Must Not Be Modified

The builders' files (except to integrate a critic's accepted fix through the revision task), shipped ids, the save format, canon.

## Input Specification

- **POI spec:** `Design/POIs/shore.landing_stage.md`; builders' handoff `shore.landing_stage_builders.md`
- **Baseline:** `ea411a2`, 39 tests
- **Approved decisions from Anthony:** `PRODUCTION_PILOT.md` §12

## Expected Deliverables

- Rebuild clean twice; the art layer and `LandingDress` survive
- Full suite green (at least 41)
- A capture run with every landing view, and no default material, missing texture, warning, or error in them
- Frame-time comparison against the Presentation Pass captures (plan §9)
- Development Win64 package and smoke launch
- The review packet for the critics; the checklist and session report current

## Required Tests

`Tools\RunTests.bat -build`, all green.

## Required Review Artifacts

The capture run, the review packet, the manifest's frame times.

## Known Dependencies

WS-04 and WS-05 content in place.

## Stop Conditions

As in the template, and plan §9: a frame-time regression over 1 ms average stops the phase for a look.

## Handoff Notes

- **Status:** not started
- **Exact next step:** after WS-04, add the hook and the tests.
