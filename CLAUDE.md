# DEAD CURRENT — Agent Orientation

First-person action RPG in a post-collapse Great Lakes archipelago. Unreal Engine 5.8, C++ systems + data-driven content. Owner and sole playtester: Anthony. Read this first, then the doc that fits your task.

## Where things are

| Need | Read |
| --- | --- |
| Vision, pillars, the 10-phase ladder | `LongTermPlan.txt` |
| Scope of each finished/active phase | `FirstPhasePlan.txt`, `ExplorationLoopPlan.txt`, `RPGPhasePlan.txt` |
| Build, test, controls, every system, conventions | `Design/technical_architecture.md` |
| What's in the game and why (decisions log) | `Design/game_design.md` |
| Canon and PROVISIONAL lore | `Design/world_bible.md` |
| Latest session's results + Anthony's test checklist | `Design/CLAUDE_SESSION_REPORT.md` |
| Art, audio, the asset library, Meshy | `Design/art_pipeline.md` |

## Where we are (2026-09-29)

- Phases 1–4 are **accepted**: Walking Skeleton, Micro RPG (Shore Watch), Exploration Loop (Wrecked Survey Launch), and RPG Layer (3 attributes / 3 skills / 3 perks as shared rule conditions on existing actors, accepted 2026-09-29). 36 automated tests pass.
- Everything is on one map, `Lvl_Boathouse`, and it is still **greybox**: prototype materials, canvas HUD, no audio, water is a slab. Playtests have flagged that clues are hard to read without real art and sound.
- **Next: Presentation Pass** (chosen 2026-09-29). A bounded art, audio, and readability pass on the existing shore, no new gameplay. Plan file: `PresentationPassPlan.txt` (PP-xx task ids). Constraints and the asset library: `Design/art_pipeline.md`.
- After that: Phase 5 World State, then Phase 6 Vertical Slice (the lighthouse-in-a-storm settlement, `LongTermPlan.txt` §23). **Anthony chooses and accepts phases. Don't start the next phase unless he's said to.**

## How work is done here

- **Plan first, then numbered tasks.** Each phase starts with a committed `<Name>PhasePlan.txt` (mission, scope lock, out-of-scope list, ordered tasks with "Done when"). Commits reference task ids (`RP-03: ...`). Stay inside the scope lock.
- **Systems, not one-offs.** New behavior goes through the shared rule language (`Core/`: conditions and consequences, Gameplay Tags). No actor-specific `if` chains. Content is data (Data Assets built by scripts in `Tools/EditorScripts/`).
- **Maps and data assets are generated.** `build_boathouse.py`, `build_test_gym.py`, and the `create_*.py` scripts regenerate their assets. **Hand edits to `Lvl_Boathouse`, `Lvl_TestGym`, items, quests, and dialogue are lost on the next `Tools\RebuildContent.bat`.** Change the script.
- **Saves are versioned** (`UDCSaveGame::SaveVersion`). Never rename a shipped persistent id, flag, or quest stage. Old saves must still load.
- **Lore is PROVISIONAL** until Anthony confirms it. Never explain the Current. Don't decide factions or Mara's backstory.
- **Tests:** `Tools\RunTests.bat` (add `-build` after C++ changes). Close the editor first. Every phase ends green, with a Development Win64 cook, and with the session report replaced.
- **Playtest:** `Tools\PlayTest.bat` (low-spec overrides: DX11, 400 MB texture pool, 60 FPS cap).
- Commit on `main` only when asked. Commit messages are one plain sentence; see `git log`.
- Other agents may be working in this repo at the same time (the editor may be open). Check `git status` and don't sweep someone else's uncommitted edits into your commit.

## Visual and audio work: read before generating or importing anything

- **Asset library: `C:\FO5_AssetLibrary`** (~39 GB, from an earlier project). Anything in it may be used for DEAD CURRENT. It contains a UE 5.8 project full of marketplace packs, raw Fab/Megascans scans, CC0 textures and models, CC0 audio, and earlier Meshy outputs. Inventory, licenses, and what fits the art direction are in `Design/art_pipeline.md`.
- **Meshy Pro API** is available for generating new 3D models. The API key is in `C:\FO5_AssetLibrary\Meshy\meshy_key.txt`. **Never print, log, commit, or copy the key into this repo**; read it at runtime. Generations spend paid credits (about 30–50 per prop so far). Follow the Meshy workflow and record-keeping in `Design/art_pipeline.md`.
