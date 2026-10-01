# Agent Handoff — WP-CELL-lighthouse: the tower, the lamp room, the vault's outer ends (VS-12) — POI Builder

Copied from `AGENT_HANDOFF_TEMPLATE.md`. Read first: `CLAUDE.md`, `Design/POIs/README.md`, `Design/POIs/sombre.lighthouse.md` (**its "Implementation boundary" section is the contract**), `Design/POIs/sombre_ids.md`, `Design/technical_architecture.md` "Pointe Sombre: map architecture" and "Shoreline recipe".

**Launch status:** written in VS-07 (2026-10-01) so the boundary exists before any branch; **not launched.** VS-12 starts after Checkpoint B's prerequisites (VS-10, VS-11) and Anthony's go-ahead.

---

## Assigned Role

POI Builder (Claude by default; plan §13.1). Narrative text for inspectables comes from the beat script and the dialogue doc, as written in the spec.

## Allowed Scope

- Build the tower exterior, the base room and its real door, the lamp room and gallery, the stair portal pair, the tower's (dark) light actors, the keeper silhouette, the sea-mouth bubble, and the vault's **outer** ends: the hatch portal in the base room and the lower-door portal in the sea cliff.
- The B3 inspectables (lamp, clockwork, feed box, burner) with their variants, the `cut_cable_end` give, and the `sombre.light` location volume.
- Dressing, review views, and the map test for those.

## Files / Content Owned

- `Tools/EditorScripts/pointe_sombre/lighthouse.py`
- `Tools/EditorScripts/dress_sombre_lighthouse.py`, `Content/Maps/Lvl_PointeSombre_Art_Lighthouse.umap`
- `Content/World/PointeSombre/Lighthouse/**`
- `Tools/Kits/great_lakes_settlement/structures/tower_base_room.json` (new composition from existing modules)
- `Tools/Review/Lvl_PointeSombre/lighthouse.json`
- `Source/DeadCurrent/Save/DCSombreLighthouseMapTest.cpp`
- the lighthouse spec's Pilot Record and status lines only

## Files That Must Not Be Modified

- `Tools/EditorScripts/pointe_sombre/vault.py`, `dress_sombre_vault.py`, `Lvl_PointeSombre_Art_Vault`, `Content/World/PointeSombre/Vault/**`, `Tools/Review/Lvl_PointeSombre/vault.json`, `DCSombreVaultMapTest.cpp`
- `Tools/EditorScripts/pointe_sombre/cable_hut.py` and the cable hut's files (the conduit's outer end is theirs)
- `Tools/EditorScripts/build_pointe_sombre.py` (anchors are the Integrator's: use `tk.anchor`, never create or move one), `toolkit.py`, `island.py`, `terrain_mesh.py`
- `Design/POIs/sombre_ids.md`, the lighthouse spec's contract sections, the other specs
- `DCSombreVaultRoutesMapTest.cpp` (the Integrator's, at the vault merge)
- `Source/**` other than the test file above; save code and `SaveVersion`; generator cores; content specs; `Config/*`; `Tools/*.bat`
- shipped ids; `Design/world_bible.md`; `CLAUDE.md`; phase plans
- the generated `Lvl_PointeSombre.umap` (the Integrator regenerates it)

## Input Specification

- **POI spec:** `Design/POIs/sombre.lighthouse.md` at the VS-07 commit (the commit that adds this file), with any Integrator revisions recorded there before launch.
- **Upstream:** the ledger; VS-08's greybox (the tower blockout and pad, the anchors `Anchor_TowerStair_Base`, `Anchor_TowerStair_Lamp`, `Anchor_VaultHatch_Top`, `Anchor_VaultHatch_Bottom`, `Anchor_VaultLower_Out`, `Anchor_VaultLower_In`, `Anchor_Respawn_TowerBase`); WP-NARR's assets (VS-09) for `cut_cable_end` and the quest.
- **Baseline:** `main` at launch; the test count printed that day (52 at VS-07).
- **Approved decisions from Anthony:** Checkpoint A's layout (when given); the plan's defaults in the spec's Open Creative Decisions.

## Expected Deliverables

- The tower reads as the primary landmark; the base room, lamp room, and gallery are walkable; the stair works both ways (anchors).
- The hatch refuses with the keyhole text and opens with `sombre_vault_key`, setting `sombre.vault_opened`; once open, "Go down". It lands at `Anchor_VaultHatch_Bottom` whether or not the vault exists yet.
- The lower door: "Pry the jam" with Engineering 2 sets `sombre.vault_opened`; it refuses without; it lands at `Anchor_VaultLower_In`.
- The B3 inspectables with every variant in the spec; the quest's `lamp_room` → `vault`.
- The tower's light actors (dark, `ActiveConditions` only; the sequence arrives in VS-17); the keeper silhouette's two presence states; the sea-mouth bubble's rule on `sombre.node_destroyed`.
- Done when the tests below are green and the views are captured.

## Required Tests

- `Tools\RunTests.bat -build`: at least the baseline count + 1, all green.
- `Map.Sombre.Lighthouse`:
  - `sombre.light` discovery → `lamp_room`
  - the feed box → `cable_cut_found` → `vault`; `cut_cable_end` once
  - the build readings
  - the stair both ways
  - the hatch locked, then unlocked
  - the lower door locked without Engineering 2, open with it
  
  **It must pass with no vault geometry**, and it does not assert the vault interior.
- `Map.Sombre.Greybox`, `BiomeExclusions`, `Architecture`, `Respawn`, `Atmosphere` unchanged.

## Required Review Artifacts

- Captures of `lighthouse_base`, `lamp_room`, `feed_box_close`, `gallery_view`, `vault_entry_lower` in `Saved/Review/<run>_Lvl_PointeSombre/`, with written pass/defect findings per expectation.
- Provenance rows and any Meshy credits (with the spec's Tier A justification and failed library searches recorded first).
- `Design/ANTHONY_CHECKLIST.md` entry.

## Known Dependencies

- VS-08 has placed the anchors (before this branch exists); VS-09 data; VS-10 and VS-11 before VS-12.
- The vault (VS-14) is serial after this and the cable hut, unless Anthony enables WP-VAULT (then `sombre_WP-VAULT.md` is written in full from the spec's boundary).

## Stop Conditions

- a portal, presence rule, or test needs an actor in `vault.py` or `cable_hut.py`, or a new anchor (ask the Integrator)
- a new C++ system, condition or consequence type, or save field is needed (the light sequence is VS-17's, not this task's)
- a shipped id, flag, or stage would change
- a lore claim beyond the spec is needed
- a baseline test fails for a reason outside this work
- the Meshy ceiling or a Tier A row's estimate would be exceeded
- the tree has uncommitted edits in files this handoff does not own
- usage running low: finish or revert the smallest unit, verify, record the next step, stop

## Handoff Notes

- **Status:** not started.
- **Commits:** none.
- **What changed:** nothing yet.
- **Findings (critics):** none yet.
- **Tests run and results:** none yet.
- **Open issues:** the tower shell method (default: a generated revolved profile).
- **Exact next step:** after VS-11 and Anthony's go-ahead, branch `vs/lighthouse` from `main`; build the base room around `Anchor_Respawn_TowerBase` without moving it.
- **Return to:** Integrator, then Anthony at Checkpoint B.
