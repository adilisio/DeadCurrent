# DEAD CURRENT — Hybrid Content Production Strategy

Status: strategic production direction. This document does **not** replace the active phase plan. Finish and accept the current phase before advancing. It defines how DEAD CURRENT should scale once its gameplay grammar is proven.

## 1. Goal

DEAD CURRENT should grow like a content factory without looking or feeling factory-made.

The production model is:

**high-quality focal assets + modular authored kits + procedural connective tissue + AI agents working against strict contracts + independent review + human acceptance**

The objective is not maximum map size. It is to make each additional location, quest, encounter, and stretch of wilderness cheaper to build while preserving the qualities the project is trying to establish:

- curiosity
- deliberate discovery
- RPG agency
- consequence
- atmosphere
- persistence
- close-range visual credibility
- original identity

The guiding distinction is:

> Procedural systems build the connective tissue. Designers and agents deliberately place the thing worth finding.

The world may be generated in places. The reason to explore it should not feel generated.

---

## 2. What DEAD CURRENT should borrow from high-throughput AI-made games

Large AI-assisted games become possible when content is assembled from reusable grammar rather than built as unrelated one-offs.

DEAD CURRENT should deliberately move toward a state where agents can say:

- place a location
- give it a discovery hook
- fill secondary dressing from a biome recipe
- author a few environmental clues
- connect existing interactions
- add loot from an existing item vocabulary
- express state through the shared rule language
- add tests
- capture review views
- hand the result to a human

That is the important lesson.

The project should **not** copy a low-fidelity procedural asset strategy wholesale. Unreal Engine gives us a different opportunity: use real PBR materials, skeletal characters, authored meshes, sound, lighting, animation, PCG, instancing, Data Assets, and hand-built hero spaces while still making most production describable as data and repeatable scripts.

---

## 3. The three asset tiers

Every visual object should be treated as one of three production tiers.

### Tier A — Focal / hero content

These are things the player studies, remembers, fights with, talks to, or makes decisions about.

Examples:

- named NPCs
- creatures
- weapons
- quest objects
- relay housings
- terminals
- unusual machinery
- landmark structures
- distinctive vehicles
- dungeon focal rooms
- faction-specific equipment

Preferred sources:

1. suitable owned marketplace/Fab assets
2. high-quality CC0 assets
3. bespoke Meshy-generated assets
4. custom modeling when genuinely required

These assets receive close-range review. They should hold up when the player stands a few feet away.

Do not procedurally approximate a story-critical object merely because it is faster.

### Tier B — Modular authored kit

These assets establish the vocabulary of a place and are reused heavily.

Examples:

- wall and roof modules
- docks
- doors
- windows
- industrial beams
- pipe sets
- electrical boxes
- crates
- barrels
- furniture
- road barriers
- shoreline structures
- generic boats
- reusable building shells
- faction dressing kits

A good modular kit turns one set of assets into dozens of believable places.

Agents should prefer composition and variation of proven modules over generating a new unique asset for every location.

### Tier C — Procedural connective tissue

These are things the player mostly absorbs rather than studies.

Examples:

- rocks
- gravel
- sand variation
- mud
- grass
- branches
- small driftwood
- generic shoreline litter
- rust/decal variation
- puddles
- small debris
- repeated background scrap
- distant foliage
- non-critical terrain dressing

These are the primary target for:

- Unreal PCG
- Hierarchical Instanced Static Meshes
- deterministic seeded placement
- material variation
- decal systems
- spline placement
- biome recipes

Tier C should make the world dense without making agents manually place every rock.

**Default rule:** if the player can make a meaningful decision about it, it probably should not exist only as procedural filler.

---

## 4. The 90/10 world-building model

A useful production target is:

**~90% connective environment**
- terrain
- vegetation
- generic debris
- repeated architecture
- ambient clutter
- background structures

**~10% deliberate authored interest**
- landmarks
- encounters
- clues
- loot moments
- NPCs
- decisions
- unique objects
- surprising compositions
- environmental stories

