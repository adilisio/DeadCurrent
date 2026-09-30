# DEAD CURRENT — Quest Arcs (PROVISIONAL)

Status: **PROVISIONAL, 2026-09-29.** Arcs, not a quest list. No quest asset, id, stage, or flag is created by this document; the working ids below (`MQ-A1` etc.) are for cross-reference only and must not be used as persistent ids. Nothing here starts Phase 5 or Phase 6.

## 0. Conventions

- **Decide by doing.** Every major decision below names the **physical act** that makes it. Dialogue can inform and persuade; it rarely decides. (Shore Watch already works this way: the reply to Mara does not lock the route.)
- **Build hooks** use the systems that exist: Grasp → **Engineering**, Fieldcraft → **Survival**, Bearing → **Persuasion**, and the perks **Schematic Eye**, **Pulse Read**, **Relay Ear**. Suggested future checks are marked *(future)*. Failed checks stay hidden, as now.
- **Consequence chain** for each decision: immediate → local → later callback → ending implication (collected in §8).
- **Scope marks:** **[SLICE]** = the Phase 6 vertical slice; **[INITIAL]** = the initial production region around Pointe Sombre (`LongTermPlan.txt` §24); **[FULL]** = full game only.

---

## 1. Main quest spine

| Id | Name | Act | Core verb | One line |
| --- | --- | --- | --- | --- |
| MQ-P0 | Shore Watch (existing) | Prologue | quiet a relay | Existing. Kill the scavenger or pull the coil. |
| MQ-P1 | Posted From the Shore | Prologue | ask, compare | Find where Liv went from the Authority Shore. |
| MQ-A1 | The Wrong Characteristic | I | investigate, repair | Why the Pointe Sombre light went dark, and what to do with it. **[SLICE]** |
| MQ-X | Cross Bearings | I–II | plot | Liv's chart: take bearings on the pattern until they fix a place. Runs across acts. |
| MQ-B1 | Lock Passage | II | pass | Get from Upper to Lower through Aubin Locks. |
| MQ-B2 | The Third Bearing | II | listen | Take the bearing at Tall Masts; the fix is the Crib. |
| MQ-B3 | Dead Letter | II | receive | Odile hands over Liv's undelivered letter. |
| MQ-B4 | Kallio at the Crib | Midpoint | react | Liv's voice on every relay; the race starts. |
| MQ-C1 | The Severance Record | III | read, power | The Authority headquarters in drowned Kenning. |
| MQ-C2 | Pumping Station | III | power | Power the pumps to reach the lower floors; watch the tables act. |
| MQ-C3 | The Intake | III | traverse | Four miles under the lake to the Crib. Who comes with you? |
| MQ-E1 | Black Start | Endgame | decide | Liv, the console, and the Line. See `ENDINGS.md`. |

### MQ-P1 — Posted From the Shore (Prologue) **[INITIAL]**
- **Hook:** the player carries Liv's last letter, posted from "the old Authority landing". The *Ida* has left them there for three nights.
- **Escalation:** Mara deflects. The relay (if inspected closely, or with Relay Ear) was seated by someone who knew the pinout. "HEAR IT TOO" under the scavenger's KEEP OUT is in handwriting whose A has no crossbar, like the letter's.
- **Approaches:**
  - **Persuasion 2:** Mara admits Liv was here, and (with trust) that Liv asked her not to tell.
  - **Survival** *(future)*: compare the chalk hand to the letter.
  - **Relay Ear:** the coil seating is a listener's.
  - **Shore Watch coil route:** Mara's transcripts include a call sign, LK-IDA, the *Ida*'s listener's sign.
  - **The *Tern*:** its bearing (with the chart and Schematic Eye) points east, toward Pointe Sombre.
