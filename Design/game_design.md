# DEAD CURRENT - Game Design

Living design document. The source plans are `LongTermPlan.txt` (vision, pillars, phase ladder) and `FirstPhasePlan.txt` (first playable scope and order) at the repository root.

## Current milestone

Next: Presentation Pass (chosen 2026-09-29, not yet started). A bounded art, audio, and readability pass on the existing `Lvl_Boathouse` shore before Phase 5. No new gameplay, place, quest, or enemy. Its plan will be `PresentationPassPlan.txt`. Constraints and the asset library are in `Design/art_pipeline.md`.

Phase 4, RPG Layer (accepted 2026-09-29 after playtest):

Allocate a small build > The same shore offers a reading or a line that another build does not get > The build survives save/load

Three attributes (Grasp, Fieldcraft, Bearing), three skills (Engineering, Survival, Persuasion), three perks (Schematic Eye, Pulse Read, Relay Ear). Effective skill is invested ranks plus 1 when the linked attribute is 2 or higher. Failed checks are hidden. A visible check is labeled, for example `[Engineering 2]`. Nothing here replaces Shore Watch or the Survey Launch; a build only adds options. Press **B** to allocate. **F10** resets the prototype allocation. Tab shows the character block. See `RPGPhasePlan.txt`.

Phase 3, Exploration Loop (accepted 2026-09-28 after playtest and fixes):

Notice something odd > Leave the direct route > Discover a place > Read the clues > Infer what happened > Avoid or disable the danger > Take the reward > World remembers

Phase 2, Micro RPG (accepted; Shore Watch was played through during the Exploration Loop playtest):

NPC > Conversation > Quest > Combat or alternative > Return > Reward > World remembers

Phase 1, Walking Skeleton, is accepted:

Wake up > Explore > Pick up weapon > Encounter enemy > Fight or avoid > Loot > Meet NPC > Save > Quit > Reload > World persists

## Micro RPG quest: Shore Watch

Mara, who watches the shore near the boathouse, has noticed that the scavenger working the path has wired a dead Maritime Authority relay at his camp, and it has started talking. She wants it quiet.

- **Combat route:** kill the scavenger. Mara pays 24 rounds of 9mm. The path is clear; the relay goes cold.
- **Coil route:** sneak into the camp and pull the coil from the relay without a fight. Mara takes the coil, pays two field dressings, and sits up listening to it. The scavenger is still alive and still hostile.
- The route is decided by what the player does in the world, not by the reply they gave Mara.
- Inspecting the live relay gives a clue (it is almost saying words) that opens an extra exchange with Mara.
- Afterwards Mara's greeting, the lookout crate by her, and the relay rig all reflect the outcome.

The purpose is to prove the systems (shared conditions/consequences, data-driven quests, dialogue integration, world state, persistence), not to deliver content volume.

## Point of interest: the Wrecked Survey Launch (PROVISIONAL)

An optional site west of the boathouse, off every route and with no marker. The player is drawn to it by a flickering amber lamp on a leaning mast, seen through a new west window in the boathouse and above the roof from the path. Walking there triggers a one-time "LOCATION DISCOVERED" banner, and the place is listed under PLACES on Tab.

- **Clues, no exposition:** eight inspectables tell the story: she was run aground on purpose, the crew cut power and walked inland, the depth sounder shows a regular "pattern", the log mentions the same thing on a radio channel with no station, and the kit was left in the tender tied off the stern.
- **Danger:** the water round the stern is live (20 damage/s, about 5 s to die at full health). A glowing blue layer, sparks, a pale ring of dead fish and a chalked warning on the beach mark it. The player can avoid it, or find the battery bank, learn from the log that the crew cut every breaker, and pull the leads. That switches the electricity off for good; the water stays.
- **Reward:** the obvious survey locker (12× 9mm, 3× wiring, 1× dressing) and the hidden kit in the tender out in the live water (a novel item, the Sounder Chart, plus 2× dressings and 18× 9mm).
- **Reactivity:** the window, breaker panel, beacon and fish change with what the player has done. After reading the log, Mara has one optional exchange about it (she deflects: "everybody decides it was a storm"). The beacon hums like the scavenger's relay if the player inspected it.
- Nothing explains the Current. The site raises a question and answers none.

## Design pillars

- Exploration
- RPG agency
- Consequence
- Atmosphere
- Systemic consistency

## Decisions log

Record design decisions here as they are made, with the date and the reason.

