# "Posted From the Shore" — Beat Script for the Prologue Additions

Status: **PROPOSAL, 2026-09-30. A document only.** Nothing here changes `create_dialogue.py`, `build_boathouse.py`, `create_quest.py`, or any asset. It describes **additions** to the accepted Authority Shore (Phases 1–4) that connect it to the Pointe Sombre slice (`SLICE_WRONG_CHARACTERISTIC.md`). All new ids are proposed.

Built on Anthony's decisions (2026-09-30): the player chooses their tie to Liv; the Shore Watch is an inherited office whose watchers speak its memory as "I"; Mara is the companion.

## 0. Hard rules for whoever builds this

- **Additive only.** Do not rename or remove the quest `shore.watch`, its stages (`accepted`, `return_killed`, `return_coil`, `done_killed`, `done_coil`), any existing flag (`shore.relay_inspected`, `shore.voice_discussed`, `shore.relay_recovered`, `shore.path_cleared`, `shore.mara_heard_kill`, `wreck.*`), or any `boat.*` persistent id. Old saves must load, with the new content in its initial state.
- **Every existing Mara line stays word for word.** New lines are new nodes and new choices. Dialogue node ids are not saved, but keep the existing ones anyway, so diffs stay readable.
- **Shore Watch and the *Tern* keep working exactly as accepted.** Nothing new can block either route.
- **No soft-locks.** Every build can reach the prologue's end.

## 1. What changes, in one paragraph

The player finds **Liv's letter** by the cot and reads it, which starts a second, quiet quest. Asking Mara about Liv leads to **"Who is she to you?"**: the tie choice (sister, partner, the one who took them in) happens in conversation, which the dialogue system already supports. Mara deflects and keeps her promise to Liv until the player earns it (Persuasion), shows it (the handwriting under the scavenger's KEEP OUT, or the call sign in the coil), or has her trust (the coil route). Pressing her about the *Tern* line reveals **the watch**. Once Shore Watch is resolved and Mara has admitted Liv went east, she decides to come, **leaves the watch's page on the boathouse door**, and the *Ida*'s boat takes them both to Pointe Sombre.

**Change from the earlier draft:** `CHARACTERS.md` proposed making the tie choice when the letter is first read. An inspectable cannot offer choices, and a "readable with choices" object would be a new system. Having Mara ask is free, and it is a better scene. In the standalone slice demo, Mara asks the same question on the crossing (Varga already knows the player and Liv).

## 2. Beats

| Beat | Where | The player | State |
| --- | --- | --- | --- |
| P1 | Boathouse, by the cot | Reads Liv's letter | `shore.letter_read`; item `liv_letter`; quest `shore.posted` starts |
| P2 | Mara | "I'm looking for a listener. Liv Kallio." → "Who is she to you?" | `shore.liv_asked`; one `player.tie.*` |
| P3 | Anywhere on the shore | Gathers evidence (optional, several routes) | `shore.chalk_matched`, `shore.callsign_heard` |
| P4 | Mara | Admission: Liv was here, and went east | `shore.liv_admitted`; quest → `knows_east` |
| P5 | Mara (after the *Tern* line) | The watch reveal | `watch.revealed` |
| P6 | Mara | The promise | `shore.promise_told` |
| P7 | Mara, then the boathouse door | Mara decides to come; the watch's page goes on the door | `watch.mara_travelling`, `watch.away` |
| P8 | The landing | Row out to the *Ida* | `shore.left_on_ida`; quest completes; prologue ends |

Shore Watch (existing) and the *Tern* (existing) run alongside, in any order.

### P1 — The letter
- **Actor:** a new inspectable on the cot's blanket, "Letter", verb **Read**. Proposed persistent id `boat.letter`. First read: `GiveItem liv_letter`, `SetWorldFlag shore.letter_read`, `StartQuest shore.posted`.
- **Text (tie-neutral):**
  > Posted from the old Authority landing, west end of the Narrows.
  > The relay here talks if you feed it. I've fed it. I think I know where it's coming from.
  > Don't come after me. You'll want to, so I'm saying it plainly: don't.
  > Tell Varga she'll need a new listener for the season. Tell her I'm sorry.
  > — L.
