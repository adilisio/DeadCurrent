# Pointe Sombre — Dialogue Text for the Slice

Status: **PROPOSAL, 2026-09-30. A document only.** No dialogue asset is created. This is the full text of the slice's conversations, keyed to the flags, items, and stages in `SLICE_WRONG_CHARACTERISTIC.md` (read that first). The format mirrors `create_dialogue.py`: each conversation has **entries** (first match wins, last one unconditional) and **nodes** (a speaker, a line, choices). Each choice has conditions, consequences, and a next node (none means the conversation ends).

## Notation

- `flag` = `WorldFlag flag` set; `!flag` = negated. Areas: `sombre.` unless shown.
- `has X` = `HasItem X`. `stage Q=S` = `QuestStage`. `notstarted Q` = `QuestNotStarted`.
- `[Skill N]` = `SkillAtLeast Skill.<Skill> N` (the HUD shows the label; failed checks stay hidden).
- `do:` consequences. `→ node` next node; `→ end` ends the conversation.
- **NO_TIE** = `!player.tie.sister, !player.tie.partner, !player.tie.took_in`.
- **Tie choices** set exactly one tie flag and are shown only under NO_TIE. **Tie variant lines** show only with their flag; the neutral line shows under NO_TIE.

## Changes to the beat script made while writing (small)

1. **The meeting is Marthe's.** The net loft is over her store, so the meeting lives in `sombre_marthe` (entry at stage `knows`). There is no separate `sombre_meeting` asset, and no presence rule is needed for a chair.
2. **The Pruitts get two conversations** (`sombre_tem`, `sombre_dell`), and **Jonas** gets one (`sombre_jonas`). An asset cannot know which NPC is speaking.
3. **One summary flag, `sombre.light_decided`,** is set by every physical act in the beat script's §4 (seat the card, throw the settlement feed, open the sea cock, light the burner). "It's done" and "Leave it as she left it" test it instead of four flags.
4. **Odette's key is an item,** `sombre_vault_key`. The vault hatch is an inspectable, "Unlock the hatch" (`has sombre_vault_key` → `do: vault_opened`).
5. **In the standalone demo, Mara asks the tie question on the crossing**, not Varga. Varga already knows the player and Liv; Mara doesn't.
6. **Varga starts both quests.** Her first conversation's choices carry `StartQuest sombre.characteristic` and `StartQuest sombre.false_light`. Quests that start with their objectives already met chain forward on their own (existing behavior).

### Applied in VS-09 (2026-10-02): plan §8.2 and what building the data found

The conversations below are now data (`Tools/ContentSpecs/dialogue/sombre_*.py`, `quests/sombre_*.py`, `items/sombre.py`). They were built as written, except for these changes. The ids are the ledger's (`Design/POIs/sombre_ids.md`). Change 4 above is superseded there: the hatch is a portal, not an inspectable (ledger §8).

