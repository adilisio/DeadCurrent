# "The Wrong Characteristic" — Beat Script for the Pointe Sombre Slice

Status: **PROPOSAL, 2026-09-30. A document only.** Nothing here creates a quest, dialogue asset, item, flag, map, or test, and nothing here starts Phase 5 or Phase 6. Every id is a **proposed** id, following the existing `<area>.<fact>` convention, and becomes immutable only when a builder ships it.

Built on Anthony's decisions of 2026-09-30: the player's tie to Liv is chosen (sister, partner, the one who took them in); the Shore Watch is an inherited office; Mara is the companion; Liv pulled the key and Remy Beaudry drowned. Story context: `QUEST_ARCS.md` §2. Rules of the Current: `MYSTERY_LEDGER.md` §5.

**Goal of this document:** a builder could turn it into data using the rule language that exists today: conditions `HasItem`, `QuestNotStarted`, `QuestActive`, `QuestComplete`, `QuestStage`, `WorldFlag`, `ActorDead`, `LocationDiscovered`, `AttributeAtLeast`, `SkillAtLeast`, `HasPerk` (each with negate); consequences `GiveItem`, `RemoveItem`, `StartQuest`, `SetQuestStage`, `SetWorldFlag`, `ClearWorldFlag`; and the existing actors (inspectables with variants and per-variant verbs, pickups, loot containers, location volumes, `ADCDamageVolume` and `ADCFlickerLight` with world-flag `ActiveConditions`, `ADCConditionalAudio`, friendly NPCs with dialogue). What that grammar cannot do is listed in §8, with a fallback for each.

---

## 1. Assumptions

- **The tie flag exists before the slice.** One of `player.tie.sister`, `player.tie.partner`, `player.tie.took_in` is set when the player first reads Liv's letter (prologue, `QUEST_ARCS.md` MQ-P1). For the **standalone demo**, a recap card (Liv's letter, a one-screen map, three reply lines) sets it, plus `watch.mara_travelling`.
- **The slice starts in the storm.** `sombre.storm` is set on arrival and drives the storm's sound, lightning flicker, the vault's live water, and the direction-finding loop. The quest clears it at the halfway point and sets it again for the last beat.
- **Mara is placed, not following** (fallback for a companion system): she stands at authored spots that change with the quest stage (§8, capability 1).
- The slice is **60 to 90 minutes**, with one dungeon (the vault). The *Ashland Grey* is an exterior and deck only.

---

## 2. The slice at a glance

| Beat | Place | ~min | The player | State out |
| --- | --- | --- | --- | --- |
| B0 | The crossing | 3–5 | Sees a light on the wrong headland; the *Ida* grazes the reef | quest starts |
| B1 | Harbor | 5 | Varga won't sail while the point is dark | `arrived` |
| B2 | Settlement | 10–15 | Meets Odette, Marthe, the Leclairs, the Pruitts, Sigrun | accounts, leads |
| B3 | Lamp room | 5–8 | Seized clockwork, dead lamp, a cable cut clean | `sombre.cable_cut_found` |
| B4 | The vault | 15–20 | Gets in, reads the log and the panel, takes the bearing, finds Liv's note | `knows` |
| B5 | Harbor | 5 | The storm passes; Hale's cutter arrives with a card | `sombre.hale_arrived` |
| B6 | West headland | 10–15 | False Light: who shows the lantern | `sombre.false_light_*` |
| B7 | Vault / lamp room | 5–10 | **Decides the light by doing** | outcome flags |
| B8 | Net loft | 5–10 | Brings evidence, or doesn't | exposure flags; quest completes |
| B9 | Harbor, night | 3–5 | The second storm shows what the player made | end card |

---

## 3. Beats

### B0 — The crossing
- **Trigger:** slice start (or the end of the prologue).
- **Sees:** dusk, rising storm (`sombre.storm`). From the *Ida*'s deck, a light burning low on a headland; beyond it, a tower with no light at all. The *Ida* turns for the burning light, then shudders on stone.
- **Mara (the watch reveal, if not already heard):** "That line you asked about. Two of them came up the beach. That wasn't me. That was the watch. It's the same thing."
- **Cost-saving option:** play B0 as a short fixed-camera scene or a fade over audio; start control on the harbor.
- **State:** `StartQuest(sombre.characteristic)` → stage `arrived`.

