# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- Local `main`, ahead of `origin/main` by the Phase 5 commits (not pushed; `git log origin/main..main`). Latest: see `git log -1`.
- The revision worktree (`C:\deadcurrent-ws09`, branch `ws09-revision`) was merged and removed.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone / Task

**Phase 5, World State: the Landing Stage is READY FOR YOU (WS-11).** It is built, reviewed by both independent critics, revised on their findings, and re-verified. Only you accept it. Plan: `WorldStatePhasePlan.txt`. Report: `Design/CLAUDE_SESSION_REPORT.md`.

| Task | State |
| --- | --- |
| WS-00..WS-07 | done (plan, capability, spec, stage, presentation, tests, review packet) |
| WS-08 Independent critics | done: Gemini (visual, 8 findings), Grok (gameplay, 4 findings) |
| WS-09 Revision | done: every verdict is in `Design/POIs/reviews/shore.landing_stage_revision.md` |
| WS-10 Verify and stabilize | done: full rebuild, 42 of 42, same-session A/B, package |
| **WS-11 Your acceptance** | **next** |

## Completed Since Last Update

- **Visual fixes (Gemini):**
  - The drifting skiff sits in the water.
  - The old piles rise from the lake bed.
  - The stage's own floating branch is removed.
  - The bulbs hang along the lean-to's front eave, where the path sees them, so the power cut reads.
  - The lid leans against the crate, and rests on the boards on the kill route.
  - The contents are visible tins and a blanket roll.
  - The lashing is thicker and darker; the pack is a duffel with a bedroll.
- **Gameplay fixes (Grok):**
  - Mara's coil payout line changed (below).
  - Conditional audio snaps on a load, so there is no stray hum after F9, here or at the relay and the wreck.
  - The tests walk the real way back from the lookout, and cross the power cut with both routes and a load.
  - The spec now says the scavenger can see the stage.

## In Progress

Nothing half-done in the tree.

## READY FOR ANTHONY TO CHECK

Run `Tools\PlayTest.bat`. Your own save (version 3) loads fine with the stage in its default state; delete `Saved\SaveGames\DeadCurrent.sav` for a clean start.

1. **Before.** Step out of the door and look right.
   - Should happen: a skiff alongside, a lantern lit on a post, a string of bulbs under the lean-to's eave, a half-packed crate with its lid leaning on it. Walking on shows `LOCATION DISCOVERED / Landing Stage`.
   - Ask: does it read as a place someone is getting ready to use?
2. **Coil route.** Take the coil and hand it to Mara.
   - Should happen: her payout line now ends "I've got packing to do." Nothing moves while you talk. Walk back: Mara is on the stage with her pack, the crate is closed and lashed, the skiff is loaded, and her lookout is empty.
   - Ask: did you notice before you were told? Did the scavenger spot you on the stage, and if so, is that tension or a nuisance? (Parked item 1.)
3. **Save, diverge, load.** F5 on the stage, walk away, pick something up, F9.
   - Should happen: everything is as saved, at once, with no second banner and no brief hum.
4. **Combat route.** New game or an earlier save. Kill the scavenger and tell Mara.
   - Should happen: at the landing the lantern is out, the card gone, the crate open and empty with its lid on the boards, and the skiff far out on the water. Mara is still at her lookout.
   - Ask: does it read as a consequence rather than a texture swap?
5. **Power.** Pull the leads at the Survey Launch.
   - Should happen: the bulbs at the landing go dark and stop humming.
6. **Overall.**
   - Ask: is anything floating, fake, in the way, or off the shore's look?

## Your Playtest Findings (WS-11)

- **2026-09-30, coil route: "Mara looks weird at the docks."** Compared from the same framing at the lookout and on the stage (review views `mara_face`, `mara_face_stage`, run `Saved/Review/2026-09-30_1334`), the model and head fit are identical. The move broke nothing. The difference is light: the shed's shade had hidden how she reads under open sky. Changed: her jacket tint was tuned for the shade and washed out to near-white in the open, so it is now about 57% of that and still reads teal in the shed; her face has a higher roughness floor so it reads less glossy. Still visible in full light: warm skin and the line where the generated head meets the body's neck. **Question for you below.**

- **2026-09-30: "Leaving the first building, the lighting changes a lot."** Cause: the boathouse's interior exposure grade (Presentation Pass) switched within one stride at the door. Fixed (`44f9eb7`): it now eases off over the last 2.5 m inside and is gone about 0.5 m outside the door. The interior itself looks the same. **Recheck:** walk out of the door slowly.
- **2026-09-30: "Almost impossible to grab the relay coil without the scavenger chasing you."** Cause: he saw you anywhere within 18 m and a 75° cone on open beach, and crouching did nothing, although Mara says "Stay low, time his walk". You chose both fixes:
  - `efde3cf`: crouched (Ctrl or C), he notices you only within 8 m and a 45° cone in front of him. Once he is chasing, crouching does not shake him.
  - `63f342c`: three crate stacks at the camp, all outside his patrol loop. One stands west of the camp, north of the path. Two stand on the beach just south of his south leg, the nearer one right below the coil.
  - **Recheck:** crouch, work from stack to stack, grab the coil when he is walking away. Is it now possible but still tense?
- Tests: 44 of 44 (33 editor, 11 map), with the new `AI.ScavengerNotice` and `Map.Boathouse.CampCover`.

## Decisions Needed From Anthony

- **What reads weird about Mara on the stage?** The head's size or shape, the join at the jaw, the skin tone, the jacket, or the pose? Tell me which and I will target it. A cheap option: stand her in the lean-to's shade instead of the open deck. [until you say: the tint and roughness changes above only]


None open from Phase 5. **Answered by Anthony (2026-09-30), all as the defaults:**

1. **The scavenger's sight (G-02):** keep the tension. The stage stays where it is.
2. **Mara's pack (V-06):** keep the duffel. No Meshy spend.
3. **The lookout shed (V-07) and the upright driftwood branch (V-02):** leave them.
4. **The lookout crate's notebook line on the coil route:** leave it.
5. **Mara's payout line "I've got packing to do.":** keep it (still PROVISIONAL lore).

Made inside the approved pilot (unchanged, defaults in the spec's Open Creative Decisions):
- The stage stands east of the door.
- Killing the scavenger and then handing over the coil stays the combat picture.
- Killing him without telling Mara leaves the stage as it was.
- Who stripped the landing on the kill route is left unstated.

Still open from before: the frame-time shortfall (deferred); the TERN name board on stakes or on the hull.

## Known Issues

- Frame time: the ~54 FPS shortfall is unchanged and deferred. Phase 5's same-session A/B shows no regression (plan §9). Frame times from different days do not compare on this machine.
- Review frames can show the engine's "Preparing Shaders" lines (editor build, not game UI).
- Not verified: how the packaged build looks and sounds.
- Unchanged: the basin slab's dotted edge; one small eye texture; the stand-in breaker sound; rebuild churn in imported binaries (discarded).

## Automated Status

- Build: `DeadCurrentEditor` builds.
- Tests: **42 of 42** (32 editor, 10 map) after a full `Tools\RebuildContent.bat`. Phase 5 baseline was 38.
- Captures: `Saved/Review/2026-09-30_1309` are clean.
- Package: after the revision, the Development Win64 cook succeeded, and the smoke launch loaded `Lvl_Boathouse` with all 8 presence rules running.
- Meshy spend: none in Phase 5 (still 430 of 500).

## Next Autonomous Task

None until you play. Your findings become revision tasks (same loop: triage into the revision log, fix, re-verify). Answers to the parked items become small tasks. Do not start Phase 6.