7. **Dell's wet boots (§8.2 #1).** In `first`, the `[Survival 2]` line "Your boots are wet to the knee. It hasn't rained since the storm." becomes **"Your boots are wet to the knee. Nobody wades the reef for fish."** The check, the confession, and `sombre.dell_confessed` are unchanged. This is the default; Anthony may replace it. Why not the plan's example ("Nobody walks the reef in this for fish."): "in this" needs the storm, and after `knows` the weather is calm.
8. **"Catch them at it" (§8.2 #2).** No line or stage was added for it. Meeting Dell at the post in the first storm is the same `sombre_dell` conversation and the same confession. False Light has two approaches: the lantern (evidence) and Dell's confession.
9. **"Force" (§8.2 #3).** No line was added for it. Tem is never hostile.
10. **The call to the loft (§8.2 #4).** Marthe's `store` gains **"Call the island to the loft."** It is shown only at stage `knows`, until used, and sets `sombre.meeting_called`. Her reply is a new node, `call`: **Marthe: "I'll send round for them. Go on up."** (new wording, PROVISIONAL). Her `loft` entry now needs `sombre.meeting_called` as well as the stage.
11. **Pickups that set flags (§8.2 #8).** Data note only. `false_lantern` and `liv_note` are given by their cells' inspectables, which also set `sombre.false_light_taken` / `sombre.liv_note_found` and hide the prop through presence. No conversation gives them.
12. **Copper Buyer (§8.2 #9).** No quest asset and no `sombre.copper_buyer` id. Sigrun's conversation and flags carry the whole story.
13. **The end card's trigger (§8.2 #11).** Data note. Varga's "Where next?" sets `sombre.slice_end`, the ledger's default. Nothing reads the flag until VS-19's story card. VS-19 may move the trigger to a "Watch the light" interaction instead.
14. **A forgiven Odette can keep the light** (a wiring fix found while building; not a §8.2 item). §3 offered "Will you keep it? By hand." to a forgiven Odette only in `knows`. Forgiveness happens at the meeting, and the meeting ends `knows`, so that choice could never show. The beat script says a forgiven Odette can still keep the light (§5, and §4's "Choose a keeper" row). The same choice, with the same conditions, is now also offered in her `after` node, which a forgiven Odette reaches after the meeting. It stays in `knows`, where it is harmless.
15. **Stage directions.** Lines carry spoken text only. Mara's "(a long look at the chalk rows)" in `ticks` is the line's opening "..."; Odette's "(a pause)" in `keeps` becomes "...Yes."
16. **Speakers.** In the meeting, the reaction lines are spoken by Tem (`r_pruitts`), Odette (`r_odette`, `r_tie`), Hale (`r_sigrun`), and Sigrun (`r_sigrun_reply`), inside Marthe's asset. Speaker names are first names or surnames as written here: Mara, Varga, Odette, Marthe, Tem, Dell, Sigrun, Hale, Jonas.

17. **Mara's crossing lines stay on the crossing** (found in VS-10). Her `crossing_tie` and `crossing` entries used `notstarted sombre.characteristic` to mean "still at sea", but the quest now starts with Varga on the quay (change 6). A player who skipped Mara on the deck would have heard "Before we get there." on the quay. Both entries now also need `!sombre.reef_struck`. After the strike she opens with `town`, and a player who never answered her keeps NO_TIE.

**Read aloud at Checkpoint B** (each line that differs from this document's text):
- Dell (player): "Your boots are wet to the knee. Nobody wades the reef for fish." (#7)
- Marthe (player choice): "Call the island to the loft." (#10)
- Marthe: "I'll send round for them. Go on up." (#10)
- Odette, after the meeting, if forgiven (player choice): "Will you keep it? By hand." (#14; same words as `knows`)
- Odette: "Every four hours. Remy used to wind it with me. ...Yes. Tell whoever lights it I'll take the first watch." (#15)
- Mara: "...Write it down." (#15)

**Open for Anthony (not changed):** if the player asks Odette to keep the light at `knows` and later exposes her at the loft without the note, she stays keeper. Her `after_keeper` entry comes before `after_disgraced`, and the keeper silhouette reads `keeper_odette` alone. The beat script says an exposed, unforgiven Odette "will not keep it". Whether exposure revokes an earlier yes is a creative call. The default is to keep the data as written.

---

## 1. `sombre_mara` — Mara (placed, not following)

**Entries**
| Node | Conditions |
| --- | --- |
| `crossing_tie` | `notstarted sombre.characteristic`, NO_TIE, `!shore.liv_asked`, `!mara_tie_asked` |
| `crossing` | `notstarted sombre.characteristic` |
| `promise` | `player_named_tie`, `!shore.promise_told` |
| `after_line` | `meeting_done`, `light_line` |
| `after` | `meeting_done` |
| `panel` | `panel_read`, `!mara_panel` |
| `ticks` | `ticks_seen`, `!mara_ticks` |
| `town` | (always) |

**Nodes**

**crossing_tie** — Mara: "Before we get there. You never said who she is to you."
- "My sister." → `crossing` · do: `player.tie.sister`, `mara_tie_asked`
- "My partner." → `crossing` · do: `player.tie.partner`, `mara_tie_asked`
- "She took me in when nobody else would." → `crossing` · do: `player.tie.took_in`, `mara_tie_asked`
- "Does it matter?" → `crossing_shrug` · do: `mara_tie_asked`

**crossing_shrug** — Mara: "Not to the lake."
- "..." → `crossing`

**crossing** — Mara: "That light's on the wrong head. Tell your captain."
- "About the *Tern*. 'Two of them came up the beach.' That was sixty years ago." → `watch` · if `wreck.mara_pressed`, `!watch.revealed` · do: `watch.revealed`
- "What is it you do, on that shore?" → `watch` · if `!wreck.mara_pressed`, `!watch.revealed` · do: `watch.revealed`
- "Hold on to something." → end

**watch** — Mara: "That wasn't me. That was the watch. It's the same thing. There's been a watch on that shore since the night the *Tern* came in. Whoever keeps it keeps the log, and says what the log says as if they'd seen it. So nobody gets to call it an old story. I'm the fourth."
- "And the third?" → `third`
- "Hold on to something." → end

**third** — Mara: "Walked out to the *Tern* one storm night. The log says I didn't follow."
- "Hold on to something." → end

**town** — Mara: "Keep your voice down. This island's listening for somebody to blame."
- "Goodbye." → end

**ticks** — Mara: (a long look at the chalk rows) "...Write it down."
- "Same spacing as the *Tern*." → `ticks_same` · if `has survey_chart` · do: `mara_ticks`
- "Goodbye." → end · do: `mara_ticks`

**ticks_same** — Mara: "Same spacing as everything. That's the part I don't write."
- "Goodbye." → end

**panel** — Mara: "Write it down. All of it. Nobody believes the second telling."
- "Liv left a note under the chart." → `panel_note` · if `liv_note_found` · do: `mara_panel`
- "Goodbye." → end · do: `mara_panel`

**panel_note** — Mara: "I know her hand. Don't read it to me."
- "Goodbye." → end

**promise** — Mara: "She asked me not to tell anyone from the *Ida* where she went. I kept it as long as I could."
- "She said I'd come?" → `promise_sister` · if `player.tie.sister` · do: `shore.promise_told`
- "She said I'd come?" → `promise_partner` · if `player.tie.partner` · do: `shore.promise_told`
- "She said I'd come?" → `promise_took_in` · if `player.tie.took_in` · do: `shore.promise_told`
- "She said someone would come?" → `promise_neutral` · if NO_TIE · do: `shore.promise_told`
- "Goodbye." → end · do: `shore.promise_told`

**promise_sister** — Mara: "She said you'd come. She said you always did." → "Goodbye." → end
**promise_partner** — Mara: "She said you'd come, and that she'd deserve it." → "Goodbye." → end
**promise_took_in** — Mara: "She said you'd come, and that it'd be her fault." → "Goodbye." → end
**promise_neutral** — Mara: "She said someone would. She didn't say who." → "Goodbye." → end

**after_line** — Mara: "It flashed the pattern before it came right. Three seconds. I counted."
- "Goodbye." → end

**after** — Mara: "Whatever that light does tonight, the watch would have written it down. So will I."
- "Goodbye." → end

---

## 2. `sombre_varga` — Captain Ines Varga

**Entries**
| Node | Conditions |
| --- | --- |
| `after_line` | `meeting_done`, `light_line` |
| `after_hand` | `meeting_done`, `light_hand` |
| `after_dark` | `meeting_done` |
| `waiting` | `QuestActive sombre.characteristic` |
| `first` | (always) |

Every choice in `first` carries **do: `StartQuest sombre.characteristic`, `StartQuest sombre.false_light`**.

**first** — Varga: "I read that light on the west head as the point and put us on the reef for it. Somebody lit it on purpose. The *Ida* isn't leaving this harbor while the point's dark."
- "What do you need?" → `need`
- "Liv came through here." → `liv`
- "I'll find out why." → end

**need** — Varga: "Find out why the light failed, or find me a keeper who'll light it. And whoever lit that lantern on the west head, I want a name."
- "Liv came through here." → `liv`
- "I'll find out." → end

**liv** — Varga: "Your listener. Four months back, on the Packet. Ask the keeper what she did up there. Nobody else on this rock will say her name."
- "Goodbye." → end

**waiting** — Varga: "Still dark. Still here. Pumps going."
- "It was the Pruitts. Tem and Dell hold the lantern." → `told` · if `dell_confessed`, `!varga_told` · do: `varga_told`
- "It was the Pruitts. Here's their lantern." → `told` · if `has false_lantern`, `!dell_confessed`, `!varga_told` · do: `varga_told`
- "Liv came through here." → `liv`
- "Goodbye." → end

**told** — Varga: "Poor people with a lantern. I've been poorer. I'd still have drowned."
- "Goodbye." → end

**after_line** — Varga: "Brightest light this side of the Narrows. For a moment there it was saying something else. You saw it. We sail tonight. You and your watcher have berths."
- "It was the Pruitts. Tem and Dell hold the lantern." → `told` · if `dell_confessed`, `!varga_told` · do: `varga_told`
- "It was the Pruitts. Here's their lantern." → `told` · if `has false_lantern`, `!dell_confessed`, `!varga_told` · do: `varga_told`
- "Where next?" → `next`
- "Goodbye." → end

**after_hand** — Varga: "Small light. Honest one. We go at first light, and I'll trust it."
- "It was the Pruitts. Tem and Dell hold the lantern." → `told` · if `dell_confessed`, `!varga_told` · do: `varga_told`
- "It was the Pruitts. Here's their lantern." → `told` · if `has false_lantern`, `!dell_confessed`, `!varga_told` · do: `varga_told`
- "Where next?" → `next`
- "Goodbye." → end

**after_dark** — Varga: "Dark as she left it. We go by daylight, and I'll steer wide of that head."
- "It was the Pruitts. Tem and Dell hold the lantern." → `told` · if `dell_confessed`, `!varga_told` · do: `varga_told`
- "It was the Pruitts. Here's their lantern." → `told` · if `has false_lantern`, `!dell_confessed`, `!varga_told` · do: `varga_told`
- "Where next?" → `next`
- "Goodbye." → end

**next** — Varga: "She went through Aubin Locks on the Packet. So will we, if the Local lets us."
- "Goodbye." → end

---

## 3. `sombre_odette` — Odette Beaudry, keeper

**Entries**
| Node | Conditions |
| --- | --- |
| `after_keeper` | `meeting_done`, `keeper_odette` |
| `after_disgraced` | `meeting_done`, `exposed_odette`, `!odette_forgiven` |
| `after` | `meeting_done` |
| `knows` | `stage sombre.characteristic=knows` |
| `first` | (always) |

**first** — Odette: "It failed. Lights fail. If you want the story, the store has more talkers than me."
- "A listener came through here four months ago." → `listener`
- "You're the keeper. You know more than 'it failed.'" `[Persuasion 2]` → `key_press` · if `!has sombre_vault_key`, `!vault_opened` · do: `GiveItem sombre_vault_key`
- "Goodbye." → end

**listener** — Odette: "Lots of people come through. She wanted to see the vault. I have the key. That's all I say to a stranger."
- "She's my sister." → `key_tie` · if `player.tie.sister`, `has liv_letter`, `!has sombre_vault_key`, `!vault_opened` · do: `GiveItem sombre_vault_key`
- "She's my partner." → `key_tie` · if `player.tie.partner`, `has liv_letter`, `!has sombre_vault_key`, `!vault_opened` · do: `GiveItem sombre_vault_key`
- "She took me in when nobody would." → `key_tie` · if `player.tie.took_in`, `has liv_letter`, `!has sombre_vault_key`, `!vault_opened` · do: `GiveItem sombre_vault_key`
- "She's why I came." → `key_tie` · if NO_TIE, `has liv_letter`, `!has sombre_vault_key`, `!vault_opened` · do: `GiveItem sombre_vault_key`
- "Goodbye." → end

**key_tie** — Odette: "...Then you know what she's like when she's decided something. Here. She had the same look."
- "What did she do down there?" → `ask_light`
- "Goodbye." → end

**key_press** — Odette: "I know it wasn't the storm. That's what I know. Here's the key. The hatch at the foot of the tower. Go and look, and don't tell me what you find."
- "Goodbye." → end

**ask_light** — Odette: "Ask the light. I've stopped."
- "Goodbye." → end

**knows** — Odette: "You've been down there. I can see it on you."
- "Will you keep it? By hand. Oil and the old clockwork." → `keeps` · if `!light_line`, `!keeper_dell`, `!keeper_odette`, `!exposed_odette` · do: `keeper_odette`
- "Will you keep it? By hand." → `keeps` · if `!light_line`, `!keeper_dell`, `!keeper_odette`, `odette_forgiven` · do: `keeper_odette`
- "Liv left you a note." → `note` · if `has liv_note`
- "Goodbye." → end

**keeps** — Odette: "Every four hours. Remy used to wind it with me." (a pause) "Yes. Tell whoever lights it I'll take the first watch."
- "Goodbye." → end

**note** — Odette: "I know what it says. I read it the day she left, and pinned it back where she'd put it. I wanted someone else to find it."
- "Goodbye." → end

**after_keeper** — Odette: "Every four hours. It's a good weight in the hand."
- "Goodbye." → end

**after_disgraced** — Odette: "Go on. Everyone else has said it."
- "Goodbye." → end

**after** — Odette: "Whatever you did to my light, it's done now."
- "Goodbye." → end

---

## 4. `sombre_marthe` — Marthe, the store and the net loft

**Entries**
| Node | Conditions |
| --- | --- |
| `after_line` | `meeting_done`, `light_line` |
| `after_hand` | `meeting_done`, `light_hand` |
| `after_power` | `meeting_done`, `power_settlement` |
| `after_dark` | `meeting_done` |
| `loft` | `stage sombre.characteristic=knows` |
| `store` | (always) |

**store** — Marthe: "No light, no ships, no stock. Do the arithmetic."
- "Who keeps the light?" → `who`
- "I need oil for a lamp." → `oil`
- "Where does the island meet?" → `loft_where`
- "Goodbye." → end

**who** — Marthe: "Odette. Her family's had it three generations. Since her boy, she doesn't go up."
- "Goodbye." → end

**oil** — Marthe: "Smokehouse can render you a can, if you can stand the smell. The Compact sells better, when the Compact comes."
- "Goodbye." → end

**loft_where** — Marthe: "Net loft, over this store. When there's something to decide."
- "Goodbye." → end

### The meeting (`SLICE_WRONG_CHARACTERISTIC.md` §5)

**loft** — Marthe: "Is the light burning tonight, or isn't it?"
- "Not yet." → end
- "It's done." → `evidence` · if `light_decided`
- "Leave it as she left it." → `evidence` · if `!light_decided`

**evidence** — Marthe: "Then say what you came to say. Everyone's here."
- "This was burning on the west head the night the *Ida* hit." → `r_pruitts` · if `has false_lantern` · do: `RemoveItem false_lantern`, `exposed_pruitts`
- "Four months ago someone let a stranger into the vault." → `r_odette` · if `has vault_access_log` · do: `RemoveItem vault_access_log`, `exposed_odette`
- "The listener pulled the card. She wrote to Odette." → `r_liv` · if `has liv_note` · do: `RemoveItem liv_note`, `exposed_liv`
- "Someone made sure it could never be fixed." → `r_sigrun` · if `has cut_cable_end` · do: `RemoveItem cut_cable_end`, `exposed_sigrun`
- "That's all." → `close` · do: `meeting_done`

**r_pruitts** — Tem Pruitt: "The lake provides. We only held the lantern."
- "Go on." → `evidence`

**r_odette** — Odette: "I told you it failed. I lied. Remy was the first boat out."
- "She let her in. Liv's the one who pulled the card." → `forgive` · if `exposed_liv`, `!odette_forgiven` · do: `odette_forgiven`
- "Go on." → `evidence`

**r_liv** — Marthe: "A listener pulled it. For what?"
- "She's my sister." → `r_tie` · if `player.tie.sister` · do: `player_named_tie`
- "She's my partner." → `r_tie` · if `player.tie.partner` · do: `player_named_tie`
- "She took me in." → `r_tie` · if `player.tie.took_in` · do: `player_named_tie`
- "She's why I came." → `r_tie` · if NO_TIE · do: `player_named_tie`
- "Odette let her in. Liv pulled the card." → `forgive` · if `exposed_odette`, `!odette_forgiven` · do: `odette_forgiven`
- "Go on." → `evidence`

**r_tie** — Odette: "Then she owes me a son. And you came all this way for her."
- "Go on." → `evidence`

**forgive** — Marthe: "...Then it's the listener this island owes a grudge, not Odette."
- "Go on." → `evidence`

**r_sigrun** — Hale: "Copper buyer. You're coming with me." (Hale is always present by the `knows` stage.)
- "Go on." → `r_sigrun_reply`

**r_sigrun_reply** — Sigrun: "Somebody had to make sure it couldn't be fixed. You'll thank us when the tables come back."
- "Go on." → `evidence`

**close** — Marthe: "Then that's decided. Whatever it is, we live under it."
- "Goodbye." → end

### After the meeting
**after_line** — Marthe: "Ships by morning. That arithmetic I can do." → "Goodbye." → end
**after_hand** — Marthe: "A light we have to feed. I'll put oil on the list, and a name for every four hours." → "Goodbye." → end
**after_power** — Marthe: "Warm houses and a dark reef. I'll sell more candles than oil." → "Goodbye." → end
**after_dark** — Marthe: "No light, no ships, no stock. Same arithmetic as before." → "Goodbye." → end

---

## 5. `sombre_tem` — Tem Pruitt

**Entries:** `exposed` (`exposed_pruitts`); `first` (always).

**first** — Tem: "The lake provides. Salvage is honest work. You want anything off a wreck, you come to me."
- "Someone lit a lantern on the west head the night the *Ida* hit." → `threat`
- "The *Ashland Grey* was yours?" → `grey`
- "Goodbye." → end

**threat** — Tem: "Someone should be careful out on that head in weather. It's a bad place to be seen."
- "Goodbye." → end

**grey** — Tem: "The *Ashland Grey* was the lake's. We just got there first."
- "Goodbye." → end

**exposed** — Tem: "You'll be wanting me sorry. I'm poor, not sorry."
- "Goodbye." → end

---

## 6. `sombre_dell` — Dell Pruitt

**Entries:** `keeper` (`keeper_dell`); `confessed` (`dell_confessed`); `first` (always).

**first** — Dell: "I found his boat. That's all."
- "That isn't all." `[Persuasion 2]` → `confess` · do: `dell_confessed`
- "Your boots are wet to the knee. It hasn't rained since the storm." `[Survival 2]` → `confess` · do: `dell_confessed`
- "Goodbye." → end

**confess** — Dell: "Remy was my friend. After the light went, Ma said the lake was giving us something back. So I hold the lantern on the west head when the weather comes in. We didn't make the dark. We just ate off it."
- "Stop. Keep the real light instead." → `keeper_offer` · if `!light_line`, `!keeper_odette`, `!keeper_dell`
- "Goodbye." → end

**keeper_offer** — Dell: "Me? ...If the lantern stops. If you tell Varga it stops."
- "It stops." → `keeper_yes` · do: `keeper_dell`
- "Think about it." → end

**keeper_yes** — Dell: "Every four hours. I know the stair. I used to race him up it."
- "Goodbye." → end

**confessed** — Dell: "You know, then. Say it at the loft if you're going to."
- "Keep the real light instead." → `keeper_offer` · if `!light_line`, `!keeper_odette`, `!keeper_dell`
- "Goodbye." → end

**keeper** — Dell: "I'm keeping it. Four hours on, four off. I sleep in pieces now."
- "Goodbye." → end

---

## 7. `sombre_sigrun` — Sigrun Dahl

**Entries:** `taken` (`exposed_sigrun`; she is taken off the island, and this is the fallback line if presence isn't built); `helping` (`sigrun_helping`); `revealed` (`sigrun_revealed`); `first` (always).

**first** — Sigrun: "Buying copper. You got any? I pay in cells. Local cells, full charge."
- "What do you want copper for?" → `copper`
- "Those are lineman's shears on your belt." `[Engineering 2]` → `shears`
- "I found the lamp's feed cut. Clean. Shears, not a saw." → `reveal` · if `has cut_cable_end` · do: `sigrun_revealed`
- "Goodbye." → end

**copper** — Sigrun: "Everyone wants copper. Copper's the only thing on this lake that remembers where it's supposed to go."
- "Goodbye." → end

**shears** — Sigrun: "Good eye. Lot of linemen in the world."
- "Goodbye." → end

**reveal** — Sigrun: "...Fine. Local Nine. I cut it. Your listener pulled their card, and the Compact's coming with a new one. Every light they relight, that thing under the rock tries to phone home. I made sure it can't."
- "Get me into the vault." → `unjam` · if `!vault_opened` · do: `vault_opened`
- "What would you do with the vault?" → `sea`
- "Goodbye." → end

**unjam** — Sigrun: "Lower door's jammed because I jammed it. Come on."
- "What would you do with it?" → `sea`
- "Goodbye." → end

**sea** — Sigrun: "Open the sea cock. Flood it. Nothing rejoins from under the lake."
- "Do it with me." → `with_me` · if `!light_line`, `!node_destroyed` · do: `sigrun_helping`
- "Not yet." → end

**with_me** — Sigrun: "Good. I've got the wrench. Tell me when."
- "Goodbye." → end

**revealed** — Sigrun: "Still here. Still buying copper, officially."
- "Get me into the vault." → `unjam` · if `!vault_opened` · do: `vault_opened`
- "About the vault." → `sea`
- "The Warden's in the harbor." → `warden` · if `hale_arrived`
- "Goodbye." → end

**warden** — Sigrun: "Hale. I know. I'd rather not meet him on a quay."
- "Goodbye." → end

**helping** — Sigrun: "Wrench is ready. Your call."
- "Goodbye." → end

**taken** — Sigrun: "Tell Nine I kept my mouth shut."
- "Goodbye." → end

---

## 8. `sombre_hale` — Warden Casimir Hale (present only once `hale_arrived`)

**Entries**
| Node | Conditions |
| --- | --- |
| `after_line` | `meeting_done`, `light_line` |
| `after_hand` | `meeting_done`, `light_hand` |
| `after_flooded` | `meeting_done`, `node_destroyed` |
| `after_power` | `meeting_done`, `power_settlement` |
| `after` | `meeting_done` |
| `offer` | `hale_met` |
| `first` | (always) |

**first** — Hale: "Warden Hale, Lights Office. I've a card for this tower and a route that needs it lit by the next storm. You've been down there. Tell me what I'm looking at."
- "Section Fourteen. It's trying to come back." → `s14` · do: `hale_met`
- "Why would I help the Compact?" → `why` · do: `hale_met`
- "Goodbye." → end · do: `hale_met`

**s14** — Hale: "Every light I've relit says that on its panel. It's a fault. A fault that sinks ships when the light's out. Seat the card and it's a light again."
- "Go on." → `offer`

**why** — Hale: "Because the Compact keeps a book of everyone this lake has drowned, and I'd like to stop writing in it."
- "Go on." → `offer`

**offer** — Hale: "The card, then?"
- "Give me the card." → `card` · if `!has section_key_compact`, `!light_decided` · do: `GiveItem section_key_compact`
- "Send your crew up to splice the feed." → `crew` · if `!feed_spliced`, `!hale_crew_up`, `!node_destroyed` · do: `hale_crew_up`
- "I need oil for a hand light." → `oil` · if `!has lamp_oil` · do: `GiveItem lamp_oil`
- "The listener. Liv Kallio." → `liv`
- "The copper buyer." → `copper` · if `sigrun_revealed`
- "There's a laker on the reef. The *Ashland Grey*." → `grey`
- "Goodbye." → end

**card** — Hale: "Seat it, splice the feed, and the *Ida* sails tonight. The Compact pays for lights that burn." → "Go on." → `offer`

**crew** — Hale: "Done within the hour. They splice. You seat the card. I won't do that part for you." → "Go on." → `offer`

**oil** — Hale: "Oil I have. Keepers I don't." → "Go on." → `offer`

**liv** — Hale: "She's yours? She took my money too. Found the pattern's source, she said, then stopped writing. If you find her before I do, tell her the Loss Book has a new page since she left, and there's a boy's name on it." → "Go on." → `offer`

**copper** — Hale: "I know what she is. Local Nine sends one to every light I relight. I've buried two linemen whose ladders were cut." → "Go on." → `offer`

**grey** — Hale: "Her log, if you find it. Her dead aren't in my book yet." (The log itself is initial-production content.) → "Go on." → `offer`

**after_line** — Hale: "It showed something before it came right. I saw it. I'm going to write that down as a fault." → "Goodbye." → end
**after_hand** — Hale: "A hand light. Honest, and dim, and someone every four hours for the rest of their life. I'll enter it as lit." → "Goodbye." → end
**after_flooded** — Hale: "You flooded a vault the Authority built to outlast us. The Board will want your name. I'll give it." → "Goodbye." → end
**after_power** — Hale: "Warm houses and a dark reef. You chose who drowns. I do it every day. Welcome to the work." → "Goodbye." → end
**after** — Hale: "Dark, then. I'll be back with another card in spring." → "Goodbye." → end

---

## 9. `sombre_jonas` — Jonas Leclair

**Entries:** `after_line` (`meeting_done`, `light_line`); `after` (`meeting_done`); `first` (always).

**first** — Jonas: "They don't sleep when the weather comes in. They sit up and draw those lines. The little one says the light used to draw them too."
- "The light drew them?" → `drew`
- "Do you want the light back?" → `want`
- "Goodbye." → end

**drew** — Jonas: "Before it went dark it flashed wrong in storms, and she'd count along. Now she counts in the dark." → "Goodbye." → end

**want** — Jonas: "I want my kids to sleep. Whatever that takes." → "Goodbye." → end

**after_line** — Jonas: "It flashed the lines again tonight. She saw it. She's awake." → "Goodbye." → end

**after** — Jonas: "Dark and quiet tonight. She slept a little." → "Goodbye." → end

(In the hand-lit ending the tower is lit but does not flash the pattern, so `after` is right there too.)

---

## 10. Checks

- **No stuck nodes:** every node above has at least one unconditional choice (Goodbye, "Go on.", "Not yet.", "Hold on to something.", "...", or "Think about it."). This meets the editor's existing rule.
- **Every entry list ends unconditional.**
- **Mutual exclusion holds:** keeper choices test `!keeper_odette` / `!keeper_dell` / `!light_line`; tie choices test NO_TIE; evidence choices need the item and remove it.
- **No tie leak:** every tie-variant line has a NO_TIE partner. Varga, Hale, Tem, Sigrun, Jonas, and Marthe (outside the meeting) never mention the tie.
- **Quest start:** the only `StartQuest` calls are on Varga's `first` choices. If the player acts before meeting her (Odette's key, Sigrun, the vault), flags are set anyway; when the quest starts, its transitions chain forward to the right stage.
- **Build coverage:** the key (Persuasion 2, or the letter plus any tie or none), the confession (Persuasion 2 or Survival 2), the vault via Sigrun (the cable end, no build), Hale's crew (no Engineering needed). A zero-investment player can reach every outcome.

## 11. New ids introduced here (add to the beat script's list)

`sombre.light_decided`, `sombre.mara_tie_asked`, `sombre.mara_panel`, `sombre.mara_ticks`, `sombre.hale_met`; item `sombre_vault_key`; inspectable `sombre.vault_hatch` ("Unlock the hatch").
