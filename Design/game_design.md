# DEAD CURRENT - Game Design

Living design document. The source plans are `LongTermPlan.txt` (vision, pillars, phase ladder) and `FirstPhasePlan.txt` (first playable scope and order) at the repository root.

## Current milestone

Phase 2, Micro RPG (implemented, awaiting playtest):

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
