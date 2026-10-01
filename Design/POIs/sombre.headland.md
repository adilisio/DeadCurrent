# POI Specification — The West Headland and the *Ashland Grey* (`sombre.headland`)

Copied from `TEMPLATE.md`. Contract: standard (`VerticalSlicePhasePlan.txt` §11); **includes the *Ashland Grey*'s stern** (there is no separate Grey spec). **The second contract-built wilderness cell:** its Pilot Record is the comparison column after the cable hut (`SLICE_PRODUCTION_REPORT.md`). Ids: `Design/POIs/sombre_ids.md` (the ledger wins). Story: beat script B6 (with plan §8.2 #1–#3 applied), §8.7; `SLICE_DIALOGUE.md` §5–§6. Written in VS-07 (2026-10-01).

**Frame (VS-04):** X north, Y east, Z up, sea level 0; metres unless marked cm. *(planned)* positions are VS-08's greybox, frozen at Checkpoint A.

---

## Identity

- **Stable POI id:** cell `sombre.headland`. Location ids: `sombre.headland` (the post and hide; False Light `asked` → `found`) and `sombre.ashland_grey` (the stern).
- **Working name:** the West Head; the *Ashland Grey* (display texts; Anthony may rename).
- **Region:** the island's west end (pad `headland post`, centre (−18, −215), radius 7 m, Z 10.5 m), the reef causeway ((−46, −222) → (−108, −227)), and the *Grey*'s reef (rock pad, centre (−118, −228), radius 12 m).
- **Biome:** `great_lakes_rocky_shore`, zone file `Tools/Biomes/zones/headland.json` (one of the three required zones; it includes the causeway shore and the Grey's reef).
- **Content status:** `SPEC`.
- **Lore status:** `PROVISIONAL`. The wrecker hand is unnamed and PROVISIONAL.
- **Owner:** Claude, VS-16. Files: `pointe_sombre/headland.py`, `dress_sombre_headland.py` → `Lvl_PointeSombre_Art_Headland`, `Content/World/PointeSombre/Headland/`, `Tools/Review/Lvl_PointeSombre/headland.json`, `Tools/Biomes/zones/headland.json`, `Source/DeadCurrent/Save/DCSombreFalseLightMapTest.cpp`, `DCSombreGreyMapTest.cpp`. **Dell is the settlement's actor:** `headland.py` never spawns or references him (his post placement is the settlement script's presence state at `Anchor_DellPost`, added in VS-16 by the same owner).

## Player Promise

> I find out who shows the false light, by what they left behind or by what they tell me, and out on the reef a wreck holds the one part I need and the one fight on the island.

## Discovery

- **What draws attention:** the false-light post seen across the harbor from the quay (and from the crossing); during the storm the lantern burning in its hide. The *Grey*'s hull is a **staged reveal**: it comes into full view from the headland and the causeway, not as another always-visible landmark (Gemini VS-08 brief; plan §5.5's overlap from the quay is judged by VS-08's capture).
- **Intended approach:** quay → west headland, 45–60 s, past the Pruitts' shed; headland → *Grey* stern, 20–30 s, over the reef causeway at low water.
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** `sombre.headland` volume at the post and hide; `sombre.ashland_grey` volume on the stern deck. Banners once; silent on load.

## Spatial Role

