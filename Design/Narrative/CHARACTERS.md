# DEAD CURRENT — Characters (PROVISIONAL)

Status: **PROVISIONAL, 2026-09-29; updated 2026-09-30 with Anthony's decisions.** Only Mara and the scavenger exist in game. **Decided by Anthony (2026-09-30):** the player chooses their tie to Liv (Decision 1: B); the Shore Watch is an inherited office whose watchers speak its memory as "I" (Decision 3: B); Mara is the companion (Decision 4: A); Liv pulled the Pointe Sombre key and a boy drowned (Decision 15: A). Everything else here, including Mara's personal history beyond the office, remains a proposal. Names are working names.

A deliberately small cast: eight major characters, a handful of supporting ones, and a few pre-collapse voices heard only in records.

---

## 0. The player

- **Working version (PROPOSAL):** a deckhand and salvager on the Compact trader *Ida Lamberton*: good with a boat, a pry bar, and a bargain. No faction oath.
- **The tie to Liv is the player's choice (DECIDED, Decision 1: B).** See §1.1.
- **Guilt (all ties):** two years ago the player salvaged an Authority tape from a wreck and gave it to Liv as a curiosity. The pattern was on it.
- **What the player carries at the start:** Liv's last letter (*don't come*), and nothing else of hers.
- **Deliberately open:** gender, appearance, age within adulthood, what the player feels about Liv. Dialogue should let the player be angry, loyal, frightened, or indifferent without a morality meter.
- **Background mechanically:** the trader-deckhand background can justify an early barter or boat-handling edge without becoming a class system.

---

## 1. Liv Kallio — the missing person