- **Decision:** none large. The small one is whether the player tells Mara who they are. (If they do, she stops lying; if they don't, she keeps her promise until Pointe Sombre.)
- **Consequence:** Mara's willingness to come.
- **Callback:** Liv, in the Crib: "Did Mara tell you? ...No. She wouldn't have."

### MQ-A1 — The Wrong Characteristic: see §2.

### MQ-X — Cross Bearings (Acts I–II)
- **Hook:** at Pointe Sombre the player finds **Liv's chart** in the vault (her pencil line from the *Tern*'s bearing, a second line, and a long thin question mark where they cross).
- **Mechanic (no new system needed beyond inspect variants on an item):** the chart's description changes as bearings are added: *Tern* (prologue, needs the Sounder Chart), Pointe Sombre's direction-finding loop (Act I, in a storm), Tall Masts (Act II). Two bearings from nearby points give a long, loose cross (true to navigation practice); the third, from far off, closes it on **the Four-Mile Crib**.
- **Build:** Schematic Eye reads bearings cleanly; Engineering 2 can power the DF loop outside a storm; Survival *(future)* can take a rough bearing on a hum by ear and compass.
- **Consequence:** the player finds the Crib **by method**, the way Liv did. If the player destroyed the Pointe Sombre node before taking its bearing, a substitute bearing must come from the Local's powerhouse loop (a detour and a favor owed).

### MQ-B1 — Lock Passage (Act II)
- **Hook:** Kenning and the Crib are on Lower. The only lock is Aubin's, and the Local has closed it to the player (or to Compact ships, or to everyone after Pointe Sombre, depending on history).
- **Approaches (each is a world act):**
  1. **Permission:** earn the Floor's vote (the Local arc, §3.2).
  2. **Force:** join Hale's convoy running the lock; combat at the gates; the lock is damaged.
  3. **Protocol:** speak the lock's phrase on its channel (from Mara's transcripts, the Lexicon, or Liv's notes). The gates move. The lock also answers with a phrase that is in no manual. Every cardholder hears the gates obey someone else.
  4. **Stealth:** climb the dam at night and work the gates by hand (Engineering 2; Survival for the climb).
- **Decision:** the method.
- **Consequence:** (1) Local ally; (2) the lock closed to the Compact for the rest of the game, the Local's infirmary closed to the player, Cutters escalate; (3) the Local knows the Line can override them: the Stewards gain the Floor, and the Cutters mark the player; (4) the Local never learns who did it, unless someone saw.
- **Callback:** who stands with whom at Kenning; whether the dam will open for a black start.

### MQ-B2 — The Third Bearing (Act II): the Tall Masts arc (§3.3) contains it. The bearing is taken at the transmitter hall's direction-finding array during the Vigil.

### MQ-B3 — Dead Letter (Act II)
- **Hook:** Odile Marchetti finds the player (at Port Carrow or Tall Masts). She holds a letter from Liv addressed to the player on the *Ida*, returned undeliverable.
- **Escalation:** the Post's oath: she will only hand it to the addressee, in person, with proof. Proof is Liv's first letter (which the player has carried since the start), or someone who knows the player's face (Varga, Mara).
- **The letter:** *I'm not lost. I'm choosing. I took Hale's money and I'm not sorry. I did something at Pointe Sombre I am sorry for. If you've come this far you've seen it. Don't follow me past the Locks. If you do, bring the tape.*
- **The tape:** the Authority tape the player gave her two years ago. The player does not have it; Liv sent it back to the player by the Post weeks later. It is in Odile's dead-letter hold. Played on a Sounding deck, it carries the pattern and, under it, a voice reading a list of names. Whether one is the player's mother's is never settled.
- **Decision:** whom the player lets hear the tape (a Reader, an Answerer, Mara, no one). Small, personal, and remembered.

### MQ-B4 — Kallio at the Crib (Midpoint)
- **Trigger:** after the third bearing, in the next storm, wherever the player is.
- **Event:** every relay broadcasts Liv's voice. What happens next depends on world state: at a relit light, the lamp flashes the pattern; at Aubin, the Floor meets; at Tall Masts, Reyes leaves with the Answerers; at Port Carrow, Hale gets his convoy.
- **What the player does:** chooses whom to go with, or goes alone.

