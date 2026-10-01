# Pointe Sombre — Slice Production Report

The phase's production metrics (`VerticalSlicePhasePlan.txt` §14). **Created in VS-07 (2026-10-01); filled at every checkpoint; finished in VS-24.** Integrator-owned. "not yet" is legal here (it is not legal inside a cell spec). Each cell spec's Pilot Record is the source of its rows; this file rolls them up.

**The key question:** do later locations get cheaper without feeling more generic?

---

## 1. Phase rows

| Metric | How it is measured | Filled when | Value so far |
| --- | --- | --- | --- |
| Spec approved → playable candidate | commit time of the cell spec → first green map test for that cell | each cell's first green test | not yet (all seven specs committed in VS-07) |
| Candidate → accepted | that commit → Anthony's checkpoint acceptance | Checkpoints A–E | not yet |
| Existing-system reuse | share of each cell's gameplay actors that are existing classes (script count) | each cell; rolled up here | not yet |
| New foundational C++ | per task; phase target: the portal + the scene-cut signal (VS-03), the light sequence (VS-17), the story card (VS-19). Anything else is a stop | the task that adds it | portal + scene cut (VS-03, `2c1d2b6`). VS-04..VS-06 added none (VS-06 added test and review-tool C++ only) |
| Meshy / bespoke | per cell, credits; phase total. 350 is a stop ceiling, not a target | the day it is spent | **0** credits through VS-07 |
| Tier B | modules reused / added (`report_kit_usage.py`) | VS-24 (gym numbers now) | VS-05 gym: 3 structures, 22 pieces, 8 reused in all three, 100% of 109 structural placements are kit pieces, 0 bespoke buildings |
| Tier C share | recipe manifest instances vs tagged hand placements; target ≥ 80% from the recipe | after the cell zones exist; final in VS-24 | VS-06 test zone: 262 recipe instances, 0 hand-placed Tier C on the island (100%) |
| Library reuse | provenance rows by source (`art_pipeline.md`) | with each asset | VS-06: 8 meshes from three owned packs, 0 generated |
| Critic findings | count by severity, `Design/POIs/reviews/sombre_<scope>_<critic>.md` | critic passes | Gemini VS-05 kit critic (advisory, no blockers) |
| Defects only Anthony found | checkpoint notes; defects no critic filed | each checkpoint | VS-05 gym: 1 (a floating crate) |
| Test growth | count added per task (reported, not targeted) | each task | 45 at VS-01 → 46 (VS-02) → 47 (VS-03) → 51 (VS-04) → 51 (VS-05) → **52** (VS-06). VS-07 adds none (docs) |
| Grammar reach | how much of VS-04..VS-16 was built with the portal as the only new capability (the sequence arrives VS-17, the card VS-19) | VS-24 | not yet |
| Performance gates | same-session A/B (plan §20) | the task that owns the gate | gate 0 (VS-04): views at the 60 FPS cap, draw calls 512–662 (its route figure is invalid: the old route probe started inside the tower rock). Biome on/off (VS-06): **+0.18 and +0.28 ms**, in-run ABBA toggle. Gate 1 (VS-08): not yet |
| Reproducibility | two consecutive rebuilds: identical biome manifest; map tests green after each | VS-06 for the test zone; VS-24 for the slice | VS-06: two scatter runs byte-identical (manifest `5c2076d5…`); a core rebuild keeps the sublevel; pure re-plan equals |
| Package gates | both production maps cook and smoke-load | VS-04, VS-08, VS-14, VS-19, VS-24 | VS-04 and VS-05: green (0 errors, 0 warnings) |

## 2. The comparison that must survive

Same rows for every column. If VS-20 builds no cell, the third column says "no cell qualified", and the first two are compared alone. No column is deleted.

| Row | Cable hut (VS-13) | Headland (VS-16) | Fresh-agent cell (VS-20) or "none" |
| --- | --- | --- | --- |
| Spec commit → first green test | not yet | not yet | not yet |
| Candidate → Anthony accepted | not yet | not yet | not yet |
| New C++ (target 0 beyond the three approved capabilities) | not yet | not yet | not yet |
| Bespoke assets / Meshy credits | not yet | not yet | not yet |
| Tier C share | not yet | not yet | not yet |
| Zone added in ≤ 20 lines with no tool change | not yet | not yet (the plan's agent test) | not yet |
| Critic blockers | not yet | not yet | not yet |
| Anthony: did this place feel procedural or generic? | not yet | not yet | not yet |

The cable hut is the baseline. The headland should be cheaper, or a recorded reason says why not. The third column is the manufacturability test: a cell built from its spec, the kit, the recipe, and the grammar, by an agent who built no other cell. **Held-back candidate:** Remy's marker and boat (`sombre_ids.md` §13).

## 3. Per-cell Pilot Records (copied from each spec; the spec is the source)

| Cell | Owner package | First green test | Existing vs new classes | Kit compositions | Zone | Review views | Meshy | Critic file | Defaults actually used |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `sombre.crossing` | VS-10 | `Map.Sombre.Crossing` | not yet | `ida_deck` (new) | N/A | `crossing_bow`, `crossing_rail_mara` | 0 | not yet | not yet |
| `sombre.harbor` | VS-10, VS-15, VS-19 | `Map.Sombre.Crossing` | not yet | `kit_harbor_shed` ×2, `cutter_deck` (new) | `harbor` | `arrival_quay`, `harbor_overview`, `harbor_mouth_remy`, `hale_arrival`, `night_quay_*` ×5 | 0 | not yet | not yet |
| `sombre.settlement` | VS-11 | `Map.Sombre.Settlement` | not yet | `kit_store`, `kit_cottage`, `leclair_house` (new), `kit_salvage_shed`, `kit_smokehouse` | N/A | 8 views | 0 | not yet | not yet |
| `sombre.lighthouse` (+ vault) | VS-12, VS-14, VS-17 | `Map.Sombre.Lighthouse`, `Map.Sombre.Vault` | not yet | `tower_base_room` (new) | N/A | 9 views | 0 | not yet | not yet |
| `sombre.net_loft` | VS-18 | `Map.Sombre.Meeting` | not yet | `kit_net_loft_shell` | N/A | `loft_meeting` | 0 | not yet | not yet |
| `sombre.cable_hut` | VS-13 | `Map.Sombre.CableHut` | not yet | `kit_cable_hut` | `cable_hut` | `cable_hut_approach`, `cable_hut_door` | 0 | not yet | not yet |
| `sombre.headland` (+ *Grey*) | VS-16 | `Map.Sombre.FalseLight`, `Map.Sombre.Grey` | not yet | N/A | `headland` | `headland_post`, `grey_stern`, `night_headland` | 0 | not yet | not yet |

## 4. Checkpoint log

| Checkpoint | Date | Accepted | Anthony-only defects | Notes |
| --- | --- | --- | --- | --- |
| A — Shape (after VS-08) | not yet | not yet | not yet | not yet |
| B — People and the first clue (VS-12) | not yet | not yet | not yet | not yet |
| C — The dungeon (VS-15) | not yet | not yet | not yet | not yet |
| D — The decision (VS-19) | not yet | not yet | not yet | not yet |
| E — The game (VS-24) | not yet | not yet | not yet | not yet |