### B1 — Harbor
- **Varga** (on the *Ida*'s deck, pumps going): "I read that light as the point and put us on the reef for it. Somebody lit it on purpose. The *Ida* isn't leaving this harbor while the point's dark. Find out why, or find me a keeper who'll light it."
- She also knows why the player came: "Your listener came through here. Four months back. Ask the keeper." (Tie-aware line variants only if the player asks her about Liv.)
- **Starts False Light** (`sombre.false_light` quest) with the same conversation: "And whoever lit that lantern, I want a name."
- **Location:** `sombre.harbor` discovered.

### B2 — The settlement
Short conversations. Each person gives one account (see `QUEST_ARCS.md` §2.3) and one lead.

| Person | Account | Lead |
| --- | --- | --- |
| **Odette Beaudry** (keeper) | "It failed. Lights fail." Won't look at the tower. | Holds the keeper's key to the vault hatch. |
| **Marthe** (store) | "No light, no ships, no stock. Do the arithmetic." | Oil: "Smokehouse can render a can if you can stand the smell." |
| **Jonas Leclair** | His children draw rows of ticks on the wall in storms. | Inspectable: the ticks (`sombre.ticks_seen`). Mara, seeing them, says nothing (her stage-specific line is silence plus "Write it down"). |
| **Tem Pruitt** | "The lake provides." Friendly, poor, watchful. | (False Light suspect.) |
| **Dell Pruitt** | Won't talk about Remy. "I found his boat. That's all." | Survival 2: his boots are wet to the knee in dry weather. |
| **Sigrun Dahl** | "Buying copper. You got any?" | Lineman's shears on her belt (noticed later, B3). |

- **Remy's marker** (inspectable, optional): "REMY BEAUDRY. The reef, the first storm after the dark." The quiet scene: if Odette is there, the player can stand with her and leave (no choice, no flag).
- **The children's ticks** (inspect): "Rows of short marks in chalk, evenly spaced, the same spacing on every wall. The youngest has drawn a lighthouse at the end of a row." With the Sounder Chart in inventory: "The same spacing as the *Tern*'s chart."

### B3 — The lamp room
- **Location:** `sombre.light` discovered. The tower door is open; the lamp room is at the top.
- **Inspectables:**
  - **Lamp:** "An electric lamp in a big glass lens, dead. Someone has set an old oil burner beside it on the gallery floor, and never lit it." (Variants in §4.)
  - **Clockwork:** "The rotation clockwork: a weight on a chain, a drum, a governor. The chain is rusted to the drum and a pawl has sheared." Engineering 2: "You could file a new pawl from a hinge. Or find one: a laker's winch uses the same part."
  - **Feed box:** "The lamp's feed comes up through the floor in a steel box. Inside, the cable ends have been cut and folded back. Clean cuts. Shears, not a saw." → `SetWorldFlag(sombre.cable_cut_found)`. Engineering 2 variant adds: "Lineman's shears. And the feed runs *down*, into the rock, not to any generator."
  - **Pickup:** `cut_cable_end` (evidence).
- **Quest:** `arrived` → `lamp_room` (on `LocationDiscovered sombre.light`) → `vault` (on `WorldFlag sombre.cable_cut_found`). Objective: "The lamp's feed runs down into the rock under the tower. Find a way in."

### B4 — The vault (the slice's dungeon)
**Getting in: four ways, each a world act.** Any one sets `sombre.vault_opened`.

| Way | How | Build or condition |
| --- | --- | --- |
| **Odette's key** | She gives the hatch key: dialogue choice | Persuasion 2, **or** the player tells her who Liv is to them (tie line) and has read Liv's letter |
| **Force the lower door** | Inspectable "Jammed door": "Pry the jam" | Engineering 2 |
| **The cable conduit** | From the **cable hut** (`sombre.cable_hut`), follow the mussel line into a crawl conduit | Survival 2, or just exploring the hut (the conduit is visible; the skill check only shows the way from the shore) |
| **Sigrun** | Confront her with `cut_cable_end`; she unjams it and comes too | HasItem `cut_cable_end`; sets `sombre.sigrun_revealed` |

Odette's tie-variant exchange (player line → her reply is shared):
- Sister: "She's my sister." Partner: "She's my partner." Took in: "She took me in when nobody would."
- Odette: "...Then you know what she's like when she's decided something. Here. She had the same look."

**Inside** (`sombre.vault` discovered). Upper room dry; lower level under a hand's depth of **live water** during the storm: an `ADCDamageVolume` with `ActiveConditions = [sombre.storm, !sombre.node_destroyed]`, the same filaments and fish-ring grammar as the *Tern* (Pulse Read variant on the dead eels: "One shock, from the middle of the floor, where there is nothing.").

| Inspectable | Default text | Variants / state |
| --- | --- | --- |
| **Authority keeper's log** | Decades of entries in three hands. "Current event, minor. Shore lamps to reserve." Again and again, years before the collapse. The last entry: "Section order received 03:12. Link locked. God help Kenning." | First read → `sombre.log_read`. If `sombre.node_destroyed` and not read: "The log is pulp." (lost) |
| **Node panel** | "GREAT LAKES MARITIME AUTHORITY — CONTINUITY SECTION 14. SEVERED. RESYNC PENDING: 2 OF 5." An empty slot where a card should sit. | First read → `sombre.panel_read`. Engineering 2: "Five conditions. Power: yes. Quiet: yes. Card: no. Line: no. Confirmation: no. Someone pulled the card, and someone else cut the line." Outcome variants in §4 |
| **Direction-finding loop** | "A big loop antenna on a turntable, a dial, a headphone jack." | With `sombre.storm`, or Engineering 2 without it: **"Take the bearing"**: "The pattern comes and goes as you turn the loop. Loudest at one-one-two." → `sombre.bearing_taken`. After `sombre.node_destroyed`, if not taken: "The loop is under water." |
| **Chart table** (Liv's chart pinned to it) | "A lake chart, pinned flat. A pencil line from the Authority Shore runs east. A second line, fainter, from here. Where they cross, a long thin question mark. The A in 'Authority' has no crossbar." | With `sombre.bearing_taken`: "Your bearing lies over hers exactly. Two lines from so close together make a long, loose cross: somewhere past the Narrows. Not a place yet." With the Sounder Chart and Schematic Eye: "Three marks agree: the *Tern*'s, hers, and yours." Take a copy → `GiveItem(liv_chart)` |
| **Liv's note** (pinned under the chart) | Pickup `liv_note`. Text: "Odette — I took the card. Your light was about to join them again and I can't let it. Light it by hand. Oil, the old clockwork, your own two arms. I'm sorry for what I'm asking. Don't let anyone put it back on the Line. Not the Compact. Not anybody. — L." | → `sombre.liv_note_found` |
| **Sign-in board** | Pickup `vault_access_log`: "Two entries in chalk, four months old: O.B. and L.K. Nothing since." | Evidence against Odette |
| **Sea cock** | "A big valve wheel on a pipe that goes down through the floor. Opens the vault to the lake." | Destruction path (§4) |

- **Mara** at the panel: "Write it down." At Liv's note: nothing. Later, if the player named the tie anywhere: "She asked me not to tell you where she went. I kept it as long as I could."
- **Quest:** `vault` → `vault_inside` (on `sombre.vault_opened`) → `knows` (on `sombre.panel_read` **and** `sombre.liv_note_found`).
- **`knows` on_enter:** `ClearWorldFlag(sombre.storm)`, `SetWorldFlag(sombre.hale_arrived)`. The storm passes. Objective: "Liv pulled the card that tied the light to something called Section 14. It is trying to come back. Decide what happens to the light."

### B5 — Hale arrives
- A Compact cutter at the harbor (`sombre.hale` present when `sombre.hale_arrived`; §8 capability 1).
- **Hale:** "Warden Hale, Lights Office. I've a card for this tower and a route that needs it lit by the next storm. You've been down there. Tell me what I'm looking at."
- He offers the **Compact card** (`section_key_compact`) for the player to seat, or his crew to do it: "Seat it, splice the feed, and the *Ida* sails tonight. The Compact pays for lights that burn."
- If the player has `liv_note`: "She's your listener? She took my money too." (Shared line; no tie leak.)
- He asks after the *Ashland Grey*'s log (the Loss Book side quest; initial production only).
- If `sombre.sigrun_revealed` and she is present: "Copper buyer." He knows exactly what she is.
- He can give **Compact lamp oil** (`lamp_oil`) if the player asks about a hand light: "Oil I have. Keepers I don't."

### B6 — False Light (side quest in the slice)
- **Hook:** Varga's question (B1).
- **West headland** (`sombre.headland` discovered): a lantern post in a hide, footprints. Survival 2: "Two people. One drags a foot." (Tem limps.)
- **Approaches:**
  - **Take the lantern now:** pickup `false_lantern` → `sombre.false_light_taken`. The headland `ADCFlickerLight` has `ActiveConditions = [sombre.storm, !sombre.false_light_taken]`. It won't burn again.
  - **Catch them at it:** come back when the storm returns (B9 setup) and confront whoever lights it. If the scavenger is alive and here (he chose the Pruitts, not the watch), he is the one at the post: "You. The coil thief."
  - **Talk to Dell** with Persuasion 2 (or with the wet-boots Survival reading): he confesses. Remy was his friend. "We didn't make the dark. We just ate off it." → `sombre.dell_confessed`.
  - **Force:** Tem keeps a shotgun. Combat is possible and ugly.
- **Offer Dell the keeper's post** (dialogue after the confession): → `sombre.keeper_dell`. He takes it if the lantern stops.
- **Completes** when the player tells Varga a name, or brings the lantern to the net loft.

### B7 — The decision (in the vault and the lamp room)
See §4. This is the slice's real choice, and it happens with the player's hands, not in a menu.

### B8 — The net loft
See §5.

### B9 — The second storm and the end card
- **Trigger:** `sombre.meeting_done` → the quest's outcome stage `on_enter` sets `sombre.storm` again.
- From the harbor at night, the player sees the result:
  - **On the Line:** the tower blazes, and for three long seconds it flashes **the pattern** before its own characteristic returns. Everyone on the quay sees it. Mara: "Write that down." The *Ida* sails.
  - **By hand:** a smaller, steady light turning slowly. The keeper's silhouette on the gallery (Odette or Dell, if chosen). With no keeper, it burns, then gutters out before dawn. The *Ida* sails at first light.
  - **Settlement power:** windows lit all along the harbor; the point dark. The headland dark too if the lantern was taken, lit if not. Varga sails at first light, furious.
  - **Destroyed:** the point dark unless hand-lit; the vault's mouth bubbling. Sigrun gone on the first boat.
  - **Untouched:** as Liv left it. Dark. The *Ida* sails by day.
- **End card** (slice only): Varga's line, Mara's line, and the chart: "Two bearings. Somewhere past the Narrows. Liv went through the locks."

---

## 4. The decision: physical acts

Mutual exclusion is done with negated conditions, so the node ends in exactly one state (**Line**, **Settlement**, **Destroyed**, or **Untouched**). The **hand light** is independent: it can combine with Settlement or Destroyed, but not with Line.

| Act | Where (actor, variant verb) | Conditions | Consequences |
| --- | --- | --- | --- |
| **Splice the feed** | Lamp room feed box: "Splice the feed" | `SkillAtLeast Skill.Engineering 2` **or** (second variant) `WorldFlag sombre.hale_crew_up` (Hale dialogue: "Send your crew up") | `SetWorldFlag sombre.feed_spliced` |
| **Seat the card** (on the Line) | Node panel: "Seat the card" | `HasItem section_key_compact`, `WorldFlag sombre.feed_spliced`, `!sombre.node_destroyed`, `!sombre.power_settlement`, `!sombre.light_hand` | `RemoveItem section_key_compact`, `SetWorldFlag sombre.light_line` |
| **Throw the settlement feed** (redirect) | Node panel: "Throw the settlement feed" (two variants: with the card, or with a jumper) | `!sombre.node_destroyed`, `!sombre.light_line`, and either `HasItem section_key_compact` or `SkillAtLeast Skill.Engineering 2` | `SetWorldFlag sombre.power_settlement` (and `RemoveItem section_key_compact` in the card variant) |
| **Open the sea cock** (destroy) | Sea cock: "Open it" | `!sombre.light_line`, and either `WorldFlag sombre.sigrun_helping` (she has the wrench) or `SkillAtLeast Skill.Engineering 2` | `SetWorldFlag sombre.node_destroyed` |
| **Free the clockwork** | Clockwork: "Fit the pawl" / "File a pawl" | `HasItem clockwork_pawl` (from the *Ashland Grey*'s deck winch) **or** `SkillAtLeast Skill.Engineering 2` | `SetWorldFlag sombre.clockwork_freed` (and `RemoveItem clockwork_pawl`) |
| **Render oil** | Smokehouse: "Render a can" | none (takes the player's time; a small scene) | `GiveItem lamp_oil` |
| **Light it by hand** | Lamp: "Fill and light the burner" | `WorldFlag sombre.clockwork_freed`, `HasItem lamp_oil`, `!sombre.light_line` | `RemoveItem lamp_oil`, `SetWorldFlag sombre.light_hand` |
| **Choose a keeper** | Dialogue with Odette ("Will you keep it?") or Dell (after confessing) | Odette: `!sombre.exposed_odette` **or** she has been forgiven at the loft; Dell: `sombre.dell_confessed` | `SetWorldFlag sombre.keeper_odette` / `sombre.keeper_dell` |

**Panel read-outs by state** (inspect variants, first match wins): destroyed ("Dark. Water to the second rung."); Line ("RESYNC PENDING: 4 OF 5." Engineering 2 adds: "Only confirmation left. It's asking."); Settlement ("RESYNC PENDING: 3 OF 5. The feed runs to the town now. The count is still climbing."); default ("2 OF 5").

**Data loss is real:** opening the sea cock before reading the log or taking the bearing loses them (the variants above). The bearing can be recovered in Act II at the Local's powerhouse loop (`QUEST_ARCS.md` MQ-X).

---

## 5. The net loft (the settlement meeting)

- **Chair:** Marthe (PROPOSAL). **Present:** Odette, the Pruitts (unless dead), the Leclairs, Sigrun (unless gone), Hale (if arrived), Varga, and Mara.
- **Available** once the quest is at `knows` or later. Opening line: "Is the light burning tonight, or isn't it?"
  - "Not yet." → ends the conversation, nothing set (the player can go and act).
  - "It's done." → shown when any of `sombre.light_line`, `sombre.light_hand`, `sombre.power_settlement`, `sombre.node_destroyed` is set.
  - "Leave it as she left it." → shown only when **none** of those four flags is set (four negated `WorldFlag` conditions). The untouched choice, stated plainly.
- **Then, evidence.** Each choice is shown only if the player **carries the object**, and giving it removes it (`RemoveItem`) and sets a flag. The player can bring several, one, or none.

| Choice (shown if carrying) | Line | Sets |
| --- | --- | --- |
| `false_lantern` | "This was burning on the west head the night the *Ida* hit." | `sombre.exposed_pruitts` |
| `vault_access_log` | "Four months ago someone let a stranger into the vault." | `sombre.exposed_odette` |
| `liv_note` | "The listener pulled the card. She wrote to Odette." | `sombre.exposed_liv` |
| `cut_cable_end` | "Someone made sure it could never be fixed." | `sombre.exposed_sigrun` |
| (tie line, shown if `sombre.exposed_liv`) | Sister: "She's my sister." / Partner: "She's my partner." / Took in: "She took me in." | `sombre.player_named_tie` |
| "That's all." | | `sombre.meeting_done` |

- **Reactions** (one line each, by flag): the room turns on the Pruitts (Tem: "The lake provides. We only held the lantern."), or on Odette (who says, "I told you it failed. I lied. Remy was the first boat out."), or on Sigrun (Hale: "Copper buyer. You're coming with me." If Hale is present, she is taken off the island). If the tie is named, **Odette's reaction is the scene**: "Then she owes me a son. And you came all this way for her." Mara watches, and that night tells the player about Liv's promise.
- **Forgiving Odette:** if `sombre.exposed_odette` **and** the player also gave `liv_note`, the room's anger moves from Odette to "the listener". Odette can still keep the light (sets `sombre.odette_forgiven`). Exposing Odette without the note leaves her disgraced; she will not keep it.

---

## 6. Quest data (proposed)

### `sombre.characteristic` — "The Wrong Characteristic"

| Stage | Objective | Transitions (first match wins) | On enter |
| --- | --- | --- | --- |
| `arrived` (start) | "The *Ida* won't sail while Pointe Sombre is dark. Find out why the light failed." | → `lamp_room` when `LocationDiscovered sombre.light` | |
| `lamp_room` | "Look over the lamp room." | → `vault` when `WorldFlag sombre.cable_cut_found` | |
| `vault` | "The lamp's feed runs down into the rock under the tower. Find a way in." | → `vault_inside` when `WorldFlag sombre.vault_opened` | |
| `vault_inside` | "Find out what the vault under the light is for." | → `knows` when `WorldFlag sombre.panel_read` and `WorldFlag sombre.liv_note_found` | |
| `knows` | "Liv pulled the card that tied the light to Section 14. It is trying to come back. Decide what happens to the light, then tell the settlement at the net loft." | → `done_line` when `sombre.meeting_done`, `sombre.light_line`; → `done_hand` when `sombre.meeting_done`, `sombre.light_hand`; → `done_dark` when `sombre.meeting_done` | `ClearWorldFlag sombre.storm`, `SetWorldFlag sombre.hale_arrived` |
| `done_line` (completes) | "The light burns on the Line again. For three seconds it showed the pattern." | | `SetWorldFlag sombre.storm` |
| `done_hand` (completes) | "The light burns by hand. Someone has to wind it every four hours." | | `SetWorldFlag sombre.storm` |
| `done_dark` (completes) | "The point is dark. What powers the vault, if anything, is your doing." | | `SetWorldFlag sombre.storm` |

Power to the settlement and a destroyed node are **flags**, not stages. The stage records the light; the flags record the node.

### `sombre.false_light` — "False Light"

| Stage | Objective | Transitions | On enter |
| --- | --- | --- | --- |
| `asked` (start) | "Someone showed a light on the west headland. Varga wants a name." | → `found` when `LocationDiscovered sombre.headland` | |
| `found` | "The lantern post is in a hide on the west head. Find who lights it." | → `named` when `sombre.dell_confessed` **or** (separate transition) `HasItem false_lantern` | |
| `named` | "Tell Varga, or bring the lantern to the net loft." | → `done` when `sombre.exposed_pruitts` or `sombre.varga_told` | |
| `done` (completes) | "The false light won't burn again." | | `SetWorldFlag sombre.false_light_taken` |

### Proposed ids

- **Flags** (`sombre.`): `storm`, `ticks_seen`, `cable_cut_found`, `vault_opened`, `log_read`, `panel_read`, `bearing_taken`, `liv_note_found`, `hale_arrived`, `hale_crew_up`, `sigrun_revealed`, `sigrun_helping`, `feed_spliced`, `light_line`, `light_hand`, `power_settlement`, `node_destroyed`, `clockwork_freed`, `keeper_odette`, `keeper_dell`, `odette_forgiven`, `dell_confessed`, `false_light_taken`, `varga_told`, `exposed_pruitts`, `exposed_odette`, `exposed_liv`, `exposed_sigrun`, `player_named_tie`, `meeting_done`. From the prologue: `player.tie.*`, `watch.mara_travelling`.
- **Items:** `liv_letter` (from the prologue, or the demo's recap card; Odette's key variant checks it), `liv_chart`, `liv_note`, `vault_access_log`, `cut_cable_end`, `false_lantern`, `section_key_compact`, `clockwork_pawl`, `lamp_oil`.
- **Locations:** `sombre.harbor`, `sombre.light`, `sombre.vault`, `sombre.headland`, `sombre.cable_hut`, `sombre.ashland_grey`.
- **Persistent actors:** `sombre.odette`, `sombre.marthe`, `sombre.jonas`, `sombre.tem`, `sombre.dell`, `sombre.sigrun`, `sombre.hale`, `sombre.varga`, `sombre.mara_*` (placements), `sombre.vault_hatch`, `sombre.vault_door`.
- **Dialogues:** `sombre_varga`, `sombre_odette`, `sombre_marthe`, `sombre_pruitts`, `sombre_sigrun`, `sombre_hale`, `sombre_mara`, `sombre_meeting`.

---

## 7. Build variety check

Every build gets in and gets a full choice; builds change *how*, not *whether*.

| Build | Vault entry | Light options | Extras |
| --- | --- | --- | --- |
| Zero investment | Odette's key via the tie line and letter, the conduit, or Sigrun | Line (Hale's crew splices); hand light (pawl from the *Ashland Grey*, rendered oil); destroy (with Sigrun) | |
| Engineering | Force the door | All, without anyone's help; file a pawl; jumper the settlement feed | Panel conditions; "lineman's shears" |
| Survival | The conduit from the shore | As zero | Wet boots (Dell), footprints (Tem), children's ticks spacing |
| Persuasion | Odette's key | As zero | Dell's confession, Odette's forgiveness |
| Schematic Eye / Pulse Read / Relay Ear | — | — | Three-bearing agreement on the chart; one-shock eels; (Relay Ear, future: Liv's seating on the vault loop's jack) |

---

## 8. Missing reusable capabilities (list, don't build)

In priority order. Each has a fallback that keeps the slice buildable.

1. **Conditional presence** (already proposed in `Design/POIs/PRODUCTION_PILOT.md` §6): show, hide, or place an actor by conditions. Needed for Hale's arrival, Sigrun's departure, Mara's placements per stage, keepers on the gallery, the Pruitts after exposure, and the vault blockers. **This one capability carries most of the slice's visible consequence.** Fallback: everyone present from the start, with dialogue variants only (much weaker).
2. **A sequenced flicker light**: `ADCFlickerLight` with an authored on/off pattern, so the tower can show its characteristic and, for three seconds, the pattern. Fallback: a steady light, and the pattern told in text.
3. **A locked door by condition.** Fallback: a blocker actor hidden by capability 1, or an inspectable "Jammed door" whose variant sets the flag and a door that simply opens after.
4. **Storm presentation by flag.** Thunder and lightning already work (`ADCConditionalAudio`, `ADCFlickerLight` on `sombre.storm`); rain and sky do not. Fallback: audio and flicker only.
5. **A companion that follows.** Not needed for the slice if capability 1 exists (placed Mara).
6. **Item descriptions that change with flags.** Avoided: the chart lives on an inspectable table; the item is a static copy.
7. **Boat travel.** The crossing is a scene or a fade.

No trading system, faction reputation, or new condition type is required. Faction standing in the slice is carried by flags (`sombre.light_line` is Compact-friendly, `sombre.node_destroyed` Local-friendly) until Phase 5 or later decides on reputation.

## 9. Tests a builder would add

- Quest graph: each outcome stage is reachable, and the untouched ending completes.
- Mutual exclusion: after each node act, the other node acts' variants are hidden.
- Data loss: destroying before reading hides the log and bearing variants.
- Evidence: each loft choice appears only while the item is carried, and giving it removes it.
- Tie: each tie flag shows exactly one variant line in each keystone node.
- Persistence: save in each outcome; the flags, panel text, damage volume, and lights restore; no re-announced locations.
- Build: zero-investment, Engineering-only, Survival-only, and Persuasion-only runs each reach the vault and each outcome.

## 10. Open questions for Anthony (small)

1. Does the slice **open in the crossing** (costlier, stronger) or on the harbor?
2. **Marthe chairs the meeting**, or Odette, or a new reeve?
3. Hale arriving **midway** (after the vault) vs. being present from the start?
4. Liv's note: is the tone right? It is the player's first direct contact with her voice after the letter.
