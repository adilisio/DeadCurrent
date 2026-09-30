# Development Session Report — Phase 5 (World State), 2026-09-30

The previous report (Presentation Pass, accepted 2026-09-29) is in git history: `git show 437e2d0:Design/CLAUDE_SESSION_REPORT.md`.

## Executive Summary

Phase 5 started on Anthony's go-ahead with a committed plan (`WorldStatePhasePlan.txt`) and ran WS-00 to WS-07 in one session:

- **One reusable capability: conditional presence** (`ADCConditionalPresence`). It is a rule that says "these actors are here, somewhere else, or not here at all, when these shared conditions pass". It saves nothing: its result is recomputed from flags that are already saved, so old saves need no migration. It snaps silently on load. Otherwise it changes only when the player is away and not looking, so Mara never vanishes mid-conversation.
- **One place that uses it: the Landing Stage** (`shore.landing_stage`), the approved production pilot. It is a timber landing in the shallows just east of the boathouse door. Resolve Shore Watch and it reads differently from the same spot:
  - Coil route: Mara is standing on it with her pack, the crate lashed, the skiff loaded.
  - Kill route: lantern out, the crate emptied, the skiff drifting far out, Mara still at her lookout.
  - The bulbs go dark when the Survey Launch's leads are pulled.
- **42 automated tests pass** (38 at the start): the capability test, the stage through both routes with save, diverge, and F9, hand-written pre-Phase-5 saves, and Anthony's own version-3 save from 2026-09-28.
- Same-session frame-time A/B: **+0.36 ms**, inside the noise. No SaveVersion bump, no new flag, quest, item, or dialogue, no renamed id, no Meshy credits spent.

The stage is a **CANDIDATE**, not accepted: the independent critics (WS-08) have not run. Those are for Anthony to launch with Gemini and Cursor/Grok (below).

## Starting SHA

`532994d` (narrative docs), `main` = `origin/main`, 38 of 38 tests. Phase 5 commits are local and not pushed.

## Phase 5 Progress

| Task | State | Commit |
| --- | --- | --- |
| WS-00 Plan | done | `5144257` |
| WS-01 Reconcile stale status | done | `437e2d0` |
| WS-02 Conditional presence + test | done | `ea411a2` |
| WS-03 Spec + handoffs | done | `bfc9a71` |
| WS-04 Build the stage | done | `53c9e36` |
| WS-05 Presentation | done | `2538f27` |
| WS-06 Stage tests | done | `5e44416`, `463230b` |
| WS-07 Review views + packet | done | `937f86c` |
| WS-08 Independent critics | **waiting on Anthony** | — |
| WS-09 Revision | after WS-08 | — |
| WS-10 Verify and stabilize | verification run early (rebuild, suite, captures, A/B, package); docs this commit | this commit |
| WS-11 Anthony's acceptance | after WS-09 | — |

## What Was Built

**Capability** (`Source/DeadCurrent/World/DCConditionalPresence.*`): targets are actors in the same level. There is an ordered list of states `{StateId, Conditions, bPresent, bMove, Placement}`, and the first state that passes wins. With no state passing, the targets stay where they were authored. Hidden means hidden in game with collision off. The rule evaluates at BeginPlay, on the world-state signals, and every 0.5 s. It defers a change while the player is within 15 m of the current or new place, or while a target was on screen. It snaps on `UDCWorldStateSubsystem::OnRestored`, which F9 and the review capture now fire. Documented in `technical_architecture.md` (Conditional presence).

**Stage** (`Tools/EditorScripts/build_landing_stage.py`, one hook in `build_boathouse.py`): the pieces, all driven by 8 presence rules:
- gangway, deck, lean-to
- storm lantern and card
- crate with a lid that takes three positions
- tins and blanket, lashing
- skiff and its cargo, mooring line and cut line
- bulbs and their hum
- tackle box (`landing.tackle`), discovery volume
- Mara's pack

Its dressing is 6 NoCollision pieces in the art level (`dress_landing.py`). Spec: `Design/POIs/shore.landing_stage.md`, with the pilot record at the end.

