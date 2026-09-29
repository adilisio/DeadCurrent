# Development Session Report

## Executive Summary

Phase 4, the RPG Layer, is accepted. Anthony played the Engineering, Survival, and Persuasion builds on 2026-09-29 and approved them. A small character build changes what the same shore will say.

Three provisional attributes (Grasp, Fieldcraft, Bearing) feed three skills (Engineering, Survival, Persuasion). One perk point chooses among Schematic Eye, Pulse Read, and Relay Ear. Those checks are ordinary shared conditions on actors and dialogue that already existed: the wreck, Mara, the relay, and the Sounder Chart. A character with nothing spent still gets the accepted Shore Watch and Survey Launch solutions. Investment adds a reading or a line.

Before that work, a save written before a persistent pickup existed was destroying that pickup on load. Taken pickups are now recorded explicitly. Phase 3 is recorded as accepted, including the human playtest.

36 automated tests pass. `DeadCurrentEditor` builds. A Development Win64 cook/package succeeded, and a null-RHI smoke launch loaded `Lvl_Boathouse`. Phase 5 was not started.

## Baseline Verified

- Starting SHA: `54686bf` — Mark the Exploration Loop accepted. `origin/main` was at that commit. Local HEAD is authoritative and has moved forward on `main` only.
- Original test count: 30, all passing, on the existing editor binary before any Phase 4 edit.
- That baseline was not broken. The pickup fix and the RPG layer were each built and retested on top of it.
- Untracked and left untracked: `Content/Variant_Shooter/`, `Tools/EditorScripts/inspect_assets.py`.

## Pre-Phase-4 Fixes

Commit `f48f097`.

A persistent `ADCItemPickup` added to the map after a save was written was absent from that save. The loader treated every live persistent id missing from `WorldActors` as removed, and destroyed the new pickup.

Save version 4 now writes `RemovedPersistentIds`. An id is noted only when the actor ends play because it was destroyed (taken, or otherwise removed), not because the map unloaded. On load, only those ids are destroyed. An id that simply was not in the save keeps its authored state, which is how a pickup added later survives an older save.

Saves from before version 4 still treat the historical boathouse pickups as taken when they are absent: `boat.pickup_pistol`, `boat.pickup_ammo`, `boat.pickup_dressing`, and `boat.pickup_coil` when the save is version 2 or newer (the coil postdates first-playable saves). Any other absent id stays. There is no general migration table.

`DeadCurrent.Save.RemovedPickup` covers taken-stays-gone, kept-stays, and added-later-stays, including a version 0 save and a version 3 save. The real-map legacy load expects the coil pickup to remain. The existing coil-route map test still takes the coil and gets it back from F9.

Documentation that still said Phase 3 was awaiting a human playtest was corrected. Phase 3 is accepted. Human playtesting did occur.

## RPGPhasePlan

`RPGPhasePlan.txt`, commit `53eb765`, then executed.

Mission: allocate a small build, walk into content that already exists, and get an option someone else does not get. The build is still there after save and load.

The plan locks the scope to three attributes, three skills, three perks, the existing shared rule language, the canvas HUD, and the actors already on `Lvl_Boathouse`. It excludes XP, a character creator, a new place, a new quest, and Phase 5.

## Architecture Added

Commit `3a74f4d`, plus `56d0a5a` so the build panel's keys do not also save or load.

**Attributes.** Grasp, Fieldcraft, Bearing. Raw values, 0–2. Three points on a new game.

**Skills.** Engineering (from Grasp), Survival (from Fieldcraft), Persuasion (from Bearing). Invested ranks, 0–2. Two points on a new game. Effective skill is the invested rank plus 1 when the linked attribute is 2 or higher. `SkillAtLeast` uses the effective skill. `AttributeAtLeast` uses the raw attribute.

**Perks.** Schematic Eye, Pulse Read, Relay Ear. Owned or not. One point. Each one is consumed by an inspect variant.

**Where it lives.** `UDCCharacterProgressionComponent` on the player, default subobject name `Progression`. Lookup is by Gameplay Tag (`Attribute.Grasp`, `Skill.Engineering`, `Perk.SchematicEye`). Unknown tags read as 0 or not owned. The character class does not grow a field per skill. There is no XP, level, or respec. F10 refunds the allocation only so a playtest can try each orientation.

**Rules.** `AttributeAtLeast`, `SkillAtLeast`, and `HasPerk` were appended to the existing condition enum. `FDCRuleContext` carries the progression component. Dialogue, quests, and inspectables use the same evaluation. Validation rejects an attribute, skill, or perk id that is not in the catalog. Failed checks stay hidden. A visible choice or the active inspect variant shows a generated label such as `[Engineering 2]` or `[Schematic Eye]`. Authors do not type that label into the line.

**Save.** Version 5 stores attribute ranks, skill ranks, and owned perks. Empty arrays are the unspent default, which is what a save from before this phase receives. Unspent points are the pool minus what is invested. They are not a separate saved field.