- 2026-09-28: Shore Watch keeps its id and premise from the earlier prototype, reframed around a transmitting relay so the first quest touches the Current mystery. Rewards differ by route (ammo for violence, medicine for the quiet route) and the quiet route leaves a living threat on the shore, so neither choice is free.
- 2026-09-28: The player's reply when accepting ("I'll put him down" / "I'll pull the coil") does not lock the route; actions in the world decide it. This keeps agency in play rather than in menus.
- 2026-09-28: Mara never refuses to acknowledge a route taken before the quest was offered (scavenger already dead, coil already taken). Order of discovery should not punish curiosity.
- 2026-09-28: Phase 3 is one small, hand-built point of interest rather than a system for many. The location, container, hazard-condition and flicker-light classes are generic so the next site is data, but only the Survey Launch exists. Discovery is polled, not collision-based, because the firearm traces WorldStatic/Dynamic and a trigger box would stop bullets.
- 2026-09-28: The wreck's danger has a non-violent answer that the world teaches (the log says the crew cut every breaker; the battery bank is still live). Avoiding the water, and pulling the leads, both work. The hidden loot sits in the danger, so the disabled route is the safer one.
- 2026-09-28: The wreck is optional and does not touch Shore Watch. Its only link to Shore Watch is flavour: the beacon hums like the scavenger's relay if the player has inspected it, and Mara has one deflecting line.
- 2026-09-28: All wreck lore (the *Tern*, the crew, the "pattern", the live battery) is PROVISIONAL and unexplained on purpose; see the world bible. New deferred decisions: the boat's name, the loot balance at the wreck, and whether Mara's "storm" line should vary with the Shore Watch outcome.
- 2026-09-28: First Exploration Loop playtest (Anthony). Kept: the pinging lamp works as a landmark, the reward is fine but wants something new, the *Tern* name and Mara's "storm" line are fine for now. Changed: (1) pulling the leads made the water disappear, because the live water's mesh was the water; the water is now a permanent surface and only the electric glow and sparks switch off. (2) The live water was a surprise; the dead fish had been placed below ground, so the ring was never visible. Fish are now pale and on the surface, the glow and sparks are stronger, and there is a chalk warning on the beach. (3) The tender now holds a novel item, the Sounder Chart. Deferred: audio and richer visuals (the clues are hard to piece together with greybox art and no sound); the plan scopes both out.
- 2026-09-28: Exploration Loop accepted by Anthony after the second playtest pass (water persists, fish visible, novel item found).
- 2026-09-28: Phase 4 attributes are Grasp, Fieldcraft and Bearing, each linked to one skill (Engineering, Survival, Persuasion). Not a SPECIAL clone. Effective skill = ranks + 1 if the linked attribute is at least 2, so both a skill point and a committed attribute can open a check. Pools are 3 / 2 / 1. Failed checks are hidden, matching dialogue. New wreck and Mara lines from those checks are PROVISIONAL and do not explain the Current.
- 2026-09-29: RPG Layer accepted by Anthony after playing the Engineering, Survival, and Persuasion builds on the existing shore. Playtest fixes kept with that pass: engine view modes no longer take F1–F5 or F9, and prompts plus inspect text sit on a dark plate.
- 2026-09-29: A Presentation Pass comes before Phase 5 (World State). Every playtest so far has said the clues are hard to read in greybox with no sound, and Phase 5 is about choices *visibly* changing places. The pass is bounded to the existing shore: settle how hand-placed art coexists with the script-built map, replace greybox clue props and NPC bodies, add first audio, and keep every test green. Art may come from `C:\FO5_AssetLibrary` and from Meshy Pro (see `Design/art_pipeline.md`).
- 2026-09-29: Presentation Pass content is in and awaiting Anthony's acceptance (no gameplay changed). Decisions made inside it, all reversible: (1) the sound follows the rules the world already has, through one cosmetic actor (`ADCConditionalAudio`), so the relay hums only while the coil, the relay, and the scavenger are all as they were, and the live water hums only while the power is on; audio never sets a flag. (2) The breaker throw is a stand-in sound (a big breaker "off", not a clamp coming off a terminal) because no CC0 clamp recording exists. (3) The wind bed keeps its very faint birds by Anthony's earlier choice, even though the boathouse window line says "No birds". (4) Mara and the scavenger wear the same pack body in different jackets (teal, rust-brown); they share one head. That is a costume, not a decision about Mara's face, history, or gender. (5) The *Tern* reads as faded white paint over grime, not teal, because the source photo hull is warm and pale; the tint is one instance parameter. (6) Pickups make a click and the inventory a switch flick, chosen by file size, to be judged by ear.
- 2026-09-29: Presentation Pass playtest 1 (Anthony): the boathouse reads well; the cot was a white block; the pistol and dry-fire are fine; only the water lap was audible; Mara and the scavenger looked like twins; the live water looked like a swimming pool; the RPG builds and the shore looked right; the player could fall off the map. Changes: a real camp cot; louder hums and wind; a new head for Mara (PROVISIONAL: a face for a provisional character, not a decision about her history); the live water is now dark water with thin electric filaments instead of a flat cyan sheet; an invisible perimeter around the east half of the map. The pass is not accepted yet. Mara has no voice lines, so there was nothing to hear when she spoke; dialogue is text.
- 2026-09-29: Presentation Pass playtest 2 (Anthony): Mara much better; everything else passes except two things that floated. Life jackets now lie on the stones. The TERN name board now stands on two stakes, because the generated hull does not reach the blockout bow it was placed on. PROVISIONAL, and cheap to change: whether the name board is a sign someone stood up on the beach or belongs on the hull.
