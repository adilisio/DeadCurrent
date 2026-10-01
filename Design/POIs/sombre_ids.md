# Pointe Sombre — the Id Ledger (Phase 6)

Every id the slice uses: who owns it, who may set or create it, where it came from. **Integrator-owned** (`VerticalSlicePhasePlan.txt` §11, §13.2). A builder who needs an id that is not here asks the Integrator; nobody mints one in a branch. Written in VS-07 (2026-10-01) from the union of the beat script (`Design/Narrative/SLICE_WRONG_CHARACTERISTIC.md`), the dialogue text (`Design/Narrative/SLICE_DIALOGUE.md`, its change list and §11), and the plan (§5.3, §8.2–§8.8, §13.2–§13.3, §19), checked against what is shipped on `main` and Codex's VS-07 preflight.

**Columns.** *Owner* defines the id and is the only one who may change its meaning (for every Phase 6 id that is the Integrator, through this file). *Set / created by* is the cell, asset, or quest that writes it. *Status*: **shipped** (in a committed script or asset; never rename), **reserved** (fixed here, built later), **alias** (a superseded spelling; do not create), **held** (reserved for VS-20 only).

**Rules.**
- Never rename a shipped id, flag, quest stage, or persistent id. Old saves must load (`CLAUDE.md`).
- Areas: `sombre.` is the slice; `shore.`, `wreck.` are the accepted shore content; `watch.`, `player.tie.` are the prologue's; `dev.` is test fixtures and no content reads it.
- Item ids carry no area prefix, as shipped (`ammo_9mm`); `sombre_vault_key` keeps the dialogue's spelling. Do not also create `sombre.vault_key`.
- A cell script references only its own actors and the anchors below (`tk.anchor`); a presence rule's targets live in one script.

---

## 1. Shipped before Phase 6 (reuse; do not remint)

