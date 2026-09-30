# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- Local `main`, ahead of `origin/main` by the Phase 5 commits (not pushed; `git log origin/main..main`). Latest: see `git log -1`.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone / Task

**Phase 5, World State: the Landing Stage is a CANDIDATE, waiting for the independent critics (WS-08), which you run.** Plan: `WorldStatePhasePlan.txt`. Session report: `Design/CLAUDE_SESSION_REPORT.md`.

| Task | State |
| --- | --- |
| WS-00 Plan | done (`5144257`) |
| WS-01 Reconcile stale status | done (`437e2d0`) |
| WS-02 Conditional presence capability + test | done (`ea411a2`) |
| WS-03 Landing Stage spec + handoffs | done (`bfc9a71`) |
| WS-04 Build the stage | done (`53c9e36`) |
| WS-05 Presentation | done (`2538f27`): library rowing boat and pier planks, brighter lantern, dressing |
| WS-06 Stage tests | done (`5e44416`, `463230b`) |
| WS-07 Review views + review packet | done (`937f86c`) |
| **WS-08 Independent critics** | Visual (Gemini) **in** and triaged (`d457e07`); Gameplay (Grok) **still running** |
| WS-09 Revision | visual findings fixed on branch `ws09-revision` (`dccd987`, worktree `C:deadcurrent-ws09`), **not merged** until Grok finishes; gameplay findings next |
| WS-10 Verify and stabilize | verification done early (42 tests, rebuilds, captures, A/B, package); re-run after WS-09 |
| WS-11 Your acceptance | after WS-09 |

## Completed Since Last Update

- The whole build side of the pilot: `ADCConditionalPresence`, the Landing Stage, its dressing, its tests, its review views, and a generic pack-migration tool (`Tools\ImportPackAssets.ps1`).
- Docs: `technical_architecture.md` (capability, stage, ids, scripts, tests), `game_design.md` (decisions), `world_bible.md` (PROVISIONAL lines only), `art_pipeline.md` (provenance, the migration tool), the spec's pilot record, and the plan's status and §9 measurements.

## In Progress

The visual revision sits on branch `ws09-revision` (worktree `C:\deadcurrent-ws09`), kept off `main` so Grok is not reviewing a moving target. 42 of 42 tests pass there; captures are in `C:\deadcurrent-ws09\Saved\Review\2026-09-30_1200`. When `Design/POIs/reviews/shore.landing_stage_gameplay.md` lands: triage it into `shore.landing_stage_revision.md`, fix on the branch, fast-forward `main`, remove the worktree (`git worktree remove C:/deadcurrent-ws09`), and re-verify (WS-10).

## WS-08: run the two critics (your step)

Each critic is a separate agent that did not build the stage. Give each the same two files:

- `Design/POIs/handoffs/shore.landing_stage_critics.md` (the contract: read-only except its own findings file)
- `Design/POIs/reviews/shore.landing_stage_REVIEW_PACKET.md` (what to look at and the questions)

Suggested split: **Gemini as Visual Critic**, which needs the images in `Saved/Review/2026-09-30_1055/` (the `landing_*.png`, `lookout_coil.png`, `lookout.png`, and `manifest.json`); upload them if it cannot read the folder. **Cursor/Grok as Gameplay Critic**, with the repo open. Findings go to `Design/POIs/reviews/shore.landing_stage_visual.md` and `..._gameplay.md`. Then tell me (or the next agent) to do WS-09 on those findings only.

## READY FOR ANTHONY TO CHECK

Playable now. The critics first will save your time, but if you want to look: delete `Saved\SaveGames\DeadCurrent.sav` (or keep it; your version-3 save loads fine with the stage in its default state), run `Tools\PlayTest.bat`.

