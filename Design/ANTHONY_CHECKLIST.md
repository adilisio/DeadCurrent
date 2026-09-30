# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- Local `main`, ahead of `origin/main` by all the Phase 5 commits (not pushed; `git log origin/main..main`). Latest: see `git log -1`.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone / Task

**Phase 5, World State: ACCEPTED by Anthony (2026-09-30).** Nothing is in progress. Phase 6 (Vertical Slice) has not been started and needs your go-ahead. Plan: `WorldStatePhasePlan.txt`. Report: `Design/CLAUDE_SESSION_REPORT.md`.

## What Phase 5 Delivered

- **Conditional presence** (`ADCConditionalPresence`): actors present, absent, or at another authored placement, chosen by the shared conditions. Saves nothing; snaps on load; changes only out of sight otherwise.
- **The Landing Stage** (`shore.landing_stage`, the production pilot): the same place reads the Shore Watch outcome. On the coil route Mara waits there with her pack. On the kill route it is stripped and the skiff cut adrift. The bulbs die with the wreck's power.
- **The pilot's workflow, run end to end**: spec, build, two independent critics (Gemini, Grok), revision, verification, your playtests. Record: `Design/POIs/shore.landing_stage.md` (Pilot Record) and `Design/POIs/reviews/`.
- **Acceptance-playtest changes**:
  - The door's exposure jump (a map-rebuild bug that left stacked volumes) is fixed.
  - The coil route has a clear sneak path behind a windbreak, with crate stacks, and crouching matters.
  - The scavenger sees 12 m and warns first; his loop is about 16 m from the door.
  - The coil sits on the relay.
  - Mara is a one-piece placeholder model.
  - Review captures wait for shaders.

## In Progress

Nothing.

## Decisions Needed From Anthony

None open. When you are ready:

- **Choose the next phase.** Phase 6, the Vertical Slice (the lighthouse-in-a-storm settlement, `LongTermPlan.txt` §23), is next on the roadmap. The narrative drafts for it (`Design/Narrative/SLICE_*`) are PROVISIONAL.
- **Later, not blocking:** a real one-piece Mara (her mannequin is a placeholder); the frame-time shortfall (about 54 FPS, deferred); the TERN name board on stakes or on the hull.

## Known Issues

- Frame time: the ~54 FPS shortfall is unchanged and deferred. Phase 5 showed no regression in same-session A/Bs. Frame times from different days do not compare on this machine.
- Mara is a faceless mannequin (PROVISIONAL placeholder). Two generated heads (`mara_head`, `mara_head_collar`) remain in Content, unused.
- Review frames can show the engine's "Preparing Shaders"/"SoundWaves" lines (editor build, not game UI).
- Content rebuilds rewrite imported binaries byte for byte; that churn is discarded, not committed.
- Not verified by ear or eye in the packaged build: audio, and the look of the packaged game (the smoke launch has neither).
- Unchanged: the basin slab's faint dotted edge; one small eye texture; the stand-in breaker sound.

## Automated Status

- Build: `DeadCurrentEditor` builds.
- Tests: **45 of 45** (33 editor, 12 map). Phase 5 started at 38.
- Package: the Development Win64 cook succeeded after acceptance; the smoke launch loaded `Lvl_Boathouse`, evaluated all 8 presence rules, and exited with code 0.
- Meshy: 30 spent in Phase 5 (`mara_head_collar`); 460 of 500 in total; account balance 437.

## Next Autonomous Task

None. Do not start Phase 6 without Anthony's go-ahead. If he gives it, begin with a committed phase plan, as every phase has.