The exact percentages are not a quota. The principle is.

Procedural generation should reduce the cost of the environment surrounding interesting content, not decide what the interesting content is.

The long-standing DEAD CURRENT rule remains:

> Someone should have deliberately placed the interesting thing behind the hill.

---

## 5. Biome recipes

As the world expands, generic environmental density should come from named recipes rather than thousands of unrelated hand placements.

Example conceptual recipe:

### Great Lakes rocky shoreline

Inputs:
- black rock set
- wet sand
- mud bands
- driftwood
- freshwater debris
- rusted maritime scrap
- sparse dead branches
- shoreline grass
- small washed-up containers

Controls:
- density
- seed
- slope limits
- distance from water
- clustering
- scale variation
- rotation variation
- exclusion zones
- collision policy
- nav exclusion
- sightline protection

The recipe creates a believable baseline.

The POI specification then says where deliberate content goes.

Biome recipes should be:

- deterministic
- reproducible
- cheap enough for the low-spec target
- visually reviewed
- authored in broad zones rather than painted over critical interactions
- prevented from blocking important routes, traces, AI, or landmarks

PCG is not a substitute for level design.

---

## 6. The content cell / POI contract

The scalable unit of production should eventually be a **content cell**: one small location or encounter that can be built, tested, reviewed, and integrated with limited impact on neighboring content.

Each POI should have a specification before an agent builds it.

Minimum POI contract:

### Identity
- stable POI id
- display name if discoverable
- area/biome
- purpose in the game

### Discovery
- what makes the player notice it
- approximate sightline / approach
- whether it is marked or unmarked
- discovery-state behavior

### Player promise
One sentence answering:

> Why will the player be glad they investigated?

### Gameplay
- primary interaction
- optional danger
- combat / non-combat possibilities
- build checks if any
- loot/reward
- exit condition

### Story
- 2–5 environmental beats
- what happened here
- what remains ambiguous
- links to existing lore
- PROVISIONAL lore called out explicitly

### State
- persistent ids
- world flags
- quest stages if relevant
- discovered-location id
- condition/consequence usage
- what visibly changes later

### Asset needs
- Tier A focal assets
- Tier B kit pieces
- Tier C biome/dressing
- audio needs
- what already exists in the library

### Tests
- actor/content validation
- interactions
- persistence
- alternate routes
- map integration
- regression targets

### Human acceptance
- exact walkthrough
- specific subjective questions
- screenshots/viewpoints worth reviewing

A POI that cannot be described cleanly in this format is probably not ready to build.

---

## 7. Shared gameplay grammar is the factory

The project's reusable systems are what make content production scalable.

Existing grammar already includes:

- interaction interface
- inventory and containers
- firearm/damage/health
- hostile AI
- dialogue
- data-driven quests
- shared conditions and consequences
- world flags
- stable persistence ids
- save/load
- location discovery
- inspectable variants
- switchable hazards
- attributes
- skills
- perks
- build-gated information/options

Future systems should become part of this grammar only when real content proves they are needed.

A content agent should normally be able to create a new situation by combining the grammar rather than modifying foundational C++.

The desired direction is:

**new content increasingly means new data, layout, art, dialogue, and tests — not a new gameplay subsystem.**

---

## 8. Agent production roles

As the contracts mature, DEAD CURRENT can use multiple specialized agents rather than one agent doing everything.

These are roles, not necessarily permanently running agents.

### World Architect

Owns:
- regional layout
- travel rhythm
- landmarks
- POI spacing
- routes
- sightlines
- biome boundaries
- dependencies between content cells

Does not write every quest or decorate every prop.

### POI Builder

Receives a POI contract.

Owns:
- local layout
- interactables
- containers
- hazards
- environmental clues
- state integration
- map/content scripts
- POI-specific tests

Should use existing systems first.

### Narrative / Quest Builder

Owns:
- dialogue
- quest stages
- authored conditions/consequences
- environmental text
- character reactions
- continuity with the world bible

Does not independently establish major canon.

