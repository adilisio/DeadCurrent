# POI Specification — The Settlement (`sombre.settlement`)

Copied from `TEMPLATE.md`. Contract: standard, with an NPC appendix (`VerticalSlicePhasePlan.txt` §11). Ids: `Design/POIs/sombre_ids.md` (the ledger wins). Story: beat script B2 and §5; `SLICE_DIALOGUE.md` §3–§7, §9. Kit: `Design/Kits/great_lakes_settlement_kit.md`. Placement guidance (advisory): Gemini's VS-05 kit critic and VS-08 spatial brief. Written in VS-07 (2026-10-01).

**Frame (VS-04):** X north, Y east, Z up, sea level 0; metres unless marked cm. *(planned)* positions are VS-08's greybox and are frozen at Checkpoint A.

---

## Identity

- **Stable POI id:** `sombre.settlement` (cell id). **No location id**: no settlement location exists in any approved list (plan §8.4, the beat script), and none is minted.
- **Working name:** the settlement (the town above the quay).
- **Region:** the settlement terrace (pad centre (−30, −15), radius 36 m, Z 5 m) above the quay.
- **Biome:** `N/A — an inland terrace. The waterline in front of it belongs to the harbor's zone (one polygon, one owner); no zones/settlement.json.`
- **Content status:** `SPEC`.
- **Lore status:** `PROVISIONAL`.
- **Owner:** Claude, VS-11. Files: `pointe_sombre/settlement.py`, `dress_sombre_settlement.py` → `Lvl_PointeSombre_Art_Settlement`, `Content/World/PointeSombre/Settlement/`, `Tools/Review/Lvl_PointeSombre/settlement.json`, `Source/DeadCurrent/Save/DCSombreSettlementMapTest.cpp`. The loft interior is `sombre.net_loft`'s; only the loft stair's up portal is this cell's.

## Player Promise

> Six people, six versions of why the light went dark, and every one of them is standing where their life happens.

## Discovery

- **What draws attention:** from the quay, the store's two-storey gable and its outside stair to the net loft, the smokehouse stack, people at work.
- **Intended approach:** up from the quay (quay → store 10–15 s, plan §5.5).
- **Marked / unmarked:** unmarked.
- **Discovery behavior:** none (no location id). The harbor's banner covers arrival.

## Spatial Role

- **Approximate footprint** *(planned, VS-08 composes; organic, not a grid; every structure seated on pilings, no flattened terrain)*:
  - `kit_store` (6 × 8 m, two storeys) at about (−40, −5), its gable to the street toward the quay, the outside loft stair on its harbor side
  - Odette's cottage (`kit_cottage`) at about (−22, 18), turned toward the harbor mouth
  - the Leclair house and porch (new composition `leclair_house`) at about (−12, −28), its ridge turned against the cottage's (Gemini: no parallel rooflines)
  - the Pruitts' salvage shed (`kit_salvage_shed`) at about (−48, −48), on the way west toward the headland (plan §5.5)
  - the smokehouse (`kit_smokehouse`) at about (−58, −22), near the shore; the net racks at about (−62, 0)
- **Nearby routes:** quay → store; quay → west headland passes the Pruitts' shed; settlement → cable hut (25–40 s) climbs north past the ridge; settlement → tower (part of the quay → tower leg).
- **Important sightlines:** the tower above the roofs (`tower_rock_from_settlement`, core, and `tower_silhouette`); the store reads as the anchor from the quay and the causeway (Gemini anchor test); roofs overlap in height, no flat block.
- **Exclusion zones:** the terrace is inside the harbor zone's `settlement terrace` pad exclusion (margin 6 m), so no recipe instance lands here. This cell requests named boxes in `zones/harbor.json` only if VS-08 moves a structure outside that pad: `store_door`, `net_racks`, `smokehouse_front`, `loft_stair_foot` (approach pads, 2 m deep). Hand-placed dressing keeps the same approaches clear: each door, each NPC's standing spot (1.5 m), the ticks wall, the loft stair foot.
- **Navigation requirements:** every door approach, porch, and the loft stair's foot is reachable on the terrain (porch steps under 45 cm; the kit's pilings carry the floors). No AI patrols; NPCs stand.

## Gameplay

