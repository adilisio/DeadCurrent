# Agent Handoff — WP-NARR: the slice's data (VS-09) — Narrative Builder

Copied from `AGENT_HANDOFF_TEMPLATE.md`. A contract between people and agents. Read first: `CLAUDE.md`, `Design/POIs/README.md`, `Design/POIs/sombre_ids.md` (the ledger), `Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md`, `Design/Narrative/SLICE_DIALOGUE.md`, `VerticalSlicePhasePlan.txt` §8.2, §13.3 (WP-NARR), §19.

**Launch status:** written in VS-07 (2026-10-01). Launched by Anthony after Checkpoint A (2026-10-02) and built by Claude on `vs/narr`. **Complete and merged** (see Handoff Notes).

---

## Assigned Role

Narrative Builder (Claude by default, plan §13.1).

## Allowed Scope

- Write the slice's items, both quests, and the nine dialogues as per-file content specs, exactly as the ledger names them.
- Apply plan §8.2's fixes to the dialogue and quest data (the wording that §8.2 assigns to VS-09), and list each changed line for Anthony.
- Write the editor content tests.
- Regenerate the assets locally to test; the Integrator commits the generated assets after merging.

## Files / Content Owned

- `Tools/ContentSpecs/items/sombre.py`
- `Tools/ContentSpecs/quests/sombre_characteristic.py`, `Tools/ContentSpecs/quests/sombre_false_light.py`
- `Tools/ContentSpecs/dialogue/sombre_mara.py`, `sombre_varga.py`, `sombre_odette.py`, `sombre_marthe.py`, `sombre_tem.py`, `sombre_dell.py`, `sombre_sigrun.py`, `sombre_hale.py`, `sombre_jonas.py`
- `Source/DeadCurrent/Quest/DCSombreContentTest.cpp` (`Content.Sombre.QuestGraph`, `Content.Sombre.Dialogue`, `Content.Sombre.Items`)
- `Design/Narrative/SLICE_DIALOGUE.md`: **the change-list section only**, for the §8.2 lines
- the generated assets in `/Game/Items`, `/Game/Quests`, `/Game/Dialogue` that these specs create (committed by the Integrator)

## Files That Must Not Be Modified

- `Source/**` other than the one test file above; `Source/**/DCSaveGame*` and `SaveVersion`
- the generator cores `create_items.py`, `create_quest.py`, `create_dialogue.py`, `content_specs.py`
- the accepted shore specs `items/shore.py`, `quests/shore_watch.py`, `dialogue/mara_intro.py`
- every map script (`build_pointe_sombre.py`, `pointe_sombre/*.py`, `build_boathouse.py`), every map
- `Design/POIs/sombre_ids.md` (read it; request a new id from the Integrator), the cell specs
- shipped ids, flags, stages, and asset names; `Design/world_bible.md` canon; `CLAUDE.md`, phase plans
- the beat script `SLICE_WRONG_CHARACTERISTIC.md`

## Input Specification

- **Specs:** the ledger and the seven cell specs (`Design/POIs/sombre.*.md`) at the VS-07 commit (the commit that adds this file).
- **Upstream:** `SLICE_DIALOGUE.md` (full text), the beat script §4–§6, plan §8.2.
- **Baseline:** `main` after VS-06, `ad50261` plus the VS-07 docs commit; **52 tests** (34 editor, 13 Boathouse, 5 Sombre). *Reconciled at launch (2026-10-02): the real baseline was `d50e59b` after VS-08 and Checkpoint A, with **53 tests** (34 editor, 13 Boathouse, 6 Sombre; `Map.Sombre.Greybox` added). The target was therefore at least 56.*
- **Approved decisions from Anthony (2026-09-30):**
  - the slice opens in the crossing
  - Marthe chairs the meeting
  - Hale arrives midway
  - Liv's note is right as written
  - the tie is the player's choice of sister, partner, or the one who took them in

## Expected Deliverables

- Every item, quest, stage, dialogue, entry node, and state-writing node in the ledger (§4–§6) exists with the ledger's id. Done when `Content.Validate` and the three new tests are green.
- §8.2 applied and listed:
  - #1: Dell's rain clause reworded minimally; the Survival 2 choice kept
  - #2: no second-storm "catch them at it" text; two approaches only
  - #3: no force branch
  - #4: Marthe's `store` at `knows` gains "Call the island to the loft." → `sombre.meeting_called`, and `loft` requires it
  - #8 and #11 as data notes
  - #9: no Copper Buyer quest
  
  Each changed line goes in the read-aloud list for Checkpoint B.
- `knows` on enter clears `sombre.storm` and sets `sombre.hale_arrived`; each `done_*` sets `sombre.storm` (ledger §3.1). **No content keys the atmosphere looks to `sombre.storm`.**
- The `knows` objective tells the player to decide and then tell the settlement at the net loft.