- **Re-read variant** (after `shore.letter_read`): "Liv's letter. You know it by heart. Her capital A has no crossbar; she always said a crossbar reads as a dash in a transcript."
- The existing cot line ("You slept here, or passed out here") is unchanged. The letter sits beside it.

### P2 — "Who is she to you?"
New choice in Mara's everyday nodes (`greeting`, `who`, `place`, `inprogress`, `done_killed`, `done_coil`), never in the Shore Watch turn-ins:

| Choice | Conditions | Next / consequences |
| --- | --- | --- |
| "I'm looking for a listener. Liv Kallio." | `WorldFlag shore.letter_read`, `!shore.liv_asked` | → `liv_ask`; `SetWorldFlag shore.liv_asked` |

**`liv_ask`** — Mara: "Lot of listeners come to this shore. Most of them I send back. Who is she to you?"

| Choice | Conditions | Consequence | Mara |
| --- | --- | --- | --- |
| "My sister." | no tie flag set | `SetWorldFlag player.tie.sister` | → `liv_deflect` |
| "My partner." | no tie flag set | `SetWorldFlag player.tie.partner` | → `liv_deflect` |
| "She took me in when nobody else would." | no tie flag set | `SetWorldFlag player.tie.took_in` | → `liv_deflect` |
| "Does it matter?" | always | none (tie stays unset; tie-variant lines everywhere fall back to neutral ones) | → `liv_deflect` |

**`liv_deflect`** — Mara: "If she came through, she went on. People do." Choices: the admission routes (P4), and Goodbye.

**Tie fallback rule (applies to the whole game):** if no tie flag is set, every tie-variant line is hidden and a neutral line shows instead ("She's why I came."). Nothing breaks.

### P3 — Evidence (all optional; any one is enough for P4)
| Evidence | How | Build | Sets |
| --- | --- | --- | --- |
| **The handwriting** | Inspect the scavenger's KEEP OUT sign while carrying `liv_letter`. New variant: "KEEP OUT in tar. Under it, smaller: HEAR IT TOO. The A has no crossbar. Neither does any A in Liv's letter." | none (Survival 2 adds: "Written from the path side, by someone standing still, taking their time.") | `shore.chalk_matched` |
| **The call sign** | Coil route: in `done_coil`, new choice "What's it saying?" (needs `shore.letter_read`). Mara: "Weather, mostly. And a call sign, over and over. L-K, *Ida*. That mean anything to you?" | none | `shore.callsign_heard` |
| **The coil's seating** | Existing Relay Ear variant on the relay rig, unchanged. With `liv_letter`, a new variant appends: "A listener's hand. Liv seated them like this." | Relay Ear | `shore.callsign_heard` (same effect) |
| **The *Tern*'s bearing** | Existing Schematic Eye + chart variant on the sounder, unchanged; a new variant after it, with `liv_letter`: "...a bearing. East. Somewhere past the next point." | Schematic Eye | `shore.bearing_east` |