## Player-Facing RPG Proof

Press **B** to open the build panel. **F1–F3** raise Grasp, Fieldcraft, Bearing. **F4–F6** raise Engineering, Survival, Persuasion. **F7–F9** take Schematic Eye, Pulse Read, Relay Ear. **F10** resets. **Tab** lists the build, including effective skill when it differs from the invested rank.

- **Engineering 2** on the breaker panel: the cuts start at the shore-power breaker, and the mast lamp is not on those lugs. Pulling the battery leads still works with no skill.
- **Survival 2** on the life jackets: they lie toward the treeline. The crew left inland together.
- **Fieldcraft 2** on the chalk warning: it was written from the shallows, looking back at the boat.
- **Persuasion 2** (effective) with Mara, after the survey log and her storm deflection: one press. She admits two people came up off that beach and that she did not follow. She does not explain the Current. The line is not offered again.
- **Schematic Eye**, carrying the Sounder Chart (`survey_chart`): the depth sounder includes the margin note, last tick marked not a shoal, and the same spacing as the Engineering reading.
- **Engineering 2** with the chart and without the perk: the sounder and the chart use the same spacing. The chart alone does not change the sounder.
- **Pulse Read** on the dead fish: one shock, not a tide.
- **Relay Ear**, only after the relay has already been inspected: the housing was seated by someone who knew the pinout. The first Shore Watch clue still happens for everyone.

## Automated Tests

`Tools\RunTests.bat -build` after the last code change. Exit code 0.

Editor context, 30, all Success:

FirearmAmmo, Health, MaraWreckLine, Shore Watch CoilRoute / CombatRoute / Shortcuts, Validate, Dialogue Branching / Conditions / Consequences, Exploration Container / Discovery / WorldConditions, Inventory Stacking, Progression ContentChecks / Lookup / Rules / Save, Quest Branches / CrossQuest / Persistence / Stages, Rules Conditions / Consequences / Validation, Save InventoryRestore / PersistentId / RemovedPickup / WorldInventorySlot, InspectVariants.

Map context on `Lvl_Boathouse`, 6, all Success:

BuildChecks, CoilRoute, CombatRoute, LegacySave, SurveyLaunch, SurveyLaunchSaves.

**Total: 36 tests, all passing.**

`DeadCurrent.Map.Boathouse.BuildChecks` is the real-map proof: the same placed actors change with Engineering, Survival, Fieldcraft, Persuasion, the three perks, and the Sounder Chart, then a real F9 restores the saved build.

## Build Status

`DeadCurrentEditor` Win64 Development: **Succeeded.**

The compiler in use is MSVC 14.51, which is newer than Unreal's preferred 14.50. That warning is pre-existing and did not fail the build. DLL-load warnings for `aqProf`, Vtune, and WinPix in the test logs are the same.

## Package / Cook Status

Development Win64 `BuildCookRun` (`-build -cook -stage -pak -archive`, archive `Saved\Packaged`): **BUILD SUCCESSFUL.** AutomationTool ExitCode 0. BuildCookRun time 123.64 s. Log: `Saved\Logs\Package.log`.

The IoStore container includes `Lvl_Boathouse.umap`, `DA_Dialogue_MaraIntro.uasset`, `DA_Item_SurveyChart.uasset`, and `IA_Build.uasset`.

Smoke launch of `Saved\Packaged\Windows\DeadCurrent.exe -nullrhi`: `[DCCONTENT] loaded 7 item and quest definitions`, then `LoadMap(/Game/Maps/Lvl_Boathouse)` completed. No fatal error. The process was stopped after the map came up. Rendering, input, and F5/F9 in the packaged window were not exercised.

## READY FOR ANTHONY TO TEST

Launch with `Tools\PlayTest.bat`, or the editor and Play. Map: `/Game/Maps/Lvl_Boathouse`. For a clean build, delete `Saved\SaveGames\DeadCurrent.sav` first. An older save loads as an unspent build. Press **B** and spend it. Taken bench and coil pickups from a save of the era that included them stay gone. A pickup added to the map after that save stays.

**Controls:** WASD move, mouse look, Shift sprint, Ctrl or C crouch, **E** interact, **1–9** dialogue, LMB fire, R reload, 1 holster, **Tab** inventory, journal, and character, **B** build panel, **F5** save, **F9** load.

While the build panel is open, F5 and F9 spend Survival and Relay Ear. They do not save or load until you close the panel with **B**. The panel says so.

### Engineering-oriented build

