# Development Session Report — Phase 5 (World State), 2026-09-30

The previous report (Presentation Pass, accepted 2026-09-29) is in git history: `git show 437e2d0:Design/CLAUDE_SESSION_REPORT.md`. The pre-critic version of this report is at `git show 6904b68:Design/CLAUDE_SESSION_REPORT.md`.

## Executive Summary

Phase 5 ran from plan to a reviewed, revised, and re-verified candidate in one session. **The Landing Stage is READY FOR ANTHONY (WS-11). Only Anthony accepts it.** Phase 6 is not started.

- **Capability.** `ADCConditionalPresence` is one generic rule: these actors are here, somewhere else, or not here at all, when the shared conditions pass.
  - It saves nothing and snaps silently on load.
  - Otherwise it changes things only when the player is away and not looking.
- **Place.** The Landing Stage is a timber landing in the shallows east of the boathouse door, and it reads Shore Watch from the same spot:
  - **Coil route:** Mara waits on it with her pack, beside a lashed crate and a loaded skiff.
  - **Kill route:** the lantern is out, the crate is emptied, and the skiff drifts far out; Mara stays at her lookout.
  - **Leads pulled at the wreck:** the string of bulbs on the stage goes dark and silent.
- **Independent critics.** Gemini (visual) filed 8 findings and Grok (gameplay) filed 4. Every finding was checked against the captures and the source. Seven were fixed in full and four in part (V-02, V-05, V-06, G-02), and one (V-07) was left alone as outside the POI. Four decisions are parked for Anthony with defaults (below).
- **Evidence.**
  - 42 of 42 tests pass after a full `Tools\RebuildContent.bat`.
  - Same-session frame-time A/B: no regression.
  - The Development Win64 package: see Package / Smoke Test.
  - No `SaveVersion` bump; no new flag, quest, or item; no renamed id; no Meshy credits.

## Starting SHA

`532994d` (narrative docs), with `main` = `origin/main` and 38 of 38 tests. All Phase 5 commits are local and not pushed.

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
| WS-10 Verify and stabilize | done | `6904b68`, this commit |
| WS-11 Anthony's acceptance | **next** | — |

## The Critic Round (WS-08, WS-09)

Full verdicts, each with what was checked: `Design/POIs/reviews/shore.landing_stage_revision.md`.

- **Visual (Gemini).**
  - The drifting skiff had no draft, so it read as hovering; it now sits in the water.
  - The old piles stood short of the waterline; they are now plain piles rising from the lake bed.
  - The stage's own driftwood branch hung over the curb; it is removed.
  - The bulbs were hidden behind the lean-to roof. They now hang along its front eave, where the path sees them, so the power cut reads.
  - The lid was edge-on and clipping. It now leans against the crate, and on the kill route it rests on the boards.
  - The contents are now visible tins and a blanket roll on a folded tarp.
  - The lashing is thicker and darker, and the pack is a duffel with a bedroll.
- **Gameplay (Grok).**
  - **G-01:** Mara's coil payout line no longer promises she will sit up with the coil ("I've got packing to do.").
  - **G-04:** conditional audio now snaps on a load too. This removes a brief hum after F9 on this stage and on the two older hums.
  - **G-03, G-04 (tests):** the tests now check Mara's move from the real walk back, and the power cut crossed with both routes and with a load.
  - **G-02:** the spec now says the scavenger can see the stage on the coil route.
- **Review mechanics.** The visual revision was done on a separate worktree branch while Grok was still reviewing, so the gameplay critic did not read a moving target. The branch was merged afterwards and the worktree removed.

## Verification (after the revision)

- `DeadCurrentEditor` builds.
- A full `Tools\RebuildContent.bat` (all seven scripts) was clean, then `Tools\RunTests.bat -build` passed on the rebuilt content: **42 of 42** (32 editor, 10 map). Imported-binary churn from the rebuild was discarded.
- Captures in `Saved/Review/2026-09-30_1309` are clean: no default materials, missing textures, warnings, errors, or material compile failures.
- **Frame time.** Same-session A/B against the pre-Phase-5 maps (`_1311`): Phase 5 averaged 5.5 ms *lower* on the 24 original views. That is run-to-run noise, as was the earlier +0.36 ms. No regression. Frame times from different days are not comparable on this machine; plan §9 records the method.
- **Old saves.** Hand-written version-5 saves and Anthony's own version-3 save load with the stage in the state their flags imply, undiscovered.

## Package / Smoke Test

`ToolsPackage.bat` after the revision: the Development Win64 cook succeeded to `SavedPackagedWindows`. In the null-RHI smoke launch, `Lvl_Boathouse` and its art level loaded and all 8 presence rules evaluated in the packaged game. Not verified: how it looks or sounds packaged (no renderer, no audio device).

## Parked for Anthony (none blocking; defaults in brackets)

1. **G-02, the scavenger's sight.** On the coil route he is alive, and from his patrol he can see the stage, so standing there can start a chase. Keep that as tension, or move the deck out of his cone? [keep]
2. **V-06, Mara's pack.** The owned library has no backpack mesh. Spend about 30 Meshy credits on one (70 of 500 left)? [keep the duffel stand-in]
3. **V-07, the lookout shed.** The critic called it greybox; it is Presentation Pass content you accepted. [leave for a later Tier B pass]
4. **V-02, the upright driftwood branch** by the path, from the Presentation Pass. Lay it down? [leave]
5. **Not filed, but noted by Grok.** On the coil route the empty lookout's crate still reads "Mara's notebook lies open on the crate", which may now read as left behind. [leave]

## Known Issues

- The frame-time shortfall (about 54 FPS against 60 at the PlayTest settings) is unchanged and deferred.
- Review frames can show the engine's "Preparing Shaders" lines; these are editor-build messages, not game UI.
- Still unverified: how the packaged build looks and sounds (the smoke launch has no renderer and no audio device).
- Unchanged from before: the basin slab's dotted edge; one small eye texture; the stand-in breaker sound; content rebuilds rewrite imported binaries (discarded, not committed).

## READY FOR ANTHONY TO TEST

`WorldStatePhasePlan.txt` §7; the short version is in `Design/ANTHONY_CHECKLIST.md`. Run `Tools\PlayTest.bat`. Your own save loads fine, or delete it for a clean start.

## Next Step

Anthony plays §7 and accepts, or sends findings back (they become revision tasks). Answer the five parked items when convenient. Do not start Phase 6.