### P4 — The admission
Choices in `liv_deflect` (and in Mara's everyday nodes once `shore.liv_asked` is set and `shore.liv_admitted` is not):

| Choice | Conditions | Mara |
| --- | --- | --- |
| "You're leaving something out." | `SkillAtLeast Skill.Persuasion 2` | → `liv_admit` |
| "She wrote on the scavenger's sign. That's her hand." | `WorldFlag shore.chalk_matched` | → `liv_admit` |
| "The coil keeps saying her call sign." | `WorldFlag shore.callsign_heard` | → `liv_admit` |
| "You kept your word about the coil. Keep it about her too." | `QuestStage shore.watch done_coil` | → `liv_admit` |
| "The *Tern*'s chart points east. So did she." | `WorldFlag shore.bearing_east` | → `liv_admit` |

**`liv_admit`** — Mara: "She was here. Five months back. Asked about the *Tern*, stood on that beach two nights with a radio in her lap. Seated that coil herself, before he ever found it. Then she went east. Boats stopped coming round the east point not long after. The light at Pointe Sombre went dark." → `SetWorldFlag shore.liv_admitted` (quest → `knows_east`).

The admission is reachable with no build investment (the handwriting needs only the letter and the sign), with each skill, and with each Shore Watch route. The combat route never makes it harder, only colder: Mara's line in `done_killed` stays as accepted.

### P5 — The watch
New choice in the existing `wreck_pressed` node (after the accepted line "Two of them came up off that beach..."), before Goodbye:

| Choice | Conditions | Mara |
| --- | --- | --- |
| "That was sixty years ago. You'd have been a child." | `WorldFlag wreck.mara_pressed`, `!watch.revealed` | → `watch_reveal` |

**`watch_reveal`** — Mara: "That wasn't me. That was the watch. It's the same thing." Choice: "The watch?" → **`watch_explain`**: "There's been a watch on this shore since that night. Whoever keeps it keeps the log, and says what the log says as if they'd seen it. So nobody gets to call it an old story. I'm the fourth." → `SetWorldFlag watch.revealed`.

Optional follow-up, "What happened to the third?" (after `watch.revealed`): "Walked out to the *Tern* one storm night. The log says I didn't follow." (This is the proposal about the third watcher; Mara's own history otherwise stays open.)

**If the player never presses her about the *Tern*,** the reveal happens on the crossing instead (slice beat B0, already written).

### P6 — The promise
After `shore.liv_admitted`, in Mara's everyday nodes:

| Choice | Conditions | Mara |
| --- | --- | --- |
| "Why didn't you tell me straight away?" | `WorldFlag shore.liv_admitted`, `!shore.promise_told` | "She asked me not to tell anyone from the *Ida* where she went. I've kept worse promises. Not many." → `SetWorldFlag shore.promise_told` |

Tie-aware follow-up (one line per tie, shown only with the matching flag): sister: "She said you'd come. She said you always did." / partner: "She said you'd come, and that she'd deserve it." / took in: "She said you'd come, and that it'd be her fault." (Neutral if no tie: "She said someone would come.")

### P7 — Mara decides to come; the watch's page
- **When:** `shore.liv_admitted` **and** `QuestComplete shore.watch` (either route). If Shore Watch isn't finished, Mara: "Not while he's still working my shore." (That is the only gate, and it points the player back to content that already exists.)
- **Mara:** "When's your boat? ...Then I'm coming. Someone sensible should be in it."
- Choice: "What about the watch?" → Mara: "The watch doesn't walk off. It leaves a page." → `SetWorldFlag watch.mara_travelling`.
- **The page:** a new inspectable on the boathouse door, present when `watch.away` (§6, capability 1). Mara sets `watch.away` in the same exchange. Text: "Nailed to the door, in careful block capitals: WATCH AWAY. KEEP OUT OF THE WATER. THE LOG IS WITH ME. Under it, smaller, in the same hand, a date."
- **Future only (not in the prologue build):** another keeper could hold the watch while Mara is gone (the scavenger talked down, Dell Pruitt, Odette). That needs a talkable hostile and a return trip, so it stays in `QUEST_ARCS.md` §5.

### P8 — The *Ida*'s boat
- **Actor:** the *Ida*'s rowboat at the landing, an inspectable, verb **"Row out to the *Ida*"**, shown when `watch.mara_travelling`. It sets `shore.left_on_ida`, and the quest completes.
- **Before that:** the rowboat is present from the start (the *Ida* left it for the player) with a default variant: "The *Ida*'s rowboat, pulled up above the waterline. Varga said three nights."
- **What happens next:** in the current single-map build, an end card: "Pointe Sombre" (the slice). In a multi-map build: travel (§6, capability 2).

## 3. Quest data (proposed)

### `shore.posted` — "Posted From the Shore"
| Stage | Objective | Transitions | On enter |
| --- | --- | --- | --- |
| `asking` (start) | "Liv posted her last letter from this landing. Find out where she went." | → `knows_east` when `WorldFlag shore.liv_admitted` | |
| `knows_east` | "Liv went east, toward the dark light at Pointe Sombre. The *Ida*'s boat is at the landing." | → `done` when `WorldFlag shore.left_on_ida` | |
| `done` (completes) | "You left the Authority Shore for Pointe Sombre. Mara came with you, and the watch's page is on the boathouse door." | | |

### New ids (proposed)
- **Flags:** `shore.letter_read`, `shore.liv_asked`, `shore.chalk_matched`, `shore.callsign_heard`, `shore.bearing_east`, `shore.liv_admitted`, `shore.promise_told`, `shore.left_on_ida`, `player.tie.sister`, `player.tie.partner`, `player.tie.took_in`, `watch.revealed`, `watch.mara_travelling`, `watch.away`.
- **Items:** `liv_letter`.
- **Persistent actors:** `boat.letter`, `boat.ida_rowboat`, `boat.watch_page`.
- **Quest:** `shore.posted`.
- **Dialogue:** additions to `mara_intro` only (nodes `liv_ask`, `liv_deflect`, `liv_admit`, `watch_reveal`, `watch_explain`, `watch_third`, `liv_promise`, `mara_comes`, and the tie-variant follow-ups).

## 4. Everything still works

| Player path | Result |
| --- | --- |
| Ignores the letter | Shore Watch and the *Tern* play exactly as accepted. The prologue never ends; the rowboat keeps its default line. |
| Reads the letter, never asks Mara | The quest sits at `asking`. The objective points at Mara. |
| Asks, chooses "Does it matter?" | Everything works; tie lines fall back to neutral. |
| Combat route, zero build | Handwriting evidence → admission → Mara comes (colder, same lines otherwise). |
| Coil route | The trust choice or the call sign → admission. |
| Old save (before these additions) | Loads; the letter appears unread on the cot; no new flags; all accepted flags intact. |

## 5. Tests a builder would add

- The letter: first read gives the item, sets the flag, starts the quest, once; the re-read shows the variant.
- Tie: exactly one tie flag can be set; "Does it matter?" sets none; tie-variant lines show only with their flag, and the neutral line otherwise.
- Admission: reachable by each of the five routes, including a zero-investment combat-route run.
- Gate: Mara won't come until Shore Watch is complete; then the page and the rowboat verb appear.
- Additivity: every accepted Shore Watch and Survey Launch map test still passes unchanged; an old save loads with the new actors in their initial state.
- Persistence: save after each beat; F9 restores quest stage, flags, the page, and the rowboat verb.

## 6. Missing reusable capabilities

1. **Conditional presence** (the pilot's proposal): the watch page on the door (`watch.away`). Fallback: the page is always there, with a variant ("A bare nail." before, the page text after). **No new system needed with the fallback.**
2. **Map travel** (the rowboat to Pointe Sombre). Fallback: an end card in the current single-map build.
3. **Talking to a hostile** (the scavenger as a keeper). Deferred; not part of the prologue build.

Without capability 1 or 2, the prologue additions are **buildable today** with inspect variants, dialogue conditions and consequences, a quest, and one item.

## 7. How this touches existing text (no edits)

- Mara's `wreck_pressed` line becomes the watch speaking. Unchanged.
- "Name's Mara. I watch the shore..." now names an office. Unchanged.
- "Things come to a signal like that": Liv's coil, fed by the scavenger. Unchanged.
- The Relay Ear reading ("seated by someone who knew the pinout") is Liv's hand. Unchanged; a later variant adds her name.