**Pipeline**: `Tools\ImportPackAssets.ps1` is the Survival_Character migration made generic. It brought the `Smugglers_cove` rowing boat and pier planks in at 1K (21 MB), and it refuses high-poly scans.

## Decisions Made Inside the Approved Pilot (all reversible)

1. **Where the stage stands.** It is east of the door, not "between the camp and the lookout": there is no water there, and on the coil route the scavenger is alive in that stretch.
2. **Combat is checked before coil.** Kill him, then hand over the coil anyway, and the drifted skiff does not come back.
3. **Killing him without telling Mara** leaves the stage as it was: Shore Watch is not resolved.
4. **Mara's lookout on the kill route** does not change in the world. Her existing greeting and the lookout crate already read the outcome.
5. **No new dialogue.** Her existing lines do not assume where she stands.
6. **Changes happen out of sight;** a load shows the saved state at once.

All are listed with defaults in the spec's Open Creative Decisions.

## Verification

- `DeadCurrentEditor` builds. `Tools\RunTests.bat`: **42 of 42** (32 editor, 10 map).
- `Tools\RebuildContent.bat build_boathouse` is clean, run five times. `dress_landing` is clean. The art layer keeps its sentinel, its audio beds, and the `LandingDress` actors (asserted in `LandingStage`).
- Captures: final run `Saved/Review/2026-09-30_1055`. The nine landing views are written, with no default materials, missing textures, warnings, errors, or material compile failures.
- **Frame time.** The machine was about 11.7 ms slower today than last night on the unchanged pre-Phase-5 map, so yesterday's numbers cannot be compared. A same-session A/B (the pre-Phase-5 maps against the Phase 5 maps, back to back) gives **+0.36 ms** on the 24 original views. Runs vary by ±5 ms per view; a later run came in 2.3 ms under. No material regression.
- Old saves: `LandingStageSaves` (version 5, coil and kill) and `LandingStagePlayerSave` (your real version-3 save, read only) show the right state, with the stage undiscovered. The existing `LegacySave` test is unchanged and passes.
- Package: see Package / Smoke Test.

## Package / Smoke Test

`ToolsPackage.bat`: the Development Win64 cook succeeded, packaged to `SavedPackagedWindows`. The cook compiled the migrated pack's shaders for the packaged target. In the null-RHI smoke launch, `Lvl_Boathouse` and its art level loaded, and all 8 presence rules evaluated in the packaged game (log lines `[DCPRESENCE]`). Not verified: how it looks or sounds in the packaged build (the smoke launch has no renderer and no audio device).

## Known Issues

- **The frame-time shortfall** (about 54 FPS against 60 at the PlayTest settings) is unchanged and still deferred. Today's machine ran slower still; comparing across days needs a same-session A/B (plan §9).
- **The review frames show the engine's "Preparing Shaders" lines** at the top left: editor-build messages, not game UI.
- **Things for the critics to judge, not pre-judged here:** the plank decking's pale colour; whether the power-cut view differs enough in daylight; the pack and lashing as flat-colour boxes.
- Unchanged from before: the basin slab's dotted edge; one small eye texture; the stand-in breaker sound; packaged audio not verified by ear; content rebuilds rewrite unrelated material instances byte-for-byte (discarded, not committed).

## READY FOR ANTHONY TO TEST

The stage is playable now. The walkthrough is `WorldStatePhasePlan.txt` §7; `Design/ANTHONY_CHECKLIST.md` has the short version. It is worth running the critics first (below), so your time goes on how it feels.

## Next Step

1. **You** run the two critics (WS-08). Hand each agent `Design/POIs/handoffs/shore.landing_stage_critics.md` and `Design/POIs/reviews/shore.landing_stage_REVIEW_PACKET.md`; the captures are in `Saved/Review/2026-09-30_1055/`. They write `Design/POIs/reviews/shore.landing_stage_visual.md` and `..._gameplay.md`.
2. The builder revises on their findings only (WS-09), then re-verifies (WS-10).
3. You play §7 and accept or send it back (WS-11). Do not start Phase 6.
