# Agent Handoff — Phase 6 — WP-SYS-PORTAL (VS-03) — Systems Engineer

Filled from `AGENT_HANDOFF_TEMPLATE.md`. Read first: `CLAUDE.md`, `Design/POIs/README.md` (ownership, branches, worktrees), `Design/POIs/handoffs/PHASE6_WAVE1.md`, `VerticalSlicePhasePlan.txt` §5.3, §5.7, §6 (row 1), §13.3 (WP-SYS-PORTAL), §15 (VS-03), and `Design/technical_architecture.md` ("Conditional presence", "Interaction", "World state").

This package builds **one** reusable capability and nothing else: a **cell portal** with a **scene-cut** signal. The other two approved capabilities (the light sequence and the story card) are **not** in this package and must not be started.

## Assigned Role

Systems Engineer. Builds the smallest generic extension the plan has proven necessary. Does not author content, maps, quests, dialogue, or art. Does not approve its own work.

## Why this capability exists (evidence, so you build the right thing)

The slice puts its interiors (the vault, the net loft, the lamp-room stair) in the **same map** as the island and moves the player between them, because the quest log, flags, registry, and build are per-world and a save holds one map's actors (plan §5.1). The transitions (plan §5.3) are:

- a locked door that opens only when conditions pass (the vault hatch needs a key **or** the `vault_opened` flag; the lower door needs Engineering 2 **or** `vault_opened`)
- a use that can have **consequences** (the first use of the hatch with the key sets `sombre.vault_opened`; the loft stair's "Go down" after the meeting sets `sombre.night_quay_seen`)
- a change of scene during which **presence rules must snap** (the net loft fills with its attendees; Hale and his cutter appear while the player is away). Without a scene-cut signal, `ADCConditionalPresence`'s deferral (`bDeferWhileObserved`, 15 m) would hold the change forever, because the player arrives inside the 15 m bubble of the new placement.

## Allowed Scope

Build, test, and document **`ADCCellPortal`** and the scene-cut signal. Nothing else.

### The contract (implement this; refine details only inside these constraints)

**`ADCCellPortal`** (`World/`), an `AActor` implementing `IDCInteractable`, in the style of `ADCInspectableActor`, `ADCLootContainer`, and `ADCDoor`.

| Property | Type | Meaning |
| --- | --- | --- |
| `DisplayName` | `FText` | the name in the prompt (for example "Vault hatch") |
| `Variants` | `TArray<FDCPortalVariant>` | ordered; **first whose `Conditions` pass wins**, exactly like inspect variants |
| `LockedText` | `FText` | shown (as a timed HUD message) when **no** variant passes |
| `Destination` | `TObjectPtr<AActor>` (same level) | where the player arrives: this actor's location; its yaw too when `bUseDestinationYaw` |
| `bUseDestinationYaw` | `bool`, default true | set the player's control rotation to the destination's yaw |
| `FadeOutSeconds`, `HoldSeconds`, `FadeInSeconds` | `float`, defaults about 0.35 / 0.15 / 0.35 | the transition. **All three zero means instant and synchronous** (required: headless tests and the later map test depend on it) |
| `CardText` | `FText`, optional | shown (timed HUD message is enough; no new UI) while the screen is dark |

`FDCPortalVariant` (a `USTRUCT`): `FName VariantId`, `TArray<FDCGameplayCondition> Conditions`, `FText Verb` (default "Go"), `TArray<FDCGameplayConsequence> Consequences`.

Behavior:

1. **Prompt** (`GetInteractionPrompt`): the first passing variant's verb plus `DisplayName` (`[E] Unlock Vault hatch`); when none passes, a default verb ("Try") plus `DisplayName`. **`CanInteract` is true** whenever no transition is running, even when locked, because the player must be able to read `LockedText`. Interaction type: the existing `Interaction.*` tag used for doors (add none).
2. **Interact**:
   - a transition already running: ignore.
   - no variant passes: show `LockedText`; change nothing; apply no consequences; do not move the player.
   - a variant passes: run the transition (below) with **that variant's consequences**.
3. **Transition** (timed mode): block movement and look input; fade the camera to black (`APlayerCameraManager::StartCameraFade` is the obvious tool); **at black**: end any conversation, apply the variant's consequences through `UDCGameplayRules::ApplyConsequencesFor(Interactor, ...)`, move the player to `Destination` (teleport with physics reset; zero velocity), broadcast the scene cut (next), show `CardText` if any; hold; fade back in; restore input. Instant mode does the same steps in one call with no fade and no input blocking.
4. **Conditions** are evaluated against the **interacting player** (`FDCRuleContext::ForActor(Interactor)`), so item, quest, and build conditions work.
5. **Scene cut.** Add to `UDCWorldStateSubsystem`: `FSimpleMulticastDelegate OnSceneCut` and `void NotifySceneCut()`, beside the existing `OnRestored` / `NotifyRestored()` (same style, same documentation standard). The portal calls `NotifySceneCut()` **after** applying consequences and moving the player, while the screen is dark. `ADCConditionalPresence` binds it exactly as it binds `OnRestored`: **snap** (silent, immediate, ignoring deferral). Nothing else changes in presence.
6. **Saves nothing.** No persistent id, no `IDCPersistent`, no save field, no `SaveVersion` bump. A portal's effects live entirely in the world flags its consequences set (which already save). A save made mid-transition is out of scope; document it as a known edge in the subsection (do not touch save code).

### The scene cut is not a restore

`OnRestored` means "state was replaced wholesale by a load; do not re-run anything". `OnSceneCut` means "the player was moved between scenes; presence snaps, nothing else is implied". They are separate signals on purpose (a later task relies on the difference: a light's one-time sequence plays on a scene cut but never on a restore).

## Files / Content Owned

Only these may be created or edited:

- **New:** `Source/DeadCurrent/World/DCCellPortal.h`
- **New:** `Source/DeadCurrent/World/DCCellPortal.cpp`
- **New:** `Source/DeadCurrent/World/DCCellPortalTest.cpp`
- **Additive edit:** `Source/DeadCurrent/World/DCWorldStateSubsystem.h` and `.cpp` — the `OnSceneCut` delegate and `NotifySceneCut()` only
- **Additive edit:** `Source/DeadCurrent/World/DCConditionalPresence.cpp` (and `.h` only if a member is needed) — bind and unbind the scene cut to `Snap`, beside `OnRestored`
- **Additive edit:** `Design/technical_architecture.md` — **one new subsection**, "Cell portal (Phase 6)", placed immediately after the "Conditional presence" section and before "Exploration". What it must contain: the properties above, how to author a portal from a map script (spawn it, set `Variants`, `Destination`, `LockedText`; one portal per doorway; both directions are two portals), the scene-cut signal and how it differs from `OnRestored`, instant mode, what it must **not** be used for (anything that must be remembered on its own; it saves nothing), and the mid-transition save edge.
- This handoff's "Handoff Notes" section (status, commits, results).

## Files That Must Not Be Modified

- Every other file under `Source/**`, especially: `Core/DCGameplayTypes.h`, `Core/DCGameplayRules.*` (**no new condition or consequence types**), `Save/**` (**no save field, no `SaveVersion` change, no change to `UDCSaveSubsystem`**), `Quest/**`, `Dialogue/**`, `Interaction/**` (**do not change `IDCInteractable`**), `World/DCDoor.*`, `World/DCFlickerLight.*` (the light sequence is a later task), `UI/**` (the story card is a later task), `Character/**`, `AI/**`, and every existing test file
- `DeadCurrent.Build.cs` — if you believe a module dependency is needed, **stop** (the plan expects none; `AIModule`, `UMG`, `Slate` are already there)
- every file under `Tools/**`, `Config/**`, `Content/**`, `Design/**` other than the two listed above
- every `Tools/ContentSpecs/**` file, every map script (`build_*.py`), every generated `.umap`
- shipped persistent ids, flags, quest stages, asset names
- `CLAUDE.md`, `LongTermPlan.txt`, the phase plans, `Design/ANTHONY_CHECKLIST.md` (the Integrator maintains them)
- any file another active handoff owns (see `PHASE6_WAVE1.md`)

## Input Specification

- **Plan:** `VerticalSlicePhasePlan.txt` at commit `034cbc7` (approved).
- **Code baseline:** the **VS-02 commit `e833811`**. Branch `vs/sys-portal` from `origin/main` (the wave-handoff commit: a docs-only child of it that carries this handoff).
- **Baseline tests:** **46 of 46** (33 editor, 13 map). Record the baseline in your notes before changing anything.
- **Existing code to read first:** `World/DCConditionalPresence.{h,cpp}` (how it binds `OnRestored`, `Snap`, deferral), `World/DCWorldStateSubsystem.{h,cpp}`, `World/DCInspectableActor.{h,cpp}` (variants, conditions, consequences, HUD message), `World/DCDoor.*` and `AI/DCFriendlyNPC.*` (how interactables are made traceable), `Core/DCGameplayRules.h`, `Core/DCTestHelpers.h` (`FDCTestWorld`), `World/DCConditionalPresenceTest.cpp` (an editor test in the style to follow).
- **Decisions already made (do not reopen):** in-map interior cells, not map travel; conditional presence is unchanged except for binding the scene cut; no save change. Anthony's approval of the plan, 2026-09-30.

## Expected Deliverables

1. `ADCCellPortal` and `FDCPortalVariant` per the contract, compiling cleanly with `DeadCurrentEditor`.
2. `UDCWorldStateSubsystem::OnSceneCut` / `NotifySceneCut()`; `ADCConditionalPresence` snaps on it.
3. The editor test **`DeadCurrent.World.CellPortal`** (file `DCCellPortalTest.cpp`; editor context, using `FDCTestWorld`; it may use several `IMPLEMENT_SIMPLE_AUTOMATION_TEST`s under the same `DeadCurrent.World.CellPortal.*` prefix if that is cleaner, but the suite must list a test named `DeadCurrent.World.CellPortal`). It must prove, at minimum:
   - no variant passes: `LockedText` is shown, the player does not move, no consequence is applied
   - first-match: two passing variants, the first one's verb and consequences are used
   - the prompt: a passing variant's verb and the display name; the locked prompt; `CanInteract` true while locked
   - consequences are applied by a use (a flag is set), through the real rules
   - arrival: the player ends at the destination's location and, when set, yaw, with zero velocity
   - **the scene cut snaps presence**: an `ADCConditionalPresence` rule with deferral active and the player inside its observed distance holds a change before the use (asserted via `HasPendingChange()`), and has applied it after the portal use; and a plain flag change alone (no portal) still defers
   - `NotifySceneCut()` does **not** fire on a restore, and `NotifyRestored()` does **not** fire on a portal use
   - a portal saves nothing: it has no persistent id component and is not `IDCPersistent`
   - a use while a timed transition is running is ignored (tick the test world to drive the timing)
   - instant mode (all three durations zero) is synchronous
4. The "Cell portal (Phase 6)" subsection of `Design/technical_architecture.md`.
5. The Handoff Notes filled in below, including the exact commands run and their results.

## Required Tests

- `Tools\RunTests.bat -build` from **your worktree** (close any editor first): **47 of 47**, that is the 46 baseline plus `DeadCurrent.World.CellPortal`, with **no existing test changed and all green**. If the number differs, say why in the notes.
- `DeadCurrent.Content.Validate` and every `Map.Boathouse.*` test stay green (they are part of the run).
- No packaging is required for this package; the Integrator packages after merging.

## Required Review Artifacts

- Commit(s) on `vs/sys-portal`, one plain sentence each, prefixed `VS-03:`.
- The test log excerpt (`Saved/Logs/RunTests.log` in your worktree): the list of completed tests with results.
- The authoring-contract subsection in `technical_architecture.md`.
- A short list, in Handoff Notes, of anything you decided that the contract above left open (for example the exact default verb text and fade defaults), so the Integrator can review it.

## Known Dependencies

- VS-02 must be committed and pushed (it is: `e833811`). Nothing else. Do not wait for the kit or the research packages.
- `ADCConditionalPresence` (Phase 5) must behave exactly as before for every existing caller.

## Commits, pushes, and integration

- **May commit:** yes, on branch `vs/sys-portal` only, in your worktree `C:\DeadCurrent_wt\sys-portal`.
- **May push:** yes, **only** `git push origin vs/sys-portal`. **Never** push `main`, never merge into `main`, never force-push, never rewrite pushed history.
- **Integration owner:** the Integrator (Claude). When you finish, say so in Handoff Notes and stop; the Integrator merges, reruns the suite, and pushes `main`.
- Commit messages: one plain sentence, prefixed `VS-03:`, following `git log`.

## Stop Conditions

Stop, write the reason at the top of Handoff Notes, and return control when any of these is true:

- the contract needs a new condition or consequence type, a save field, a `SaveVersion` bump, or a change to `IDCInteractable`
- a file outside **Files / Content Owned** would have to change (including `DeadCurrent.Build.cs`)
- a persistent id, flag, or quest stage would have to be renamed
- the existing `ADCConditionalPresence` tests or any other baseline test fails and the cause is not this work
- `Tools\RunTests.bat -build` cannot be run in your worktree for environmental reasons after one honest attempt to fix it (say exactly what failed)
- the tree contains uncommitted edits in files this handoff does not own
- you find yourself building the light sequence, the story card, a door replacement, a companion, or map travel: that is another task
- usage is running low: finish or revert the smallest unit, verify, record the exact next step, stop

## Handoff Notes

Filled in by the receiving agent when it stops or finishes.

- **Status:** not started
- **Commits:**
- **What changed:**
- **Tests run and results:**
- **Decisions the contract left open:**
- **Open issues:**
- **Exact next step:**
- **Return to:** the Integrator (Claude)