### Art / Presentation Builder

Owns:
- asset selection
- Tier A/B sourcing
- biome dressing integration
- material assignment
- lighting/audio hooks
- readability
- art-layer work

Does not change gameplay to make the art easier.

### Systems Engineer

Used only when a POI exposes a real missing reusable capability.

Owns:
- smallest generic system extension
- tests
- documentation
- backwards compatibility

A Systems Engineer should not invent features merely because future content might use them.

### Visual Critic

Receives fixed captures and written expectations.

Checks:
- composition
- readability
- scale
- material failures
- lighting
- clutter
- landmark pull
- visual hierarchy
- important clues disappearing

Does not silently redesign gameplay.

### Gameplay Critic

Checks:
- player agency
- route clarity
- checks and consequences
- exploit/soft-lock risk
- pacing
- reward
- persistence behavior
- whether the location actually fulfills its player promise

### Verifier / Integrator

Owns:
- full automated suite
- rebuild
- cook/package
- smoke test
- merge/integration conflicts
- stable ids
- regression confirmation

The builder should not be the only reviewer of its own work.

---

## 9. Recommended agent loop

For substantial content:

**Specification**
→ World Architect / creative direction

**Build**
→ POI + Narrative + Presentation work

**Independent review**
→ Visual Critic + Gameplay Critic

**Revision**
→ original builder or a focused reviser

**Verification**
→ automated tests + rebuild + package + integration

**Human acceptance**
→ Anthony plays the result

Only accepted content becomes the baseline for the next expansion.

This is more important than raw agent count.

Ten agents without contracts can create ten times the integration debt.

---

## 10. Parallelism rules

Parallel agents become valuable only after boundaries are stable.

Safe candidates for parallel work:

- two different POIs in separate sublevels/content folders
- art asset sourcing for one POI while another agent authors dialogue
- independent critics
- isolated tests
- documentation/review
- separate Data Assets with no shared generated file

Dangerous concurrent work:

- multiple agents editing the same generated map script
- multiple agents changing shared rule enums
- simultaneous save-format changes
- multiple agents rewriting the same quest/dialogue asset generator
- large shared config changes
- agents moving/renaming persistent ids

Before large-scale parallel production, establish ownership boundaries so each agent has a small set of files it is allowed to change.

Prefer content-specific files/sublevels/data specs over giant shared files.

---

## 11. Content packaging structure

As the project grows, prefer a predictable per-area/per-POI shape where practical.

Conceptually:

```
Content/
  World/
    Regions/
      <Region>/
        Biomes/
        POIs/
          <PoiId>/
            Art/
            Audio/
            Data/

Design/
  POIs/
    <poi_id>.md

Tools/
  ContentSpecs/
    <poi_id>.*

Tests/
  or matching source-level test grouping
```

The exact Unreal folder structure should follow proven engine constraints rather than this example literally.

The important property is **ownership**: an agent should be able to build one POI without touching half the project.

---

## 12. Procedural generation guardrails

Procedural systems should obey hard exclusion rules.

Never procedurally place content where it can:

- block a critical doorway
- obstruct an NPC patrol
- break navigation
- intercept firearm traces
- cover an interactable
- hide a required clue
- destroy a deliberate sightline
- overlap a persistent actor
- make save/load nondeterministic

Prefer seeded generation.

Keep gameplay-critical actors authored.

If a procedural result becomes narratively important, promote it into authored content.

---

## 13. Quality gates

A scalable content pipeline needs a consistent definition of done.

### Gate 1 — Build
- editor compiles
- content regenerates
- no broken references

### Gate 2 — Automated gameplay
- relevant unit/system tests
- map integration tests
- save/load
- regression suite

### Gate 3 — Visual review
- fixed screenshots
- contact sheet
- expected landmark/readability checks
- obvious defects fixed

### Gate 4 — Performance
- low-spec PlayTest settings
- no obvious frame-time regression
- reasonable texture and draw-call budget

### Gate 5 — Package
- Development cook/package
- smoke launch

