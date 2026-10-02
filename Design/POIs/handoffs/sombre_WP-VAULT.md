# Agent Handoff — WP-VAULT: the vault interior (VS-14, parallel) — POI Builder

Copied from `AGENT_HANDOFF_TEMPLATE.md`. A contract between people and agents. Read first, in this order: `CLAUDE.md`, `Design/POIs/README.md`, this file, `Design/POIs/sombre.lighthouse.md` (the integrated spec: its **Implementation boundary** first, then every vault row), `Design/POIs/sombre_ids.md` (the ledger), `Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md` B4 and §4, `VerticalSlicePhasePlan.txt` §13.3 ("The lighthouse and the vault"), and `Design/technical_architecture.md` ("Pointe Sombre: map architecture").

**Launch status:** written 2026-10-02 by the Integrator (Claude) after VS-09 merged and VS-10 staged the boundary on `main`. Anthony's instruction (2026-10-02) asked for this package to be prepared for Grok as soon as its dependencies were real. **It launches when Anthony relays the prompt below.** Until then the vault stays Claude's serial VS-14.

---

## Assigned Role

POI Builder for the vault interior (Grok, through Cursor). You build **only inside the vault**: the boundary in `sombre.lighthouse.md` is frozen and not yours to move. You do not critique your own work; Gemini is your gameplay and visual critic, and only Anthony accepts.

## Allowed Scope

- Replace the placeholder `Tools/EditorScripts/pointe_sombre/vault.py` with the real vault: the six spaces, every gameplay actor inside them, and the three return portals.
- Take over the greybox's stub vault by declaration (`GREYBOX_RETIRE_GROUPS = ("vault",)` in `vault.py`), never by editing `greybox.py`.
- Dress the vault in its own art sublevel through the shared helper (`pointe_sombre/art.py`, `ArtLevel("Vault")`).
- Write the vault's map test and review views.
- Migrate library assets into `Content/World/PointeSombre/Vault/**` with the existing pack tooling (use it, do not change it).

## Files / Content Owned

- `Tools/EditorScripts/pointe_sombre/vault.py`
- `Tools/EditorScripts/dress_sombre_vault.py` and the sublevel it writes, `Content/Maps/Lvl_PointeSombre_Art_Vault.umap`
- `Content/World/PointeSombre/Vault/**` (meshes, textures, material instances you make for the vault)
- `Tools/Review/Lvl_PointeSombre/vault.json`
- `Source/DeadCurrent/Save/DCSombreVaultMapTest.cpp` (`DeadCurrent.Map.Sombre.Vault`, inside the vault only)
- this file's **Handoff Notes** section

## Files That Must Not Be Modified

- `Tools/EditorScripts/build_pointe_sombre.py` (the core, its `CELLS`, its anchors), `pointe_sombre/toolkit.py`, `pointe_sombre/art.py`, `pointe_sombre/greybox.py`, `pointe_sombre/layout.py`, `pointe_sombre/island.py`, `Tools/PointeSombre/*.json`
- every other cell script: `crossing.py`, `harbor.py`, `headland.py`, `arch_test.py`, and the future `lighthouse.py`, `cable_hut.py` (the hatch `VaultHatch_Out`, the lower door `VaultLower_Out`, and the conduit mouth `VaultConduit_Out` are theirs)
- the generated maps `Content/Maps/Lvl_PointeSombre.umap` and `Lvl_PointeSombre_Biome.umap`: build products. Your local rebuilds change them; **never commit them** (`git restore` them before committing)
- `Design/POIs/sombre_ids.md`, every `Design/POIs/sombre.*.md` spec (the lighthouse spec is frozen), the narrative documents, phase plans, `CLAUDE.md`
- `Tools/ContentSpecs/**` (items, quests, dialogue), the `create_*.py` generators
- `Source/**` other than your one test file; `Source/**/DCSaveGame*` and `SaveVersion`; any rule enum
- `Tools/*.bat`, `Config/*`, other review files, `Source/DeadCurrent/Save/DCSombreVaultRoutesMapTest.cpp` (the Integrator writes `Map.Sombre.VaultRoutes` at the merge)

## Input Specification

