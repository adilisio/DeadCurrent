# Hand-off: Phase 3 (Exploration Loop), in progress

Written 2026-09-28 by Claude Opus 5.5 at the user's request, mid-session, for the next agent (Sonnet) to finish Phase 3. Read `ExplorationLoopPlan.txt` first; it is the scope. **Do not start the RPG Layer.**

## State right now

- Branch `main`, clean tree. The only untracked items are pre-existing and not ours: `Content/Variant_Shooter/` and `Tools/EditorScripts/inspect_assets.py`. Leave them alone.
- Nothing is pushed. Local commits only.
- `DeadCurrentEditor Win64 Development` builds.
- **30/30 tests pass**: 25 editor-context + 5 in-map (`Tools\RunTests.bat`). The baseline was 24 (21 + 3) at `1c7a0ee`.

| Commit | Subject |
| --- | --- |
| b856648 | EL-00: Add the Exploration Loop plan. |
| f871bf1 | EL-01/EL-02: Reusable location discovery and loot containers. |
| 5954355 | EL-03/EL-05: Inspectable verbs, world-conditioned hazards, flicker light. |
| ea05ad9 | EL-04/EL-06/EL-08: Wrecked Survey Launch POI, Mara's line, map tests. |

The implementation of EL-01..EL-08 is **done and tested**. What remains is verification, docs, packaging, and the report.

## Remaining work, in order

1. **Visual check (recommended; never done).**
   - The new materials have not been seen rendered: `/Game/Environment/Materials/MI_DC_*`, children of `M_FlatCol` and the translucent unlit `M_SimpleGlow`.
   - Nor have the landmark sightlines: the mast lamp over the boathouse roof from the path, and through the new west window.
   - Suggested method: a rendered run (drop `-nullrhi`) with `FScreenshotRequest`, as `DeadCurrent.Map.Boathouse.CoilRoute` already does. Or launch `Tools\PlayTest.bat` and look.
   - Tune colors and intensities in `build_boathouse.py` (`build_survey_launch`), rerun `Tools\RebuildContent.bat build_boathouse`, then rerun the tests. Greybox is acceptable; only fix things that are clearly unreadable.
2. **Docs.** Update:
   - `Design/technical_architecture.md`: new systems (see below); tests table (the `Exploration.*` group, `Content.Exploration.MaraWreckLine`, the new `Map.Boathouse.*`); boathouse IDs (`boat.wreck_locker`, `boat.wreck_tender`); save format 3; the POI section.
   - `Design/game_design.md`: current milestone is Phase 3 implemented and awaiting playtest; the POI; a decisions-log entry.
   - `Design/world_bible.md`: add the new lore, **all marked PROVISIONAL** (see Lore below). Do not turn anything into canon.
3. **Packaging smoke test** (plan §6). Try a Development Win64 cook/package, e.g. `RunUAT BuildCookRun -project=... -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive`. Classify failures as project defect or toolchain. Don't burn the session on toolchain problems. Items, quests and dialogue are Asset Manager primary assets (`Config/DefaultGame.ini`). The new material instances are hard-referenced by the map, so they should cook.
4. **Replace `Design/CLAUDE_SESSION_REPORT.md`** with this session's report, in the structure the user's original prompt required, including **READY FOR ANTHONY TO TEST**. Use the facts below. Then delete this hand-off file (or leave it; git keeps it) and commit.

## What was built (for docs and report)

**Location discovery (`World/DCLocationVolume`, `World/DCWorldStateSubsystem`).**
- The world state holds `DiscoveredLocations` beside the flags:
  - `DiscoverLocation(Id)` is true only the first time and fires `OnLocationDiscovered` + `OnChanged`.
  - `IsLocationDiscovered`, and `ReplaceDiscoveredLocations` (silent).
- New condition `LocationDiscovered(Id)`.
- The volume is a box with `LocationId` + `DisplayName`.
  - It **polls player positions at 4 Hz instead of using collision**. Two reasons: the firearm uses `LineTraceSingleByObjectType` (WorldStatic/WorldDynamic/Pawn), so a trigger box would stop bullets; and overlap events fire during the load teleport.
  - It stops ticking once its location is discovered.
- The HUD shows a `LOCATION DISCOVERED / <name>` banner, and the Tab journal has a PLACES list.

