# POI Specification — <working name>

Copy this file to `Design/POIs/<poi_id>.md` and fill every section. Source contract: `Design/content_production_strategy.md` §6. Ownership rules: `Design/POIs/README.md`.

**Ready to build** means: no field below is blank or "TBD", except Open Creative Decisions, which must be answered or explicitly parked with Anthony's sign-off. A POI that cannot be written down cleanly here is not ready. Do not begin building it. Write "N/A — <reason>" when a field truly does not apply; do not leave it empty.

Grounding examples: the Wrecked Survey Launch (`shore.survey_launch`) in `Design/technical_architecture.md` "Point of interest" is the reference for how specific each section should be.

---

## Identity

- **Stable POI id:** `<area>.<name>` (lowercase, snake_case after the dot). Immutable once accepted. Becomes the `LocationId` if the POI is discoverable.
- **Working name:** (player-facing display name, if discoverable)
- **Region:**
- **Biome:** (name of the biome recipe, or "hand-dressed" if none exists yet)
- **Content status:** `SPEC` / `BUILDING` / `CANDIDATE` / `READY FOR ANTHONY` / `ACCEPTED`
- **Lore status:** `CANON` / `PROVISIONAL` (list every provisional claim in Environmental Story). Never explain the Current. Do not decide factions or Mara's backstory.

## Player Promise

One sentence. Why will the player be glad they investigated?

> 

## Discovery

- **What draws attention:** (a shape, a sound, a light, an out-of-place object. It must be visible or audible before the player is close.)
- **Intended approach:** (direction and distance from the nearest known route)
- **Marked / unmarked:** (unmarked is the default; say why if marked)
- **Discovery behavior:** (location volume, banner, journal entry, or none. Silent restore on load.)

## Spatial Role

- **Approximate footprint:** (cm bounds, in the map's frame: +X out the boathouse door, lake at -Y)
- **Nearby routes:** (the paths and POIs it sits beside)
- **Important sightlines:** (from where it must be visible; what must stay hidden until close)
- **Exclusion zones:** (where nothing procedural or decorative with collision may go: doorways, patrols, sightlines, interaction traces)
- **Navigation requirements:** (nav mesh, AI patrol squares, reachable-on-foot proof)

## Gameplay

- **Core interaction:**
- **Possible danger:** (hazard, enemy, or none)
- **Combat route:**
- **Non-combat route:**
- **RPG / build checks:** (attribute, skill, or perk conditions; what each adds. A character with nothing spent must still complete the POI.)
- **Reward:** (items from the existing item vocabulary, information, access, or a state change)
- **Exit state:** (what is true when the player is done, and what the POI looks like afterwards)

## Environmental Story

Two to five deliberate beats, in the order a player is likely to find them. Each beat names a place, an object, and what it shows, not what it means.

1. **Clue:** 
2. **Clue:** 
3. **Clue:** 

- **What happened here:** (author-only; may never be stated in game)
- **What the player can infer:**
- **What deliberately remains unresolved:**
- **PROVISIONAL claims:** (each one, marked. Anthony confirms before it is treated as canon.)
- **Links to existing lore:** (`Design/world_bible.md` sections)

## State / Persistence

- **Persistent ids:** (every actor that saves state: `<area>.<thing>`; never rename once accepted)
- **Discovered-location id:**
- **World flags:** (`<area>.<fact>`)
- **Quest stages:** (quest id and stage ids, or none)
- **Conditions:** (which existing `FDCGameplayCondition` types, on which actors)
- **Consequences:** (which existing consequence types, and when they fire)
- **Visible world-state variants:** (what changes in the world for each flag or stage, and what the change looks like from the review views)
- **Save impact:** none / new ids only / needs a `SaveVersion` bump (say why)

## Asset Plan

Apply the tier rule: the player studies it, use a focal asset; a repeated authored vocabulary, use the kit; the player absorbs it, use dressing.

### Tier A — Focal

Story-critical or close-view assets. One row each. Prefer: owned pack/Fab → CC0 → Meshy → custom. Meshy only for Tier A, and only after the library has been searched.

| Asset | Role in the clues | Source | Meshy credits (est.) |
| --- | --- | --- | --- |

### Tier B — Modular

Reusable authored kit pieces. Reuse before adding.

| Piece | Where used | Already in the kit? |
| --- | --- | --- |

### Tier C — Procedural

Connective dressing. NoCollision unless a task says otherwise. List the recipe or the manual placement, the density, the seed, and the exclusion zones. No bespoke generation.

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |

## Audio

(Loops, one-shots, and what state each follows. CC0 or owned only. Gaps are written down, not invented.)

## Existing Systems Used

(List each from the shared grammar: interaction, inventory, containers, health/damage, AI, dialogue, quests, conditions/consequences, world flags, persistence, location discovery, hazards, attributes, skills, perks.)

## Missing Reusable Capability

**Normally empty.** If it is not, name the capability, show the exact content that the existing grammar cannot express, show why data plus layout plus a combination of existing conditions cannot, and describe the smallest generic extension. A POI builder does not implement it. Stop and hand it to a Systems Engineer with Anthony's approval.

## Tests

- **Content validation:** (ids resolve; item, quest, and dialogue references validate)
- **Interaction:** (each interaction fires its consequence; the prompt and inspect text match)
- **Persistence:** (save, diverge, F9, and the state is restored; no re-announce; legacy save loads)
- **Alternate routes:** (each route in Gameplay, including the zero-investment build)
- **Integration:** (the POI beside its neighbors; patrols, sightlines, and traces are not blocked)
- **Regression:** (existing suites that must stay green, with the baseline count)

## Review Views

Fixed captures for the Visual Critic. Each has a camera position, a target, and an expectation of what reads first.

| Id | Camera (x, y, z), yaw/pitch | Subject | Expected observation |
| --- | --- | --- | --- |

Add these to `Tools/Review/<map>.json`.

## Human Acceptance Checklist

The walkthrough Anthony performs, in order. Each step: where to go, what to do, what should happen, and one subjective question.

1. 

## Open Creative Decisions

Questions only Anthony can answer. Each one names the default the builder will use if he does not choose.

- 