### MQ-C1 — The Severance Record (Act III) **[FULL]**
- Kenning's Authority tower, flooded to the second floor. Holt's logs; the Pattern Desk memos; the Severance record (03:12, Holt's code, launch capsized 02:30); the Authority's **last order** (no reconnection without all section controllers; there are none left). Dumont wants the succession papers; they do not say what she needs.
- **Build:** Engineering reads the logs' power traces; Persuasion with the squatters opens the dry stairwell; Survival finds the diver's route.
- **Decision:** what the player does with the last order (publish it via the Post, give it to Dumont, destroy it).

### MQ-C2 — Pumping Station (Act III) **[FULL]**
- **Hook:** the headquarters' lower floors (the tables archive) are under water. The pumping station can clear them, if powered.
- **The demonstration:** powering it wakes a small section. The Line reallocates by its old table: it lights the drowned hospital's circuits and **cuts the squatters' heat**. The squatters' spokesperson finds the player.
- **Approaches:** accept it (get the archive, lose the squatters' trust); undo it by hand (Engineering: manual override, and the archive floods again); **rewrite one table row** at the station's console (Schematic Eye or Liv's census): the first time the player edits the tables. It works, for one block.
- **Why it matters:** the ending's stakes shown at the scale of one building, before the player has to decide them for a region.

### MQ-C3 — The Intake (Act III) **[FULL]**
- **Four routes, one per ally:** the Local's power runs the tunnel pumps (dry, safe, slow); a Compact boat takes the flooded shaft (fast, exposed); the Sounding's protocol opens the bulkheads (quiet, and the tunnel's relays talk the whole way); force with Engineering and explosives (costly: the tunnel partly collapses behind the player, and no one else can follow).
- **Who comes:** the companion, and whichever faction's team the player allied with. **This decides who is physically in the Crib at the end.**
- **Found on the way:** Tam Reyes's body; Liv's lantern; mussel lines on the tunnel floor running straight to the Crib.

### MQ-E1 — Black Start: see `ENDINGS.md`.

---

## 2. The Pointe Sombre vertical slice: "The Wrong Characteristic" [SLICE]

### 2.1 Where it sits and why the player comes
- **Story position:** Act I, directly after the prologue shore. It is the first place the player follows Liv to, and the first place the player learns she hurt someone.
- **Why the player arrives:** the *Tern*'s bearing and Mara point to Pointe Sombre, and the *Ida* is due there on its dues run. The slice **opens in the storm crossing**: the *Ida* makes for a light on the headland, but it is the wrong light (a false one). The real tower is dark. The *Ida* grazes the reef, limps into the harbor, and will not sail again while the point is dark. The player's way onward is stuck until the light is settled. That is the slice's clock, and it is personal.
- **Standalone framing for the demo:** a short letter-and-map intro recaps the prologue in one screen. Mara is aboard. The slice works for players who never saw the Authority Shore.

### 2.2 What the lighthouse is really doing (author truth)
- The Pointe Sombre light sits over a **storm vault**: the node for **Section 14** of the Line.
- **Thirty years ago** Odette's father tapped the vault's live line to power the lamp: "the light that never needed oil", the pride of the point.
- **Three years ago**, in Current storms, the lamp began flashing **the pattern** instead of its own characteristic. The laker *Ashland Grey* misread it and struck the reef. Her salvage started the Pruitts' trade.
- **Four months ago** Liv came, saw the panel read **SECTION 14 — SEVERED — RESYNC PENDING (3 of 5)**, and **pulled the section key**. The vault went cold; so did the lamp. She told Odette to relight it by hand. The old clockwork was seized. In the first storm, **Remy Beaudry**'s boat hit the reef in the dark. Liv had already sailed. Odette told the village the light failed.
- **Three months ago** Sigrun Dahl (a Local Cutter) followed Liv's trail here, **cut the cable** from the vault to the lamp room and jammed the vault's lower door, so no one could relink it even with a key. She stayed, "buying copper".
- **Since then** the Pruitts hang a **false light** on the west headland in storms. Ships think they are rounding the point.
- **Now** Warden Hale is coming with a Compact-made section key to relight the tower on the Line for the dues route.

**Who sabotaged it:** everyone a little. The slice's answer to "was the malfunction intentional?" is **yes, four times, for four different reasons.**

### 2.3 What each local believes

| Person | Believes |
| --- | --- |
| Odette Beaudry (keeper) | "It failed." Privately: she let a stranger break it, and the light lied to her son once before it went dark. |
| Tem Pruitt (wrecker) | "The lake provides." The flashing light was cursed; the dark is honest. |
| Dell Pruitt | Found Remy's boat. Has not slept well since. |
| Marthe (trader) | Light means ships, ships mean stock. |
| Jonas Leclair (parent) | His children don't sleep in storms and draw rows of ticks on the walls. He wants the tower to stop, whatever that means. |
| Sigrun Dahl | "It's a node. It's calling home." |
| Warden Hale (arrives mid-slice) | "A fault that sinks ships." |
| Mara | "Write it down." And, seeing the children's ticks: silence. |

### 2.4 Locations (all small)
1. **Harbor and settlement** (hub): Marthe's store, Odette's cottage, the Pruitts' "salvage" shed, the infirmary shack, the smokehouse, the net loft where the settlement meets.
2. **The lighthouse**: tower, lamp room (lens, clockwork, the old oil lamp), gallery.
3. **The storm vault** (dungeon 1): Authority node room, section panel, direction-finding loop, the Authority keeper's log, a flooded lower level with **live water** (reuses the *Tern*'s hazard grammar), Liv's traces (her chart, a pencil stub, her note to Odette).
4. **The *Ashland Grey*** on the reef (dungeon 2, a wreck interior): cargo hold half under water, the Pruitts' cache, the laker's log (for Hale's Loss Book).
5. **The false-light headland**: the Pruitts' lantern post and a hide.
6. **The cable hut**: where Sigrun cut the feed; **mussel lines** run from it into the water.
7. **Remy's boat** on the rocks, and his marker. Environmental story only.

### 2.5 Quests in the slice region

| Quest | Scope | Core |
| --- | --- | --- |
| **The Wrong Characteristic** (main) | SLICE | Investigate the light; decide its fate in the tower and the vault. |
| **False Light** | SLICE | Find who shows the headland lantern. The *Ida* nearly died of it. |
| **Copper Buyer** | SLICE (trimmed) / INITIAL | Sigrun's real job. |
| **Loss Book** | INITIAL | Hale asks for the *Ashland Grey*'s log so her dead can be entered. The log records that the light flashed the wrong characteristic. |
| **Storm-Waking** | INITIAL | The Leclair children draw the pattern. A hint of the Sounding. |
| **Liv's Note** (part of main) | SLICE | Her note to Odette, found in the vault. |

About fifteen smaller encounters fit here (the crossing, wreckers on the reef, the vault's live water, a Cutter cache, storm wildlife on the cable line, a Compact enforcer at the harbor, a drowned-deer ring at the cable hut, the headland at night, and so on).

### 2.6 Resolution paths (each is a world act)

The **physical** decision is made in the vault and the lamp room. **Exposure** is a separate decision made at the settlement's storm meeting in the net loft, by **bringing evidence** (an object) there.

| Path | What the player does | Requires | Immediate consequence |
| --- | --- | --- | --- |
| **1. Relight on the Line** | Seat a section key in the vault panel and splice Sigrun's cut. | Hale's key (help Hale) or the player's own route to one; Engineering 2 for the splice, or Hale's crew | The tower blazes, free power. The *Ida* sails. Compact goods in Marthe's store; dues chits on doors. Hale is an ally; Sigrun leaves to report. **The panel reads RESYNC 4 of 5.** |
| **2. Relight by hand** | Free the clockwork, fuel the old lamp, and find a keeper. | Engineering 2 **or** parts from the *Ashland Grey*; oil (Compact trade **or** fish oil rendered at the smokehouse); a keeper: Odette (if she is forgiven), Dell Pruitt (if the wreckers are turned), or nobody | A weaker, honest light that is safe for local boats. The *Ida* sails by day. No resync. Hale is thoughtful rather than pleased. The Local respects it. **The settlement owns its light**, and someone has to wind it every night. |
| **3. Redirect the power** | Use the vault's live line to power the settlement (heat, infirmary, smokehouse) instead of the lamp. | A key **or** Engineering bypass | The settlement thrives through the winter; the sick child in the infirmary recovers. The reef stays dark; the Pruitts carry on unless exposed. The Compact is hostile; Varga is grim. **Hidden cost:** a powered node keeps rejoining. |
| **4. Kill the node** | Help Sigrun wreck the exchange and open the vault's sea cock. | Sigrun's trust, or force | Section 14 can never rejoin. The light can only be hand-lit (combine with 2) or stays dark. Sigrun becomes the player's Local contact (easier Aubin access). Hale is an enemy. **The vault's data is lost unless the player took the bearing and the log first.** |

**Exposure** (combine with any path, at the net loft):

| Bring | Effect |
| --- | --- |
| The false lantern (and the scavenger's word, if he is alive and here) | The Pruitts are expelled, fined, or offered the keeper's post. |
| Liv's note to Odette and the vault's access log | The village turns on Odette, or forgives her. Affects who can keep a hand-lit light. |
| The player's own admission that the listener was their sister | The village's view of the player shifts. Odette's reaction is the scene. Mara watches. |
| Sigrun's cut cable end | Hale arrests her (if present). The Local's standing drops. |
| Nothing | Everyone keeps their version. The meeting ends in the dark. |

### 2.7 How the Current relates, without explanation
- In the storm, the vault's lower level has live water, and the direction-finding loop carries the pattern on a **bearing**.
- If the tower is relinked (path 1), in the next storm the lamp flashes the pattern for three long seconds before its own characteristic returns. Everyone sees it.
- The Leclair children's ticks match the *Tern*'s spacing.
- The pattern tightens before the second storm: a forecast, if anyone knows how to read it.

### 2.8 What the slice contributes to the larger mystery
- **E3:** the Authority keeper's log records Current events as routine hazards for decades before the collapse.
- **E4, E5:** SECTION 14 — SEVERED. The Line exists and was cut by an order. The collapse-night log line: *"Section order received 03:12. Link locked. God help Kenning."* (Plants Kenning and the time.)
- **E6:** RESYNC PENDING. It is trying to come back.
- **Bearing two** on Liv's chart.
- **Liv's character:** she pulled the key, a boy died, she left.

### 2.9 Consequence chain (slice)

| Decision | Immediate | Local | Later callback | Ending implication |
| --- | --- | --- | --- | --- |
| Relight on the Line | Bright light, *Ida* sails | Compact store, dues, Hale ally | Flashes the pattern at the midpoint; Cutters come to cut it in Act II unless defended | Section 14 is joined; on the census tables Pointe Sombre is included and powered |
| Relight by hand | Weak honest light | Keeper chosen; local pride | Hale may end as a hand keeper; the model for "keep the light, cut the line" | Strengthens the Commons ending |
| Redirect power | Warm settlement | Reef stays dark | Node keeps rejoining; a Compact embargo | Pointe Sombre thrives in any ending that keeps its node powered |
| Kill the node | Dark or hand light | Sigrun ally | The Local trusts the player at Aubin | One fewer section for any black start |
| Expose Odette / the Pruitts / Liv / Sigrun | Shifts who leads and who keeps | Who stays in the settlement | Returning later, different faces | Who holds Pointe Sombre's vote under the Commons |

### 2.10 Kept deliberately small
No new faction HQ, no companion recruitment scene beyond Mara, no Sounding presence beyond the children's ticks, no Kenning. One settlement, three wilderness sites, two dungeons, two factions, one companion. See `AUDITS.md` §4.

---

## 3. Faction arcs

### 3.1 The Compact: "Light Dues" [FULL; one beat INITIAL]
- **Hook:** at Port Carrow, Dumont offers the player work: escort a relight crew to **Kestrel Light** on a small island outside the charter. Varga (Open Water) asks the player to see that Kestrel is lit **free**.
- **Escalation:** in the Lights Office the player finds the Surveyors' **resync ledger**: every relight's node count, climbing. Hale has been keeping it off the Board's table.
- **Approaches:** publish it by the Post; give it to Kowalczyk; use it to press Dumont (Persuasion); bury it for Hale (his trust, later).
- **Build:** Engineering reads the ledger's meaning; Persuasion with the Captains' Hall; Survival for the crossing to Kestrel in weather.
- **Decision (by doing):** at Kestrel, **seat the Compact key** (dues, resync), **relight by hand** (free, no resync, oil owed), or **leave it dark** (the islanders leave in spring).
- **Persistent consequence:** the balance of Board and Open Water; Kestrel's fate; whether the islands trust the Compact.
- **Later callback:** the Compact's force at Kenning is Board marines or Open Water captains depending on who won; Kestrel is on the census tables only if the player went there.

### 3.2 The Local: "Hand Log" [FULL]
- **Hook:** Aubin's lock is closed to the player. Kowalczyk will talk if the player can prove what the Compact's relights are doing (the resync ledger, or first-hand evidence from Pointe Sombre).
- **Escalation:** the Cutters are planning to bring down Port Carrow's breakwater light in a storm, with a convoy due. The Stewards propose sending a crew to seize the Crib.
- **Approaches:** stop the Cutters (stealth, combat, or talking Sobczak down with the Loss Book, Persuasion 3 *(future)*); let them (the convoy loses a ship); warn Port Carrow (the Local learns who warned).
- **Decision (by doing):** at the **Floor vote** at shift change, the player can speak, and the evidence physically laid on the table (the Table Day printout, the Loss Book, the resync ledger, Liv's census) decides which wing leads.
- **Persistent consequence:** Hands, Cutters, or Stewards lead the Local into Act III.
- **Later callback:** Cutters at Kenning bring charges; Stewards bring a seizure crew; Hands stay home, and the dam only opens for a black start if the Floor votes it.

### 3.3 The Sounding: "The Vigil" [FULL]
- **Hook:** Tall Masts will not share the third bearing with a stranger. The player must sit a storm **vigil** in a listening hut, transcribing, beside storm-waking children.
- **Escalation:** mid-storm, the channel carries **Liv's voice** for the first time. Reyes moves to key the transmitter and **reply**. A child walks out toward the masts' copper ground radials, which are live.
- **Approaches:** save the child (Survival); stop Reyes (combat, Persuasion, or Engineering to pull the transmitter's final stage); let him reply. If he replies, **every light the player relit anywhere flashes the pattern at once.**
- **Build:** Relay Ear transcribes cleanly (a better transcript earns the Readers' respect); Pulse Read tells which radials are live; Schematic Eye takes the bearing off the array.
- **Decision (by doing):** return the **Sounder Chart** to the *Tern* Book (it completes Ellis Tennant's last margin note); hand the **Lexicon** to the Readers or the Answerers; take the children's case to Tennant (end the child vigils or not).
- **Persistent consequence:** Readers or Answerers ascendant; forecasts for the Compact on or off; the children's fate.
- **Later callback:** a Reader recorder or an Answerer strike team at the Crib; the children in the epilogue.

---

## 4. The missing-person arc: Liv's trail

The thread must never disappear for more than one region. Each stop has **an object** and **a person**.

| Act | Object | Person | What it adds |
| --- | --- | --- | --- |
| Prologue | Her last letter (carried); "HEAR IT TOO"; the coil | Mara | She was here; she chose to go on |
| Act I | Her chart; her note to Odette | Odette, Sigrun | She pulled the key; a boy died |
| Act II Port Carrow | Her contract and advance | Hale, Varga | She took the money |
| Act II Aubin | Her confiscated kit; the key in the Local's safe | Kowalczyk | She traded the key for passage |
| Act II Tall Masts | Her annotations in the *Tern* Book | Tennant, Reyes | She learned the protocol; Tennant sent her; Tam went with her |
| Act II | The dead letter; the tape | Odile | *I'm choosing* |
| Midpoint | Her voice | everyone | She's alive, in the Crib |
| Act III | Her census notebook; Tam's body; her lantern | the squatters | Her tables, and whom she left off |
| Endgame | Liv | Liv | The reunion |

**The census notebook** is the arc's hinge. It is Liv's rewritten priority table: every settlement she has seen, what it needs, in what order. It is careful and humane, and it leaves off places she never visited. The player can **add** the places they have discovered (the existing discovered-locations list becomes story data). What the player adds, and what they strike, are real decisions.

---

## 5. The companion arc: "Mara's Book" (PROPOSAL; Mara's history is not decided)

- **Prologue:** Shore Watch. If she gets the coil, she transcribes it. She deflects about the *Tern* and about Liv (her promise).
- **Act I:** at Pointe Sombre she sees the Leclair children's ticks and goes quiet. If the player exposes Liv as their sister at the net loft, Mara tells them about the promise that night.
- **Act II, Tall Masts:** June Okafor, the last *Tern* crew member, and Mara: the woman who walked up the beach and the girl who didn't follow. With high Persuasion and Mara present, June says one sentence. On the Mourners' list, Mara finds her brother's name, entered decades ago by someone else.
- **Decision (by doing):** her **book** (decades of transcripts, the most complete protocol record outside the Sounding): burn it, give it to the Readers, give it to the Answerers, or carry it to the Crib.
- **Endgame:** at the Crib Mara can read the one phrase that **stands a section down** (it is in her book, if it survives), refuse, or, if the player has pushed her toward it, **answer**.
- **Opinion triggers:** she approves of hand-lit lights, returned charts, protected children, and listening first. She disapproves of speaking protocol casually, of using the children, and of lying to her about Liv.
- **Ending states:** see `CHARACTERS.md` §2.

---

## 6. Regional side stories

Each is a human story first, and each touches the Line in one concrete way.

### 6.1 The Schedule [FULL]
- **Hook:** locals set their clocks by an empty pre-collapse **automatic car ferry**, the *Cedar Queen*, that has run between two ruined terminals on its timetable for sixty years (rule 4: working devices repeat their last task).
- **Escalation:** the Pruitts or a Compact salvage crew mean to board her and pull her engine.
- **Approaches:** ride her (she docks at a terminal island no one can otherwise reach: a sealed Authority depot); stop her for salvage; **reprogram her route** (Engineering 3 *(future)*, or protocol) to serve a living island.
- **Decision:** salvage, ride, or reroute.
- **Consequence:** the salvagers get rich, or the depot opens, or an island gets a free ferry.
- **Callback:** the rerouted ferry appears in the epilogue running a new schedule, unless a Severance ending stops her mid-lake.

### 6.2 The Grand Harmon [FULL]
- **Hook:** a flooded lakeside resort hotel; squatters in the upper floors. During storms the ballroom's public-address system plays a pre-collapse **wedding announcement** (archive). **Adelaide Voss**, very old, believes it is her parents' wedding.
- **Escalation:** the PA's relay is a node, and the Compact wants it.
- **Approaches:** remove the relay (the voice stops; Compact standing); leave it (Adelaide keeps listening); **record it first** (Engineering: rig a tape deck) and then remove it.
- **Callback:** in Kenning, the same announcer's voice turns up in the Authority archive: an Authority broadcaster, reading storm warnings. Adelaide's parents' names are not on any Authority list. It was never her parents' wedding, or it was. The game does not say.

### 6.3 Salt Under the Lake [FULL]
- **Hook:** a salt mine whose drifts run under the lake bed. Salt is the region's winter food preservative. The miners hear voices in the far drift, and the conveyor runs by itself at night.
- **Escalation:** a Line trunk cable crosses the far drift. The miners want it cut; the Compact needs it (it feeds Port Carrow's lights); the Cutters want to use the mine to reach it.
- **Approaches:** cut it (the miners sleep; Port Carrow's breakwater light dims); protect it; let the Cutters in; island the drift (Engineering: a breaker the miners can throw themselves).
- **Callback:** salt prices across the region; whether Port Carrow's light is fully on at the midpoint.

### 6.4 Mayfly Night [INITIAL candidate]
- **Hook:** the annual mayfly hatch, which an island's fishers depend on. This year the swarm rises in a **ring** over a live cable, and the fish follow it into water where the fishers cannot net.
- **Escalation:** a boy wades out toward the ring.
- **Approaches:** Survival to read the ring and bring him back; follow the **mussel lines** to a cable junction and island it (the island loses a trickle of free power it didn't know it had); leave it.
- **Callback:** the junction is on the route to Kenning; the fishers remember.

### 6.5 The Ice Road [FULL; only if seasonal content exists]
- **Hook:** an ice road between two islands, marked with old Authority reflectors. During events the ice cracks in straight lines over cables.
- **Escalation:** a family must cross with medicine before the ice goes.
- **Approaches:** Survival to find the safe line; take the Local's cells to power the reflectors; wait.
- **Decision:** which island gets the medicine if there is only enough for one crossing.

### 6.6 Dead Letters [FULL]
- **Hook:** Odile asks the player to deliver one sixty-year-old letter to a descendant on a remote island only the player can reach.
- **The letter:** a lockkeeper's, written on the collapse night: *"They turned us off from Kenning again. Holt's voice on the channel, sorry, sorry. We're opening the gates by hand."* It ties the Local's founding, Holt, and the Severance to one family.
- **Decision:** deliver it, open it, or give it to the Local's archive.
- **Callback:** the descendant can hold a key under the Commons.

---

## 7. Environmental story seeds (small, no quest required)

1. **The half-repainted crate:** GREAT LAKES MARITIME SUPPLY with COMPACT stenciled halfway over it, and the painter's brush left on top. (Prologue shore; a cheap Tier B dressing variant.)
2. **The lock freighter:** a freighter held in a disused lock chamber for sixty years, gates shut by storm protocol. A village lives inside the hull. The water in the chamber is theirs as long as the Line never resyncs.
3. **Weather buoys** still reporting, drifted into a bay, chained together by someone who uses them as a fence.
4. **Channel buoys that all moved in a line** one night, toward the Crib's bearing.
5. **A farmhouse radio log** from the collapse week: a family listening to storm warnings, then Holt's voice, then silence.
6. **A ship's bell** on a wreck that rings in the pattern in storms, although the wreck is on its side.
7. **The mark off the point**: from the prologue shore at low water in clear weather, a **mussel line** runs straight out from the *Tern* to a dark shape on the bottom. Nothing more.
8. **The Pattern Desk's office** in Kenning: three desks, three coffee mugs, one wall of sounder rolls, one wall of memos pinned over each other.
9. **Loss Book pages** nailed up in a harbor church, each name crossed out when a body came back. Few are crossed out.
10. **Children's ticks** drawn on walls in every settlement that has a storm-waking child. The player starts to notice them everywhere.

---

## 8. Consequence chains for the major decisions

| Decision | Immediate | Local | Later callback | Ending implication |
| --- | --- | --- | --- | --- |
| Shore Watch route (existing) | Relay silent; scavenger dead or alive | Mara warm or cold; the landing stage pilot's variants | The scavenger at Pointe Sombre; Mara's transcripts (coil route) | Mara's book exists only if she had the coil |
| Sounder Chart kept, sold, returned | Item held or gone | Hale buys it; or the Readers | Completes the *Tern* Book | Readers' trust; one line from June Okafor |
| Pointe Sombre light | See §2.9 | See §2.9 | See §2.9 | See §2.9 |
| Lock Passage method | Gates open | Local ally, enemy, or frightened | Who comes to Kenning; whether the dam opens | Black start possible without force? |
| Pointe Sombre key custody | Key moves | The holder gains a vote | The holder brings it to the Crib | Required for the Commons; missing keys must be forced |
| Kestrel Light | Lit on the Line, by hand, or dark | Island stays or leaves | Board or Open Water at Kenning | Kestrel on the census tables or not |
| The Floor vote | Wing leads the Local | Aubin's posture | Charges, seizure crew, or no one at Kenning | Severance ending prepared or blocked |
| The Vigil reply | Every relit light flashes | Readers or Answerers ascendant | Who comes to the Crib | Open Channel ending available or not |
| Mara's book | Burned or given | Mara's trust | The stand-down phrase at the Crib | Whether the Crib can be stood down safely |
| Pumping station | Squatters' heat cut, or a table rewritten | Squatters ally or enemy | Squatters in the tunnel, or not | First proof the tables can be edited |
| Who comes through the tunnel | The team | Tunnel route and cost | Who stands in the Crib | Which endings are physically possible |
| What goes on the census | Places added or struck | n/a | The tables | Who gets power, heat, and light in the census endings |

## 9. Decisions made by doing (summary)

| Kind of act | Examples |
| --- | --- |
| Which machine they repair | Pointe Sombre clockwork; Kestrel's lamp; the pumping station |
| Which route they open | The lock; the ferry's new route; the tunnel |
| Who receives an object | The Sounder Chart; the Pointe Sombre key; Mara's book; the dead letter; the Authority's last order |
| Whether they activate infrastructure | Seat a key; power a pump; speak a phrase at a gate |
| Whether they reveal information | Evidence carried to the net loft; the resync ledger by the Post |
| Which community they physically connect | Pointe Sombre's power; Kestrel; the census tables |
| Which facility they disable | The vault's exchange; the salt-mine cable; the Crib's trunk cables |
| Whom they bring to a location | The vigil (Mara); the Floor (evidence); the Crib (the team) |
| What evidence they preserve or destroy | Vault data before the sea cock; the Severance record; the tape |