**Save format 3.**
- `UDCSaveGame::DiscoveredLocations`.
- `UDCSaveSubsystem::ApplyWorldState` (flags + locations) now runs **first** in `ApplyPendingLoad`, before world actors and the player, so nothing re-announces on load.
- Older saves load with no discoveries.

**Loot container (`World/DCLootContainer`).**
- One generic class: a mesh, a `UDCInventoryComponent` (authored `Stacks` = starting contents), and a persistent id.
- E takes the next stack. The prompt reads `Take 9mm Rounds (12) from Survey locker`; the message lists what's still inside. When empty it shows `Search X (empty)`.
- Persistence reuses the existing flat world-inventory arrays, with no container code in the save.
- A container missing from a save (added later) keeps its contents.

**Inspectable verbs.** `ADCInspectableActor::Action` (default "Inspect") and `FDCInspectVariant::Action` (per-variant override, used for "Read" and "Pull the leads"). Also `GetDisplayName()`.

**Hazard conditions.** `ADCDamageVolume::ActiveConditions` are world conditions, evaluated against the volume itself, so item/quest conditions never pass. While inactive: no damage, mesh hidden. It re-checks every tick because save restores replace flags silently.

**Flicker light (`World/DCFlickerLight`).** A cosmetic point light plus a glow cube with a scaled `Color` parameter, random flicker and dropouts, and optional `ActiveConditions`.

**Unchanged:** `ADCPlayerCharacter` (not touched), and all Shore Watch writing, rewards, routes and outcomes.

## The POI (player perspective; coordinates in cm; +X is out the boathouse door, the lake is -Y)

- **Notice.**
  - A new **west window** in the boathouse back wall (behind the spawn, south of the faded notice). Inspecting it ("Look out") mentions a mast and a light.
  - Outside, a few metres down the path, look back west: a leaning mast (~12.6 m) with a flickering **amber lamp** shows above the boathouse roof.
- **Route.** Out the door, turn back around the south (lake) side of the boathouse, and head west about 15 m along the shore. Discovery triggers around X -700, roughly 7 m past the back wall. The discovery box spans X -2600..-700, Y -1850..450.
- **The wreck.**
  - The bow sits on the beach near (-1500, -300); the stern is in the water near (-1500, -1260).
  - Board by the **plank** from the beach on the east side of the hull near the bow (foot at about (-1000, -510)).
  - The wheelhouse fore door is on the east side.
- **Clues** (inspectable display names):
  - Name board (bow front): she was driven ashore on purpose, name "T_RN".
  - Life jackets (beach, (-1080, -380)): straps cut; they walked inland.
  - Depth sounder (wheelhouse console): the regular spikes and "AGAIN".
  - Survey log (console, verb **Read**, sets `wreck.log_read`): pattern, "channel with no station", cut every breaker, still felt it, "Kit's in the tender, tied off the stern."
  - Breaker panel (wheelhouse west wall): text changes after the log is read.
  - Battery bank (aft deck, west side):
    - first Inspect sets `wreck.battery_seen`
    - then the verb becomes **Pull the leads** → `wreck.power_cut`
    - afterwards "leads hang loose"
  - Emergency beacon (on the transom, east side):
    - default: dead for decades
    - if `shore.relay_inspected`: hums like the scavenger's relay
    - after the power is cut: "the lamp is still flickering"
  - Dead fish (edge of the live water, near (-950, -1000)): changes after the power is cut.
  - West window: changes after discovery, and again after the power is cut.
- **Hazard.**
  - Live water: a glow slab over X -2020..-980, Y -1780..-1000.
  - 20 dmg/s, with a one-time message "The water is live."
  - A ring of dead fish at its edge, and two blue spark lights.
  - All off after Pull the leads, and that persists.
  - About 5 s of exposure kills at full health.
- **Loot.**
  - **Survey locker** (`boat.wreck_locker`, in the wheelhouse, west/aft corner): 9mm ×12, Salvaged Wiring ×3, Field Dressing ×1.
  - **Tender** (`boat.wreck_tender`): about 4 m off the stern at (-1500, -1650), inside the live water. Field Dressing ×2, 9mm ×18. A line runs from the transom to it.
- **Mara.**
  - After `wreck.log_read`, her greeting/who/place/in-progress/epilogue nodes (never turn-ins) offer "There's a wrecked survey launch west of the boathouse. I read her log."
  - She answers: "The Tern… since the lights went out… everybody decides it was a storm."
  - The player asks "Was it a storm?" (sets `wreck.mara_told`); she replies "It's always a storm. Stay out of the water round her stern. It bites."
  - The exchange is offered once.
