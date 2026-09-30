# Visual Critic Findings — shore.landing_stage

## Defect Findings

* **V-01** | `blocker` | `landing_adrift`, `landing_close_combat` | The skiff intended to be far out on the water is instead floating high in the air above the water. | This completely breaks immersion and reads as a blatant bug rather than a believable consequence. | Anchor the skiff actor to the water surface elevation in its adrift location.
* **V-02** | `blocker` | `landing_far`, `landing_coil`, `landing_combat`, `landing_power_cut` | The large pieces of driftwood on the beach are floating several feet in the air and clipping the beached boat. | Destroys the grounded visual identity and looks like unpolished, randomly arranged props. | Drop the driftwood assets to the landscape/ground collision.
* **V-03** | `blocker` | `landing_close`, `landing_adrift`, all close views | The wooden posts supporting and surrounding the dock are floating above the water surface with visible gaps beneath them. | Destroys the illusion of a physical dock built into the water. | Extend the dock post meshes downward so they intersect the water plane.
* **V-04** | `should fix` | `landing_power_cut` vs `landing_far` | The "string of bulbs" mentioned in the expectation is missing entirely from the default `landing_far` state, making `landing_power_cut` visually identical to the default state. | Fails the requirement that the power cut state is meaningfully different and readable at a glance. | Add the lit string of bulbs to the default state so they can be visibly turned off.
* **V-05** | `should fix` | `landing_close` | The crate lid is missing entirely rather than "leaning against it", and the contents are untextured greybox rectangular prisms instead of reading as tins and a blanket. | Fails the expectation of seeing recognizable supplies being packed. | Replace the generic block contents with actual supply props and place the lid leaning against the side of the crate.
* **V-06** | `should fix` | `landing_close_coil` | Mara's pack is represented by two untextured geometric blocks, and the crate is missing the "rope lashing" described in the expectation. | Reads as an unfinished greybox implementation rather than a completed, authored stage. | Replace the pack blocks with a proper backpack asset and add a rope detail to the closed crate.
* **V-07** | `should fix` | `lookout`, `lookout_coil` | The lookout structure Mara stands in is an untextured, three-walled concrete-like greybox. | Completely breaks the visual language of the shore; looks like a level design blockout rather than a scavenged lean-to. | Replace the greybox walls with proper corrugated metal, wood, or scavenged scrap assets fitting the environment.
* **V-08** | `nit` | `landing_close_combat` | The crate lid placed on the boards slightly clips into the wooden floor of the dock. | Lessens the polished feel of the stage close-up. | Raise the lid mesh slightly so it rests cleanly on the boards without clipping.

## Passes
* Mara's visual identity: At conversation range (`mara_face`), she has a distinct human face, avoiding the "twin" problem with the scavenger. The scavenger (`scavenger_close`) is also distinct in a brown jacket.
* State logic: The correct objects swap or disappear in the intended states (e.g., the crate opens/closes, the skiff moves to the background, Mara appears/disappears).
* `lookout_coil`: Correctly removes Mara and her pack while leaving the environment unchanged, meeting the expectation perfectly.

## Answers to Questions
1. **For each landing view: does it meet its written expectation?**
   - `landing_far`: Defect (No string of bulbs visible; driftwood floats).
   - `landing_coil`: Defect (Driftwood floats; pack is greybox).
   - `landing_combat`: Defect (Driftwood floats; skiff is hovering in the sky in the background).
   - `landing_power_cut`: Defect (Identical to `landing_far` since bulbs are missing; driftwood floats).
   - `landing_close`: Defect (Lid is missing, not leaning; contents are greyboxes; dock posts float).
   - `landing_close_coil`: Defect (No rope lashing; pack is greyboxes; dock posts float).
   - `landing_close_combat`: Defect (Skiff floats in the sky in background; dock posts float).
   - `landing_adrift`: Defect (Skiff and dock posts float in the air).
   - `lookout_coil`: Pass (Empty as expected, though shed is greybox).

2. **In the four-state group, can you tell the states apart in one glance, without the setup label? Which pair is weakest, and what single visual change would separate them most?**
   It is difficult to tell them apart quickly. The weakest pair is `landing_far` and `landing_power_cut`, which are visually identical because the string of bulbs is missing from the baseline. Adding a prominent, brightly lit string of bulbs to `landing_far` that visibly goes dark in `landing_power_cut` is the single change needed to separate them most.

3. **Does anything read as greybox, floating, clipped into something, or at the wrong scale?**
   Yes. **Floating:** Driftwood on the beach, dock posts, the far-out skiff in the combat/adrift states. **Greybox:** Mara's pack, the crate contents, the three-walled shed at the lookout. **Clipped:** The crate lid clips into the dock boards.

4. **Does the stage read as a place people made and use, or as props set down on the shore?**
   It reads as props set down. The floating dock posts and hovering driftwood make the environment feel unauthored and disconnected from the terrain. The generic greybox supplies and lack of lashing further reduce believability.

5. **Does the combat state read as a consequence (someone closed this) rather than a different texture?**
   It leans toward feeling like objects despawned rather than a deliberate shutdown. The missing skiff moving to an impossible hovering location breaks the consequence entirely, and the missing lid in the default state makes the "open empty crate" less impactful.

6. **Is anything too bright, too loud in colour, or off the shore's palette?**
   No, the colors are appropriately muted, cold, and fit the blue-grey environment, perhaps too much so (the lack of the string of bulbs means the states blend together without strong lighting contrast).

7. **Anything in the frame that is not part of the stage but now looks wrong because of it?**
   The large driftwood branches on the beach have been placed incorrectly and are now floating in mid-air, drawing the eye immediately in all wide shots and making the beach look buggy.