### Gate 6 — Human acceptance
Anthony receives:
- exact walkthrough
- things to judge
- known limitations
- decisions needing creative direction

No phase or important POI should be considered accepted solely because an agent says it is complete.

---

## 14. Metrics worth watching

Do not optimize for lines of code or number of locations.

Useful production metrics:

- time from approved POI spec to playable candidate
- time from candidate to accepted
- percentage of POI behavior expressed with existing systems
- number of new foundational systems required per POI
- asset reuse vs unique hero asset count
- automated regression count
- bugs found by independent critic before human playtest
- rebuild reproducibility
- package success rate
- frame time on the low-spec profile
- human-reported "worth exploring" rate

The production system is improving when later locations become cheaper **without becoming more generic**.

---

## 15. Anti-patterns

Avoid:

### Proceduralizing the memorable parts
A thousand generated landmarks are not a substitute for ten memorable ones.

### Unique code for every location
If every POI needs new C++, the content factory has failed.

### Generating a new asset for every prop
Reuse Tier B aggressively.

### Building the whole world before the vertical slice
The vertical slice is still the first real game.

### Scaling agent count before contracts
Parallelism magnifies ambiguity.

### One agent building and approving its own work
Independent review catches different failures.

### Filler for the sake of density
Empty quiet space is better than meaningless clutter.

### Letting PCG own gameplay state
Gameplay-critical placement and persistence must remain deterministic and explicit.

### Chasing open-world acreage
Density, discovery rhythm, and authored payoff matter more.

---

## 16. Rollout plan

### Now — Presentation Pass

Finish the current Presentation Pass.

This strategy does not expand its scope.

The Presentation Pass is building important prerequisites:
- art-layer separation
- reusable asset import
- materials
- presentation conventions
- fixed review captures
- provenance
- packaging discipline

### Phase 5 — World State

Use Phase 5 as the first place to formalize the content-cell contract.

The milestone should still focus on its actual purpose:

**choices visibly alter locations and NPC behavior.**

Do not turn Phase 5 into mass content production.

Build one or a few tightly bounded examples and make them conform to the POI/state contract.

### Phase 6 — Vertical Slice

The 60–90 minute lighthouse slice is where the production model should be proven end-to-end.

By the end of the vertical slice, aim to have reusable production recipes for:

- settlement spaces
- wilderness POIs
- dungeons
- encounter dressing
- dialogue/quest content
- environmental storytelling
- biome dressing
- world-state variants
- review and verification

The vertical slice should demonstrate that several distinct locations can be built from the same grammar without feeling copied.

### Production

Only after the vertical slice is accepted should DEAD CURRENT aggressively scale through parallel POI/content agents.

At that point:
- freeze mature contracts where possible
- assign content cells to isolated agents
- use PCG for connective tissue
- reserve unique assets for focal content
- run critic/reviser/verifier loops
- integrate in small batches
- keep human acceptance as the creative gate

---

## 17. Near-term infrastructure to build only when needed

Potential production infrastructure, in priority order:

1. formal POI specification template
2. per-POI ownership/folder conventions
3. reusable biome/PCG recipe architecture
4. deterministic exclusion volumes and anchor points
5. POI content validation
6. screenshot expectations per POI
7. content-specific automated map tests
8. branch/worktree conventions for parallel agents
9. integration checklist
10. production dashboard/report generated from specs/tests

Do not build all ten because they are listed here.

Introduce each when the next real milestone creates the need.

---

## 18. The intended end state

Eventually the cost profile should look like this:

The first shoreline location requires:
- systems
- save architecture
- interaction
- quests
- assets
- art pipeline
- testing
- review tooling

A later shoreline location should require mostly:
- a POI spec
- composition
- content data
- a few unique focal assets
- dialogue/story
- tests
- review

A new region should require:
- a new biome recipe
- a modular kit
- a small number of landmark/hero assets
- new POI specifications

That is the content-production inflection point.

The target is not to eliminate authored work.

The target is to make authored attention land almost entirely on the things players will remember.
