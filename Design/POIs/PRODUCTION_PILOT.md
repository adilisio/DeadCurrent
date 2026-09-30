# Production Pilot — APPROVED, NOT STARTED

Status: **approved by Anthony, 2026-09-30, to run inside Phase 5 (World State). Nothing is built yet.** Approval does not start Phase 5: the pilot begins only after Anthony gives the go-ahead for Phase 5 and its `WorldStatePhasePlan.txt` is committed with the pilot in its scope lock. The decisions are recorded in section 12.

Everything below marked PROVISIONAL is a suggestion to react to, not canon. Faction, backstory, and the Current stay unexplained.

## 1. Why a pilot, and what it must prove

Phase 5 (World State) exists so that **choices visibly alter locations and NPC behavior**. That is also the hardest thing for the production model to do, because a POI is no longer one static state. The pilot builds exactly one cell to answer four questions:

1. Can a builder go from a filled-in spec to a playable, tested cell using only existing systems, layout, data, assets, and dialogue?
2. Does the asset-tier split (A focal, B kit, C dressing) hold up in practice, and how much of the cell needed a bespoke asset?
3. Does an independent critic-and-reviser pass catch things the builder missed before Anthony plays it?
4. Is the cell cheaper to make than the Survey Launch was, without feeling cheaper to play?

If the answer to (1) is "no, we needed a new system", that is a result, not a failure. See section 6.

## 2. The cell: the Landing Stage (working name)

- **POI id:** `shore.landing_stage` (PROVISIONAL name). Discoverable, unmarked.
- **What it is:** a small timber landing stage and lean-to on the shore between the scavenger's camp and Mara's lookout, with a moored skiff. It is a place people use. It is not a quest hub.
- **Why this cell:** it is downstream of the choice the player already makes at Shore Watch (coil route or combat route) and of the Survey Launch (`wreck.power_cut`). It needs no new quest. Its whole job is to look and behave differently depending on what the player did.
- **Player promise (draft):** "I did something on this shore, and when I come back to the landing, it shows."
- **Size:** one stage, one lean-to, one skiff, two containers, one inspectable, one NPC placement rule. About 30 m across.

### World-state variants (the point of the pilot)

| State (existing flags and quest stages) | The landing stage looks and behaves like |
| --- | --- |
| Before Shore Watch is resolved | Skiff moored, lantern lit, a crate half-packed. Mara is at her lookout, as now. |
| Coil route resolved (`shore.relay_recovered`) | Lantern lit, crate packed and closed, skiff loaded. Mara is now at the stage. |
| Combat route resolved (scavenger dead, quest not by coil) | Lantern out, skiff cut adrift, crate open and empty. Mara is not at the stage and her lookout has a different line. |
| `wreck.power_cut` set (either route) | A small extra: the stage lamp string that hummed is dark. Cosmetic. |

Mara's placement change is the first "NPC behavior" the pilot exercises. It is data plus a conditional presence rule (see section 6).

## 3. Spec

The full spec is `Design/POIs/shore.landing_stage.md`, copied from `TEMPLATE.md` and filled in **before** any builder starts. Its Open Creative Decisions section is where Anthony's approval happens. The other sections are pre-answered here so the pilot can be judged:

- **Existing systems used:** inspectables with variants, loot containers, `ADCLocationVolume`, world flags and conditions, `ADCConditionalAudio`, `ADCFlickerLight`, persistence ids, dialogue conditions, quest stages (read only).
- **Persistent ids (new, immutable once accepted):** `shore.landing_stage` (location), `landing.crate`, `landing.skiff` (only if it is an interactable), `landing.note`.
- **Flags:** none new if possible. State comes from `shore.relay_recovered`, `boat.scavenger` death, and `wreck.power_cut`. A new flag is a spec change.
- **Save impact:** new ids only. No `SaveVersion` bump expected.

## 4. Asset tiers

| Tier | What | Source | Bespoke generation |
| --- | --- | --- | --- |
| A focal | The skiff (moored and adrift variants) and the packed crate | Library boat or `motorboat_wreck` scaled first; Meshy only if no library asset reads | At most one Meshy prop (about 40 credits), and only after the library is searched |
| B kit | Timber planks and posts, lean-to walls, the lantern, ropes, the steel and plaster instances already in `import_art.py` | Existing materials, `dress_structures.py` pieces, migrated pack meshes at 1K | None |
| C dressing | Shore stones, driftwood, mud variation, small scrap around the stage | The Tier C candidates in `content_production_strategy.md` §5, hand-placed the way `dress_shore.py` does it | None. This is the pilot's test of whether a recipe is worth building. |

## 5. Roles, ownership, and workflow

Each role gets an `AGENT_HANDOFF_TEMPLATE.md` filled in. The important part is the file ownership, so no two of them touch the same generated file.

| Role | Owns | Must not touch |
| --- | --- | --- |
| World Architect (or Anthony) | `Design/POIs/shore.landing_stage.md`, its placement on the shore | any script or map |
| POI Builder | `Tools/EditorScripts/build_landing_stage.py` (new, one file), its sublevel `Lvl_Boathouse_Landing` if that works (see below), its test | `build_boathouse.py`, `Source/**`, items, quests, dialogue |
| Narrative Builder | `Tools/EditorScripts/create_dialogue_landing.py` (new) and the inspect text in the POI script | canon docs, existing dialogue assets |
| Presentation Builder | `dress_landing.py` (new, art-level, tag `LandingDress`), audio placement, import additions in a new `import_art_landing.py` | gameplay actors, the POI script |
| Systems Engineer | only the capability in section 6, if approved | everything else |
| Visual Critic | reads captures only; writes findings | any file except its findings note |
| Gameplay Critic | plays the routes headless via tests and reads the spec; writes findings | any file except its findings note |
| Verifier / Integrator | `RebuildContent.bat` registration, review viewpoints in `Tools/Review/`, the full suite, `Package.bat`, merge | authoring content |