| Kind | Id | Lives in |
| --- | --- | --- |
| Quest | `shore.watch`, stages `accepted`, `return_killed`, `return_coil`, `done_killed`, `done_coil` | `Tools/ContentSpecs/quests/shore_watch.py` |
| Flags | `shore.path_cleared`, `shore.relay_recovered`, `shore.relay_inspected`, `shore.voice_discussed`, `shore.mara_heard_kill` | shore quest and `dialogue/mara_intro.py` |
| Flags | `wreck.log_read`, `wreck.mara_told`, `wreck.mara_pressed` | `mara_intro.py`. **`wreck.log_read` is not `sombre.log_read`** (§3): different facts; keep both prefixes |
| Items | `ammo_9mm`, `pistol_service`, `field_dressing`, `salvage_wiring`, `radio_coil`, `survey_chart` | `items/shore.py`. **The Sounder Chart is `survey_chart`.** Do not mint `sounder_chart` |
| Persistent actor | `boat.scavenger` | Shore Watch. The slice's wrecker reuses the archetype with its own id (§7) |
| Location | `shore.landing_stage`, `shore.survey_launch` | Phases 3 and 5 |
| Dialogue | `mara_intro` | shore Mara. The slice's Mara is a different asset, `sombre_mara` |
| Skills | `Skill.Engineering`, `Skill.Survival`, `Skill.Persuasion` (the slice's checks use 2) | Phase 4 |

## 2. Cells, places, and discovery

| Cell id | Kind | Spec | Discoverable location id | Status |
| --- | --- | --- | --- | --- |
| `sombre.crossing` | exterior set (the *Ida*'s deck, offshore) | `sombre.crossing.md` | **none** (plan §8.4: not discoverable) | reserved |
| `sombre.harbor` | exterior | `sombre.harbor.md` | `sombre.harbor` | reserved |
| `sombre.settlement` | exterior | `sombre.settlement.md` | **none** (no settlement location exists in any approved list; do not mint one) | reserved |
| `sombre.lighthouse` | exterior + base room + lamp room at height + the vault cell | `sombre.lighthouse.md` (integrated) | `sombre.light` (the tower), `sombre.vault` (the vault) | reserved |
| `sombre.net_loft` | abstracted interior | `sombre.net_loft.md` | **none** (the meeting is not a discovery; do not mint one) | reserved |
| `sombre.cable_hut` | exterior + open hut | `sombre.cable_hut.md` | `sombre.cable_hut` | reserved |
| `sombre.headland` | exterior headland + the *Ashland Grey*'s stern on the reef | `sombre.headland.md` | `sombre.headland`, `sombre.ashland_grey` | reserved |
| `sombre.remy` | Remy's marker and boat at the harbor mouth | none until VS-20 | none now (VS-20's spec decides) | **held** (§13) |
| `arch_test` | VS-04 architecture fixture, not content | none | none | shipped (fixture) |

Cell-script tags: `Cell:<cell>` on every actor a cell script makes (`tk.own`); outliner folder `Cells/<cell>`. Script names: `pointe_sombre/<cell>.py` with `<cell>` in `crossing`, `harbor`, `settlement`, `lighthouse`, `vault`, `net_loft`, `cable_hut`, `headland`, and `remy` (held). The vault has its own script because §13.3 splits implementation, not because it is an eighth spec.

**§11 vs §8.4.** §8.4's cell sentence lists the *Ashland Grey*'s stern as a cell and omits the net loft; §11 (the list VS-07 follows) folds the *Grey* into the headland and gives the loft its own spec. Recorded, not "fixed" by adding or dropping a spec.

## 3. World flags

### 3.1 The flag whose meaning had to be decided: `sombre.storm`

**Decision (VS-07, 2026-10-01): `sombre.storm` is gameplay world state, not the atmosphere selector.**
- **Meaning:** "a storm is on Pointe Sombre now", for content that only happens in weather: the vault's live water, the false light burning, Dell at the post, "Take the bearing" without Engineering.
- **Owner:** the Integrator (this ledger).
- **Set by:**
  - **the crossing's wheelhouse door, variant `strike`** (`SetWorldFlag sombre.storm`, beside the shipped `sombre.reef_struck`). Implemented in VS-10 by the crossing's owner.
  - each `sombre.characteristic` `done_*` stage on enter (WP-NARR)
- **Cleared by:** `sombre.characteristic` `knows` on enter (WP-NARR).
- **Why the strike sets it** (not "the core at new game", as the preflight proposed): nothing in the existing grammar sets a flag at a new game without C++ or a player act. Every new game passes through the strike before reaching the island, so the strike is the earliest existing act. It needs no new capability and no save change.
- **The deck before the strike** (B0) is the only time the storm is "on" without the flag. One thing there needs it: the false light seen from the deck. It is a separate light at the post, `Lantern_Crossing`, with `ActiveConditions = [!sombre.reef_struck]`, owned by the headland script (§3.3, `sombre.headland.md`).
- **The three atmosphere looks are not driven by `sombre.storm`,** and no cell may make them so. They stay exactly as shipped in VS-04 and tested by `Map.Sombre.Atmosphere`:
  - dusk storm: neither `hale_arrived` nor `meeting_done`
  - calm: `hale_arrived`, not `meeting_done`
  - night: `meeting_done`
  
  Plan §8.8's row naming `sombre.storm` as a look driver is superseded by the shipped behavior (`technical_architecture.md`, "Atmosphere").
- **Coherence check:** strike → storm on (dusk-storm look) → `knows` clears it, sets `hale_arrived` (calm look) → meeting → `done_*` sets it again with `meeting_done` (night look). Look and flag agree at every stage, though neither drives the other.

### 3.2 Story flags (`sombre.`)

Source codes: **B** the beat script's id list, **D** `SLICE_DIALOGUE.md` §11, **P** plan §8.2 / §8.7 new ids, **L** this ledger.

| Flag | Meaning | Set by | Cleared by | Read by | Src | Status |
| --- | --- | --- | --- | --- | --- | --- |
| `sombre.reef_struck` | the crossing is over; the player is on the island | crossing portal `strike` | — | respawn rule (core), Mara/Ida presence | P | **shipped** (core, `crossing.py`) |
| `sombre.storm` | a storm is on now (§3.1) | crossing portal `strike`; quest `done_*` | quest `knows` | vault live water, lantern, Dell's post, DF loop | B | reserved |
| `sombre.mara_tie_asked` | Mara asked the tie question | `sombre_mara` `crossing_tie` | — | `sombre_mara` entries | D | reserved |
| `sombre.ticks_seen` | the children's chalk ticks were inspected | settlement ticks inspectable | — | `sombre_mara` `ticks` | B | reserved |
| `sombre.mara_ticks` | Mara has spoken about the ticks | `sombre_mara` `ticks` | — | `sombre_mara` | D | reserved |
| `sombre.cable_cut_found` | the cut feed was found | lighthouse feed box | — | quest `lamp_room` → `vault` | B | reserved |
| `sombre.vault_opened` | the vault is open, by any route | hatch portal (key variant), lower-door portal (`pry`), conduit portal (either side), `sombre_sigrun` (`reveal` → help) | — | quest `vault` → `vault_inside`; hatch and lower-door second variants; respawn rule (core) | B | **shipped** (read by core respawn) |
| `sombre.log_read` | the Authority keeper's log was read | vault log inspectable | — | lost-log variant; Mara | B | reserved |
| `sombre.panel_read` | the node panel was read | vault panel (normal **and** destroyed variant, §8.2 #5) | — | quest `knows` gate | B | reserved |
| `sombre.mara_panel` | Mara has spoken about the panel | `sombre_mara` `panel` | — | `sombre_mara` | D | reserved |
| `sombre.bearing_taken` | the DF bearing was taken | vault DF loop | — | chart table variant | B | reserved |
| `sombre.liv_note_found` | Liv's note was taken | vault chart table / note (inspectable + presence, §8.2 #8) | — | quest `knows` gate | B | reserved |
| `sombre.vault_floor_isolated` | the lower gallery's floor grid is isolated at the breaker | vault breaker (Engineering 2) | — | live-water `ActiveConditions` | P | reserved |
| `sombre.hale_arrived` | the storm has passed; Hale's cutter is in | quest `knows` on enter | — | atmosphere (core), Hale presence, cutter presence | B | **shipped** (core reads it) |
| `sombre.hale_met` | the player has spoken to Hale | `sombre_hale` | — | `sombre_hale` `offer` | D | reserved |
| `sombre.hale_crew_up` | Hale's crew is sent up the tower | `sombre_hale` | — | feed-box "Splice the feed" second variant; crew dressing `[P]` | B | reserved |
| `sombre.sigrun_revealed` | Sigrun was confronted with the cable end | `sombre_sigrun` `reveal` | — | Hale's line; `sombre_sigrun` | B | reserved |
| `sombre.sigrun_helping` | Sigrun helps (has the wrench; came down) | `sombre_sigrun` | — | sea cock variant | B | reserved |
| `sombre.feed_spliced` | the lamp feed is spliced | lamp-room feed box (Engineering 2, or `hale_crew_up` variant) | — | panel "Seat the card" | B | reserved |
| `sombre.light_line` | the card is seated: the light is on the Line | vault panel "Seat the card" | — | exclusions, outcomes, after-lines, tower light | B | reserved |
| `sombre.power_settlement` | the node feeds the settlement | vault panel "Throw the settlement feed" | — | exclusions, outcomes, window lights | B | reserved |
| `sombre.node_destroyed` | the sea cock was opened | vault sea cock | — | flooded variants, live water, lighthouse sea-mouth bubble | B | reserved |
| `sombre.clockwork_freed` | the clockwork turns again | lamp-room clockwork | — | "Fill and light the burner" | B | reserved |
| `sombre.light_hand` | the light is lit by hand | lamp-room lamp | — | outcomes, tower light | B | reserved |
| `sombre.light_decided` | any physical decision act was done (summary) | every act that sets `light_line`, `power_settlement`, `node_destroyed`, `light_hand` | — | `sombre_marthe` meeting choices | D | reserved |
| `sombre.keeper_odette` | Odette keeps the light | `sombre_odette` | — | keeper silhouette (lighthouse), after-lines | B | reserved |
| `sombre.keeper_dell` | Dell keeps the light | `sombre_dell` | — | keeper silhouette (lighthouse), after-lines | B | reserved |
| `sombre.dell_confessed` | Dell confessed (Persuasion 2 or Survival 2) | `sombre_dell` | — | false light `found` → `named`; Dell's post presence | B | reserved |
| `sombre.false_light_taken` | the false lantern will not burn again | lantern inspectable "Take the lantern"; quest `sombre.false_light` `done` on enter (both may set it) | — | lantern lights, Dell's post | B | reserved |
| `sombre.varga_told` | the player gave Varga the name | `sombre_varga` | — | false light `named` → `done` | B | reserved |
| `sombre.meeting_called` | Marthe called the island to the loft (§8.2 #4) | `sombre_marthe` `store` at `knows` | — | `sombre_marthe` `loft` entry; loft attendees presence | P | reserved |
| `sombre.exposed_pruitts` | the lantern was given at the loft | `sombre_marthe` meeting | — | false light `done`; reactions; Tem's entry | B | reserved |
| `sombre.exposed_odette` | the sign-in board was given at the loft | `sombre_marthe` meeting | — | Odette's entries, keeper offer | B | reserved |
| `sombre.exposed_liv` | Liv's note was given at the loft | `sombre_marthe` meeting | — | tie line in the meeting | B | reserved |
| `sombre.exposed_sigrun` | the cable end was given at the loft | `sombre_marthe` meeting | — | Sigrun gone (with `hale_arrived`) | B | reserved |
| `sombre.player_named_tie` | the player named the tie at the loft | `sombre_marthe` meeting | — | `sombre_mara` `promise` | B | reserved |
| `sombre.odette_forgiven` | Odette forgiven (board and note both given) | `sombre_marthe` meeting | — | Odette's entries, keeper offer | B | reserved |
| `sombre.meeting_done` | the loft has decided | `sombre_marthe` meeting ("That's all.") | — | atmosphere (core: night), quest outcomes, after-lines, loft stair night variant | B | **shipped** (core reads it) |
| `sombre.night_quay_seen` | the player came down from the loft onto the night quay once | loft down-portal night variant | — | the Line light's one-time prelude (VS-17) | P | reserved |
| `sombre.slice_end` | the end card has been shown | default: `sombre_varga` "Where next?" (VS-19 chooses; §8.2 #11) | — | story card (VS-19) | P | reserved |

### 3.3 Prologue and shore flags the slice reads or writes (existing names, not slice mints)

| Flag | Status | In the slice |
| --- | --- | --- |
| `player.tie.sister`, `player.tie.partner`, `player.tie.took_in` | named in the prologue doc and `SLICE_DIALOGUE.md`; in no content spec yet | the crossing's `sombre_mara` `crossing_tie` sets exactly one, or none. Owner: the ledger; set only by Mara's tie node (and, later, the prologue) |
| `watch.mara_travelling` | prologue doc | set by Liv's letter on the crossing |
| `watch.revealed` | prologue doc | set by `sombre_mara` `watch` |
| `watch.away` | prologue doc | not read by the slice |
| `shore.liv_asked`, `shore.promise_told`, `shore.letter_read`, `shore.liv_admitted`, `shore.left_on_ida` | prologue doc only; not in `shore_watch.py` or `mara_intro.py` | `crossing_tie` reads `!shore.liv_asked`; `sombre_mara` `promise_*` sets `shore.promise_told`. Owner: the ledger (the prologue may later set them; the slice never renames them) |
| `wreck.mara_pressed` | **shipped** | `sombre_mara` `crossing` reads it |

### 3.4 Fixture flags (never read by content)

`dev.arch_moved`, `dev.arch_unlocked`, `dev.crossmap_shore`, `dev.crossmap_slice` (shipped; `arch_test.py`, `DCSombreArchitectureMapTest.cpp`).

## 4. Items

| Item | Role | Given by | Removed by | Owner of the definition | Status |
| --- | --- | --- | --- | --- | --- |
| `liv_letter` | recap on the crossing; Odette's tie-key condition | crossing letter inspectable | — | WP-NARR (`items/sombre.py`) | reserved |
| `liv_note` | evidence (Liv); `liv_note_found` gates `knows` | vault note (inspectable + presence) | loft meeting | WP-NARR | reserved |
| `liv_chart` | static copy of Liv's chart | vault chart table "Take a copy" | — | WP-NARR | reserved |
| `vault_access_log` | evidence (Odette) | vault sign-in board | loft meeting | WP-NARR | reserved |
| `cut_cable_end` | evidence (Sigrun); her reveal | lamp-room feed box | loft meeting | WP-NARR | reserved |
| `false_lantern` | evidence (the Pruitts) | headland lantern "Take the lantern" | loft meeting | WP-NARR | reserved |
| `section_key_compact` | Hale's card | `sombre_hale` | panel "Seat the card" / card variant of the settlement feed | WP-NARR | reserved |
| `clockwork_pawl` | hand light, zero investment | *Grey* winch (headland) | lamp-room clockwork "Fit the pawl" | WP-NARR | reserved |
| `lamp_oil` | hand light | smokehouse "Render a can"; `sombre_hale` | lamp "Fill and light the burner" | WP-NARR | reserved |
| `sombre_vault_key` | Odette's hatch key (the beat script's list omits it; §8.5 and `SLICE_DIALOGUE.md` include it) | `sombre_odette` (`key_press`, `key_tie`) | not removed (the hatch keeps working by `vault_opened`) | WP-NARR | reserved |
| `ammo_9mm`, `field_dressing`, `salvage_wiring` | slice loot (reward, not economy) | the slice's containers (§7) | — | **shipped** (`items/shore.py`); placement is the container's cell | shipped |
| a slice-only novel loot item | `[P]` only if Checkpoint C finds loot flat | — | — | none until Anthony asks | not an id |

## 5. Quests and stages

| Quest | Stages, in order (stage ids are saved; never rename) | Owner | Status |
| --- | --- | --- | --- |
| `sombre.characteristic` | `arrived` (start), `lamp_room`, `vault`, `vault_inside`, `knows`, `done_line`, `done_hand`, `done_dark` | WP-NARR (`quests/sombre_characteristic.py`); proposed asset `DA_Quest_SombreCharacteristic` | reserved |
| `sombre.false_light` | `asked` (start), `found`, `named`, `done` | WP-NARR (`quests/sombre_false_light.py`); proposed asset `DA_Quest_SombreFalseLight` | reserved |

- `knows` on enter: `ClearWorldFlag sombre.storm`, `SetWorldFlag sombre.hale_arrived`. Each `done_*` on enter: `SetWorldFlag sombre.storm`. Power and destruction are flags, not stages.
- Both quests start only from `sombre_varga` `first` (every choice there). Flags set before then chain the quest forward when it starts (existing behavior).
- **Copper Buyer** (`QUEST_ARCS.md` §2.5, trimmed) is Sigrun's dialogue and flags only: **no quest id.** Do not mint `sombre.copper_buyer`.

## 6. Dialogue assets and the nodes that write state

Nine assets (plan §8.4, `SLICE_DIALOGUE.md`), owner WP-NARR (`Tools/ContentSpecs/dialogue/sombre_<name>.py`). Node ids are not saved, but builders and tests copy them; the entry nodes and every node that writes state are fixed here.

| Asset | Speaker / actor | Entry nodes (first match wins) | Nodes that write state |
| --- | --- | --- | --- |
| `sombre_mara` | Mara, `sombre.mara` | `crossing_tie`, `crossing`, `promise`, `after_line`, `after`, `panel`, `ticks`, `town` (plus non-entry `watch`, `third`) | `crossing_tie`: one tie flag or none, and `sombre.mara_tie_asked`. `watch`: `watch.revealed`. `ticks`/`ticks_same`: `sombre.mara_ticks`. `panel`/`panel_note`: `sombre.mara_panel`. `promise_*`: `shore.promise_told` |
| `sombre_varga` | Captain Ines Varga, `sombre.varga` | `after_line`, `after_hand`, `after_dark`, `waiting`, `first` | `first`: `StartQuest` both quests. The name: `sombre.varga_told`. Default end-card trigger (VS-19): "Where next?" sets `sombre.slice_end` |
| `sombre_odette` | Odette Beaudry, `sombre.odette` | `after_keeper`, `after_disgraced`, `after`, `knows`, `first` | `key_press` / `key_tie`: `GiveItem sombre_vault_key`. Keeper offer: `sombre.keeper_odette` |
| `sombre_marthe` | Marthe, `sombre.marthe` | `after_line`, `after_hand`, `after_power`, `after_dark`, `loft`, `store` | `store` at `knows`: the call (§8.2 #4) sets `sombre.meeting_called`; `loft` gains that condition. Meeting: `exposed_*`, `player_named_tie`, `odette_forgiven`, `meeting_done`; `RemoveItem` of each evidence item given |
| `sombre_tem` | Tem Pruitt, `sombre.tem` | `exposed`, `first` | none (no new flag) |
| `sombre_dell` | Dell Pruitt, `sombre.dell` | `keeper`, `confessed`, `first` | `first` (`[Persuasion 2]` or `[Survival 2]`): `sombre.dell_confessed`. Keeper offer: `sombre.keeper_dell` |
| `sombre_sigrun` | Sigrun Dahl, `sombre.sigrun` | `taken`, `helping`, `revealed`, `first` | `first` with `cut_cable_end`: `sombre.sigrun_revealed`; help: `sombre.sigrun_helping` and the unjam sets `sombre.vault_opened` |
| `sombre_hale` | Warden Casimir Hale, `sombre.hale` | `after_line`, `after_hand`, `after_flooded`, `after_power`, `after`, `offer`, `first` | `first`: `sombre.hale_met`. Card: `GiveItem section_key_compact`. Crew: `sombre.hale_crew_up`. Oil: `GiveItem lamp_oil` |
| `sombre_jonas` | Jonas Leclair, `sombre.jonas` | `after_line`, `after`, `first` | none |

**Aliases (do not create):** `sombre_pruitts` (split into `sombre_tem` and `sombre_dell`), `sombre_meeting` (the meeting is an entry of `sombre_marthe`). Both appear in the beat script's id list; `SLICE_DIALOGUE.md`'s change list retired them.

## 7. Persistent actor ids

Actors that save state (classes implementing `IDCPersistent`: friendly NPCs, the scavenger archetype, doors, loot containers, item pickups). Evidence "pickups" in the slice are inspectables with `GiveItem` + a flag + presence, which save through the flag and need no persistent id (§8.2 #8).

| Id | Actor | Spawned by (exactly one script) | Status |
| --- | --- | --- | --- |
| `sombre.mara` | Mara, **one actor**, five placements by presence (deck rail, quay, store porch, loft, night quay) | `harbor.py` spawns her (her quay placement is the slice's longest-lived); every placement is one presence rule in that script. *The beat script's `sombre.mara_*` is a documentation habit, not ids.* | reserved |
| `sombre.varga` | Varga on the *Ida* at the quay | `harbor.py` | reserved |
| `sombre.odette` | Odette (cottage door; loft; gallery if keeper) | `settlement.py`; her loft and gallery placements are presence rules in `settlement.py` | reserved |
| `sombre.marthe` | Marthe (counter; loft chair) | `settlement.py` | reserved |
| `sombre.jonas` | Jonas (porch; loft) | `settlement.py` | reserved |
| `sombre.tem` | Tem (salvage shed; loft) | `settlement.py` | reserved |
| `sombre.dell` | Dell (shed; headland post in the first storm; loft; gallery if keeper) | `settlement.py` only. His post placement is a presence state in `settlement.py`, added by the same owner in VS-16. `headland.py` never spawns or references him | reserved |
| `sombre.sigrun` | Sigrun (net racks; loft; gone once `exposed_sigrun` and `hale_arrived`) | `settlement.py` | reserved |
| `sombre.hale` | Hale (absent until `hale_arrived`; gangway; loft) | `harbor.py` (VS-15) | reserved |
| `sombre.wrecker` | the unnamed wrecker hand on the *Grey* (PROVISIONAL); scavenger archetype; persistent death. **Not in the narrative id block; required by plan §8.7** | `headland.py` | reserved (gap filled here) |
| `sombre.tower_door` | the tower base room's real door (`ADCDoor`) | `lighthouse.py` | reserved |
| `sombre.vault_locker` | loot container | `vault.py` | reserved |
| `sombre.pruitt_cache` | loot container on the *Grey* | `headland.py` | reserved |
| `sombre.grey_wheelhouse` | loot container | `headland.py` | reserved |
| `sombre.sigrun_bin` | loot container (copper bin) | `settlement.py` | reserved |
| `sombre.cable_hut_kit` | loot container | `cable_hut.py` | reserved |
| `sombre.smokehouse_shelf` | loot container | `settlement.py` | reserved |
| `sombre.remy_marker` | Remy's marker | VS-20 only | **held** |

**Hale's crew** at the tower base is `[P]` dressing: no id unless Anthony promotes them. **Children** are not modeled (§8.2 #10).

## 8. Portals

Portal labels are unique in the map; variant ids are unique within their portal. **The hatch and the lower door are portals, not inspectables** (resolves the clash between `SLICE_DIALOGUE.md` change 4, "the hatch is an inspectable", and plan §5.3, where both are `ADCCellPortal`s). The beat script's `sombre.vault_hatch` / `sombre.vault_door` persistent ids are **aliases**: a portal saves nothing, and its use sets `sombre.vault_opened`.

| Portal label | Owner script | Variants (first match wins) | Destination anchor | Status |
| --- | --- | --- | --- | --- |
| `Ida_WheelhouseDoor` | `crossing.py` | `strike` "Tell Varga about the light": `SetWorldFlag sombre.reef_struck`, `SetWorldFlag sombre.storm` (the second consequence is added in VS-10, §3.1) | `Anchor_CrossingExit_Quay` | **shipped** (first consequence) |
| `TowerStair_Up` | `lighthouse.py` | `climb` "Climb the stair" | `Anchor_TowerStair_Lamp` | reserved |
| `TowerStair_Down` | `lighthouse.py` | `descend` "Go down" | `Anchor_TowerStair_Base` | reserved |
| `VaultHatch_Out` (base room) | `lighthouse.py` | `open` "Go down" if `sombre.vault_opened`; `unlock` "Unlock the hatch" if `HasItem sombre_vault_key` → `SetWorldFlag sombre.vault_opened`; locked text "The hatch is locked. There's a keyhole, and fresh scratches round it." | `Anchor_VaultHatch_Bottom` | reserved |
| `VaultHatch_In` (vault upper room) | `vault.py` | `up` "Go up" | `Anchor_VaultHatch_Top` | reserved |
| `VaultLower_Out` (sea cliff) | `lighthouse.py` | `open` "Go in" if `sombre.vault_opened`; `pry` "Pry the jam" if `SkillAtLeast Skill.Engineering 2` → `SetWorldFlag sombre.vault_opened`; locked text: VS-12 writes it (default "The door is jammed in its frame.") | `Anchor_VaultLower_In` | reserved |
| `VaultLower_In` (vault lower gallery) | `vault.py` | `out` "Go out" | `Anchor_VaultLower_Out` | reserved |
| `VaultConduit_Out` (cable hut) | `cable_hut.py` | `crawl` "Crawl in" → `SetWorldFlag sombre.vault_opened` | `Anchor_VaultConduit_In` | reserved |
| `VaultConduit_In` (vault cable gallery) | `vault.py` | `crawl` "Crawl out" → `SetWorldFlag sombre.vault_opened` (idempotent) | `Anchor_VaultConduit_Out` | reserved |
| `LoftStair_Up` | `settlement.py` | `up` "Go up to the loft" | `Anchor_LoftStair_Loft` | reserved |
| `LoftStair_Down` | `net_loft.py` | `day` "Go down" | `Anchor_LoftStair_Store` | reserved |
| `LoftStair_DownNight` | `net_loft.py` | `night` "Go down" → `SetWorldFlag sombre.night_quay_seen` | `Anchor_LoftStair_NightQuay` | reserved |
| `Test_DoorIn`, `Test_DoorOut`, `Test_BackDoor` | `arch_test.py` | fixture | fixture markers | shipped (fixture) |

The order in the hatch and lower-door rows puts the open variant first so an opened vault never asks for the key or the skill again. **Sigrun is not a portal:** she sets `sombre.vault_opened`, and the existing hatch opens. Four ways in, three portals.

**Loft stair.** Plan §5.3 says "quay ↔ net loft", §5.2 says "the outside stair beside Marthe's store". Resolution: one up portal, owned by the settlement, at the store; two landings on the way down (the store by day, the night quay after the meeting). Not two up portals. A shipped portal has one destination, so the way down is two portals at the stair head. One presence rule in `net_loft.py` shows `LoftStair_DownNight` while `sombre.meeting_done` and `!sombre.night_quay_seen`, and `LoftStair_Down` otherwise. Exactly one is present, and a hidden portal cannot be focused. That is plan §5.3's "first variant", expressed with existing classes.

## 9. Anchors (cross-owner meeting points)

Created by the Integrator in `build_pointe_sombre.py` `anchors_table()` before any cell hook runs (`Anchor_<Name>`, tag `Anchor:<Name>`, at standing capsule-centre height: floor + 100 cm). Cells find them with `tk.anchor(name)` and never create or move them. **`Anchor_Respawn_TowerBase` is a respawn point only: never the stair, never the hatch.** The vault side uses §13.3's published names (`Top`/`Bottom`, `Out`/`In`); do not invent synonyms.

| Anchor | Where | Consumed by | Status |
| --- | --- | --- | --- |
| `Anchor_NewGame_Deck` | the *Ida*'s deck (−330 m, −360 m) | core player start; crossing | **shipped** |
| `Anchor_CrossingExit_Quay` | the quay arrival | crossing portal (destination); harbor builds around it | **shipped** |
| `Anchor_Respawn_TowerBase` | outside the tower base | core respawn rule only | **shipped** |
| `Anchor_TowerStair_Base` | inside the base room, at the stair foot | `TowerStair_Down` destination | **placed** (VS-08) |
| `Anchor_TowerStair_Lamp` | in the lamp room, at the stair head | `TowerStair_Up` destination | **placed** (VS-08) |
| `Anchor_VaultHatch_Top` | base room, beside the hatch | `VaultHatch_In` destination | **placed** (VS-08) |
| `Anchor_VaultHatch_Bottom` | vault upper room, under the hatch (slot `vault`) | `VaultHatch_Out` destination | **placed** (VS-08) |
| `Anchor_VaultLower_Out` | the sea cliff below the tower, outside the jammed door | `VaultLower_In` destination | **placed** (VS-08) |
| `Anchor_VaultLower_In` | vault lower gallery, inside the door (slot `vault`) | `VaultLower_Out` destination | **placed** (VS-08) |
| `Anchor_VaultConduit_Out` | the cable hut, at the conduit mouth | `VaultConduit_In` destination | **placed** (VS-08) |
| `Anchor_VaultConduit_In` | vault cable gallery (slot `vault`) | `VaultConduit_Out` destination | **placed** (VS-08) |
| `Anchor_LoftStair_Store` | the foot of the store's outside stair | `LoftStair_Down` `day` destination | **placed** (VS-08) |
| `Anchor_LoftStair_Loft` | inside the loft, at the stair head (slot `net_loft`) | `LoftStair_Up` destination | **placed** (VS-08) |
| `Anchor_LoftStair_NightQuay` | the night quay, facing the tower | `LoftStair_Down` `night` destination | **placed** (VS-08) |
| `Anchor_IdaBerth` | the *Ida*'s berth along the quay wall | `crossing.py`'s *Ida* presence rule (deck → berth after `sombre.reef_struck`); the harbor builds the berth around it | **placed** (VS-08) |
| `Anchor_Mara_Rail` | the *Ida*'s rail on the deck | `harbor.py`'s Mara presence rule (her new-game placement) | **placed** (VS-08) |
| `Anchor_Mara_Porch` | the store porch | `harbor.py`'s Mara presence rule | **placed** (VS-08) |
| `Anchor_DellPost` | beside the false-light post's hide | `settlement.py`'s Dell presence rule (first storm, VS-16) | **placed** (VS-08) |
| `Anchor_LoftSeat_Marthe`, `_Odette`, `_Jonas`, `_Tem`, `_Dell`, `_Sigrun`, `_Hale`, `_Varga`, `_Mara` | the meeting's places in the loft (slot `net_loft`) | each person's owner script (`settlement.py`: Marthe, Odette, Jonas, Tem, Dell, Sigrun; `harbor.py`: Hale, Varga, Mara), a `loft` presence state each; `net_loft.py` builds the room around them | **placed** (VS-08) |
| `Anchor_RemyMarker` | harbor-mouth shore, inside the harbor zone's `remy_holdback` exclusion | VS-20 only; harbor owns the exclusion | **placed, held** (VS-08 placed the empty anchor; VS-20 only) |

**People between cells.** Each person is one actor spawned by one script (§7). A placement in another cell's space goes to an anchor, never to that cell's actor or a coordinate the other cell owns. The keeper on the gallery at night is not Odette or Dell moved there. It is a lighthouse-owned silhouette (`Keeper_Silhouette`, two presence states, reading `keeper_odette` / `keeper_dell` and `meeting_done`), so no person's rule targets the tower.

**When they are spawned.** VS-07 records the names (docs only). The Integrator adds the reserved rows to `anchors_table()` once, on `main`, in VS-08 (the greybox places them with the footprints). That happens before any cell branch exists. A cell branch that adds an anchor itself conflicts on the core script and is refused at merge.

**Interior slots** (`INTERIOR_SLOTS`, shipped): `arch_test` (0, 2500 m, 400 m) stays the fixture; `vault` (150 m, 2500 m, 400 m); `net_loft` (300 m, 2500 m, 400 m). Nothing else goes in a slot without a new ledger row.

## 10. Tier C: recipe and zones

| Id | File | Owner | Status |
| --- | --- | --- | --- |
| recipe `great_lakes_rocky_shore` | `Tools/Biomes/great_lakes_rocky_shore.json` | WP-BIOME (VS-06) | **shipped** (`ad50261`) |
| zone `shore_test`, polygons `sheltered_bight`, `headland_tip`, exclusion `test_landing_clear` | `Tools/Biomes/zones/_test.json` | WP-BIOME. A proof, not a cell; not extended into the slice | **retired from the map** (VS-08): a fixture now (a `_` file is read by `verify_plan.py`, never put on the map) |
| zone `harbor`, polygons `harbor_shore`, `harbor_bight` | `Tools/Biomes/zones/harbor.json` | the harbor cell (written by the Integrator in VS-08; the harbor's file from VS-10) | **placed** (VS-08) |
| zone `cable_hut`, polygons `north_shore` | `Tools/Biomes/zones/cable_hut.json` | the cable-hut cell (Integrator in VS-08; the cell's file from VS-13) | **placed** (VS-08) |
| zone `headland`, polygons `headland_tip`, `causeway_shore`, `grey_reef` | `Tools/Biomes/zones/headland.json` | the headland cell (Integrator in VS-08; the cell's file from VS-16) | **placed** (VS-08) |

**Exactly three cell zones** (plan §10.4 reuse row): harbor, cable hut, headland. The crossing, settlement, lighthouse, and loft have no zone. Their keep-clear needs on a zoned shore are written as exclusions in the zone owner's file, requested by name. Zone exclusions use the **landed VS-06 schema** (`_test.json`):
- shapes `box` (`center`, `half`, `yaw`), `circle` (`center`, `radius`), or `polygon` (`points`), in metres, each with a stable `id` and an optional `margin_m`
- pads by name (`exclude_pads`: `name`, `margin_m`, joined to `island.json`)
- trails by id (`exclude_paths`: `id`, `margin_m`, joined to `island.json` `paths`; added in VS-08, the same pattern as pads: the trail's half width plus the margin stays clear)
- automatic radii around every interactable, portal, player start, character, and location volume
- `BiomeExclude`-tagged actors are honored by the tool; cells may use them for corridors without a zone edit

The biome sublevel `Lvl_PointeSombre_Biome` is a build product; only the Integrator commits it.

## 11. Review view ids

Unique across `Tools/Review/Lvl_PointeSombre.json` and every `Tools/Review/Lvl_PointeSombre/<cell>.json`. Exactly one route (the core file's). `art_sentinels` is set only by the core file (0).

| View ids | File | Status |
| --- | --- | --- |
| `quay_storm`, `quay_calm`, `quay_night`, `crossing_deck`, `island_overview`, `tower_rock_from_settlement`, `arch_cell_inside`, `arch_cell_ceiling`; the route | `Lvl_PointeSombre.json` (core) | **shipped** |
| `biome_harbor`, `biome_exposed`, `biome_overview` | `Lvl_PointeSombre/biome.json` | **shipped** (VS-06) |
| `crossing_bow`, `crossing_rail_mara` | `crossing.json` | reserved |
| `arrival_quay`, `harbor_overview`, `harbor_mouth_remy`, `hale_arrival`, `night_quay_line`, `night_quay_hand`, `night_quay_power`, `night_quay_destroyed`, `night_quay_untouched` | `harbor.json` | reserved |
| `settlement_street`, `store_marthe`, `cottage_odette`, `porch_leclair`, `shed_pruitts`, `racks_sigrun`, `ticks_close`, `tower_silhouette` | `settlement.json` | reserved |
| `lamp_room`, `feed_box_close`, `gallery_view`, `lighthouse_base`, `vault_entry_lower` | `lighthouse.json` | reserved |
| `vault_entry_hatch`, `vault_entry_conduit`, `vault_lower_gallery`, `vault_node_room`, `vault_flooded` | `vault.json` | reserved |
| `loft_meeting` | `net_loft.json` | reserved |
| `cable_hut_approach`, `cable_hut_door` | `cable_hut.json` | reserved |
| `headland_post`, `grey_stern`, `night_headland` | `headland.json` | reserved |
| `greybox_quay_arrival`, `greybox_quay_west`, `greybox_settlement_core`, `greybox_tower_path_mid`, `greybox_tower_base_reversal`, `greybox_ridge_crest`, `greybox_headland_approach`, `greybox_grey_reveal`, `greybox_gallery`, `greybox_shore_transition`, `greybox_cable_hut`, `greybox_headland_post`, `greybox_grey_stern`, `greybox_ida_berth` | `greybox.json` (Integrator, VS-08; retired cell by cell as cell files take over) | **shipped** (VS-08) |
| `remy_*` | `remy.json` | **held** (VS-20) |

These are plan §21's minimum set, assigned to files. `harbor_mouth_remy`'s expectation until VS-20 is **clear rocks where Remy's boat will be, not a boat**.

## 12. Tests

| Test (`DeadCurrent.…`) | File | Owner | Status |
| --- | --- | --- | --- |
| `Map.Sombre.Architecture`, `CrossMapLoad`, `Respawn`, `Atmosphere` | `DCSombreArchitectureMapTest.cpp` | Integrator | **shipped** |
| `Map.Sombre.BiomeExclusions` | `DCSombreBiomeMapTest.cpp` | WP-BIOME | **shipped** (VS-06) |
| `World.CellPortal` | `World/DCCellPortalTest.cpp` | Systems (VS-03) | **shipped** |
| `Map.Sombre.Greybox` | `DCSombreGreyboxMapTest.cpp` | Integrator (VS-08) | **shipped** (VS-08) |
| `Map.Sombre.Crossing` (B0 and B1: the harbor arrival lives here; there is **no** `Map.Sombre.Harbor`) | `DCSombreCrossingMapTest.cpp` | crossing + harbor (VS-10) | reserved |
| `Map.Sombre.Settlement` | `DCSombreSettlementMapTest.cpp` | settlement (VS-11) | reserved |
| `Map.Sombre.Lighthouse` | `DCSombreLighthouseMapTest.cpp` | lighthouse (VS-12) | reserved |
| `Map.Sombre.CableHut` | `DCSombreCableHutMapTest.cpp` | cable hut (VS-13) | reserved |
| `Map.Sombre.Vault` | `DCSombreVaultMapTest.cpp` | vault (VS-14) | reserved |
| `Map.Sombre.VaultRoutes` | `DCSombreVaultRoutesMapTest.cpp` | Integrator, written at the vault merge | reserved |
| `Map.Sombre.Midpoint` | `DCSombreMidpointMapTest.cpp` | VS-15 | reserved |
| `Map.Sombre.FalseLight`, `Map.Sombre.Grey` (**not** one "Headland" test) | `DCSombreFalseLightMapTest.cpp`, `DCSombreGreyMapTest.cpp` | headland (VS-16) | reserved |
| `Map.Sombre.Resolution` | `DCSombreResolutionMapTest.cpp` | VS-17 | reserved |
| `Map.Sombre.Meeting` (**not** "NetLoft") | `DCSombreMeetingMapTest.cpp` | net loft (VS-18) | reserved |
| `Map.Sombre.SecondStorm`, `Map.Sombre.Saves`, `Map.Sombre.Discovery` | `DCSombreSecondStormMapTest.cpp`, `DCSombreSavesMapTest.cpp`, `DCSombreDiscoveryMapTest.cpp` | VS-19 / VS-24 | reserved |
| `Content.Sombre.QuestGraph`, `Content.Sombre.Dialogue`, `Content.Sombre.Items` | `Quest/DCSombreContentTest.cpp` | WP-NARR (VS-09) | reserved |
| `World.SignalSequence`, `UI.StoryCard` | capability packages (VS-17, VS-19) | Systems | reserved |
| `Map.Sombre.Remy` | `DCSombreRemyMapTest.cpp` | VS-20 only | **held** |

`Map.Sombre.KitGym` is not a test (Wave 1 replaced it with `kit/verify_kit.py`). Plan §19's Settlement row says that test covers Dell's two confession routes. VS-11's done-when defers the confession to VS-16, where `Map.Sombre.FalseLight` covers both. The settlement test follows VS-11.

## 13. Held back for the VS-20 fresh-agent test: Remy's marker and boat

The plan's default candidate (§15 VS-20), the beat script's optional B2 quiet scene. No other task depends on it, and none may start to.

- **Reserved and unused until VS-20:** cell `sombre.remy`, persistent id `sombre.remy_marker`, anchor `Anchor_RemyMarker`, review ids `remy_*`, test `Map.Sombre.Remy`. No flag, no location id (VS-20's spec decides whether it is discoverable).
- **Files nobody creates before VS-20:** `Tools/EditorScripts/pointe_sombre/remy.py`, `dress_sombre_remy.py`, `Content/Maps/Lvl_PointeSombre_Art_Remy.umap`, `Content/World/PointeSombre/Remy/**`, `Tools/Review/Lvl_PointeSombre/remy.json`, `Source/DeadCurrent/Save/DCSombreRemyMapTest.cpp`.
- **What exists for them beforehand:**
  - the harbor zone's exclusion `remy_holdback`, owned by the harbor; the rocks stay clear
  - `Anchor_RemyMarker`, created by the Integrator
  - the kit, the recipe, the grammar
- **What must not depend on it:** VS-08 landmarks and route times (the quay→tower leg's "something to notice" is the tower, not the marker); VS-09 lines that mention Remy (they check no actor); VS-10 harbor (no marker, no boat, no Remy-only dialogue); VS-11 Odette (cottage door; no presence target on a Remy actor); every quest transition.
- **The fresh agent must not edit:** any other cell script, the core, this ledger (they request ids), `harbor.py`, `zones/harbor.json`.
- If Checkpoint D picks another candidate or none, these ids stay unused. Claude builds the marker afterwards, as the plan says.
- Odette standing at the marker is `[P]` and belongs to the same hold.

## 14. Art sublevels and content folders

One art sublevel per owner: `Lvl_PointeSombre_Art_<Cell>` for Crossing, Harbor, Settlement, Lighthouse, Vault, NetLoft, CableHut, Headland, and Remy (held). That is eight content art sublevels plus the biome sublevel, from seven specs, because the lighthouse and the vault are separate implementation owners (§13.3). Plan §8.3's "×7" counted specs. Content folders: `Content/World/PointeSombre/<Cell>/`.

## 15. Open discrepancies kept visible (none blocks VS-08)

1. `sombre.storm`: decided in §3.1. Plan §8.8 and the beat script's "set on arrival" are superseded only where they made the flag drive the atmosphere.
2. Hatch: portal, not inspectable (§8).
3. Loft stair: one up portal at the store; two landings (§8).
4. §11 vs §8.4 cell lists (§2).
5. `sombre_pruitts` / `sombre_meeting`: aliases (§6).
6. `sombre_vault_key` missing from the beat script's item list: minted (§4).
7. The beat script's flag list omits §8.2's ids and the dialogue §11 ids: unioned (§3.2).
8. `sombre.mara_*` and the missing wrecker id (§7).
9. §19's Settlement test vs VS-11 (§12).
10. Art-sublevel count (§14).
11. **Zero investment and Odette's key.**
    - The beat script's build table says a zero-investment player gets the key "via the tie line and letter". That holds only if the player answered Mara's tie question.
    - A player who said "Does it matter?" (no tie) and has no Persuasion still enters by the conduit or by Sigrun.
    - "Zero investment" therefore means "some way in", not "Odette always gives the key". The lighthouse spec says so.
12. **`knows` objective text and the call.** The objective tells the player to decide and then tell the settlement. Marthe's call (§8.2 #4) is what gathers them. Both the lighthouse spec (quest note) and the settlement spec (Marthe) say so.
13. **The night look and `sombre.storm`.** `done_*` sets `sombre.storm` while the night look follows `meeting_done`. That is coherent only as written in §3.1. A builder who keys the night picture to `sombre.storm` alone would fight `Map.Sombre.Atmosphere`.