- **Core interaction:** six conversations; the children's ticks; the salvage marked "A. GREY"; "Render a can" at the smokehouse; climb the loft stair. Later, Marthe's call to the loft (at `knows`).
- **Possible danger:** none.
- **Combat route:** `N/A — no combat. Tem never becomes hostile ("Force the Pruitts" is cut, §8.2 #3).`
- **Non-combat route:** the only route.
- **RPG / build checks:**
  - Odette's key needs `[Persuasion 2]`, or a tie plus the letter
  - Sigrun's shears need `[Engineering 2]`
  - the ticks show a chart variant if the player has `survey_chart`
  - Dell's confession (`[Persuasion 2]`, or `[Survival 2]` with the boots line) is **VS-16's** to place at the post; his conversation asset is the same
  
  A character with nothing spent still gets every account and every lead.
- **Reward:** `sombre_vault_key` (Odette), `lamp_oil` (smokehouse), information (accounts and leads), loot: `sombre.sigrun_bin`, `sombre.smokehouse_shelf` (`ammo_9mm`, `field_dressing`, `salvage_wiring`).
- **Exit state:** after the meeting, after-lines by outcome; Sigrun gone if exposed and Hale is present; windows lit along the harbor if `sombre.power_settlement` (the harbor's window lights).

## Environmental Story

1. **Clue:** the store — "No light, no ships, no stock." Empty shelves through the open front; the outside stair to the loft where the island meets.
2. **Clue:** the Leclair porch — rows of chalk ticks, evenly spaced, the same spacing on every wall; the youngest has drawn a lighthouse at the end of a row. With `survey_chart`: "The same spacing as the *Tern*'s chart."
3. **Clue:** the Pruitts' open shed — salvage marked "A. GREY" stacked inside.
4. **Clue:** the smokehouse — "Render a can" if you can stand the smell (oil for a light that never needed it).
5. **Clue `[P]`:** Compact dues chits nailed on doors (the Compact's reach before Hale). Default: kept if VS-11 has time; cut first if not.

- **What happened here (author-only):** the island lives off what the dark brings (the Pruitts) and waits for what the light brought (Marthe). PROVISIONAL.
- **What the player can infer:** the light failed on purpose; several people know more than they say.
- **What deliberately remains unresolved:** what the children's ticks are counting.
- **PROVISIONAL claims:** every account and line; the ticks' spacing matching the *Tern*'s chart; "A. GREY" salvage; the chits.
- **Links to existing lore:** `Design/Narrative/QUEST_ARCS.md` §2.3 (accounts), `MYSTERY_LEDGER.md` §5 (the pattern; never explained).

## State / Persistence

- **Persistent ids:** `sombre.odette`, `sombre.marthe`, `sombre.jonas`, `sombre.tem`, `sombre.dell`, `sombre.sigrun` (this script spawns each, exactly once, and owns every placement — appendix); containers `sombre.sigrun_bin`, `sombre.smokehouse_shelf`.
- **Discovered-location id:** `N/A — no settlement location id exists; do not mint sombre.settlement as a location.`
- **World flags:** sets `sombre.ticks_seen` (ticks); Marthe's `store` at `knows` sets `sombre.meeting_called` (§8.2 #4); Odette's keeper offer `sombre.keeper_odette`; Sigrun `sombre.sigrun_revealed`, `sombre.sigrun_helping`, and her unjam `sombre.vault_opened`. Reads `sombre.storm` (Dell's post, VS-16), `meeting_called`, `meeting_done`, `exposed_*`, `hale_arrived`.
- **Quest stages:** Marthe's `loft` entry at `sombre.characteristic` = `knows` gains the condition `sombre.meeting_called`; Odette's `knows` entry.
- **Conditions:** `WorldFlag`, `HasItem` (`survey_chart`, `cut_cable_end`, `sombre_vault_key`, `liv_letter`, `lamp_oil`), `QuestStage`, `SkillAtLeast` (dialogue checks).
- **Consequences:** `GiveItem` (`sombre_vault_key`, `lamp_oil`), `SetWorldFlag`.
- **Visible world-state variants:** the NPC appendix's presence states (the loft gathering, Dell at the post in the first storm, Sigrun gone); after-lines by outcome; Compact goods crates at Marthe's on the Line outcome `[P]` (default: cut).
- **Save impact:** new ids only.

## Asset Plan

### Tier A — Focal

| Asset | Role | Source | Meshy credits (est.) |
| --- | --- | --- | --- |
| Six islanders | the accounts | `Survival_Character` pipeline; the NPC body choice is Checkpoint B's (plan §12.3); distinct silhouettes by props meanwhile | 0 |
| The ticks wall and drawing | the pattern clue | `M_DC_Chalk` (shipped) on the kit's board wall | 0 |
| "A. GREY" salvage | the Pruitts' link to the wreck | `Smugglers_cove` crates and barrels (migrated, VS-05) + a stencil decal from project art | 0 |
| Shears on Sigrun's belt | the reveal | library prop if found in `Scene_Junkyard`; else a simple authored mesh | 0 |

### Tier B — Modular

| Piece | Where used | Already in the kit? |
| --- | --- | --- |
| `kit_store` | the store and the loft's outside stair and gable (`Stair_280`, `Gable_100`) | yes (one only: Gemini, no repeated store footprint or exterior stair) |
| `kit_cottage` | Odette's cottage | yes |
| `leclair_house` (house front and porch: `Wall_*`, `Deck_100`, `Post_100`, `Railing_200`) | the Leclair porch | modules yes; **composition no** — VS-11 adds `structures/leclair_house.json` (no new module) |
| `kit_salvage_shed` | the Pruitts' open shed | yes |
| `kit_smokehouse` | the smokehouse | yes |
| Net racks | Sigrun's place | trim (`Post_100`, `Beam_100`), not a composition |

### Tier C — Procedural

| Dressing | Recipe / placement | Collision | Exclusions |
| --- | --- | --- | --- |
| `N/A — inland terrace, inside the harbor zone's settlement-terrace pad exclusion. Clutter is hand-placed in working clusters (shed, store porch, racks), never a uniform scatter.` | hand placement in the art sublevel | NoCollision | door approaches, NPC spots, ticks wall, loft stair foot |

## Audio

The exterior wind bed; the smokehouse fire (loop, CC0, new in VS-11); a door creak at the store; hammering or net work near the racks (CC0 if found; else none). Gap: no voices, no music.

## Existing Systems Used

Friendly NPCs and dialogue (five assets placed here: `sombre_odette`, `sombre_marthe`, `sombre_tem`, `sombre_dell`, `sombre_sigrun`, plus `sombre_jonas` — six), inspectables with variants and per-variant verbs, inventory, loot containers, conditions/consequences, world flags, conditional presence, the cell portal (loft stair up), build checks (attributes/skills).

## Missing Reusable Capability

None. The meeting's movement is presence on the scene cut (shipped VS-03); the loft stair is a portal.

## Tests

- **Content validation:** `Content.Sombre.Dialogue` (every node has an unconditional choice; no tie leak; keeper choices exclusive; evidence choices need and remove the item; Marthe's call).
- **Interaction:** `Map.Sombre.Settlement`:
  - every first conversation opens
  - the ticks set `sombre.ticks_seen` once, with the chart variant when `survey_chart` is held
  - "Render a can" gives `lamp_oil`
  - Odette's key by `[Persuasion 2]`, and by tie plus letter
  - Sigrun's reveal with `cut_cable_end`
  - Marthe's call at `knows` sets `sombre.meeting_called`
  - the loft stair's up portal lands at `Anchor_LoftStair_Loft`
  
  **Not here:** Dell's two confession routes. Plan §19's Settlement row lists them, but VS-11's done-when defers them to VS-16 (`Map.Sombre.FalseLight`); the ledger records the discrepancy.
- **Persistence:** F5 after the key and the oil; diverge; F9.
- **Alternate routes:** zero investment: every account, the ticks (neutral text), oil, the stair.
- **Integration:** NPC spots and doors clear of dressing; the store reads from the quay.
- **Regression:** full suite; baseline 52 at VS-07.

## Review Views

`Tools/Review/Lvl_PointeSombre/settlement.json`. Exterior views trace `tower_site`. Cameras *(planned)*.

| Id | Camera (x, y, z) cm, pitch/yaw | Subject | Expected observation |
| --- | --- | --- | --- |
| `settlement_street` | (−6500, −1800, 700), 2 / 25 | the street | Setup `arrived`. People at work; the store's gable and loft stair; smoke; the tower above the roofs. |
| `store_marthe` | subject Marthe, offset in front | Marthe | Marthe at the store front, her trade legible. |
| `cottage_odette` | subject Odette | Odette | Odette at her cottage door, not looking at the tower. |
| `porch_leclair` | subject Jonas | Jonas | Jonas on his porch; the chalk wall behind him. |
| `shed_pruitts` | subject Tem | Tem | The open salvage shed with Tem, "A. GREY" salvage visible; not a copy of the store. |
| `racks_sigrun` | subject Sigrun | Sigrun | Sigrun at the net racks, the shears on her belt. |
| `ticks_close` | close on the ticks wall | the ticks | The chalk rows and the drawn lighthouse, readable. |
| `tower_silhouette` | (−3000, −2500, 700), 6 / 70 | the tower | Storm. The tower against the storm sky from the settlement. |

## Human Acceptance Checklist

1. Walk up from the quay. *Does it look like a place where people work, not a set of houses?*
2. Talk to everyone who will talk. *Which people did you remember? Did anyone feel like a quest dispenser?* (plan §22 step 3)
3. Find the children's chalk. *What do you think they are counting?*
4. Render a can at the smokehouse. *Worth it?*
5. Climb the loft stair. *Did the stair read as the way up?*

## Open Creative Decisions

- **Where Marthe stands.** Default: at a counter in the store's open front (the store's back room is not built, §8.9).
- **Dues chits `[P]`.** Default: kept if time allows; cut first.
- **Tem after exposure.** Default: stays (plan §8.4).
- **Dell's wet-boots wording (§8.2 #1).** Default: VS-09's proposed rewording of only the rain clause (for example, "Nobody walks the reef in this for fish."); Anthony may replace it at Checkpoint B. The choice stays `[Survival 2]` → `sombre.dell_confessed`.

---

## NPC Appendix

Each person is one friendly NPC spawned once by `settlement.py`, with one presence rule in `settlement.py` (first match wins; the default is the last state). Off-cell places are Integrator anchors (`sombre_ids.md` §9). The keeper on the gallery is the lighthouse's silhouette, not the person.

| Person | Persistent id / dialogue | Default place | Presence states (first match wins) | Notes |
| --- | --- | --- | --- | --- |
| Odette Beaudry | `sombre.odette` / `sombre_odette` | her cottage door | `loft` at `Anchor_LoftSeat_Odette` while `meeting_called`, `!meeting_done`; default | Remy's marker visit is `[P]` and **held for VS-20** (no presence target on a Remy actor) |
| Marthe | `sombre.marthe` / `sombre_marthe` | the store front | `loft` at `Anchor_LoftSeat_Marthe` while `meeting_called`, `!meeting_done`; default | chairs the meeting (the meeting is her `loft` entry) |
| Jonas Leclair | `sombre.jonas` / `sombre_jonas` | his porch | `loft` at `Anchor_LoftSeat_Jonas` (same conditions); default | the children are not modeled (§8.2 #10) |
| Tem Pruitt | `sombre.tem` / `sombre_tem` | the salvage shed | `loft` at `Anchor_LoftSeat_Tem` (same); default | never hostile; stays after exposure (default) |
| Dell Pruitt | `sombre.dell` / `sombre_dell` | the salvage shed | `loft` at `Anchor_LoftSeat_Dell` (same); `post` at `Anchor_DellPost` while `sombre.storm`, `!false_light_taken`, `!dell_confessed` (**added by the same owner in VS-16**); default | the post state comes after `loft` so the meeting wins; the confession conversation is the same asset wherever he stands. `headland.py` never spawns or references him |
| Sigrun Dahl | `sombre.sigrun` / `sombre_sigrun` | the net racks | `gone` (hidden, parked) while `exposed_sigrun`, `hale_arrived`, `meeting_done`; `loft` at `Anchor_LoftSeat_Sigrun` while `meeting_called`, `!meeting_done`; default | her unjam is dialogue setting `sombre.vault_opened`; she has no portal and no vault actor |

Mara (`sombre.mara`), Varga, and Hale are the harbor's actors. Their loft seats and Mara's porch placement are the harbor script's presence states (`Anchor_Mara_Porch`, `Anchor_LoftSeat_*`).

**The meeting's movement:** the attendees' `loft` states are deferred presence. They commit on the scene cut when the player climbs the loft stair (shipped VS-03 behavior), so nobody is seen walking into an empty loft. Before the call, everyone stays where the `knows` conversations need them.

## Pilot Record

- Spec committed: VS-07. First green map test: `Map.Sombre.Settlement` (VS-11). Existing classes only; new C++ 0; Meshy 0; kit `kit_store`, `kit_cottage`, `leclair_house` (new composition), `kit_salvage_shed`, `kit_smokehouse`; zone `N/A`; views as above; critic files at Checkpoint B.