- **Approximate footprint** *(planned)*: the post and a stone-and-driftwood hide on the pad (−18, −215); the causeway, 7 m wide at Z 0.6 m; the *Grey*'s stern deck on its reef at (−118, −228), with the winch, the wheelhouse, and the Pruitts' cache.
- **Nearby routes:** the quay → headland leg (past the Pruitts' shed); the causeway (deliberately quiet and short, plan §5.5).
- **Important sightlines:** from the post, the tower across the harbor (`tower_site` traced) and the quay; from the quay, the post. The *Grey* reveals from the headland's south-west brow and the causeway.
- **Exclusion zones** (in `zones/headland.json`, landed VS-06 schema, metres; *(planned)*):
  - `exclude_pads`: `headland post` (margin 4)
  - `post_hide`: circle, the hide, radius 5
  - `footprint_read`: box at the footprints, half (2, 3)
  - `dell_post`: circle at `Anchor_DellPost`, radius 2
  - `causeway_corridor`: box along the causeway, centre (−77, −225), half (32, 3.5), yaw −4 (the walking line stays clear)
  - `grey_combat`: circle at (−118, −228), radius 14 (the fight's ground and cover)
  - `winch_approach`, `cache_approach`: boxes, half (1.5, 1.5), at each
  - automatic radii: the wrecker, the inspectables, the containers, the location volumes
  - sightlines to the tower and across the harbor: `N/A — Tier C instances are under 3 m tall; no box.`
  - **Landed in VS-08** (`zones/headland.json`, written by the Integrator; this cell's file from VS-16): `exclude_pads` `headland post` (4); `post_hide` r 5; `footprint_read` box (−21, −209) half (2, 3); `dell_post` r 2; `grey_combat` circle centred on the hull (−119, −229), **r 11** (planned r 14 at (−118, −228); widen it here if the fight needs it); and, in place of `causeway_corridor`, `exclude_paths` `head_grey` (margin 1.5: the trail's 2 m half width plus 1.5 m, the planned 3.5 m) and `quay_head` (margin 1). `winch_approach` and `cache_approach` wait for the winch and the cache (VS-16). The causeway is now a rough, wandering bedrock spine (crest 0.36–0.9 m; `island.json` `roughness`), and the trail follows it.
- **Navigation requirements:** the causeway is walkable (terrain collision); the stern deck by hidden collision; **the wrecker needs a nav area on the stern deck** (a `NavMeshBoundsVolume` added by the Integrator when VS-16 needs it; VS-04's nav bounds cover the terrain grid, which includes the reef).

## Gameplay

- **Core interaction (False Light, two approaches only, §8.2 #2):**
  - **evidence:** take the lantern ("Take the lantern" → `false_lantern`, `sombre.false_light_taken`), with no build needed
  - **confession:** Dell confesses through `[Persuasion 2]`, or `[Survival 2]` with the wet-boots line reworded (§8.2 #1). In the first storm he is at the post, so "catching them at it" is meeting him here; it is the same conversation and the same confession, not a separate approach
  - Survival 2 at the footprints: "Two people. One drags a foot." (Tem limps.)
  - False Light closes by telling Varga a name (`sombre.varga_told`) or by bringing the lantern to the loft (`sombre.exposed_pruitts`)
  - the keeper offer to Dell after the confession is an outcome, not a third approach
- **Possible danger:** the wrecker hand on the *Grey*'s stern (scavenger archetype: warns first, avoidable by the hull's cover and crouch, fights with the pistol, persistent death). The reef at high water is visual only `[P]`.
- **Combat route:** fight the wrecker; loot the body and the cache.
- **Non-combat route:** crouch and use the hull's cover past the wrecker to the winch; the pawl is reachable without killing him.
- **RPG / build checks:** Survival 2 (footprints, Dell's boots), Persuasion 2 (Dell). The pawl needs no build. Force is cut: no "Tem keeps a shotgun" approach (§8.2 #3; no ranged NPC, no friendly-to-hostile switch). The beat script's scavenger-at-the-post variant needs the prologue and is cut (§8.2 #2).
- **Reward:** `false_lantern` (evidence), `clockwork_pawl` (from the winch), loot (`sombre.pruitt_cache`, `sombre.grey_wheelhouse`, the wrecker's body).
- **Exit state:** the lantern gone or still burning in storms; Dell confessed or not; the wrecker dead or alive (persistent); the pawl taken.

## Environmental Story

1. **Clue:** the hide — a driftwood-and-stone shelter, a lantern post, a burnt wick smell; the lantern burning in the storm.
2. **Clue:** footprints in the mud of the hide: two people, one drags a foot (Survival 2).
3. **Clue:** the causeway — rocks at low water, the hull looming ahead.
4. **Clue:** the *Grey*'s stern — a laker's stern on the reef, its deck winch with the pawl the tower's clockwork needs, the Pruitts' cache.

- **What happened here (author-only):** the Pruitts show the lantern so wrecks come to them; the *Grey* was one. PROVISIONAL.
- **What the player can infer:** the false light is deliberate and profitable; the Pruitts live off it.
- **What deliberately remains unresolved:** whether Remy's death is on them.
- **PROVISIONAL claims:** Dell's confession; the wrecker hand (unnamed); the *Grey*'s cargo and owners.
- **Links to existing lore:** `Design/Narrative/QUEST_ARCS.md` §2 (False Light).

## State / Persistence

- **Persistent ids:** `sombre.wrecker` (scavenger archetype; not in the narrative id block, required by plan §8.7), `sombre.pruitt_cache`, `sombre.grey_wheelhouse` (containers).
- **Discovered-location ids:** `sombre.headland`, `sombre.ashland_grey`.
- **World flags:** sets `sombre.false_light_taken` (lantern; also the quest's `done`). Reads `sombre.storm`, `sombre.reef_struck`, `sombre.dell_confessed`.
- **Quest stages (`sombre.false_light`):** `asked` → `found` on `LocationDiscovered sombre.headland`; `found` → `named` on `sombre.dell_confessed`, or (separately) `HasItem false_lantern`; `named` → `done` on `sombre.exposed_pruitts` or `sombre.varga_told`; `done` on enter sets `sombre.false_light_taken`.
- **Conditions:** `WorldFlag`, `HasItem`, `SkillAtLeast`, `ActorDead sombre.wrecker` (body loot variant), `LocationDiscovered`.
- **Consequences:** `GiveItem` (`false_lantern`, `clockwork_pawl`), `SetWorldFlag`.
- **Visible world-state variants:** three light actors at the post, one for each moment:
  - `Lantern_Crossing`: `ActiveConditions = [!sombre.reef_struck]`, the light seen from the deck in B0 (ledger §3.1)
  - `Lantern_Storm`: `[sombre.storm, !sombre.false_light_taken]`, burning in the first storm and, if never taken, the second
  - the lantern prop: hidden by presence once `false_light_taken`
  
  Also: Dell at the post (the settlement's rule: `sombre.storm`, `!false_light_taken`, `!dell_confessed`); the wrecker's body after death. The pawl inspect says "The pawl is gone." once `HasItem clockwork_pawl` or `sombre.clockwork_freed`. `night_headland` (B9): lit or dark.
- **Save impact:** new ids only.

## Asset Plan

### Tier A — Focal

| Asset | Role | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| False lantern | evidence | CC0 `Lantern_01` (shipped) or Smugglers `SM_wooden_lantern_01` | 0 |
| The *Grey*'s stern | landmark, the fight's ground | composed: `AbandonedPowerPlant` walls and beams + hull-plate pieces + `M_DC_Wreck` (plan §12.2) | 0 |
| Deck winch with the pawl | the pawl | `Scene_Junkyard` gears and drum composed | 0 |
| The wrecker hand | the fight | the scavenger archetype's body (shipped) | 0 |
| The hide | the false light's place | driftwood (`DriftWoodPack`, migrated VS-06) and rock | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| `N/A — the post is a landmark, not a kit building; the Grey's stern is composed, not kit. The Pruitts' shed is the settlement's kit_salvage_shed.` | — | — |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| Shore rock, talus, cobble, driftwood, scrub | `great_lakes_rocky_shore`, zone `headland`, polygons `headland_tip` (the exposed tip; fetch reads it exposed), `causeway_shore` (the causeway's sheltered side), `grey_reef` (the reef pad's rim), exposure `auto`, owner's seed, density 1.0. **The ≤ 20-line agent test** (plan §10.4): this zone file is added with no tool change | NoCollision | the list under Spatial Role |

## Audio

Heavy surf and wind (exposed), the lantern's flame in the storm, the *Grey*'s hull groaning on the reef, the winch's chain. The wrecker's warning line (shipped archetype). CC0. Gap: no music.

## Existing Systems Used

Inspectables (with skill variants), the inspectable-plus-presence pickup pattern (§6 #13), flicker lights with world `ActiveConditions`, conditional presence, hostile AI (scavenger archetype: notice, warn, chase, persistent death), loot containers and body loot, quests (transitions), location discovery, dialogue (Dell, Varga, by their owners), the shoreline recipe.

## Missing Reusable Capability

None. (A second hostile in a fallback layout is `[P]`.)

## Tests

- **Content validation:** `Content.Sombre.QuestGraph` (False Light reaches `done` by both closings).
- **Interaction:** `Map.Sombre.FalseLight`:
  - **both approaches:** evidence (take the lantern with no build, then Varga or the loft); confession via `[Persuasion 2]`, and separately via `[Survival 2]`, then Varga; both combined
  - the keeper offer
  - the lights' conditions: the crossing lantern before the strike; the storm lantern in the storm, not after the taking
  - Dell at the post during the first storm
  
  `Map.Sombre.Grey`: the wrecker warns, is avoidable, dies, is looted, stays dead through F9; the pawl with no build. **Not one test named Headland.**
- **Persistence:** F5 after the lantern and after the wrecker's death; diverge; F9.
- **Alternate routes:** zero investment: take the lantern; sneak to the pawl.
- **Integration:** `Map.Sombre.BiomeExclusions` green with `headland.json` (nothing in the causeway corridor, the combat area, the hide).
- **Regression:** full suite; baseline 52 at VS-07.

## Review Views

`Tools/Review/Lvl_PointeSombre/headland.json`. Cameras *(planned)*.

| Id | Camera (x, y, z) cm, pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `headland_post` | beside the hide, about (−2600, −20500, 1250), 0 / 60 | the lantern | Storm. The lantern burning in the hide; Dell (once VS-16 adds him); the tower across the harbor (`tower_site` traced). |
| `grey_stern` | on the causeway's end, about (−10500, −22600, 250), 2 / −160 | the stern | Default. The stern deck, the winch, the wrecker's position and the hull's cover. |
| `night_headland` | from the quay toward the post | the post | Lantern taken vs not: lit or dark. |

## Human Acceptance Checklist

1. (Checkpoint D) Walk out to the headland. *Did the post pull you, and did the Grey surprise you?*
2. Find out who shows the lantern: take it, or get Dell to confess if your build allows. *Did your build give you different options here?* (plan §22 step 8)
3. Deal with the wrecker however you like. *Was he avoidable? Was the fight worth having?*

## Open Creative Decisions

- **The wrecker** (parked, plan §16.1): default an unnamed wrecker hand, PROVISIONAL (Checkpoint C).
- **Is one fight enough?** Default yes; a ranged enemy is a scope change only Anthony makes (Checkpoint C).
- **Dell's boots line (§8.2 #1).** Default: VS-09's minimal rewording of the rain clause.
- **The *Grey* from the quay.** Default: a staged reveal; at most a small partly-hidden silhouette from the quay (VS-08 judges by capture).

## Pilot Record (comparison column after the cable hut)

- Spec committed: VS-07. First green map tests: `Map.Sombre.FalseLight`, `Map.Sombre.Grey` (VS-16). Existing classes only; new C++ 0 (target 0); Meshy 0; kit `N/A`; zone `headland` (the ≤ 20-line test); views `headland_post`, `grey_stern`, `night_headland`; critic files at Checkpoint D.