1. Press **B**. **F1** twice (Grasp 2). **F4** twice (Engineering 2). Optional: **F7** (Schematic Eye).
2. **Tab**. The character block shows Grasp 2, Engineering 2, and effective Engineering 3 if Grasp is 2.
3. Go to the Wrecked Survey Launch, west of the boathouse. Inspect the breaker panel in the wheelhouse. The prompt is labeled `[Engineering 2]`. The text names the shore-power breaker and the mast lamp. Pulling the battery leads still works with no extra skill, and still cuts the live water.
4. The tender is tied off the stern, in the live water. Pull the leads first, or cross if you accept the shock. Loot the Sounder Chart.
5. Inspect the depth sounder. With Engineering 2 it mentions the same spacing. With Schematic Eye it also includes "NOT A SHOAL".

### Survival-oriented build

1. **B**, then **F10** to reset. **F2** twice (Fieldcraft 2). **F5** twice (Survival 2). **F8** (Pulse Read). Close with **B**.
2. Life jackets: the reading mentions the treeline. Prompt `[Survival 2]`.
3. Chalk warning: written from the shallows. Prompt `[Fieldcraft 2]`.
4. Dead fish: one shock. Prompt `[Pulse Read]`.
5. The breaker panel does not show the mast-lamp reading.

### Persuasion-oriented build

1. **B**, **F10**. **F3** twice (Bearing 2). **F6** once (Persuasion 1; Bearing 2 makes it effective 2). Close the panel.
2. Read the survey log on the wreck first.
3. Talk to Mara. Take the wreck line, then "Was it a storm?", then `[Persuasion 2] You're leaving something out.`
4. She says she saw two people come up off the beach and that she didn't follow. That choice does not come back.

### Perk proof

Schematic Eye and Pulse Read are in the builds above.

Relay Ear: inspect the scavenger's relay on the shore path once. The ordinary words clue still happens. Open **B** and press **F9** while the panel is open (this does not load). Inspect the relay again. The housing mentions the pinout.

### Sounder Chart

The chart is in the tender. Carrying it does nothing by itself. Engineering 2 or Schematic Eye, at the depth sounder, is what reads it. Neither reading explains the Current.

### Save and load

1. Close the build panel. Press **F5**.
2. Quit completely. Relaunch. Press **F9**.
3. **Tab**. Attributes, skills, effective ranks, and the perk match what you saved.
4. Optional: after saving, press **B** and **F10**, then **F9** without closing over a new save. The saved build comes back.

### Shore Watch regression

With a fresh or unspent character, both routes still work. The pistol, ammo, and dressing on the bench are still there. The relay can still be powered and brought to Mara, and the camp can still be fought. Mara's original storm deflection is still there when Persuasion is too low to press her.

### Survey Launch regression

Walk-in discovery, the name board, the log, the live water, pulling the leads, the locker, the tender, and the beacon's reaction to the relay all still work with nothing invested. The new readings are extra variants. They do not replace those actions.

## Known Issues / Technical Debt

- The world is still a greybox. There is no audio pass. The water is a walkable slab. Firearm traces versus query volumes are unchanged.
- The HUD is still the temporary canvas. The build panel is a prototype, not a character creator. F10 is not a product respec.
- The Win64 package cooked before the playtest still has the old F-key view modes and the unbacked prompts. The editor and `Tools\PlayTest.bat` have the fixes. Recook before using that package.
- One save slot.
- The historical pickup list is only the four `boat.pickup_*` ids. Gym pickups are not in it.
- Unspent points are derived from the current pool constants. Changing a pool later is a design change, not a save migration.
- New wreck and Mara lines from these checks are PROVISIONAL.
- Packaged rendering and a rendered F5/F9 were not smoked. Null-RHI confirmed boot, content, and the map.
- Navigation data for `Lvl_Boathouse` is still rebuilt at runtime. That warning is pre-existing.

## Design Decisions Made

- Attributes are not SPECIAL. Each one is the capacity behind one skill, so a later skill is another row.
- Effective skill is invested ranks plus 1 at attribute 2. A spread of 1s gives no bonus. Specializing is the point.
- Failed checks are hidden, the same policy dialogue already used. No greyed-out row in this phase.
- Check labels are generated from the condition.
- Perks are qualitative readings, not percentages. All three have a consumer.
- Skill investment adds options. The battery leads, both containers, and both Shore Watch routes stay available with nothing spent.
- The Sounder Chart is a reading, not a quest, and it does not explain the Current.
- Mara's press is one limited admission. Her faction and the Current stay undecided.
- Removed pickups are an explicit set. Absence from an old save is not removal.
- While the build panel is open, F5 and F9 belong to the panel.

## Deferred Product Decisions

- Final attribute and skill names, pool sizes, and whether the attribute bonus stays at +1.
- XP, levels, caps, and a real respec.
- Whether failed checks should become visible-but-unavailable later.
- More skills, and whether one attribute should feed more than one skill.
- Mara's faction, the crew's fate, and what the sounder pattern is.
- Phase 5: choices visibly altering locations and NPC behavior beyond what Shore Watch and the wreck already do.

## Recommended Next Step

Phase 5, World State: choices visibly alter locations and NPC behavior. Do not start it until that phase is opened on purpose.
