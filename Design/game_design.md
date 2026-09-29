# DEAD CURRENT - Game Design

Living design document. The source plans are `LongTermPlan.txt` (vision, pillars, phase ladder) and `FirstPhasePlan.txt` (first playable scope and order) at the repository root.

## Current milestone

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
- 2026-09-28: Exploration Loop accepted by Anthony after the second playtest pass (water persists, fish visible, novel item found). Next phase to plan: RPG Layer.
