# POI Specification — The Cable Hut (`sombre.cable_hut`)

Copied from `TEMPLATE.md`. Contract: standard (`VerticalSlicePhasePlan.txt` §11). **The first contract-built wilderness cell:** its Pilot Record is the baseline column of the production comparison (`Design/POIs/SLICE_PRODUCTION_REPORT.md`). Ids: `Design/POIs/sombre_ids.md` (the ledger wins). Story: beat script B4 (the conduit). Written in VS-07 (2026-10-01).

**Frame (VS-04):** X north, Y east, Z up, sea level 0; metres unless marked cm. *(planned)* positions are VS-08's greybox, frozen at Checkpoint A.

---

## Identity

- **Stable POI id:** `sombre.cable_hut` (cell id and location id).
- **Working name:** the Cable Hut (display text; Anthony may rename).
- **Region:** the island's north shore below the north ridge, west of the tower rock (pad `cable hut`, centre (80, 95), radius 9 m, Z 3.2 m).
- **Biome:** `great_lakes_rocky_shore`, zone file `Tools/Biomes/zones/cable_hut.json` (one of the three required zones; it includes the north waterline under the tower so no other cell scatters there).
- **Content status:** `SPEC`.
- **Lore status:** `PROVISIONAL`.
- **Owner:** Claude, VS-13. Files: `pointe_sombre/cable_hut.py`, `dress_sombre_cable_hut.py` → `Lvl_PointeSombre_Art_CableHut`, `Content/World/PointeSombre/CableHut/`, `Tools/Review/Lvl_PointeSombre/cable_hut.json`, `Tools/Biomes/zones/cable_hut.json`, `Source/DeadCurrent/Save/DCSombreCableHutMapTest.cpp`. It never builds the vault's cable gallery.

## Player Promise

> An Authority hut on a lonely shore, a mast you saw from the ridge, and a crawlway that goes somewhere nobody meant you to go.

## Discovery

- **What draws attention:** the Authority mast seen over the north ridge from the settlement and from the tower (orientation aid, Gemini's hierarchy); mussel lines running out from the hut to the rock; the hut's loose door heard before it is seen (`[P]`, audio later).
- **Intended approach:** from the settlement over the ridge (25–40 s) or down the cut stair on the tower's north side (15–25 s, plan §5.5). Just off the natural route.
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** `ADCLocationVolume` `sombre.cable_hut` around the hut and the mast foot; banner once; silent on load.

## Spatial Role

- **Approximate footprint** *(planned)*: `kit_cable_hut` (3 × 3 m) on the pad at (80, 95); the mast foot about (86, 104); the conduit mouth at `Anchor_VaultConduit_Out` in the hut's floor or behind it in the rock; the mussel lines from the waterline to the rock under the tower.
- **Nearby routes:** settlement → cable hut over the ridge; tower → cable hut by the cut stair; the north shore east to the lower door under the tower's cliff (the lighthouse's lower entrance).
- **Important sightlines:** the mast from the settlement and the tower gallery; the tower from the hut (`tower_site` traced); the mussel lines from above (the tower path).
- **Exclusion zones** (in `zones/cable_hut.json`, landed VS-06 schema, metres; *(planned)*):
  - `exclude_pads`: `cable hut` (margin 3)
  - `hut_door`: box, centre at the door, half (1.5, 2)
  - `conduit_mouth`: circle, at `Anchor_VaultConduit_Out`, radius 3
  - `mast_foot`: circle, radius 3
  - `tower_stair_path`: box along the cut stair's foot, half (2, 8)
  - `mussel_read`: box at the Survival reading spot, half (2, 2)
  - `lower_door_approach` (requested by the lighthouse spec): circle at the lower door, about (36, 194), radius 4; `lower_door_shore_path`: box along the shore path to it, half (2, length)
  - automatic radii: the portal, the notice and mussel inspectables, the location volume, the container
- **Navigation requirements:** the pad, the hut floor, the conduit mouth, and the shore path to the lower door are walkable on the terrain collision; the hut is open (no door to open, or a real `ADCDoor` left ajar `[P]`).

## Gameplay

- **Core interaction:** read the Authority notice; inspect the mussel lines (Survival 2 shows the way into the rock); "Crawl in" at the conduit (the third way into the vault); loot the hut's kit box.
- **Possible danger:** none on the shore (the vault's cable gallery is the vault's).
- **Combat route:** `N/A.`
- **Non-combat route:** the only route.
- **RPG / build checks:** Survival 2 on the mussel lines: the reading that shows the way from the shore. **The conduit has no check:** it is visible from inside the hut; the skill only points to it from outside (beat script B4).
- **Reward:** access to the vault (`sombre.vault_opened`); loot `sombre.cable_hut_kit` (`ammo_9mm`, `field_dressing`, `salvage_wiring`).
- **Exit state:** the vault open; the box looted.

## Environmental Story