1. **Before.** Step out of the door and look right. Where: the landing in the shallows. Should happen: a skiff alongside, a lantern lit on a post, a half-packed crate under a lean-to, bulbs along the edge; walking on shows `LOCATION DISCOVERED / Landing Stage`. Ask: does it read as a place someone is getting ready to use?
2. **Coil route.** Take the coil and hand it to Mara. Should happen: nothing moves while you talk to her. Walk back to the landing: Mara is on the stage with her pack, the crate is closed and lashed, the skiff loaded; her lookout is empty. Ask: did you notice before you were told?
3. **Save, diverge, load.** F5 on the stage, walk away, pick something up, F9. Should happen: as saved, at once, no second banner.
4. **Combat route.** New game or an earlier save. Kill the scavenger and tell Mara. Should happen: at the landing the lantern is out, the card gone, the crate open and empty, the skiff far out on the water; Mara still at her lookout. Ask: does it read as a consequence rather than a texture swap?
5. **Power.** Pull the leads at the Survey Launch. The bulbs at the landing go dark. Ask: can you even tell in daylight?
6. **Overall.** Anything floating, fake, in the way, or off the shore's look?

## Decisions Needed From Anthony

**New from the Visual Critic (parked, none blocking; details in `Design/POIs/reviews/shore.landing_stage_revision.md`):**

- **V-06, Mara's pack:** the owned library has no backpack mesh, so it is a canvas duffel with a bedroll in flat colours. Spend about 30 Meshy credits on a real pack (70 of 500 left), or keep the stand-in? Default: keep it.
- **V-07, the lookout shed:** the critic calls it greybox. It is Presentation Pass geometry you accepted, outside this POI. Default: leave it for a later Tier B pass.
- **V-02, the upright driftwood branch** by the path (Presentation Pass dressing) reads as floating to the critic. Default: leave it; say if you want it laid down.

Made inside the approved pilot, reversible, with defaults in the spec's Open Creative Decisions:

- **Where the stage stands:** east of the door, not between the camp and the lookout (no water there; the scavenger is alive in that stretch on the coil route).
- **Kill, then hand over the coil:** stays the combat picture.
- **Killed but not told Mara:** the stage stays as before.
- **Mara's lookout on the kill route:** unchanged in the world (her lines and the lookout crate already read the outcome).
- **No new Mara dialogue.**
- **Who stripped the landing on the kill route:** unstated, on purpose.
- **The display name** "Landing Stage" and **the card's text** are working text.

Still open from before: the frame-time shortfall (deferred); the TERN name board on stakes or on the hull.

## Known Issues

- **Frame time:** the machine ran about 11.7 ms slower today than last night on the unchanged pre-Phase-5 map, so cross-day comparisons are meaningless. The same-session A/B for Phase 5 is +0.36 ms, inside the noise. The deferred ~54 FPS issue is unchanged.
- Review frames show the engine's "Preparing Shaders" lines (editor-build messages, not game UI).
- For the critics to judge: the plank decking's pale colour, whether the power-cut view is visible in daylight, the pack and lashing as flat-colour boxes.
- Unchanged: the basin slab's faint dotted edge; one small eye texture; the stand-in breaker sound; packaged audio not verified by ear; content rebuilds rewrite unrelated material instances (discarded, not committed).

## Automated Status

- Build: `DeadCurrentEditor` builds.
- Tests: **42 of 42** (32 editor, 10 map). Baseline at the start of Phase 5: 38.
- Rebuild: `build_boathouse` and `dress_landing` clean; the art layer and `LandingDress` survive.
- Captures: `Saved/Review/2026-09-30_1055`; no default materials, missing textures, warnings, or errors in the landing views.
- Package: Development Win64 cook succeeded (2026-09-30); the null-RHI smoke launch loaded `Lvl_Boathouse`, and the 8 presence rules ran in the packaged game.
- Meshy spend: none in Phase 5 (still 430 of 500).

## Next Autonomous Task

WS-09, once `Design/POIs/reviews/shore.landing_stage_visual.md` and `..._gameplay.md` exist: fix, reject with a reason, or park each finding for Anthony, one commit per coherent group; then re-run WS-10 (rebuild twice, `Tools\RunTests.bat -build`, `Tools\ReviewCapture.bat` with a same-session A/B, `Tools\Package.bat`) and replace the session report. Do not start Phase 6.