- **Baseline:** `main` at the commit that adds this file (record its SHA in your notes: `git log -1 origin/main`). **57 tests** (37 editor, 13 Boathouse, 7 Sombre). Run `Tools\RunTests.bat -build` in your worktree first, and stop if it is not green.
- **The vault's facts are already data (VS-09):** the items `liv_note`, `liv_chart`, `vault_access_log`, `section_key_compact`; the quest `sombre.characteristic` (`vault` → `vault_inside` on `sombre.vault_opened`; `vault_inside` → `knows` on `sombre.panel_read` **and** `sombre.liv_note_found`; `knows` clears `sombre.storm` and sets `sombre.hale_arrived`). You set flags; the quest moves itself.
- **Every id you may use is in the ledger.** Flags (§3.2): `vault_opened`, `log_read`, `panel_read`, `bearing_taken`, `liv_note_found`, `signin_board_taken`, `vault_floor_isolated`, `light_line`, `power_settlement`, `node_destroyed`, `light_decided`. You also read `storm`, `feed_spliced`, `sigrun_helping`, and `light_hand`. Persistent id (§7): `sombre.vault_locker`. Portals (§8): `VaultHatch_In`, `VaultLower_In`, `VaultConduit_In`. Location (§2): `sombre.vault`. Views (§11): `vault_entry_hatch`, `vault_entry_conduit`, `vault_lower_gallery`, `vault_node_room`, `vault_flooded`. Test (§12): `Map.Sombre.Vault`. Need another? **Stop and ask the Integrator; never mint one.**
- **The slot and the landings are fixed.** The vault is built at `tk.interior_origin("vault")` = (15000, 250000, 40000) cm. Keep it within ±6500 cm of that origin in X: the net loft's slot is 150 m away. The three landings are Integrator anchors, already placed (cm offsets from the slot origin, at floor level, plus a standing capsule height):
  - `Anchor_VaultHatch_Bottom` (−700, −300, 0): the upper room, under the hatch
  - `Anchor_VaultLower_In` (700, −300, 0): the lower gallery, inside the jammed door
  - `Anchor_VaultConduit_In` (0, 400, 0): the cable gallery

  Your spaces go around them. If your layout needs a landing elsewhere (for example, the lower gallery lower than the upper room), stop and ask the Integrator to move the anchor. Do not compensate inside your script.
- **The toolkit (`tk`) has what you need:** `tk.box`/`block`/`wall_segment`, `tk.make_interior` (every fixed piece of an interior: lighting channel 1 and a draw distance), `tk.portal`/`tk.portal_variant`, `tk.anchor`, `tk.presence`/`tk.state`/`tk.placement`, `tk.inspectable`/`tk.variant`, `tk.flicker_light`, `tk.conditional_audio`, `tk.damage_volume`, `tk.loot_container`, `tk.location_volume`, `tk.point_light`, `tk.post_process_box`, `tk.cond`/`tk.flag`/`tk.cons` (`GIVE_ITEM` sets the item reference), and materials (`tk.surface`, `tk.flat`, `tk.glow`, `tk.tinted_surface`). Examples of every one: `crossing.py`, `harbor.py`, `headland.py`, `arch_test.py`, and the Shore's `build_boathouse.py` (the *Tern*'s live water).
- **Interior rules (VS-04, proved):** enclosed so no exterior light leaks; every fixed piece goes through `tk.make_interior`; ceilings use `tk.flat` (the triplanar surfaces light a downward face as if it faced up); the cell's own post-process box and lights; no exterior ambience reaches the slot. `arch_test.py` is the worked example.

## Expected Deliverables

**1. Take over the stub.** In `vault.py`, set `GREYBOX_RETIRE_GROUPS = ("vault",)`. That removes the stub room, the three stub return portals, and the stub's `sombre.vault` location volume, so you build all of them again.

**2. The six spaces** (abstracted, Authority concrete; library meshes first): the upper room, the stair, the lower gallery (flooded, with a catwalk and pipe route), the node room, the sea-cock chamber, and the cable gallery. Walkable floors; no AI; no navigation needed.

**3. The return portals** (ledger §8, exactly):
- `VaultHatch_In`, variant `up`, "Go up", to `Anchor_VaultHatch_Top`
- `VaultLower_In`, variant `out`, "Go out", to `Anchor_VaultLower_Out`
- `VaultConduit_In`, variant `crawl`, "Crawl out", consequence `SetWorldFlag sombre.vault_opened` (idempotent), to `Anchor_VaultConduit_Out`

**4. Discovery:** one `tk.location_volume` (or several boxes with the same id), `sombre.vault`, "The Vault", covering the upper room and each landing. The first entry by any route discovers it.

**5. The vault's actors.** Texts are the beat script's (B4 table, §4); keep them verbatim, and list any line you had to write in your notes, PROVISIONAL, for Anthony's read-aloud. First match wins within each actor. One actor shows one verb at a time, so acts that can be available together are separate actors.
- **The keeper's log** (inspect):
  - `node_destroyed`, `!log_read`: "The log is pulp." (lost)
  - `!log_read`: the full text, and sets `log_read`
  - default: the full text
