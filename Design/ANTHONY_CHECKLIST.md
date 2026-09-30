# Anthony Return Checklist

Live handoff file. Updated after every coherent milestone. If this session ends suddenly, this is the truth.

## Current Head

- `main`, with the VS-00 plan commit on top of `9640c05` (see `git log -1`).
- **`origin/main` matched `9640c05` at plan time** (`git fetch`; 0 ahead, 0 behind). The Phase 5 commits *are* pushed. Earlier text here said "local, not pushed"; that was stale. The VS-00 commit itself is local until you push it.
- Left untracked on purpose: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py` (earlier leftovers, not ours to commit).

## Current Milestone

**PHASE 6: PLANNED. NOT STARTED. AWAITING ANTHONY APPROVAL.**

Plan: `VerticalSlicePhasePlan.txt` (VS-00..VS-25). No Phase 6 code, map, asset, quest, item, or dialogue exists. Phase 5 (World State) is accepted (2026-09-30).

## What the Phase 6 Plan Proposes

- **The slice:** "The Wrong Characteristic" at Pointe Sombre, built as written in `Design/Narrative/SLICE_*`: the played crossing, the harbor, the settlement, the lighthouse, the vault dungeon, Hale's midway arrival, False Light, the decision made with your hands, Marthe's net loft, the second storm, and the end card. It runs 60–90 minutes.
- **Two proofs:** that it works as a game, and that more of it can now be made cheaply (the kit, the shoreline recipe, seven cells built through the contract, production metrics).
- **Three small new capabilities, nothing else in C++:**
  - a cell portal (a fade, a move, a fade back; locked by conditions; a "scene cut" that lets presence snap)
  - an authored light sequence (the tower's characteristic, plus three seconds of the pattern, played once when you are looking)
  - a story card (the end card)
- **No save-format change and no `SaveVersion` bump.**
- **Four playable checkpoints for you before the final run** (§16): A the island's shape; B people and the first clue; C the dungeon; D the decision.
- **The final run** (§22) comes after them.

## Major Architecture Choices

- **One new map, `Lvl_PointeSombre`, not World Partition.** The playable exterior is about 450 × 350 m.
- **Interiors are cells inside the same map**, reached through portals: the vault (three entrances), the net loft, and the tower stair. The code decided this. Flags, quests, the registry, and the build are all per-world, so real map travel would need a save-format change. In-map cells need none.
- **The lamp room stays in place at the top of the real tower**, so its view and the keeper's silhouette on the gallery are true. Only the stair is abstracted.
- **The crossing** is a walkable deck of the *Ida* offshore in the same map, facing the real false light and the dark tower. "Tell Varga about the light" is the strike.
- **`Lvl_Boathouse` is unchanged**, and all its tests and old saves are kept. One save slot serves both maps; F9 opens whichever map the save is for.
- **Mara is placed, not following.** Her lines already work wherever she stands. A companion system is deferred.
- **The prologue ("Posted From the Shore") stays out of Phase 6.** The slice stands alone with the letter on board and Mara's tie question on the crossing. It is proposed as a later "Connection" milestone. Slice ids match the prologue's, so that milestone will be additive.

## Fallout NYC / Production-Scale Choices

- **Compressed geography:** 10–60 s legs between beats, landmarks overlapping (tower, false-light post, and the *Grey* all visible from the quay), and optional places just off the routes. It is measured by a route-timing report, not guessed. If a stretch is empty, it gets shorter, not padded.
- **Interior-cell abstraction:** yes, where it saves cost without breaking the view (the vault, the loft, the stair). Not for the lamp room.
- **Modular settlement kit (Tier B):** shell modules plus material skins plus library trim, composed from data. It must build 10 or more structures with 0 bespoke buildings. Target: 75% or more of structural placements are kit pieces. The library survey found no timber-building kit, so the shells and a CC0 wood texture are new.
- **First biome recipe (Tier C):** a "Great Lakes rocky shoreline" recipe, data-driven, seeded, and deterministic. It writes HISM instances, NoCollision, with authored plus automatic exclusions. PCG is adopted only if a one-day spike shows it regenerates headless and deterministically with no runtime cost; otherwise the tool is a scripted HISM scatter.
- **Content-cell model:** seven cell specs (crossing, harbor, settlement, lighthouse + vault as one integrated contract, net loft, cable hut, headland + *Ashland Grey* stern). An id ledger means no two builders mint the same id. A late cell is built "contract-only" by a fresh agent, to measure the cost curve.
- **Planned agent parallelism:**
  - Claude: lead, integrator, narrative, most cells.
  - Grok: the three capabilities, the biome tool, possibly the vault; gameplay critic.
  - Gemini: the kit survey (docs only); visual critic.
  - Rules: generated maps are build products that only the integrator commits; content specs are per file; tests and review views are per cell; one worktree per builder.

## Decisions Needed From Anthony (before VS-01)

1. **Approve or change the plan.** Approving it also approves the Phase 6 Meshy ceiling of **350 credits** (balance 437), spent only through a cell spec's Tier A row.
2. **The prologue boundary.** Default: the prologue is *not* in Phase 6; the slice stands alone (plan §3.2). Say so if you want it in.
3. **Which agents you will run, and as what.** Default:
   - Grok: the Systems Engineer for VS-03 and the biome tool, then the gameplay critic.
   - Gemini: the visual critic and the kit-survey researcher.
   - Claude: everything else.
   - If you want Grok to build the vault too, say so.

Everything else has a default in the plan and is decided at a checkpoint:

- NPC bodies (Checkpoint B)
- the packaged build starting in the slice
- the tower's characteristic
- the wrecker on the *Grey*
- no music
- the end card's trigger

## Known Risks

- **Named NPCs may read as twins or mannequins.** Eight people on one pack body. A bounded spike (MetaHuman low-LOD, or a rigged generated body) goes to you at Checkpoint B.
- **Combat is thin by design:** one melee archetype, one fight on the *Grey*. Checkpoint C asks whether that is enough. A ranged enemy would be your scope call.
- **The storm, calm, and night looks may not switch with presence alone** (post-process volumes). A spike happens in VS-04; the fallback is one small actor.
- **The crossing could read as a static set.** Its fallback (a fixed view) needs your sign-off.
- **Vessels and the tower are the bespoke art.** The fallbacks are a revolved-profile tower, one hull reused, and the *Grey* composed from parts.
- **Continuity fixes in the slice docs** (plan §8.2), each the smallest change and shown to you at Checkpoint B:
  - Dell's "hasn't rained" line, in a slice that opens in rain
  - "Catch them at it" placed after the meeting
  - "Force the Pruitts" (cut)
  - the net loft gathering
  - two soft-lock guards in the vault
- **One save slot for both maps:** F5 in the slice overwrites a shore save.

## Automated Baseline

- Build: `DeadCurrentEditor` builds (Phase 5 acceptance).
- Tests: **45 of 45** (33 editor, 12 map). The Phase 6 target is at least 70.
- Package: the Development Win64 cook plus smoke of `Lvl_Boathouse` succeeded after Phase 5 acceptance.
- Frame time: about 54 FPS at low spec, deferred. Phase 6 uses same-session A/B gates only (plan §20).
- Meshy: 460 of the earlier 500 spent; balance 437.
- Not re-run in VS-00 (docs only).

## Recommended Next Task

After your approval: **VS-01 — Baseline, stale status, and the start record**:

- `git fetch`
- full suite (45/45)
- a same-session reference capture of `Lvl_Boathouse`
- one package run
- living status lines updated to "Phase 6 started"

Then VS-02 (per-file content specs and multi-map tooling). Do not start VS-01 without your go-ahead.
