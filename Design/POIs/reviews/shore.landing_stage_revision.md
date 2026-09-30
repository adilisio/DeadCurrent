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

Not in yet. Visual fixes are made in a separate worktree branch (`ws09-revision`) so the gameplay review is not reading a moving target. The branch merges into `main` after the gameplay findings are in.