- **World-state ids:** location `shore.survey_launch`; flags `wreck.log_read`, `wreck.battery_seen`, `wreck.power_cut`, `wreck.mara_told`.

## Lore to mark PROVISIONAL in world_bible.md

- Maritime Authority **survey launches**. This one is the *Tern* (name partly flaked: "T_RN"). She was run aground deliberately "since the lights went out" (during the collapse). The two-person crew cut every breaker and walked inland.
- The log describes a regular "pattern" on the depth sounder at a mark off the point, and the same thing on the radio "on a channel with no station".
- The launch's battery bank is still live sixty years on.
- The masthead lamp flickers with no power at all.
- Mara (and "everybody on this shore") treats it as "a storm". She deflects.
- Nothing explains the Current. Do not add an explanation.

## Tests added

- `DeadCurrent.Exploration.Discovery`: once-only discovery, rotated box, announce count, save round-trip, silent restore, older save.
- `DeadCurrent.Exploration.Container`: prompt, partial/full looting, a slot round-trip into a fresh world, and a container newer than the save.
- `DeadCurrent.Exploration.WorldConditions`: damage volume and flicker light switched by a flag, including silent restore.
- `DeadCurrent.Rules.Conditions`: a LocationDiscovered case.
- `DeadCurrent.World.InspectVariants`: verb cases.
- `DeadCurrent.Content.Exploration.MaraWreckLine`: shipped dialogue across Shore Watch states, plus save/reload.
- `DeadCurrent.Map.Boathouse.SurveyLaunch`: real map, walk-in discovery, real hazard damage, clues, partial loot, pull the leads, hidden kit, save, diverge, F9, no re-announce.
- `DeadCurrent.Map.Boathouse.SurveyLaunchSaves`: slots A/B/C combining the POI with Shore Watch accepted or complete, loading back and forth.
- `DeadCurrent.Map.Boathouse.LegacySave`: now also checks the POI is untouched by a pre-Phase-3 save.

## Known issues / notes for the report

- **No human playtest.** Tests teleport and call interactions directly.
- The glow materials and sightlines have not been visually verified (see step 1).
- **Latent save issue (not triggered):** loading an older save destroys any `ADCItemPickup` added to the map after it, because "missing from the save" means "taken". The POI uses only containers, so nothing is affected today. Worth a fix before content adds pickups: track removed ids explicitly.
- The firearm's object-type trace also hits query-only overlap volumes (damage volumes). This is pre-existing; the location volume avoids it by not using collision.
- A player can jump from the transom onto the tender and loot it without touching the water. Acceptable, maybe good.
- Location display names live on the volume actors. A future map screen or multi-map setup will want a location data asset.
- The water is a solid walkable block (pre-existing prototype limitation). The west region has bluffs and breakwaters so you can't walk off the map there.
- No new item was added; existing items were enough.
- Deferred decisions are unchanged (the six words, Mara's role, factions, GLMA naming, reward balance, the scavenger after the coil route, "no shooting"). New ones to add: the *Tern* name, the loot balance at the wreck, and whether Mara's "storm" line should vary with Shore Watch outcome.

## Commands

```
Tools\RunTests.bat [-build] [-nomap] [Filter]    full suite ~2 min; Map / Exploration filters work
Tools\RebuildContent.bat build_boathouse          regenerate the map (close the editor, build C++ first)
Tools\RebuildContent.bat create_dialogue
Tools\PlayTest.bat                                 standalone playtest window
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" DeadCurrentEditor Win64 Development -Project="C:\deadcurrent\DeadCurrent.uproject" -WaitMutex
```

Gotchas:
- Bash has no `python`. Edit files with the editor tools.
- `SM_Cube` pivots at its min corner: use bounds centers, not actor locations, to find greybox actors (see `Nearest<>` in `DCBoathouseMapTest.cpp`).
- Commit messages end with `Co-Authored-By: Claude …`. Don't push or force.

## Recommended next step (for the report; do not start it)

After Anthony accepts the Exploration Loop: plan Phase 4 (RPG Layer) as its own numbered checklist before building anything. Consider fixing the pickup/older-save persistence issue first.
