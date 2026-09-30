# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- Local `main`, ahead of `origin/main` by the Phase 5 commits (not pushed; see `git log origin/main..main`).
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone / Task

**Phase 5, World State: in progress (started 2026-09-30).** Plan: `WorldStatePhasePlan.txt`.

| Task | State |
| --- | --- |
| WS-00 Plan | done (`5144257`) |
| WS-01 Reconcile stale status | done (this commit) |
| WS-02 Conditional presence capability + test | **next** |
| WS-03 Landing Stage spec + handoffs | not started |
| WS-04 Build the stage | not started |
| WS-05 Presentation | not started |
| WS-06 Stage tests | not started |
| WS-07 Review views + review packet | not started |
| WS-08 Independent critics (Gemini, Cursor/Grok; run by you) | not started |
| WS-09 Revision | not started |
| WS-10 Verify and stabilize | not started |
| WS-11 Your acceptance | not started |

## Completed Since Last Update

- WS-00: `WorldStatePhasePlan.txt` committed before any Phase 5 code.
- WS-01: stale status fixed in `CLAUDE.md`, `Design/game_design.md` (Current milestone), `Design/POIs/README.md`, `Design/POIs/PRODUCTION_PILOT.md` (status line), and this file. `Design/CLAUDE_SESSION_REPORT.md` keeps its history; its pilot-status section is marked superseded. It will be replaced at the end of the phase.

## In Progress

Nothing half-done in the tree.

## READY FOR ANTHONY TO CHECK

Nothing yet. The Phase 5 walkthrough is `WorldStatePhasePlan.txt` §7; it becomes playable after WS-06.

## Decisions Needed From Anthony

None blocking. Made inside the approved pilot, all reversible, recorded in the plan:

- **Where the stage stands:** in the shallows just east of the boathouse door, south of the path (roughly X 860..1260, Y −580..−1250). The pilot said "between the scavenger's camp and Mara's lookout", but the only water is south of the path, and on the coil route the scavenger is still alive, so putting Mara inside his patrol square would read wrong. This spot is on the way the player walks every time. Say if you want it elsewhere.
- **Which state wins when both routes happen:** killing the scavenger and then handing over the coil keeps the combat picture (the drifted skiff does not come back). Coil first, kill later keeps the coil picture.
- **Mara's lookout on the combat route:** she stays, her pack stays, and her existing lines and the lookout crate already read the outcome. On the coil route the lookout is empty (she and her pack are on the stage).
- **Stage changes happen out of sight:** Mara does not vanish mid-conversation; the stage changes once you have walked away and are not looking at it. A load shows the saved state at once.

Still open from before (none blocks anything): the frame-time shortfall (deferred); the TERN name board on stakes or on the hull.

## Known Issues

- **Frame time 18.4 ms average (about 54 FPS)** vs the 16.7 ms target at PlayTest settings, uniform across views; not bisected. Deferred by you. Phase 5 watches for a regression of more than 1 ms (plan §9).
- Basin water slab has a faint dotted edge. Cosmetic.
- One eye texture (`T_EyeMidPlaneDisplacement`) is still at source size (small).
- Breaker throw is a stand-in sound. Audio balance and the packaged game's audio are not verified by ear.
- Content rebuilds rewrite imported binaries differently each run; that churn is discarded rather than committed.
- Superseded (fixed after the report that listed them): the live water is no longer the flat cyan sheet (`M_DC_Current` filaments); Mara has her own head.

## Future Tier-C Candidates (for the first biome recipe; not automated)

Also in `Design/content_production_strategy.md` §5: shore stones, driftwood, minor debris, mud and wet-sand variation, shoreline grass, generic maritime scrap, an ambience zone owning the wind and lap beds. The Landing Stage's dressing is hand-placed and will say which of these a recipe would have saved.

## Automated Status

- Build: `DeadCurrentEditor` builds (unchanged since the Presentation Pass).
- Tests: **38 of 38** at the start of Phase 5 (31 editor, 7 map). Target at the end: at least 41.
- Package: last Development Win64 cook succeeded at the end of the Presentation Pass.
- Meshy spend: 430 of 500 credits. Phase 5 budget: at most one prop (about 40), only if the spec justifies it.

## Next Autonomous Task

WS-02: `ADCConditionalPresence` in `Source/DeadCurrent/World/`, the `OnRestored` signal on `UDCWorldStateSubsystem`, the load and review-capture hooks, and the editor test `DeadCurrent.World.ConditionalPresence`. Then `Tools\RunTests.bat -build`.