1. **Clue:** the Authority notice on the hut — a Great Lakes Maritime Authority cable station notice, faded: no entry, live cable.
2. **Clue:** the mussel lines — ropes from the shore to the rock under the tower, crusted; Survival 2: "Somebody tended these. They run straight to a hole in the rock."
3. **Clue:** the conduit — a crawlway with a cable tray, going down toward the tower.
4. **Clue:** the mast — an Authority mast, guyed, its aerial gone.

- **What happened here (author-only):** the cable from the node ran out through here to the lake. PROVISIONAL.
- **What the player can infer:** the tower is wired to something beyond the island.
- **What deliberately remains unresolved:** where the cable goes.
- **PROVISIONAL claims:** the notice text; the mussel lines' purpose; the mast.
- **Links to existing lore:** `Design/world_bible.md` (the Maritime Authority, PROVISIONAL).

## State / Persistence

- **Persistent ids:** `sombre.cable_hut_kit` (container).
- **Discovered-location id:** `sombre.cable_hut`.
- **World flags:** the conduit portal sets `sombre.vault_opened`. No flag of its own.
- **Quest stages:** none (the quest moves on `vault_opened`, lighthouse spec).
- **Conditions:** `SkillAtLeast Skill.Survival 2` (mussel reading variant).
- **Consequences:** `SetWorldFlag sombre.vault_opened` (portal).
- **Visible world-state variants:** none (the hut does not change).
- **Save impact:** new ids only.

## Asset Plan

### Tier A — Focal

| Asset | Role | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| Authority mast | landmark | composed from `AbandonedPowerPlant` / `Scene_Junkyard` pipes and frame | 0 |
| The notice | clue | `SM_Notes` + an authored texture | 0 |
| Mussel lines | clue | rope (`Rope001` CC0) on the shore; mussel decals `[P]` | 0 |
| Conduit mouth and cable tray | the entry | `AbandonedPowerPlant` pipe/tray pieces | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| `kit_cable_hut` | the hut | yes |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| Shore rock, cobble, driftwood, scrub | `great_lakes_rocky_shore`, zone `cable_hut`, polygon `north_shore` (the north waterline from the north ridge's end to under the tower's cliff), exposure `auto` (open north: mostly exposed), owner's seed, density 1.0 | NoCollision | the list under Spatial Role |

## Audio

Wind (exposed shore), surf, a guy wire's hum and slap on the mast, the hut's loose door `[P]` (later content, not VS-08). CC0. Gap: no voice.

## Existing Systems Used

Inspectables with a skill variant, loot container, location discovery, the cell portal (conduit), world flags, the shoreline recipe.

## Missing Reusable Capability

None.

## Tests

- **Content validation:** the container's items resolve (`Content.Validate`).
- **Interaction:** `Map.Sombre.CableHut`: discovery once; the notice; the mussel reading only with Survival 2; "Crawl in" sets `sombre.vault_opened` and lands at `Anchor_VaultConduit_In` (with or without the vault's geometry: the anchor exists); the loot.
- **Persistence:** F5 after looting; diverge; F9; the box stays looted.
- **Alternate routes:** zero investment enters by the conduit (the zero-investment vault route; `Map.Sombre.VaultRoutes` re-asserts it at the merge).
- **Integration:** `Map.Sombre.BiomeExclusions` green with `cable_hut.json` (nothing at the door, the conduit, the lower door's approach).
- **Regression:** full suite; baseline 52 at VS-07.

## Review Views

`Tools/Review/Lvl_PointeSombre/cable_hut.json`. Both trace `tower_site`. Cameras *(planned)*.

| Id | Camera (x, y, z) cm, pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `cable_hut_approach` | from the ridge path, about (6000, 4000, 1800), −8 / 50 | the hut from above | The mast over the ridge; mussel lines in the water; the dark tower reads to the east. |
| `cable_hut_door` | in front of the hut, about (7600, 9000, 500), 0 / 70 | the hut | The Authority notice, the open hut, the conduit mouth visible inside. |

## Human Acceptance Checklist

1. (Checkpoint B/C) From the ridge, find the hut. *Did the mast pull you here?*
2. Read the notice; look at the mussel lines. *Did you see the way in before you found it?*
3. Crawl in. *Did it feel like a discovery rather than a door?*

## Open Creative Decisions

- **The hut's door.** Default: open (no door actor); the loose-door sound is later content.
- **Mussel decals `[P]`.** Default: rope only in VS-13.

## Pilot Record (baseline column of the production comparison)

- Spec committed: VS-07. First green map test: `Map.Sombre.CableHut` (VS-13). Existing classes only; new C++ 0 (target 0); Meshy 0; kit `kit_cable_hut`; zone `cable_hut`; views `cable_hut_approach`, `cable_hut_door`; Tier C share from the manifest vs hand placements; critic files at Checkpoint C. Spec commit → first green test, and candidate → accepted, are measured from commit timestamps (plan §14).
