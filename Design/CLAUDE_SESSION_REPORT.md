# Development Session Report — Phase 5 (World State), ACCEPTED 2026-09-30

Earlier reports are in git history:
- The Presentation Pass: `git show 437e2d0:Design/CLAUDE_SESSION_REPORT.md`.
- Phase 5 before the critics: `git show 6904b68:Design/CLAUDE_SESSION_REPORT.md`.
- Phase 5 handed to Anthony: `git show 397be89:Design/CLAUDE_SESSION_REPORT.md`.

## Executive Summary

**Anthony accepted Phase 5 (World State) on 2026-09-30**, after the critic round and two rounds of acceptance playtests. WS-00..WS-11 are done. Phase 6 is not started.

- **Capability.** `ADCConditionalPresence` is one generic rule: these actors are here, somewhere else, or not here at all, when the shared conditions pass.
  - It saves nothing and snaps silently on load.
  - Otherwise it changes things only when the player is away and not looking.
  - `ADCConditionalAudio` now snaps on load the same way.
- **Place.** The Landing Stage (`shore.landing_stage`, the production pilot) reads Shore Watch from the same spot:
  - **Coil route:** Mara waits on it with her pack, beside a lashed crate and a loaded skiff.
  - **Kill route:** the lantern is out, the crate is emptied, and the skiff drifts far out.
  - **Power cut at the wreck:** the stage's bulbs go dark.
- **The pilot's workflow** ran end to end: spec, build, two independent critics (Gemini: 8 findings; Grok: 4), revision, verification, Anthony's playtests. Record: `Design/POIs/shore.landing_stage.md` (Pilot Record) and `Design/POIs/reviews/`.
- **Evidence.**
  - 45 of 45 tests pass.
  - Same-session frame-time A/B: no regression.
  - The Development Win64 package cooks and smoke-launches.
  - No `SaveVersion` bump; no new flag, quest, or item; no renamed id. Old saves, including Anthony's own, load.

## Starting SHA

`532994d`, with `main` = `origin/main` and 38 of 38 tests. All Phase 5 commits (40 so far) are local and not pushed. (Correction, VS-00, 2026-09-30: they were pushed. After `git fetch`, `origin/main` matched `9640c05`.)

## Phase 5 Progress

| Task | State | Commits |
| --- | --- | --- |
| WS-00 Plan | done | `5144257` |
| WS-01 Reconcile stale status | done | `437e2d0` |
| WS-02 Conditional presence + test | done | `ea411a2` |
| WS-03 Spec + handoffs | done | `bfc9a71` |
| WS-04 Build the stage | done | `53c9e36` |
| WS-05 Presentation | done | `2538f27` |
| WS-06 Stage tests | done | `5e44416`, `463230b` |
| WS-07 Review views + packet | done | `937f86c` |
| WS-08 Independent critics | done (Gemini, Grok; run by Anthony) | `d457e07`, `e89ade8` |
| WS-09 Revision | done | `82b808a`, `3cb9865`, `18fa58c`, `5fd47b9`, `1803d5f` |
| WS-10 Verify and stabilize | done | `6904b68`, `397be89` |
| WS-11 Anthony's acceptance | **accepted** | `7e3bd90`..`c3e4043`, this commit |

## Acceptance Playtests (WS-11)

Anthony's findings and what changed. Each has a test or a review view.

| Finding | Change |
| --- | --- |
| Mara looked wrong in open light on the stage | Toned-down jacket, matte face, and a close-up review view (`81202dc`). |
| Leaving the boathouse, the lighting jumped | Each map rebuild had left a stacked copy of every volume, so the interior grade doubled at the door. Rebuilds now clear old volumes, and the grade eases over the last steps (`44f9eb7`, `160910f`). |
| The coil could hardly be taken without a fight | Crouching now matters: he notices a crouched player only within 8 m and 45°. There are crate stacks for cover, and a scrap windbreak gives a clear sneak path. Tests show the windbreak blocks his sight from his whole south leg (`efde3cf`, `63f342c`, `160910f`, `851b901`). |
| He saw the player at the door and charged at once | His standing sight is 12 m, and his loop is moved east, about 16 m from the door (`27245b8`). |
| Mara read as a head floating on a coat | A lower seat and then a new generated head (30 Meshy credits) did not solve it. She is now one single model: the one-piece mannequin in matte teal and dark brown, a PROVISIONAL placeholder (`58d81b1`, `14c8e0a`, `dba3476`). |
| The relay coil floated | It rests on top of the relay housing (`2eac178`). |
| Nothing told the player to avoid him rather than fight | When he spots the player from his patrol, he stops and warns them to turn around (PROVISIONAL line). He charges only if they come within 6 m or shoot. Out of sight, he goes back to his patrol. There is a map test (`456ae1a`). |

Anthony's answers to the five parked items were all the defaults (`7e3bd90`). One side effect is recorded in the spec: with his loop moved east, the scavenger can no longer see the stage, so the G-02 tension Anthony chose to keep is gone. Re-adding it would be a Phase 6 layout question.

Review captures now wait for shader compilation, so a rebuilt material is never judged or timed on its grey fallback (`d9bde10`).

## Verification

- `DeadCurrentEditor` builds.
- `Tools\RunTests.bat`: **45 of 45** (33 editor, 12 map). New since the handoff:
  - `DeadCurrent.AI.ScavengerNotice`
  - the `CampCover` map test
  - the `ScavengerWarning` map test
- Frame time: the same-session A/B method is in plan §9. Phase 5 showed no regression.
- Old saves: hand-written version-5 saves and Anthony's own save load with the stage in the state their flags imply.

## Package / Smoke Test

`Tools\Package.bat` after acceptance: the Development Win64 cook succeeded to `Saved\Packaged\Windows`. In the null-RHI smoke launch (`Saved/Logs/PackageSmoke.log`), `Lvl_Boathouse` loaded, all 8 presence rules evaluated, and the run exited with code 0. The only failed loads are optional profiler DLLs. Not verified: how the packaged game looks or sounds (no renderer, no audio device).

## Meshy

30 credits in Phase 5 (`mara_head_collar`, now unused). The project total is 460 of 500, and the account balance is 437. Details are in `Design/art_pipeline.md`.

## Known Issues

- The frame-time shortfall (about 54 FPS against 60 at the PlayTest settings) is unchanged and deferred.
- Mara is a faceless mannequin placeholder. Two generated heads remain in Content, unused.
- Review frames can show the engine's "Preparing Shaders" lines; these are editor-build messages, not game UI.
- Unchanged from before:
  - the basin slab's dotted edge
  - one small eye texture
  - the stand-in breaker sound
  - content rebuilds rewrite imported binaries (discarded, not committed)

## Next Step

Anthony chooses the next phase. Phase 6, the Vertical Slice (`LongTermPlan.txt` §23), is next on the roadmap and **is not started**. If he gives the go-ahead, it begins with a committed phase plan.
