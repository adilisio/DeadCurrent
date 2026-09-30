# Review Packet — shore.landing_stage (Landing Stage)

For the **Visual Critic** and the **Gameplay Critic** (`Design/POIs/handoffs/shore.landing_stage_critics.md`). Run them independently. This packet holds what to look at and what to answer. It deliberately does not contain the builder's opinion of the work.

## What is under review

- **Spec:** `Design/POIs/shore.landing_stage.md`
- **Phase plan:** `WorldStatePhasePlan.txt` (§2 the state table, §3 scope, §7 the human walkthrough, §9 performance)
- **Commit:** `2538f27` (WS-05). The review views and this packet were committed right after it (WS-07); nothing in the stage changed between them.
- **Test baseline at that commit:** 41 of 41 pass (32 editor, 9 map), including `DeadCurrent.World.ConditionalPresence`, `DeadCurrent.Map.Boathouse.LandingStage`, and `DeadCurrent.Map.Boathouse.LandingStageSaves`.
- **Capture run:** `Saved/Review/2026-09-30_1055/` (PNGs named by view id, `contact_sheet.png`, `manifest.json` with each view's setup, expectation, camera, frame time, and automatic checks)
- **Test logs:** `Saved/Logs/RunTests.log`, `Saved/Logs/RunTests_Map.log`

Captures are taken at the PlayTest settings (DX11, Low scalability, 70% screen percentage, no Lumen), so lighting is flatter than a high-end machine would show. Judge readability, not fidelity. The white "Preparing Shaders" and "Preparing SoundWaves" lines at the top left are the engine's editor-build messages, not game UI; ignore them. Frame times in the manifest are not comparable with earlier runs on another day (see the checklist); do not judge performance from them.

## The claim to test

From one place, the player can see what they did on Shore Watch, without reading anything:

| State | Flag (first match wins) | Should read as |
| --- | --- | --- |
| Before | none | a landing someone is getting ready to use: lantern lit, skiff alongside, crate half packed |
| Coil route | `shore.relay_recovered` | the same landing, ready to leave, and Mara standing on it with her pack |
| Combat route | `shore.path_cleared` | the same landing shut down and stripped: no light, the skiff far out, the crate open and empty |
| Power cut | `wreck.power_cut` (additive) | the bulbs along the stage dark |

## Where to look

| Files | What they are |
| --- | --- |
| `Tools/EditorScripts/build_landing_stage.py` | every gameplay actor, presence rule, and inspect text on the stage |
| `Tools/EditorScripts/dress_landing.py` | the art-level dressing (NoCollision) |
| `Tools/EditorScripts/build_boathouse.py` (`build_landing_stage_poi`) | the one hook that calls the POI script |
| `Source/DeadCurrent/World/DCConditionalPresence.*` | the Phase 5 capability the stage uses |
| `Source/DeadCurrent/Save/DCLandingStageMapTest.cpp` | the stage's map tests |
| `Tools/Review/Lvl_Boathouse.json` | the views (ids starting `landing_`, plus `lookout_coil`) and their expectations |

## Views (compare in these groups)

- `landing_far`, `landing_coil`, `landing_combat`, `landing_power_cut`: one camera, four states
- `landing_close`, `landing_close_coil`, `landing_close_combat`: one camera on the crate, three states
- `landing_adrift`: the skiff after the kill route
- `lookout_coil` against `lookout`: Mara's lookout with and without her

## Questions for the Visual Critic

1. For each landing view: does it meet its written expectation (in `manifest.json`)? Pass or defect, one line each.
2. In the four-state group, can you tell the states apart in one glance, without the setup label? Which pair is weakest, and what single visual change would separate them most?
3. Does anything read as greybox, floating, clipped into something, or at the wrong scale (skiff, crate, lantern, Mara on the deck, the pack)?
4. Does the stage read as a place people made and use, or as props set down on the shore?
5. Does the combat state read as a consequence (someone closed this) rather than a different texture?
6. Is anything too bright, too loud in colour, or off the shore's palette (`Design/art_pipeline.md` art direction)?
7. Anything in the frame that is not part of the stage but now looks wrong because of it?

## Questions for the Gameplay Critic

1. Do the rules in `build_landing_stage.py` implement the state table exactly, including "kill, then hand over the coil" staying the combat picture and "killed but not yet told Mara" staying the default?
2. Can the player get stuck, fall, or be blocked on the gangway, the deck, or around the crate and Mara's coil-route spot? Is anything with collision on the lake-side walk, the review route, the door apron, or the scavenger's patrol square?
3. Is every inspectable reachable and its text right in every state? Can a hidden inspectable (the card, the cut line) still be interacted with?
4. Save/load: do the tests actually prove "save → diverge → F9 → restored at once", and would a save from before Phase 5 load correctly? Anything they miss?
5. Mara: can she vanish or appear in front of the player? Is there a way to catch her mid-move? Does anything about her new spot break her dialogue, the scavenger's behaviour, or the coil route?
6. Does the stage add anything that saves on its own, or any new flag, id, or save field beyond `shore.landing_stage` and `landing.tackle`?
7. Does the inspect text explain the Current, decide a faction, or decide who Mara is or where she is going? (It must not.)
8. What would you expect Anthony to notice first that the tests would not?

## Where to write findings

- Visual Critic: `Design/POIs/reviews/shore.landing_stage_visual.md`
- Gameplay Critic: `Design/POIs/reviews/shore.landing_stage_gameplay.md`

Format: see the critics' handoff (id, severity, view or file and line, what, why, suggested fix; then a passes list; then answers to the questions above).