- **The sign-in board** (the inspectable-plus-presence pattern, plan §8.2 #8):
  - take variant, `!signin_board_taken`: shows "Two entries in chalk, four months old: O.B. and L.K. Nothing since." Gives `vault_access_log`, and sets `signin_board_taken`.
  - a presence rule hides the board on `signin_board_taken`
- **The panel's read-out** (inspect, sets `panel_read` in every variant):
  - destroyed: "Dark. Water to the second rung." (**the soft-lock guard, §8.2 #5**)
  - Line: "RESYNC PENDING: 4 OF 5."; with Engineering 2 it adds "Only confirmation left. It's asking."
  - Settlement: "RESYNC PENDING: 3 OF 5. The feed runs to the town now. The count is still climbing."
  - default: "GREAT LAKES MARITIME AUTHORITY — CONTINUITY SECTION 14. SEVERED. RESYNC PENDING: 2 OF 5." and the empty card slot. With Engineering 2 it adds the five conditions ("Power: yes. Quiet: yes. Card: no. Line: no. Confirmation: no. Someone pulled the card, and someone else cut the line.").
- **Seat the card** (its own actor at the panel's slot), "Seat the card":
  - conditions: `HasItem section_key_compact`, `feed_spliced`, `!node_destroyed`, `!power_settlement`, `!light_hand`
  - consequences: `RemoveItem section_key_compact`, `light_line`, `light_decided`
- **Throw the settlement feed** (its own actor), "Throw the settlement feed":
  - card variant: `!node_destroyed`, `!light_line`, `HasItem section_key_compact`; it removes the card
  - jumper variant: `!node_destroyed`, `!light_line`, `SkillAtLeast Skill.Engineering 2`
  - both set `power_settlement` and `light_decided`
- **The DF loop:**
  - `node_destroyed`, `!bearing_taken`: "The loop is under water."
  - "Take the bearing", with `!bearing_taken` and `storm`, or `!bearing_taken` and Engineering 2: "The pattern comes and goes as you turn the loop. Loudest at one-one-two." and sets `bearing_taken`
  - default: "A big loop antenna on a turntable, a dial, a headphone jack."
- **The chart table:**
  - "Take a copy", with `!HasItem liv_chart`: shows the chart's text and gives `liv_chart`
  - with `bearing_taken`: "Your bearing lies over hers exactly…"
  - with `HasPerk Perk.SchematicEye` and `HasItem survey_chart`: "Three marks agree: the *Tern*'s, hers, and yours."
  - default: the chart's text
- **Liv's note**, pinned under the chart (the same pattern): the first read gives `liv_note` and sets `liv_note_found`, with the text exactly as `Tools/ContentSpecs/items/sombre.py` holds it (Anthony approved it as written). A presence rule hides it on `liv_note_found`.
  - **The chart table and the note stand above the flood line** (**the soft-lock guard, §8.2 #6**).
- **The sea cock**, "Open it":
  - `!light_line` and `sigrun_helping`, or `!light_line` and Engineering 2
  - sets `node_destroyed` and `light_decided`
  - default: "A big valve wheel on a pipe that goes down through the floor. Opens the vault to the lake."
- **Live water** in the lower gallery: `tk.damage_volume` with `ActiveConditions = [sombre.storm, !sombre.node_destroyed, !sombre.vault_floor_isolated]`. The catwalk and pipe route cross above it. Sparks and the hum on the same conditions use the existing assets (`S_DC_HumLiveWater`, the spark sounds), as the Shore's *Tern* does.
- **The breaker**, "Isolate the floor", Engineering 2: sets `vault_floor_isolated`.
- **The dead eels** (the shipped `dead_fish` prop, retinted):
  - Survival 2 reads the safe edge
  - `HasPerk Perk.PulseRead`: "One shock, from the middle of the floor, where there is nothing."
- **Flooded:** the vault's raised water on `node_destroyed` (a vault presence rule; the sea-mouth bubble outside is the lighthouse's). It reaches the second rung, and the chart table stays dry.
- **The locker:** `tk.loot_container`, persistent id `sombre.vault_locker`, with a few of `ammo_9mm`, `field_dressing`, `salvage_wiring` (reward, not economy).
- **Light and sound:**
  - emergency lamps (`tk.flicker_light`, made interior)
  - the node's sixty-cycle hum (`S_DC_HumRelay` family)
  - drip and room tone only from sounds already in `/Game/Audio` (new sounds need Anthony's Freesound authorization; list wishes in your notes)
  - the cell's own post-process box

**6. The art sublevel:** `dress_sombre_vault.py` with `art.ArtLevel("Vault")`, NoCollision, stateless dressing only. Anything that follows state, blocks, or is stood on belongs in `vault.py`.

**7. Assets:** use the library first (`C:\FO5_AssetLibrary`; see `Design/art_pipeline.md`): `AbandonedPowerPlant`, `Scene_Junkyard`, and the shipped `lighthouse_logbook`, `dead_fish`, relay and breaker-panel Meshy props. **No Meshy generation without the Integrator's written approval** (it spends Anthony's credits). Record every migrated asset's provenance in your notes.

## Required Tests

- `Tools\RunTests.bat -build` green, **at least 58** (57 + `Map.Sombre.Vault`), with **every existing test unchanged**. In particular, `Map.Sombre.Greybox` must still pass: its portal walk now finds your `VaultHatch_In`, `VaultLower_In`, and `VaultConduit_In` by their ledger labels and uses each one from the default state.
- **`Map.Sombre.Vault`** (inside the vault only). Teleport to the landings; set the outside facts as flags; do not cross the boundary. It covers:
  - each return portal lands on its anchor
  - `sombre.vault` is discovered once
  - live water: on in the storm, off when isolated, off after `node_destroyed`, and the catwalk route is dry
  - the evidence: the board and the note each given once and then hidden; the copy of the chart
  - data loss after the sea cock: the log is pulp, and the loop is under water
  - both soft-lock guards: destroy first, then the panel still sets `panel_read`; the note above the flood line
  - `knows` reached in both orders (panel first, note first)
  - the storm clears at `knows`
  - the three acts' mutual exclusion (Line, Settlement, Destroyed) and their consequences
  - a save inside the vault (F5, diverge, F9)
- `Map.Sombre.BiomeExclusions` stays green (the vault slot is outside every zone).

## Required Review Artifacts

- `Tools\ReviewCapture.bat Lvl_PointeSombre` with `set DC_REVIEW_ARGS=-ReviewHideTag=Lightning`. Its five `vault_*` views must have no default material, no missing texture, and no error. Record the folder name.
- A short note of each space, its purpose, and the safe route, for the Gemini critic.
- The PROVISIONAL lines list, and the assets' provenance.

## Known Dependencies

- Done: VS-07 (spec, ledger, boundary), VS-08 (anchors, slot, stub), VS-09 (items, quests, flags), and the VS-10 staging (the `vault` hook, greybox retirement, the toolkit's interior and story helpers, `art.py`, the greybox test's ledger labels).
- Not needed: the lighthouse (VS-12) and the cable hut (VS-13). Your test stays inside; the Integrator joins the routes in `Map.Sombre.VaultRoutes` at the merge.

## Stop Conditions

- the tree is not green at the start, or a test outside your files fails for a reason outside your work
- you need an id, an anchor move, a toolkit helper, a new condition or consequence type, or a change to any file you do not own (ask the Integrator; write the request in your notes)
- a presence rule would need targets on both sides of the boundary
- a line would decide canon (mark it PROVISIONAL and ask)
- a Meshy generation seems needed
- usage running low: finish or revert the smallest unit, verify, record the next step, stop

## Commit and Push

- Worktree `C:\DeadCurrent_wt\vault`, branch `vs/vault`, from `origin/main` at the commit that adds this file.
- Commit only your owned files (one plain sentence each, prefixed `VS-14 (WP-VAULT):`). Push only `vs/vault`. Never `main`. Never the generated persistent or biome maps.
- The Integrator (Claude) merges, regenerates the maps, adds `dress_sombre_vault` to `Tools\RebuildContent.bat`, writes `Map.Sombre.VaultRoutes`, and runs the full suite. After the merge the vault files return to Claude (plan §13.3).

## Launching (for Anthony to relay)

```
git fetch origin
git worktree add C:\DeadCurrent_wt\vault -b vs/vault origin/main
```

> You are the POI Builder for DEAD CURRENT Phase 6, package WP-VAULT (the vault interior, VS-14). Work in the worktree `C:\DeadCurrent_wt\vault` on branch `vs/vault`, created from `origin/main`. Read `CLAUDE.md`, `Design/POIs/README.md`, and your handoff `Design/POIs/handoffs/sombre_WP-VAULT.md`, then the files it lists, and do exactly what it says. Stay inside its owned files. Stop at its stop conditions and write the reason at the top of its Handoff Notes. Commit only on your branch; push only your branch; never push `main`. Do not start any task that is not in your handoff. End with a summary: commits, test counts, the capture folder, and anything you need from the Integrator.

## Handoff Notes

- **Status:** not started (ready to launch; waiting for Anthony to relay the prompt).
- **Commits:** none.
- **What changed:** nothing yet.
- **Findings (critics):** none yet.
- **Tests run and results:** none yet.
- **Open issues:** none.
- **Exact next step:** create the worktree, record the baseline SHA, run the baseline suite.
- **Return to:** the Integrator (Claude), then Gemini (gameplay and visual critic), then Anthony at Checkpoint C.