| | |
| --- | --- |
| **Public role** | A listener on the *Ida*; the best forecaster on the Narrows. About forty. Now "missing, probably drowned". |
| **Private goal** | Keep the Line from black-starting on the old tables until someone, preferably her, can write new ones. |
| **Fear** | That she is the only one who sees it, and that she is wrong. Second fear, never spoken: that the voice she first heard on the player's tape was **her mother**, lost on the water when Liv was a girl. (She has never said this to anyone. The player can find it in her notebook. It is never confirmed or denied.) |
| **Contradiction** | She despises the factions for wanting to control the Line, and she is controlling it, alone, by hand, with tables she wrote herself. |
| **Personal stake** | The player. The tape. A drowned boy at Pointe Sombre she has not let herself think about. |
| **Relationship to the Current** | Hears it better than anyone alive. Believes "it's asking". Has not slept a full night in weeks. Hum-ear in her left ear. |
| **Relationship to the missing person** | She is the missing person. |
| **What she wants from the player** | At first: to go home. Then: help holding the Line. Never succession: she refuses to hand the chair to anyone, the player included. |
| **What changes the relationship** | What the player has learned about her on the way (Pointe Sombre, the Compact money, the traded key, Tam Reyes's death in the tunnel), and whom the player brings into the Crib. |
| **Where she can oppose the player** | In the Crib. If the player moves to burn the Line, hand it to Hale, or open it to the Answerers, Liv will try to stop them: at the console, by locking bulkheads, by refusing to leave. She will not shoot the player. She will make them choose. |
| **Ending states** | Stays as the Crib's operator (alone, or under the Commons); comes home with the player; tried by the Compact; walks into the tunnel with a lantern and is not seen again; dies in the Crib (if the player burns it and does not carry her out). See `ENDINGS.md`. |

**Voice:** quick, dry, impatient, very funny when she is not frightened. Apologizes badly. How she talks *to the player* depends on the tie (§1.1).

**Opinion arc for the player:** lost → reckless → manipulative → right about something no one else sees → someone who has decided she alone should run the lakes → the player's verdict.

### 1.1 The tie (DECIDED: the player chooses; details PROPOSAL)

Anthony's option read "sister, partner, or the captain who took them in". In this draft Liv is the *Ida*'s listener, not her captain, so the third tie is written as **"she took you in"**. If Liv should have been a captain (of her own boat, before the *Ida*), that is a small change; say so.

| Tie | Who Liv is to the player | What she does for them | The grievance when she leaves | How she talks to them |
| --- | --- | --- | --- | --- |
| **Sister** | Older by about twelve years; half-raised the player on the *Ida*. They share a mother, lost on the water. | Kept them fed, taught them the boats | She decided alone, as always, and told them not to follow | As if they are still twelve |
| **Partner** | Equals. Years together aboard the *Ida*. An established relationship, not a romance system: no meter, no courtship scenes. | Shared the work, the cabin, the plans | She went without them, and let them hear she'd drowned | As an equal she has wronged |
| **She took you in** | Found the player on the docks as a child, orphaned by the lake, and brought them aboard. Liv's own mother was lost the same way. | Gave them a berth and a trade | She went exactly where she always told them never to go | As someone she is still responsible for |

**Where the choice is made (revised 2026-09-30):** in conversation. When the player first asks Mara about Liv, Mara asks, "Who is she to you?" (`PROLOGUE_POSTED_FROM_THE_SHORE.md` P2). In the standalone slice demo, Mara asks the same question on the crossing (Varga already knows the player and Liv). An inspectable cannot offer choices, and asking costs no new system. "Does it matter?" is also allowed and sets no tie. The choice is stored as one world flag (proposed ids: `player.tie.sister`, `player.tie.partner`, `player.tie.took_in`), read by the existing `WORLD_FLAG` condition. With no tie flag set, every tie-variant line falls back to a neutral one.

**Writing rules:**

1. **About 90% of Liv lines are tie-neutral.** Everyone calls her Liv. Most NPCs never learn the tie unless the player says it.
2. **Only keystone moments vary**, one line per tie:
   - the moment someone first asks who she is to them (the choice itself);
   - Mara's admission of her promise;
   - the player's admission at the Pointe Sombre net loft ("the listener was my sister / my partner / the one who took me in");
   - the dead letter's closing;
   - Liv's first line at the reunion;
   - Liv's refusal ("Nobody should be *given* this");
   - Bring Her Home's last scene.
3. **People who knew both** (Varga, Odile, Mara after the promise) know the tie and may refer to it.
4. **The mother subplot is Liv's**, not the player's. Only for the sister tie is it shared.
5. **The partner tie** must never become a romance mechanic, a jealousy beat, or a reward.

---

## 2. Mara — the Shore Watch (companion)

### Established in game (PROVISIONAL)
Watches the shore by the boathouse "for people who still listen before they shoot". Cautious, dry, practical. Has heard the relay's words and does not want them "repeated on the boats". Deflects about the *Tern* ("it's always a storm"). Pressed: "Two of them came up off that beach the night she grounded... I didn't follow." Keeps the coil if given it and sits up transcribing. A face was made for her.

### The Shore Watch (DECIDED, Decision 3: B; details PROPOSAL)
**The Shore Watch is an inherited office, and its watchers speak its memory as "I".** Mara's eyewitness line about the *Tern* is the watch speaking, not Mara's own childhood. Her age is therefore open, and her current face needs no change.

- **Origin (PROPOSAL):** the watch began the night the *Tern* grounded. The first watcher saw two people come up off the beach, would not follow them, and stayed. Since then there has always been one watcher on this stretch of shore.
- **Its rule:** *the shore does not forget because a watcher did.* A watcher never says "before my time" about anything that can still kill someone. The watch's memory is spoken in the first person, so it cannot be dismissed as an old story.
- **Its work:** warn boats off the *Tern*'s water, turn back people who come looking for "the mark off the point", and keep **the watch log**. The log is sixty years of entries: who came asking, which boats were lost, what the storms did, and, since the relays began to talk, what they said. It is the most complete record of the words outside Tall Masts.
- **Its line of succession (PROPOSAL):** Mara is the fourth watcher. The third, who trained her, walked out to the *Tern* one storm night and was not found. The watch's own keeper broke its rule. Mara took the log.
- **How the player learns it:** pressed again after the *Tern* line (Persuasion, or trust after the coil route), or on the crossing to Pointe Sombre: "That wasn't me. That was the watch. It's the same thing." At Tall Masts it pays off: June Okafor, the last of the *Tern*'s crew, meets the watch (see `QUEST_ARCS.md` §5).
- **Leaving the shore (the companion's cost):** a watcher who leaves leaves the shore unwatched. Before she sails, Mara nails the watch's current page to the boathouse door: WATCH AWAY. KEEP OUT OF THE WATER. The player can instead **bring someone to keep the watch** while she is gone (decided by doing; see `QUEST_ARCS.md` §5). If no one keeps it, the shore shows the cost later.
- **Fits the production pilot:** in the Landing Stage's coil-route variant, Mara's crate is packed and her skiff loaded, which now reads as the watch getting ready to travel.

### Mara (PROPOSAL beyond the office; her personal history is still open)
- **Liv:** five months ago Liv came asking about the *Tern*. Mara, as the watch, tried to turn her back and failed. Before she left, Liv asked one thing: *if someone from the Ida comes asking after me, don't tell them where I went.* Mara's deflection in the prologue is that promise. She holds to it until she decides the player will follow anyway, and then comes along "so someone sensible is in the boat".
- "People who still listen before they shoot" means listeners, and people with enough sense to hear something out before they kill it.

| | |
| --- | --- |
| **Public role** | The Shore Watch. The player's companion. |
| **Private goal** | To see the mark's other end once, and not to follow it. |
| **Fear** | That she wants to answer. That the third watcher did. |
| **Contradiction** | Tells everyone never to repeat the words. The watch writes down every one of them. |
| **Personal stake** | Her promise to Liv. The watch. The log. |
| **Relationship to the Current** | Refuses to interpret it: "I write down what it says. I don't decide what it means." |
| **Relationship to Liv** | Liked her. Kept her secret. Blames the watch a little for the *Tern*. |
| **What she wants from the player** | At first: quiet on her shore (Shore Watch). Then: that the player not follow Liv. Then: that someone sensible goes with them. |
| **What changes the relationship** | Shore Watch's route (the coil route earns trust; the combat route makes her colder, and "went back for him anyway" colder still). Whether the player reads the *Tern* and asks honestly. Whether the player treats the words as tools. Whether anyone keeps the watch while she is gone. |
| **Where she can oppose the player** | If the player means to **speak the words** at the lock or the Crib, or hand the log to the Answerers, she objects and can leave. At the Crib she can refuse, or call the stand-down sequence (it is in the log). |
| **Ending states** | Returns to her shore and the watch, to a landing stage that looks like whatever the player made of it; hands the log to a successor; makes the Crib a watch post and stays as its keeper (the watch is an office for staying, which is exactly what the Crib needs); burns the log; gives the log to Tall Masts; leaves the player's company after a betrayal; dies in the tunnel (only if the player takes her there without the pumps or the protocol, and chooses to go on). |

**Why Mara is the companion (DECIDED):** she is already built and liked in playtest, holds the coil and the watch log (the Lexicon's missing pages), is the living voice of the night the Sounding began, and can travel to Pointe Sombre in the vertical slice.

---

## 3. Warden Casimir Hale — the Compact's Warden of Lights (the rival)

| | |
| --- | --- |
| **Public role** | Head of the Compact's Lights Office. Relit twelve lights in twenty years. Hunts wreckers. |
| **Private goal** | Make the lights permanent, with or without the Board. If he reaches the Crib, he will run the Line himself on "lights first" tables: every navigation aid lit before any town. |
| **Fear** | Another name in the Loss Book he could have prevented. His first ship, the *Constance*, went onto a dark reef twenty-five years ago; he was the only one who swam out. |
| **Contradiction** | Serves the Board and plans to overrule it. Hates wreckers, and would put a town in the dark to keep a lighthouse lit. |
| **Personal stake** | His daughter lives on a small island outside the charter that loses someone every winter. Liv took his money and his trust. |
| **Relationship to the Current** | "Physics. A fault that sinks ships." He has seen the resync counts and believes a firm hand fixes them. |
| **Relationship to Liv** | Hired her to find the pattern's source. She vanished with the advance. He believes she is dangerous and brilliant, and he is right on both counts. |
| **What he wants from the player** | At Pointe Sombre: help relight the light. After: find Liv for him, and he pays well. |
| **What changes the relationship** | The Pointe Sombre outcome (on the Line, he respects the player; hand-lit, he is thoughtful; destroyed, he is an enemy). Whether the player reports Liv's location. Whether the player shows him the stale tables; he is one of the few people the tables actually move. |
| **Where he can oppose the player** | Lock Passage (he will force it with a convoy); the Kenning beachhead; the Crib, with an armed crew. He is capable of killing Liv if she will not step away from the console. |
| **Ending states** | Holds the Crib (Compact or Hale tables); holds the Compact's key under the Commons; arrested by his own Board; dies in the intake tunnel; **keeps a single hand-lit light for the rest of his life** (a possible end if Pointe Sombre was relit by hand and he was shown the tables). |

He is the game's antagonist because he is competent, sincere, and willing to do what Liv is doing with fewer doubts.

---

## 4. Commissioner Aurelie Dumont — the Compact's Board

| | |
| --- | --- |
| **Public role** | Harbor master of Port Carrow; chair of the Charter Board. |
| **Private goal** | Turn the Compact into a government before it turns into a cartel of captains. She wants **legitimacy**: the Authority's succession papers, which she believes are in Kenning. |
| **Fear** | That after her the Compact splits into armed captains, and the lakes get warlords instead of harbor masters. |
| **Contradiction** | A legalist who will forge the succession papers if they say the wrong thing. |
| **Personal stake** | Port Carrow; forty years of the Compact; the dues that pay her keepers. |
| **Relationship to the Current** | Doesn't matter what it is. It matters who holds the switch. |
| **Relationship to Liv** | Knows of her as "Hale's listener" and a debt on the books. |
| **What she wants from the player** | The Severance record, the succession documents, and Liv's census (because a census is the start of a government). |
| **What changes the relationship** | Whether the player exposes the Surveyors' resync counts; whether the player brings her the real Authority order (which forbids reconnection without all controllers). |
| **Where she can oppose the player** | If the player gives keys to the Local or publishes the resync counts, she embargoes the player and the islands that helped. |
| **Ending states** | Chair of a Compact that runs the Line; one voice among four under the Commons; deposed by Open Water; the Compact fractures after an Open Channel or Severance ending. |

---

## 5. First Reader Abigail Tennant — the Sounding

| | |
| --- | --- |
| **Public role** | First Reader of the Sounding at Tall Masts. Daughter of Ellis Tennant, the *Tern*'s deckhand-surveyor. About fifty-five. |
| **Private goal** | To know what her father heard the night the *Tern* grounded. He never told her. |
| **Fear** | That her father's life and hers were spent transcribing an echo. |
| **Contradiction** | Preaches patience and never answering. **Quietly gave Liv the Lexicon** and pointed her at the Crib, so someone else would do what she would not. |
| **Personal stake** | Her father's book. The storm-waking children in her care. |
| **Relationship to the Current** | Believes it is correspondence. Not sure anymore that refusing to answer was wise. |
| **Relationship to Liv** | Recognized a better listener than herself and used her. Feels responsible for Tam Reyes. |
| **What she wants from the player** | The *Tern*'s Sounder Chart, returned to the *Tern* Book. News of Liv. Later: that the player carry a Reader to the Crib to record, not answer. |
| **What changes the relationship** | Returning the chart (a large trust gain). Learning she sent Liv. Whether the player treats the children as instruments. |
| **Where she can oppose the player** | If the player moves to burn the Line, she will send every operator the Sounding has to the Crib to stand in the way, and she will withhold forecasts from the Compact. |
| **Ending states** | Records the Crib under the Commons; loses the Readers' Table to the Answerers; closes Tall Masts to the children; keeps listening to a Line that has gone silent (Severance). |

### June Okafor — the Silent Reader (supporting)
The *Tern*'s survey technician, about eighty-five, the last living founder. She has not spoken of that night in sixty years. **She still will not say what she heard** (continuity with Mara's existing line). When she meets Mara she recognizes the watch, not the woman: she asks, "Did you follow?" and Mara, speaking as the watch, answers, "I didn't follow." It is the first time in sixty years that the two sides of that beach have spoken. With high Persuasion and Mara present, June says one sentence more, which is never the answer. She is the Sounding's conscience: she has never answered, and never let anyone ask her why.

---

## 6. Chief Operator Bram Kowalczyk — the Local

| | |
| --- | --- |
| **Public role** | Elected chief of the Aubin lock and powerhouse. About sixty-two. |
| **Private goal** | Retire knowing Aubin stays free. Get his granddaughter to a school that does not exist. |
| **Fear** | Table Day again. And that the Cutters are right and he is too soft to do what they would. |
| **Contradiction** | Against any central power, while running the only dam and charging tolls on the only lock. |
| **Personal stake** | His father was one of the men who broke back into the plant on the collapse night. |
| **Relationship to the Current** | "The old machine, still running on stolen current. I don't care what it is. I care what it's plugged into." |
| **Relationship to Liv** | Detained her when she tried to talk her way into the powerhouse node. **Took the Pointe Sombre key in exchange for her passage.** He is complicit, and knows it. |
| **What he wants from the player** | Proof of what the Compact's relights are doing to the Line. |
| **What changes the relationship** | Pointe Sombre (destroyed node: trust; relit on the Line: suspicion). How the player passes the lock (speaking protocol at his gates is the worst thing the player can do in his eyes). Showing him the Compact's Loss Book. |
| **Where he can oppose the player** | Closes the lock. Refuses the black start (the Line cannot restart without his dam). At worst, lets the Cutters go. |
| **Ending states** | Holds the Local's key under the Commons; opens the dam for a black start he voted for; loses the Floor to the Cutters (Severance) or the Stewards (the Local seizes the Crib); retires. |

---

## 7. Odile Marchetti — the Packet (independent)

| | |
| --- | --- |
| **Public role** | Captain of the packet boat *Ondine*. Carries mail between every harbor, faction or not, under the Post's oath: anything sealed, for anyone, unread. |
| **Private goal** | Deliver the **dead letters**. The Post inherited the regional post office's undelivered mail from the collapse week, sixty years of it. She still tries to find descendants. |
| **Fear** | That some faction will conscript the Post and the oath will end with her. |
| **Contradiction** | Swears she never reads the mail. Knows exactly what is in all of it. |
| **Personal stake** | The oath. A dead letter of her own family's she has never opened. |
| **Relationship to the Current** | "The lake delivers everything eventually." Not a believer, not a skeptic. |
| **Relationship to Liv** | Carried her letters and carried her through the Locks. Holds **one undelivered letter from Liv to the player**, returned because the player had left the *Ida*. She will only hand it over in person. |
| **What she wants from the player** | Nothing at first. Later: a favor for the Post, delivering one dead letter to a place only the player can reach. |
| **What changes the relationship** | Respecting the oath (not reading, not using the Post for a faction). |
| **Where she can oppose the player** | If the player tries to use the Post to carry a weapon or a faction's orders, she refuses and the Post closes to the player. |
| **Ending states** | The Post carries the keys between keyholders (Commons); carries the evidence to every harbor (a "publish" variant of several endings); is conscripted by whichever faction wins outright. |

---

## 8. Supporting cast

| Name | Role | Where | Note |
| --- | --- | --- | --- |
| **Captain Ines Varga** | Captain of the *Ida*; leader of Open Water in the Compact | Prologue framing; Port Carrow | Liv's old employer; brought the player; knows the player's tie to Liv; wants dues abolished. |
| **Callum Reyes** | Leader of the Answerers | Tall Masts; Kenning; the Crib | His brother **Tam** went with Liv and died in the intake tunnel. Shares Liv's beliefs and hates her. If he reaches the Crib, he tries to open it. |
| **Hollis Marr** | The Namekeeper (Mourners) | Tall Masts | Reads names of the drowned into the channel. Kind, and the Sounding's best fundraiser. |
| **Nadia Sobczak** | Leader of the Cutters | Aubin; Kenning | Her husband died when a Compact-relit light woke a section and a surge came down the river cable. |
| **Wes Tolliver** | Leader of the Stewards; Kowalczyk's deputy | Aubin | Would "hold the Line shut" from Aubin. Reasonable, patient, and exactly what the Local fears. |
| **Odette Beaudry** | Keeper of the Pointe Sombre light | Pointe Sombre | Let Liv into the vault. Her son **Remy** drowned on the reef the week the light went dark. She told the village it failed. |
| **Tem and Dell Pruitt** | Wrecker mother and son | Pointe Sombre | Profit from the dark reef; show a false light in storms. Poor, not monsters. |
| **Sigrun Dahl** | Local lineworker (a Cutter) | Pointe Sombre; Aubin | "Buying copper." Made sure the light stays off the Line. |
| **The scavenger** (existing, unnamed) | Keeps the relay powered | Prologue | PROPOSAL: a "battery man" who wires dead relays to sell storm warnings; found Liv's coil in the housing and kept it fed. If alive after Shore Watch, he either keeps the Shore Watch while Mara is away (if the player talks him down) or turns up at Pointe Sombre working for the Pruitts. |
| **Kenning spokesperson** | The squatters' voice | Kenning | The table demonstration happens to her people. |

## 9. Sigrun Dahl (supporting; no longer a companion candidate)

Mara is the companion (Decision 4: A). Sigrun stays a supporting character: a young Cutter lineworker, sent to Pointe Sombre to keep the node off the Line. She grew up on Table Day stories and has followed Liv's trail for the Cutters since the Locks. Her arc: from "burn it all" to understanding the islands' dependence on the lights, or deeper into the Cutters. If the player allies with the Cutters, she is the one carrying the charges at the Crib.

---

## 10. Voices from before (records only)

Heard in logs, memos, and archived transmissions. Each is a found-object voice, one short log per location, never a narrator.

- **Controller Maren Holt**: the Authority's section controller on the collapse night. Calm, overworked, human. Her code severed the lakes at 03:12, forty minutes after her launch was found capsized.
- **The Pattern Desk** (three memos in Kenning): **Dr. Nils Arvidson** ("feedback in our own system"), **Dr. Grace Oduya** ("correspondence, and we should answer carefully"), **Paul Dorsey** ("an intrusion; build a firebreak"). The present factions are replaying their argument.
- **Ellis Tennant and June Okafor**: the *Tern*'s crew. The *Tern*'s last log page (existing) is theirs.

---

## 11. Relationship map (who pulls on whom)

```
                 Dumont (Board) ── employs ──> Hale (Warden) ── hired, then hunts ──> LIV
                      │                           │                                   │
          Open Water: Varga ── brought ──> PLAYER <── hunts through ──┘               │
                                            │                                          │
         Mara ── promised Liv silence ──────┤                                          │
                                            │                                          │
 Tennant ── gave the Lexicon to ────────────┼──────────────────────────────────────────┤
 Reyes ── brother died following ───────────┼──────────────────────────────────────────┤
 Kowalczyk ── took the key from ────────────┼──────────────────────────────────────────┤
 Odette ── let into the vault ──────────────┼──────────────────────────────────────────┤
 Odile ── carried, holds a letter from ─────┴──────────────────────────────────────────┘
```

Every major character has a **debt or grievance with Liv**, so the missing-person thread is present in every faction conversation without anyone needing to "have seen her".

## 12. Disagreement scenes (interpretation, not exposition)

Moments where two or more characters look at the same thing and argue. Each works with whoever is present (companion, faction contacts), so the player's choices shape who argues.

1. **The lamp room at Pointe Sombre.** Hale: "A fault." Sigrun: "It's calling home." Odette: "It lied to my son." Mara: "Write it down." The player decides what to do while they are still arguing.
2. **The lock gate moves at a phrase.** Varga (on the convoy): "Then we don't need Aubin's permission." Kowalczyk: "Then anyone can open my river." A Reader aboard: "It answered before she finished the phrase."
3. **The seiche match at Tall Masts.** A Compact Surveyor: "Physics we can plan around." Tennant: "Everything alive has a rhythm." June Okafor says nothing.
4. **The pumping station in Kenning.** The squatters' spokesperson: "Your machine took our heat to light a hospital full of fish." Hale: "The tables can be rewritten." Sobczak: "By whom?"
5. **The Crib console.** Liv: "It's asking." The engineer present: "It's a retry loop with jitter." Nobody is lying.
