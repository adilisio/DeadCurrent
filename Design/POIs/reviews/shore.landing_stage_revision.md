# Revision Log — shore.landing_stage (WS-09)

The builder's response to the critics, finding by finding. Each verdict was checked against the captures (`Saved/Review/2026-09-30_1055`) and the scripts before acting. Fixes cite the finding id in their commit. Findings that need a decision go to Anthony and are not decided here.

## Visual Critic (`shore.landing_stage_visual.md`, Gemini)

| Id | Verdict | What was checked | Action |
| --- | --- | --- | --- |
| V-01 skiff floats when adrift | **Accept** (the cause differs a little) | The adrift skiff's lowest vertex sits about 9 cm under the lake sheet, so the hull shows no draft and reads as hovering at distance (`landing_close_combat`). "High in the air" is overstated; the symptom is real. | Give the adrift skiff a real draft (about 25 cm). The moored skiff keeps its height, because a deeper hull would show the lake sheet through its floor from the deck. |
| V-02 driftwood floats | **Accept for the stage's branch; park the other** | The lying branch from `dress_landing.py` crosses the curb with a fork hanging in the air. The upright branch at (1400, −530) is Presentation Pass dressing (`dress_shore.py`), accepted by Anthony on 2026-09-29, and outside this POI. | Remove the stage's branch. The upright branch is parked for Anthony. |
| V-03 posts float | **Accept** | The "old piles" are the pack's multi-pole frame scaled down; some of its poles end above the water. The deck's own piles reach Z −90 and are fine. | Replace the old piles with plain dark piles standing from the lake bed. |
| V-04 bulbs not visible | **Accept** | The bulbs sit on the lean-to's east eave, behind the roof from the door, 7 cm each, and read as nothing in daylight. | Move the string to the lean-to's front eave, facing the path; more and brighter bulbs, so the power cut reads as a change. |
| V-05 lid missing, greybox contents | **Accept in part** | The lid is there, but edge-on and intersecting the crate's front face, so it reads as a slat. The blanket is a flat box; the tins are hidden below the rim. The library has no better supply props. | Lean the lid against the crate's west end with its face toward the gangway; the blanket becomes a roll; the tins sit on it where they can be seen. |
| V-06 pack greybox, no lashing | **Accept in part; park the pack asset** | The lashing exists but is 3 cm thick and as pale as the lid. The owned library has no backpack static mesh (checked; the Megascans sack is 136 MB). A Meshy prop is outside the spec's budget (the pack is Tier C, and Meshy is for Tier A only). | Darker, thicker lashing. The pack becomes a duffel roll with a bedroll, which reads as a bag. Whether to spend about 30 Meshy credits on a real pack is **parked for Anthony**. |
| V-07 lookout shed greybox | **Park for Anthony** (out of the pilot's scope) | The shed is Micro RPG geometry, dressed in the Presentation Pass and accepted by Anthony on 2026-09-29. It is not part of this POI and does not change with state. | No change in Phase 5. A candidate Tier B task (corrugated scrap kit) for later. |
| V-08 lid clips into the boards | **Accept** | The thrown lid rests exactly at the plank-top height with a 4° roll, so one edge dips into the boards. | Rest it flat, 1 cm above the plank tops. |

## Gameplay Critic (`shore.landing_stage_gameplay.md`, Cursor/Grok)

Visual fixes were made on a separate worktree branch (`ws09-revision`) while this review was running, so it was not reading a moving target. That branch was merged into `main` (`82b808a`) once these findings were in.

| Id | Verdict | What was checked | Action |
| --- | --- | --- | --- |
| G-01 reward line promises she stays | **Accept** | `reward_coil` ends "I'm going to sit up with this coil tonight and listen"; minutes later she is packed and waiting at the landing. `done_coil` ("The coil talked all night") still fits, because she listened at the landing. | The closing clause is now "I've got packing to do." PROVISIONAL, no destination. This is the one Mara line the plan allows a critic to change (plan §3). Recorded in the spec. |
| G-02 the scavenger can see the stage | **Accept the spec fix; park the placement for Anthony** | The south-west corner of the patrol is about 11 m from the deck, the deck is inside the 75° cone facing west, and the flat beach blocks nothing. Only on the coil route, where he is alive. | The spec's "Possible danger: none" now names this. Whether to keep the tension or move the deck out of the cone is Anthony's call; default: keep it. |
| G-03 test does not walk the return path | **Accept** | The test released Mara from the Survey Launch beach, far from both bubbles, rather than from the real walk. | After the turn-in, the test now teleports to (1500, 950), west of the ridge and north of the door, outside both 15 m bubbles, and asserts the coil picture there. The "still at the lookout while you talk to her" check is kept. |
| G-04 power cut not crossed with the routes; hum untested | **Accept** | The power cut was only set in the default picture, and the hum's audibility was never read. On a load, `ADCConditionalAudio` played from BeginPlay (flags not restored yet) until its first tick, the same brief leak the relay and live-water hums have. | The test now cuts the power in the coil picture, and saves a combat picture with the power cut and loads it: bulbs dark, hum silent, Mara's and the skiff's rules unchanged, all asserted right after F9. `ADCConditionalAudio` now snaps on `OnRestored` (a loop starts or stops without a fade, a one-shot re-primes and never replays on load), with restore steps added to `DeadCurrent.Presentation.ConditionalAudio`. Generic; it also fixes the two older hums. |

Noted by the Gameplay Critic but not filed, left for Anthony: on the coil route the empty lookout's crate still reads "Mara's notebook lies open on the crate", which may now read as left behind.

## Anthony's answers to the parked items (2026-09-30)

All as the defaults: G-02, keep the tension (the deck stays in the scavenger's sight on the coil route); V-06, keep the duffel stand-in (no Meshy spend); V-07 and V-02, leave the lookout shed and the upright branch; the lookout crate's notebook line, leave it; Mara's payout line "I've got packing to do.", keep it (PROVISIONAL). Nothing further changes in the game for these.