Workflow: **spec approved → build (POI, Narrative, Presentation, in that order because they share layout) → Visual and Gameplay critics in parallel, independent of the builders → reviser (the original builder, on the critics' findings only) → Verifier → Anthony.** The critics do not see the builder's self-assessment.

Open question for the pilot itself: whether `build_boathouse.py` can call a per-POI script (so the POI is a separate file the persistent map loads) or whether the POI must live in its own always-loaded sublevel. The first is simpler; the second isolates better. The Presentation Pass proved the sublevel pattern for dressing but not for gameplay actors. The POI Builder should try the per-POI script first and record what broke.

## 6. Missing reusable capability (expected to be non-empty)

The variants in section 2 need an actor or relocation to appear, disappear, or move by condition. The grammar today can change an inspectable's text (variants), a hazard's effect (`ActiveConditions`), a light or a sound (conditions), and a dialogue line. It cannot say "this actor is present, hidden, or at this other place when these conditions pass".

That is the one thing Phase 5 most plausibly needs as a reusable capability: a small, generic **conditional presence** rule (show or hide an actor, or pick one of N placements, from an `FDCGameplayCondition` list, restored silently on load, no save field beyond what the flags already give). It should be built once, by a Systems Engineer, with its own test, **before** the POI Builder starts, and only if Anthony approves it. The pilot's spec records exactly which content needs it, which is the evidence the strategy asks for.

If it is not approved, the pilot falls back to cosmetic variants only (lights, sounds, inspect text, the skiff's presence as a static mesh swap done at build time), and Mara's placement change is dropped. That is a smaller result but still answers questions 1 to 4.

## 7. Tests

- **Content validation:** every id resolves; inspect variants and dialogue conditions validate.
- **Interaction:** each interaction on the stage fires and reads correctly in each state.
- **Persistence:** save in each of the three states, diverge, F9, and the stage and Mara's placement match the save; no re-announce of the location; a save from before the cell existed loads with the stage in its initial state.
- **Alternate routes:** the stage is reachable and fully usable with a zero-investment build and with each build in the RPG layer; it does not block either Shore Watch route, the scavenger's patrol square, or Mara's sightline.
- **Integration:** `RunTests.bat` at least at the pre-pilot baseline (38 today), plus the new cell tests; rebuild leaves the art layer and the stage intact.
- **Regression:** existing map tests unchanged.

## 8. Review views (for the Visual Critic)

Add to `Tools/Review/Lvl_Boathouse.json`, each with an expectation:

- `landing_far`: the stage from the path, before anything is resolved. The lantern and skiff read as a place someone uses.
- `landing_coil`, `landing_combat`: the same camera in each resolved state. The difference must be legible in one glance without reading text.
- `landing_close`: the crate and note at interaction range.
- `landing_night_lamp` or the lamp with `wreck.power_cut` set: the dark lamp string.

## 9. Human acceptance (draft)

1. Play Shore Watch by the coil route. Walk to the landing stage. What changed since before, and did you notice without being told?
2. Reload the pre-Shore-Watch save. Play the combat route. Same camera, same place: is it clearly different, and does it feel like a consequence rather than a texture swap?
3. Save, quit, relaunch, load: is everything as saved?
4. Did it feel like a place someone made on purpose?

## 10. What success costs and what it measures

Record for the pilot: time from approved spec to playable candidate, time from candidate to accepted, number of new C++ classes (expected: one, the conditional presence rule), bespoke assets generated (target: at most one) and Meshy credits, Tier B pieces reused vs added, defects caught by critics before Anthony, defects Anthony found that the critics did not, and the test count added. The pilot is a success if the second cell after it would need no new C++ and no bespoke asset.

## 11. Stop conditions

- The spec cannot be filled in without a decision only Anthony can make: stop and ask.
- Anything needs a `SaveVersion` bump or a renamed id: stop.
- The Systems Engineer step is not approved: switch to the fallback in section 6.
- A shared file (`build_boathouse.py`, `Source/**`) would need an edit by the POI Builder: stop and hand it to the Integrator.
- Meshy spend would exceed the spec's number: stop.

## 12. Decisions (Anthony, 2026-09-30)

1. **Cell:** the Landing Stage (`shore.landing_stage`) is approved. The name is still a working name.
2. **Conditional presence:** approved. The Systems Engineer builds it, with its own test, before the POI Builder starts. The cosmetic-only fallback in section 6 is not needed unless the capability fails.
3. **Mara:** she moves to the stage on the coil route, as in section 2. This fits her role as the companion (`Design/Narrative/CHARACTERS.md` §2): the packed crate and loaded skiff read as the watch getting ready to travel.
4. **Timing:** the pilot runs during Phase 5, not the vertical slice.

Still out of scope for the pilot: the narrative draft's proposal that a talked-down scavenger keeps the watch while Mara is away (`QUEST_ARCS.md`). It is a candidate for the second cell.