## Required Tests

- `Tools\RunTests.bat -build`: **at least 55** (52 + `Content.Sombre.QuestGraph`, `Content.Sombre.Dialogue`, `Content.Sombre.Items`), all green.
- `QuestGraph`: every outcome stage reachable by some flag sequence; `knows` needs both the panel and the note; the untouched ending completes; False Light's `done` sets `false_light_taken`.
- `Dialogue`:
  - every node has an unconditional choice, and every entry list ends unconditional
  - no tie leak: every tie-variant line has a NO_TIE partner, and each tie flag shows exactly one variant per keystone node
  - evidence choices require the item and remove it
  - keeper choices exclude each other
  - Varga's `first` starts both quests
- `Items`: every slice item id resolves and has a category.
- `Tools\DumpContent.bat` before and after: the shore content is byte-identical.

## Required Review Artifacts

- The read-aloud list of changed lines (§8.2) for Anthony, in Handoff Notes.
- No visual captures (data only).
- `Design/ANTHONY_CHECKLIST.md` entry.

## Known Dependencies

- The ledger (VS-07) — done.
- The cell scripts that place these assets are VS-10 onward; this package does not wait for them.
- The Integrator regenerates and commits the assets after the merge.

## Stop Conditions

- a line would decide canon beyond Anthony's decisions (mark it PROVISIONAL and ask)
- a new condition or consequence type seems needed
- a shipped id, flag, stage, or asset name would change
- a file outside Files / Content Owned would have to change, or an id outside the ledger is needed (ask the Integrator)
- a baseline test fails for a reason outside this work
- the tree has uncommitted edits in files this handoff does not own
- usage running low: finish or revert the smallest unit, verify, record the next step, stop

## Handoff Notes

- **Status:** complete (2026-10-02). Merged into `main`; the commit is recorded in `Design/ANTHONY_CHECKLIST.md` (VS-09 Record).
- **Preflight:** the tree was clean apart from the two recorded leftovers; no editor was running. `Tools\RunTests.bat -build` on `d50e59b` gave 53 of 53 (34 + 13 + 6). `Tools\DumpContent.bat vs09_before` covered the 8 Shore assets.
- **What changed:**
  - `items/sombre.py`: the 10 ledger items, all `Item.Quest`, using existing meshes (no new art)
  - `quests/sombre_characteristic.py` (8 stages) and `quests/sombre_false_light.py` (4 stages), exactly per the ledger §5
  - the nine `dialogue/sombre_*.py` files, entry lists exactly per the ledger §6
  - `Source/DeadCurrent/Quest/DCSombreContentTest.cpp`
  - the change-list section of `SLICE_DIALOGUE.md`
  - 21 generated assets
  - the Integrator's `technical_architecture.md` lines (the tests table, the content specs, one stale "add a quest" sentence)
- **§8.2 applied:**
  - #1: Dell's line is now "...Nobody wades the reef for fish."
  - #2 and #3: no catch-them-at-it path and no force path
  - #4: the call, with Marthe's reply "I'll send round for them. Go on up.", and the `loft` entry gated on `meeting_called`
  - #8 and #11: data notes; "Where next?" sets `slice_end` as the default
  - #9: no Copper Buyer quest
- **Beyond §8.2 (recorded, change list #14):** a forgiven Odette's keeper choice was unreachable as written (it lived only in `knows`, which the meeting ends). The same choice is now also in her `after` node.
- **Open question for Anthony:** whether exposing Odette at the loft revokes an earlier "yes" to keeping the light. The data is as written: she stays keeper.
- **Tests run and results:**
  - `Tools\RunTests.bat -build`: **56 of 56** (37 editor, 13 Boathouse, 6 Sombre)
  - `Content.Validate` covers 3 quests and 10 dialogues
  - `DumpContent` before and after: the 8 Shore assets are identical, 21 assets are new, and no tracked `.uasset` changed
  - mutation check: three deliberate breaks (a missing NO_TIE partner, evidence not removed, `knows` on the panel alone) each failed the intended assertions; the specs were then restored, and the regenerated dump matched the tested one exactly
  - a fidelity script found all 272 quoted strings of `SLICE_DIALOGUE.md` §1–§9 verbatim in the data, except the three changed lines
- **Notes for the cell builders:**
  - **(VS-11/VS-18)** After "Call the island to the loft." the attendees' loft states are deferred, so Marthe stays at her counter while the player is near. Talking to her there opens the meeting at the counter. The settlement and loft owners decide whether that is acceptable or whether her counter placement hides on the call.
  - **(VS-17/VS-19)** The keeper silhouette reads `keeper_*` with `meeting_done` (see the open question above).
- **Exact next step:** VS-10 (the crossing and the harbor) by the Integrator.
- **Return to:** Integrator (Claude), then Anthony at Checkpoint B (read-aloud list in the change list).
