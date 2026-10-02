#include "Character/DCCharacterProgressionComponent.h"
#include "Core/DCContentSubsystem.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCGameplayTags.h"
#include "Core/DCTestHelpers.h"
#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  The Pointe Sombre slice's data (VS-09, WP-NARR), tested without the map: the ten slice items, both quests, and the
 *  nine conversations, through the shipped generated assets. Ids are the ledger's (Design/POIs/sombre_ids.md).
 *  World acts that live in cells (inspectables, portals) are stood in for by setting the flags they set.
 */
namespace DCSombreContentTest
{
	const FName Characteristic = TEXT("sombre.characteristic");
	const FName FalseLight = TEXT("sombre.false_light");
	const FName Sister = TEXT("player.tie.sister");
	const FName Partner = TEXT("player.tie.partner");
	const FName TookIn = TEXT("player.tie.took_in");

	const TCHAR* SliceItems[] = {
		TEXT("liv_letter"), TEXT("liv_note"), TEXT("liv_chart"), TEXT("vault_access_log"), TEXT("cut_cable_end"),
		TEXT("false_lantern"), TEXT("section_key_compact"), TEXT("clockwork_pawl"), TEXT("lamp_oil"), TEXT("sombre_vault_key") };

	/** The nine assets and their entry nodes in ledger order (§6; first match wins, the last unconditional). */
	struct FDialogueSpec
	{
		const TCHAR* Asset;
		const TCHAR* DialogueId;
		TArray<FName> Entries;
	};

	TArray<FDialogueSpec> DialogueSpecs()
	{
		return {
			{ TEXT("DA_Dialogue_SombreMara"), TEXT("sombre_mara"), { "crossing_tie", "crossing", "promise", "after_line", "after", "panel", "ticks", "town" } },
			{ TEXT("DA_Dialogue_SombreVarga"), TEXT("sombre_varga"), { "after_line", "after_hand", "after_dark", "waiting", "first" } },
			{ TEXT("DA_Dialogue_SombreOdette"), TEXT("sombre_odette"), { "after_keeper", "after_disgraced", "after", "knows", "first" } },
			{ TEXT("DA_Dialogue_SombreMarthe"), TEXT("sombre_marthe"), { "after_line", "after_hand", "after_power", "after_dark", "loft", "store" } },
			{ TEXT("DA_Dialogue_SombreTem"), TEXT("sombre_tem"), { "exposed", "first" } },
			{ TEXT("DA_Dialogue_SombreDell"), TEXT("sombre_dell"), { "keeper", "confessed", "first" } },
			{ TEXT("DA_Dialogue_SombreSigrun"), TEXT("sombre_sigrun"), { "taken", "helping", "revealed", "first" } },
			{ TEXT("DA_Dialogue_SombreHale"), TEXT("sombre_hale"), { "after_line", "after_hand", "after_flooded", "after_power", "after", "offer", "first" } },
			{ TEXT("DA_Dialogue_SombreJonas"), TEXT("sombre_jonas"), { "after_line", "after", "first" } },
		};
	}

	/** Every flag the slice's data may read or write: ledger §3.2 and §3.3. A typo in a spec fails here. */
	const TSet<FName>& LedgerFlags()
	{
		static const TSet<FName> Flags = {
			"sombre.reef_struck", "sombre.storm", "sombre.mara_tie_asked", "sombre.ticks_seen", "sombre.mara_ticks",
			"sombre.cable_cut_found", "sombre.vault_opened", "sombre.log_read", "sombre.panel_read", "sombre.mara_panel",
			"sombre.bearing_taken", "sombre.liv_note_found", "sombre.vault_floor_isolated", "sombre.hale_arrived",
			"sombre.hale_met", "sombre.hale_crew_up", "sombre.sigrun_revealed", "sombre.sigrun_helping", "sombre.feed_spliced",
			"sombre.light_line", "sombre.power_settlement", "sombre.node_destroyed", "sombre.clockwork_freed", "sombre.light_hand",
			"sombre.light_decided", "sombre.keeper_odette", "sombre.keeper_dell", "sombre.dell_confessed",
			"sombre.false_light_taken", "sombre.varga_told", "sombre.meeting_called", "sombre.exposed_pruitts",
			"sombre.exposed_odette", "sombre.exposed_liv", "sombre.exposed_sigrun", "sombre.player_named_tie",
			"sombre.odette_forgiven", "sombre.meeting_done", "sombre.night_quay_seen", "sombre.slice_end",
			"player.tie.sister", "player.tie.partner", "player.tie.took_in", "watch.mara_travelling", "watch.revealed",
			"shore.liv_asked", "shore.promise_told", "wreck.mara_pressed" };
		return Flags;
	}

	const UDCDialogueAsset* LoadDialogue(const FString& AssetName)
	{
		return LoadObject<UDCDialogueAsset>(nullptr, *FString::Printf(TEXT("/Game/Dialogue/%s.%s"), *AssetName, *AssetName));
	}

	bool IsTieFlag(FName Id)
	{
		return Id == Sister || Id == Partner || Id == TookIn;
	}

	bool HasCondition(const TArray<FDCGameplayCondition>& Conditions, EDCConditionType Type, FName Id, bool bNegate)
	{
		return Conditions.ContainsByPredicate([&](const FDCGameplayCondition& C) { return C.Type == Type && C.Id == Id && C.bNegate == bNegate; });
	}

	bool HasConsequence(const TArray<FDCGameplayConsequence>& Consequences, EDCConsequenceType Type, FName Id)
	{
		return Consequences.ContainsByPredicate([&](const FDCGameplayConsequence& C) { return C.Type == Type && C.Id == Id; });
	}

	bool IsNoTie(const TArray<FDCGameplayCondition>& Conditions)
	{
		return HasCondition(Conditions, EDCConditionType::WorldFlag, Sister, true)
			&& HasCondition(Conditions, EDCConditionType::WorldFlag, Partner, true)
			&& HasCondition(Conditions, EDCConditionType::WorldFlag, TookIn, true);
	}

	void LoadContent()
	{
		TArray<UObject*> Definitions;
		UDCContentSubsystem::LoadAllDefinitions(Definitions);
	}

	/** A player in a throwaway world: inventory, quest log, dialogue, a build, and the world state. */
	struct FSombre
	{
		FDCTestWorld World;
		AActor* Player = nullptr;
		UDCInventoryComponent* Inventory = nullptr;
		UDCQuestComponent* Quests = nullptr;
		UDCDialogueComponent* Dialogue = nullptr;
		UDCCharacterProgressionComponent* Build = nullptr;
		UDCWorldStateSubsystem* WorldState = nullptr;

		FSombre()
		{
			Player = World.SpawnActor();
			Inventory = FDCTestWorld::AddComponent<UDCInventoryComponent>(Player);
			Quests = FDCTestWorld::AddComponent<UDCQuestComponent>(Player);
			Dialogue = FDCTestWorld::AddComponent<UDCDialogueComponent>(Player);
			Build = FDCTestWorld::AddComponent<UDCCharacterProgressionComponent>(Player);
			WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		}

		FName Talk(const TCHAR* Asset) const
		{
			Dialogue->EndDialogue();
			Dialogue->StartDialogue(LoadDialogue(Asset), nullptr);
			return Dialogue->GetCurrentNodeId();
		}

		FName Node() const { return Dialogue->GetCurrentNodeId(); }

		TArray<FString> Visible() const
		{
			TArray<FString> Texts;
			if (const FDCDialogueNode* Current = Dialogue->GetCurrentNode())
			{
				for (const int32 Index : Dialogue->GetVisibleChoiceIndices())
				{
					Texts.Add(Current->Choices[Index].Text.ToString());
				}
			}
			return Texts;
		}

		bool Can(const FString& Text) const { return Visible().Contains(Text); }

		int32 Count(const FString& Text) const
		{
			return Visible().FilterByPredicate([&Text](const FString& T) { return T == Text; }).Num();
		}

		bool Say(const FString& Text) const
		{
			const int32 Index = Visible().IndexOfByKey(Text);
			return Index != INDEX_NONE && Dialogue->SelectChoice(Index);
		}

		bool Flag(FName Name) const { return WorldState->HasFlag(Name); }
		void Set(FName Name) const { WorldState->SetFlag(Name); }
		FName Stage(FName Quest) const { return Quests->GetStage(Quest); }

		int32 Has(const TCHAR* ItemId) const { return Inventory->GetQuantity(UDCItemDefinition::FindByItemId(ItemId)); }
		void Give(const TCHAR* ItemId) const { Inventory->AddItem(UDCItemDefinition::FindByItemId(ItemId), 1); }

		void Skill(const TCHAR* Name, int32 Value) const
		{
			Build->SetSkillValue(UDCCharacterProgressionComponent::SkillTag(Name), Value);
		}

		int32 TieFlags() const { return (Flag(Sister) ? 1 : 0) + (Flag(Partner) ? 1 : 0) + (Flag(TookIn) ? 1 : 0); }

		/** Varga's first reply, then the vault's facts as the cells would set them: the quest stands at knows. */
		void ReachKnows() const
		{
			Talk(TEXT("DA_Dialogue_SombreVarga"));
			Say(TEXT("I'll find out why."));
			WorldState->DiscoverLocation(TEXT("sombre.light"));
			Set(TEXT("sombre.cable_cut_found"));
			Set(TEXT("sombre.vault_opened"));
			Set(TEXT("sombre.panel_read"));
			Set(TEXT("sombre.liv_note_found"));
		}

		/** At knows: Marthe's call, then her meeting opened, to the evidence node. */
		void OpenMeeting() const
		{
			Talk(TEXT("DA_Dialogue_SombreMarthe"));
			Say(TEXT("Call the island to the loft."));
			Talk(TEXT("DA_Dialogue_SombreMarthe"));
			Say(Flag(TEXT("sombre.light_decided")) ? TEXT("It's done.") : TEXT("Leave it as she left it."));
		}

		TUniquePtr<FSombre> SaveAndReload() const
		{
			UDCSaveGame* Save = NewObject<UDCSaveGame>();
			UDCSaveSubsystem::CaptureProgress(Save, Quests, WorldState);
			TArray<FDCSavedItemStack> Stacks;
			Inventory->CaptureStacks(Stacks);
			TUniquePtr<FSombre> Reloaded = MakeUnique<FSombre>();
			Reloaded->Inventory->ReplaceFromSaved(Stacks);
			UDCSaveSubsystem::ApplyProgress(Save, Reloaded->Quests, Reloaded->WorldState);
			return Reloaded;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreItemsTest, "DeadCurrent.Content.Sombre.Items",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreItemsTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreContentTest;
	LoadContent();

	for (const TCHAR* Id : SliceItems)
	{
		const UDCItemDefinition* Item = UDCItemDefinition::FindByItemId(Id);
		if (!TestNotNull(FString::Printf(TEXT("%s resolves"), Id), Item))
		{
			continue;
		}
		TestTrue(FString::Printf(TEXT("%s has a category"), Id), Item->Category.IsValid());
		TestTrue(FString::Printf(TEXT("%s is a quest object"), Id), Item->Category == DCTags::Item_Quest.GetTag());
		TestFalse(FString::Printf(TEXT("%s has a name"), Id), Item->DisplayName.IsEmpty());
		TestFalse(FString::Printf(TEXT("%s has a description"), Id), Item->Description.IsEmpty());
		TestFalse(FString::Printf(TEXT("%s has a world mesh"), Id), Item->WorldMesh.IsNull());
		TestEqual(FString::Printf(TEXT("%s does not stack"), Id), Item->MaxStackSize, 1);
	}

	// The slice's loot and the Sounder Chart are the shipped shore items, reused, not reminted (ledger §1, §4).
	for (const TCHAR* Id : { TEXT("ammo_9mm"), TEXT("field_dressing"), TEXT("salvage_wiring"), TEXT("survey_chart") })
	{
		TestNotNull(FString::Printf(TEXT("Shipped %s still resolves"), Id), UDCItemDefinition::FindByItemId(Id));
	}
	for (const TCHAR* Alias : { TEXT("sounder_chart"), TEXT("sombre.vault_key") })
	{
		TestNull(FString::Printf(TEXT("Alias %s is not created"), Alias), UDCItemDefinition::FindByItemId(Alias));
	}

	// Liv's note carries the approved text (Anthony, 2026-09-30: right as written).
	if (const UDCItemDefinition* Note = UDCItemDefinition::FindByItemId(TEXT("liv_note")))
	{
		TestTrue(TEXT("Liv's note text"), Note->Description.ToString().StartsWith(TEXT("Odette — I took the card.")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreQuestGraphTest, "DeadCurrent.Content.Sombre.QuestGraph",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreQuestGraphTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreContentTest;
	LoadContent();
	const UDCQuestDefinition* Main = UDCQuestDefinition::FindByQuestId(Characteristic);
	const UDCQuestDefinition* Side = UDCQuestDefinition::FindByQuestId(FalseLight);
	if (!TestNotNull(TEXT("sombre.characteristic exists"), Main) || !TestNotNull(TEXT("sombre.false_light exists"), Side))
	{
		return false;
	}

	// Stage ids are saved: exactly the ledger's (§5), in order, with the ledger's outcomes.
	auto StageIds = [](const UDCQuestDefinition* Quest)
	{
		TArray<FName> Ids;
		for (const FDCQuestStage& Stage : Quest->Stages)
		{
			Ids.Add(Stage.StageId);
		}
		return Ids;
	};
	const TArray<FName> MainStages = { "arrived", "lamp_room", "vault", "vault_inside", "knows", "done_line", "done_hand", "done_dark" };
	const TArray<FName> SideStages = { "asked", "found", "named", "done" };
	TestTrue(TEXT("characteristic stages are the ledger's"), StageIds(Main) == MainStages);
	TestTrue(TEXT("false_light stages are the ledger's"), StageIds(Side) == SideStages);
	TestEqual(TEXT("characteristic starts at arrived"), Main->GetStartStage(), FName(TEXT("arrived")));
	TestEqual(TEXT("false_light starts at asked"), Side->GetStartStage(), FName(TEXT("asked")));
	for (const FName Done : { FName("done_line"), FName("done_hand"), FName("done_dark") })
	{
		TestTrue(*FString::Printf(TEXT("%s completes"), *Done.ToString()), Main->IsCompletingStage(Done));
		const FDCQuestStage* Stage = Main->FindStage(Done);
		TestTrue(*FString::Printf(TEXT("%s sets the storm again"), *Done.ToString()),
			Stage && HasConsequence(Stage->OnEnter, EDCConsequenceType::SetWorldFlag, TEXT("sombre.storm")));
	}
	TestTrue(TEXT("false_light done completes"), Side->IsCompletingStage(TEXT("done")));
	TestNull(TEXT("No Copper Buyer quest (plan §8.2 #9)"), UDCQuestDefinition::FindByQuestId(TEXT("sombre.copper_buyer")));
	const FDCQuestStage* Knows = Main->FindStage(TEXT("knows"));
	TestTrue(TEXT("knows objective sends the player to the net loft"),
		Knows && Knows->ObjectiveText.ToString().Contains(TEXT("then tell the settlement at the net loft")));

	// The main line, step by step, with knows needing both the panel and the note.
	{
		FSombre S;
		S.Set(TEXT("sombre.storm")); // the strike sets it before the quest exists
		S.Quests->StartQuest(Characteristic);
		TestEqual(TEXT("Starts at arrived"), S.Stage(Characteristic), FName(TEXT("arrived")));
		S.Set(TEXT("sombre.cable_cut_found"));
		TestEqual(TEXT("The feed alone does not skip the tower"), S.Stage(Characteristic), FName(TEXT("arrived")));
		S.WorldState->DiscoverLocation(TEXT("sombre.light"));
		TestEqual(TEXT("Tower discovered chains through the found feed"), S.Stage(Characteristic), FName(TEXT("vault")));
		S.Set(TEXT("sombre.vault_opened"));
		TestEqual(TEXT("Vault open"), S.Stage(Characteristic), FName(TEXT("vault_inside")));
		S.Set(TEXT("sombre.panel_read"));
		TestEqual(TEXT("The panel alone is not enough"), S.Stage(Characteristic), FName(TEXT("vault_inside")));
		TestTrue(TEXT("Still storming before knows"), S.Flag(TEXT("sombre.storm")));
		S.Set(TEXT("sombre.liv_note_found"));
		TestEqual(TEXT("Panel and note: knows"), S.Stage(Characteristic), FName(TEXT("knows")));
		TestFalse(TEXT("knows clears the storm"), S.Flag(TEXT("sombre.storm")));
		TestTrue(TEXT("knows brings Hale"), S.Flag(TEXT("sombre.hale_arrived")));
		TestFalse(TEXT("knows does not end the meeting"), S.Flag(TEXT("sombre.meeting_done")));

		// A save at knows reloads at knows without re-running its OnEnter.
		TUniquePtr<FSombre> Reloaded = S.SaveAndReload();
		TestEqual(TEXT("Reload at knows"), Reloaded->Stage(Characteristic), FName(TEXT("knows")));
		TestFalse(TEXT("Reload keeps the storm cleared"), Reloaded->Flag(TEXT("sombre.storm")));
	}

	// The note first, then the panel.
	{
		FSombre S;
		S.Quests->StartQuest(Characteristic);
		S.WorldState->DiscoverLocation(TEXT("sombre.light"));
		S.Set(TEXT("sombre.cable_cut_found"));
		S.Set(TEXT("sombre.vault_opened"));
		S.Set(TEXT("sombre.liv_note_found"));
		TestEqual(TEXT("The note alone is not enough"), S.Stage(Characteristic), FName(TEXT("vault_inside")));
		S.Set(TEXT("sombre.panel_read"));
		TestEqual(TEXT("Note then panel: knows"), S.Stage(Characteristic), FName(TEXT("knows")));
	}

	// Every outcome stage is reachable; the untouched ending completes; power and destruction are flags, not stages.
	struct FOutcome { const TCHAR* Label; TArray<FName> Acts; FName Expected; };
	const TArray<FOutcome> Outcomes = {
		{ TEXT("Line"), { "sombre.light_line", "sombre.light_decided" }, "done_line" },
		{ TEXT("Hand"), { "sombre.light_hand", "sombre.light_decided" }, "done_hand" },
		{ TEXT("Hand + settlement power"), { "sombre.power_settlement", "sombre.light_hand", "sombre.light_decided" }, "done_hand" },
		{ TEXT("Settlement power"), { "sombre.power_settlement", "sombre.light_decided" }, "done_dark" },
		{ TEXT("Destroyed"), { "sombre.node_destroyed", "sombre.light_decided" }, "done_dark" },
		{ TEXT("Untouched"), {}, "done_dark" },
	};
	for (const FOutcome& Outcome : Outcomes)
	{
		FSombre S;
		S.ReachKnows();
		for (const FName Act : Outcome.Acts)
		{
			S.Set(Act);
		}
		TestEqual(FString::Printf(TEXT("%s: still knows before the meeting"), Outcome.Label), S.Stage(Characteristic), FName(TEXT("knows")));
		S.Set(TEXT("sombre.meeting_done"));
		TestEqual(FString::Printf(TEXT("%s: outcome stage"), Outcome.Label), S.Stage(Characteristic), Outcome.Expected);
		TestTrue(FString::Printf(TEXT("%s: complete"), Outcome.Label), S.Quests->IsComplete(Characteristic));
		TestTrue(FString::Printf(TEXT("%s: the second storm"), Outcome.Label), S.Flag(TEXT("sombre.storm")));
		TestTrue(FString::Printf(TEXT("%s: Hale stays"), Outcome.Label), S.Flag(TEXT("sombre.hale_arrived")));
	}

	// Flags set before Varga starts the quest chain it forward (the player explored first).
	{
		FSombre S;
		S.WorldState->DiscoverLocation(TEXT("sombre.light"));
		for (const TCHAR* Fact : { TEXT("sombre.cable_cut_found"), TEXT("sombre.vault_opened"), TEXT("sombre.panel_read"), TEXT("sombre.liv_note_found") })
		{
			S.Set(Fact);
		}
		S.Set(TEXT("sombre.storm"));
		S.Quests->StartQuest(Characteristic);
		TestEqual(TEXT("Explored first: starts and chains to knows"), S.Stage(Characteristic), FName(TEXT("knows")));
		TestFalse(TEXT("Explored first: the storm still clears"), S.Flag(TEXT("sombre.storm")));
	}

	// False Light: the evidence approach (no build), closed by Varga.
	{
		FSombre S;
		S.Quests->StartQuest(FalseLight);
		TestEqual(TEXT("FL asked"), S.Stage(FalseLight), FName(TEXT("asked")));
		S.WorldState->DiscoverLocation(TEXT("sombre.headland"));
		TestEqual(TEXT("FL found"), S.Stage(FalseLight), FName(TEXT("found")));
		S.Give(TEXT("false_lantern"));
		TestEqual(TEXT("FL named by the lantern"), S.Stage(FalseLight), FName(TEXT("named")));
		S.Set(TEXT("sombre.varga_told"));
		TestEqual(TEXT("FL done by Varga"), S.Stage(FalseLight), FName(TEXT("done")));
		TestTrue(TEXT("FL complete"), S.Quests->IsComplete(FalseLight));
		TestTrue(TEXT("FL done sets false_light_taken"), S.Flag(TEXT("sombre.false_light_taken")));
	}
	// The confession approach, closed at the loft.
	{
		FSombre S;
		S.Quests->StartQuest(FalseLight);
		S.WorldState->DiscoverLocation(TEXT("sombre.headland"));
		S.Set(TEXT("sombre.dell_confessed"));
		TestEqual(TEXT("FL named by the confession"), S.Stage(FalseLight), FName(TEXT("named")));
		S.Set(TEXT("sombre.exposed_pruitts"));
		TestEqual(TEXT("FL done at the loft"), S.Stage(FalseLight), FName(TEXT("done")));
		TestTrue(TEXT("FL loft close sets false_light_taken"), S.Flag(TEXT("sombre.false_light_taken")));
	}
	// Confessed at the shed before the headland, and told before the quest existed: chains to done.
	{
		FSombre S;
		S.Set(TEXT("sombre.dell_confessed"));
		S.Set(TEXT("sombre.varga_told"));
		S.Quests->StartQuest(FalseLight);
		TestEqual(TEXT("FL waits for the headland"), S.Stage(FalseLight), FName(TEXT("asked")));
		S.WorldState->DiscoverLocation(TEXT("sombre.headland"));
		TestEqual(TEXT("FL chains to done"), S.Stage(FalseLight), FName(TEXT("done")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreDialogueTest, "DeadCurrent.Content.Sombre.Dialogue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreDialogueTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreContentTest;
	LoadContent();

	// ---- Static checks over the nine assets. ----
	for (const FDialogueSpec& Spec : DialogueSpecs())
	{
		const UDCDialogueAsset* Asset = LoadDialogue(Spec.Asset);
		if (!TestNotNull(FString::Printf(TEXT("%s exists"), Spec.Asset), Asset))
		{
			return false;
		}
		TestEqual(FString::Printf(TEXT("%s id"), Spec.Asset), Asset->DialogueId, FName(Spec.DialogueId));

		TArray<FName> Entries;
		for (const FDCDialogueEntry& Entry : Asset->Entries)
		{
			Entries.Add(Entry.NodeId);
		}
		TestTrue(FString::Printf(TEXT("%s entries are the ledger's, in order"), Spec.Asset), Entries == Spec.Entries);
		TestTrue(FString::Printf(TEXT("%s entry list ends unconditional"), Spec.Asset),
			Asset->Entries.Num() > 0 && Asset->Entries.Last().Conditions.IsEmpty());
		TestNotNull(FString::Printf(TEXT("%s default entry node exists"), Spec.Asset), Asset->FindNode(Asset->EntryNodeId));

		const bool bMayMentionTie = Spec.DialogueId == FString(TEXT("sombre_mara")) || Spec.DialogueId == FString(TEXT("sombre_odette"))
			|| Spec.DialogueId == FString(TEXT("sombre_marthe"));
		for (const FDCDialogueNode& Node : Asset->Nodes)
		{
			const FString Where = FString::Printf(TEXT("%s.%s"), Spec.DialogueId, *Node.NodeId.ToString());
			TestTrue(FString::Printf(TEXT("%s has an unconditional choice"), *Where),
				Node.Choices.ContainsByPredicate([](const FDCDialogueChoice& C) { return C.Conditions.IsEmpty(); }));
			TestFalse(FString::Printf(TEXT("%s has a speaker"), *Where), Node.Speaker.IsEmpty());

			bool bTieVariant = false;
			bool bNoTiePartner = false;
			for (const FDCDialogueChoice& Choice : Node.Choices)
			{
				const FString What = FString::Printf(TEXT("%s '%s'"), *Where, *Choice.Text.ToString());
				for (const FDCGameplayCondition& C : Choice.Conditions)
				{
					if (C.Type == EDCConditionType::WorldFlag)
					{
						TestTrue(FString::Printf(TEXT("%s reads ledger flag %s"), *What, *C.Id.ToString()), LedgerFlags().Contains(C.Id));
						bTieVariant |= IsTieFlag(C.Id) && !C.bNegate;
					}
				}
				bNoTiePartner |= IsNoTie(Choice.Conditions);
				for (const FDCGameplayConsequence& C : Choice.Consequences)
				{
					if (C.Type == EDCConsequenceType::SetWorldFlag || C.Type == EDCConsequenceType::ClearWorldFlag)
					{
						TestTrue(FString::Printf(TEXT("%s writes ledger flag %s"), *What, *C.Id.ToString()), LedgerFlags().Contains(C.Id));
					}
					// Evidence (and every other hand-over) needs the item it takes.
					if (C.Type == EDCConsequenceType::RemoveItem)
					{
						TestTrue(FString::Printf(TEXT("%s requires the %s it removes"), *What, *C.Id.ToString()),
							HasCondition(Choice.Conditions, EDCConditionType::HasItem, C.Id, false));
					}
					// Keeper choices exclude each other and a light on the Line.
					if (C.Type == EDCConsequenceType::SetWorldFlag && (C.Id == "sombre.keeper_odette" || C.Id == "sombre.keeper_dell"))
					{
						for (const FName Guard : { FName("sombre.keeper_odette"), FName("sombre.keeper_dell"), FName("sombre.light_line") })
						{
							TestTrue(FString::Printf(TEXT("%s is guarded by !%s"), *What, *Guard.ToString()),
								HasCondition(Choice.Conditions, EDCConditionType::WorldFlag, Guard, true));
						}
					}
					if (C.Type == EDCConsequenceType::StartQuest)
					{
						TestTrue(FString::Printf(TEXT("%s: only Varga's first starts quests"), *What),
							Spec.DialogueId == FString(TEXT("sombre_varga")) && Node.NodeId == "first");
					}
					if (!bMayMentionTie)
					{
						TestFalse(FString::Printf(TEXT("%s never sets a tie"), *What), IsTieFlag(C.Id));
					}
				}
				if (!bMayMentionTie)
				{
					TestFalse(FString::Printf(TEXT("%s never reads a tie"), *What),
						Choice.Conditions.ContainsByPredicate([](const FDCGameplayCondition& C) { return IsTieFlag(C.Id); }));
				}
			}
			// No tie leak: a node that varies a line by tie always has the NO_TIE partner.
			if (bTieVariant)
			{
				TestTrue(FString::Printf(TEXT("%s: every tie variant has a NO_TIE partner"), *Where), bNoTiePartner);
			}
			if (Spec.DialogueId == FString(TEXT("sombre_marthe")) && Node.NodeId != "r_liv")
			{
				TestFalse(FString::Printf(TEXT("%s: Marthe mentions the tie only in the meeting"), *Where), bTieVariant);
			}
		}
	}
	for (const TCHAR* Alias : { TEXT("DA_Dialogue_SombrePruitts"), TEXT("DA_Dialogue_SombreMeeting") })
	{
		TestNull(FString::Printf(TEXT("Alias %s is not created"), Alias), LoadDialogue(Alias));
	}

	// Varga's first conversation: every reply starts both quests.
	for (const TCHAR* Reply : { TEXT("What do you need?"), TEXT("Liv came through here."), TEXT("I'll find out why.") })
	{
		FSombre S;
		TestEqual(FString::Printf(TEXT("Varga first (%s)"), Reply), S.Talk(TEXT("DA_Dialogue_SombreVarga")), FName(TEXT("first")));
		TestTrue(FString::Printf(TEXT("Reply %s"), Reply), S.Say(Reply));
		TestEqual(FString::Printf(TEXT("%s starts the main quest"), Reply), S.Stage(Characteristic), FName(TEXT("arrived")));
		TestEqual(FString::Printf(TEXT("%s starts False Light"), Reply), S.Stage(FalseLight), FName(TEXT("asked")));
		TestEqual(FString::Printf(TEXT("%s: then she is waiting"), Reply), S.Talk(TEXT("DA_Dialogue_SombreVarga")), FName(TEXT("waiting")));
	}

	// Mara's tie question: exactly one tie flag or none, asked once.
	struct FTieAnswer { const TCHAR* Text; FName Flag; };
	for (const FTieAnswer& Answer : TArray<FTieAnswer>{ { TEXT("My sister."), Sister }, { TEXT("My partner."), Partner },
		{ TEXT("She took me in when nobody else would."), TookIn }, { TEXT("Does it matter?"), NAME_None } })
	{
		FSombre S;
		TestEqual(FString::Printf(TEXT("Crossing asks (%s)"), Answer.Text), S.Talk(TEXT("DA_Dialogue_SombreMara")), FName(TEXT("crossing_tie")));
		TestTrue(FString::Printf(TEXT("Answer %s"), Answer.Text), S.Say(Answer.Text));
		TestEqual(FString::Printf(TEXT("%s: tie flags set"), Answer.Text), S.TieFlags(), Answer.Flag.IsNone() ? 0 : 1);
		TestTrue(FString::Printf(TEXT("%s: the right one"), Answer.Text), Answer.Flag.IsNone() || S.Flag(Answer.Flag));
		TestTrue(FString::Printf(TEXT("%s: asked"), Answer.Text), S.Flag(TEXT("sombre.mara_tie_asked")));
		TestEqual(FString::Printf(TEXT("%s: not asked again"), Answer.Text), S.Talk(TEXT("DA_Dialogue_SombreMara")), FName(TEXT("crossing")));
	}

	// Her crossing lines stay on the crossing: after the strike, before Varga, she has her island line (change #17).
	{
		FSombre S;
		S.Set(TEXT("sombre.reef_struck"));
		TestEqual(TEXT("After the strike, before Varga: Mara's island line"), S.Talk(TEXT("DA_Dialogue_SombreMara")), FName(TEXT("town")));
	}

	// No tie leak: under each tie, and under none, each keystone node shows exactly one tie line, and the right one.
	struct FTieCase { const TCHAR* Label; FName Flag; const TCHAR* OdetteLine; const TCHAR* LoftLine; FName PromiseNode; };
	const TArray<FTieCase> Ties = {
		{ TEXT("sister"), Sister, TEXT("She's my sister."), TEXT("She's my sister."), "promise_sister" },
		{ TEXT("partner"), Partner, TEXT("She's my partner."), TEXT("She's my partner."), "promise_partner" },
		{ TEXT("took in"), TookIn, TEXT("She took me in when nobody would."), TEXT("She took me in."), "promise_took_in" },
		{ TEXT("no tie"), NAME_None, TEXT("She's why I came."), TEXT("She's why I came."), "promise_neutral" },
	};
	const TArray<FString> OdetteTieLines = { TEXT("She's my sister."), TEXT("She's my partner."), TEXT("She took me in when nobody would."), TEXT("She's why I came.") };
	const TArray<FString> LoftTieLines = { TEXT("She's my sister."), TEXT("She's my partner."), TEXT("She took me in."), TEXT("She's why I came.") };
	for (const FTieCase& Tie : Ties)
	{
		FSombre S;
		if (!Tie.Flag.IsNone())
		{
			S.Set(Tie.Flag);
		}
		auto CountOf = [&S](const TArray<FString>& Lines)
		{
			int32 N = 0;
			for (const FString& Line : Lines)
			{
				N += S.Count(Line);
			}
			return N;
		};

		// Odette's key: the tie line needs Liv's letter.
		S.Talk(TEXT("DA_Dialogue_SombreOdette"));
		S.Say(TEXT("A listener came through here four months ago."));
		TestEqual(FString::Printf(TEXT("%s: no key line without the letter"), Tie.Label), CountOf(OdetteTieLines), 0);
		S.Give(TEXT("liv_letter"));
		S.Talk(TEXT("DA_Dialogue_SombreOdette"));
		S.Say(TEXT("A listener came through here four months ago."));
		TestEqual(FString::Printf(TEXT("%s: Odette shows one tie line"), Tie.Label), CountOf(OdetteTieLines), 1);
		TestTrue(FString::Printf(TEXT("%s: Odette's key line"), Tie.Label), S.Say(Tie.OdetteLine));
		TestEqual(FString::Printf(TEXT("%s: the key"), Tie.Label), S.Has(TEXT("sombre_vault_key")), 1);
		S.Talk(TEXT("DA_Dialogue_SombreOdette"));
		S.Say(TEXT("A listener came through here four months ago."));
		TestEqual(FString::Printf(TEXT("%s: the key is given once"), Tie.Label), CountOf(OdetteTieLines), 0);

		// The meeting's tie line.
		S.ReachKnows();
		S.Give(TEXT("liv_note"));
		S.OpenMeeting();
		TestTrue(FString::Printf(TEXT("%s: give the note"), Tie.Label), S.Say(TEXT("The listener pulled the card. She wrote to Odette.")));
		TestEqual(FString::Printf(TEXT("%s: r_liv"), Tie.Label), S.Node(), FName(TEXT("r_liv")));
		TestEqual(FString::Printf(TEXT("%s: the loft shows one tie line"), Tie.Label), CountOf(LoftTieLines), 1);
		TestTrue(FString::Printf(TEXT("%s: name the tie"), Tie.Label), S.Say(Tie.LoftLine));
		TestTrue(FString::Printf(TEXT("%s: named"), Tie.Label), S.Flag(TEXT("sombre.player_named_tie")));
		S.Say(TEXT("Go on."));
		S.Say(TEXT("That's all."));

		// Mara's promise that night.
		TestEqual(FString::Printf(TEXT("%s: Mara's promise"), Tie.Label), S.Talk(TEXT("DA_Dialogue_SombreMara")), FName(TEXT("promise")));
		TestEqual(FString::Printf(TEXT("%s: one promise line"), Tie.Label),
			S.Count(TEXT("She said I'd come?")) + S.Count(TEXT("She said someone would come?")), 1);
		S.Say(Tie.Flag.IsNone() ? TEXT("She said someone would come?") : TEXT("She said I'd come?"));
		TestEqual(FString::Printf(TEXT("%s: the right promise"), Tie.Label), S.Node(), Tie.PromiseNode);
		TestTrue(FString::Printf(TEXT("%s: promise told"), Tie.Label), S.Flag(TEXT("shore.promise_told")));
		TestEqual(FString::Printf(TEXT("%s: the promise is told once"), Tie.Label), S.Talk(TEXT("DA_Dialogue_SombreMara")), FName(TEXT("after")));
	}

	// Marthe's call (plan §8.2 #4): only at knows, and the meeting needs it.
	{
		FSombre S;
		S.Talk(TEXT("DA_Dialogue_SombreMarthe"));
		TestFalse(TEXT("No call before knows"), S.Can(TEXT("Call the island to the loft.")));
		S.ReachKnows();
		TestEqual(TEXT("At knows, before the call: the store"), S.Talk(TEXT("DA_Dialogue_SombreMarthe")), FName(TEXT("store")));
		TestTrue(TEXT("The call"), S.Say(TEXT("Call the island to the loft.")));
		TestTrue(TEXT("meeting_called"), S.Flag(TEXT("sombre.meeting_called")));
		TestEqual(TEXT("After the call: the meeting"), S.Talk(TEXT("DA_Dialogue_SombreMarthe")), FName(TEXT("loft")));
		TestTrue(TEXT("Not yet ends it"), S.Say(TEXT("Not yet.")));
		TestFalse(TEXT("Not yet sets nothing"), S.Flag(TEXT("sombre.meeting_done")));

		// "It's done." and "Leave it as she left it." are exclusive on light_decided.
		S.Talk(TEXT("DA_Dialogue_SombreMarthe"));
		TestTrue(TEXT("Undecided: leave it"), S.Can(TEXT("Leave it as she left it.")) && !S.Can(TEXT("It's done.")));
		S.Set(TEXT("sombre.light_decided"));
		S.Talk(TEXT("DA_Dialogue_SombreMarthe"));
		TestTrue(TEXT("Decided: it's done"), S.Can(TEXT("It's done.")) && !S.Can(TEXT("Leave it as she left it.")));
	}

	// The meeting's evidence: each needs its item, removes it, and sets its flag; forgiveness needs both.
	{
		FSombre S;
		S.ReachKnows();
		S.WorldState->DiscoverLocation(TEXT("sombre.headland"));
		for (const TCHAR* Item : { TEXT("false_lantern"), TEXT("vault_access_log"), TEXT("liv_note"), TEXT("cut_cable_end") })
		{
			S.Give(Item);
		}
		TestEqual(TEXT("False Light named by the lantern"), S.Stage(FalseLight), FName(TEXT("named")));
		S.OpenMeeting();
		TestEqual(TEXT("Evidence"), S.Node(), FName(TEXT("evidence")));

		struct FEvidence { const TCHAR* Line; const TCHAR* Item; FName Exposed; FName Reaction; };
		const TArray<FEvidence> Evidence = {
			{ TEXT("This was burning on the west head the night the Ida hit."), TEXT("false_lantern"), "sombre.exposed_pruitts", "r_pruitts" },
			{ TEXT("Four months ago someone let a stranger into the vault."), TEXT("vault_access_log"), "sombre.exposed_odette", "r_odette" },
			{ TEXT("The listener pulled the card. She wrote to Odette."), TEXT("liv_note"), "sombre.exposed_liv", "r_liv" },
			{ TEXT("Someone made sure it could never be fixed."), TEXT("cut_cable_end"), "sombre.exposed_sigrun", "r_sigrun" },
		};
		for (const FEvidence& E : Evidence)
		{
			TestTrue(FString::Printf(TEXT("Give %s"), E.Item), S.Say(E.Line));
			TestEqual(FString::Printf(TEXT("%s: reaction"), E.Item), S.Node(), E.Reaction);
			TestEqual(FString::Printf(TEXT("%s: removed"), E.Item), S.Has(E.Item), 0);
			TestTrue(FString::Printf(TEXT("%s: exposed"), E.Item), S.Flag(E.Exposed));
			if (E.Reaction == "r_liv")
			{
				TestTrue(TEXT("Board and note: forgive Odette"), S.Say(TEXT("Odette let her in. Liv pulled the card.")));
				TestTrue(TEXT("odette_forgiven"), S.Flag(TEXT("sombre.odette_forgiven")));
			}
			if (E.Reaction == "r_sigrun")
			{
				TestTrue(TEXT("Sigrun's reply"), S.Say(TEXT("Go on.")));
				TestEqual(TEXT("Sigrun speaks"), S.Node(), FName(TEXT("r_sigrun_reply")));
			}
			S.Say(TEXT("Go on."));
			TestEqual(FString::Printf(TEXT("%s: back to evidence"), E.Item), S.Node(), FName(TEXT("evidence")));
			TestFalse(FString::Printf(TEXT("%s: cannot be given twice"), E.Item), S.Can(E.Line));
		}
		TestEqual(TEXT("False Light closed at the loft"), S.Stage(FalseLight), FName(TEXT("done")));
		TestTrue(TEXT("That's all"), S.Say(TEXT("That's all.")));
		TestTrue(TEXT("meeting_done"), S.Flag(TEXT("sombre.meeting_done")));
		TestEqual(TEXT("Untouched: done_dark"), S.Stage(Characteristic), FName(TEXT("done_dark")));
		TestEqual(TEXT("Marthe after"), S.Talk(TEXT("DA_Dialogue_SombreMarthe")), FName(TEXT("after_dark")));

		// The VS-09 wiring fix: a forgiven Odette can still be asked to keep the light after the meeting.
		TestEqual(TEXT("Forgiven Odette: after"), S.Talk(TEXT("DA_Dialogue_SombreOdette")), FName(TEXT("after")));
		TestTrue(TEXT("Forgiven Odette keeps it"), S.Say(TEXT("Will you keep it? By hand.")));
		TestTrue(TEXT("keeper_odette"), S.Flag(TEXT("sombre.keeper_odette")));
		TestEqual(TEXT("Then she keeps it"), S.Talk(TEXT("DA_Dialogue_SombreOdette")), FName(TEXT("after_keeper")));
	}
	// Exposed without the note: disgraced, and no offer.
	{
		FSombre S;
		S.ReachKnows();
		S.Give(TEXT("vault_access_log"));
		S.OpenMeeting();
		S.Say(TEXT("Four months ago someone let a stranger into the vault."));
		TestFalse(TEXT("No forgiveness without the note"), S.Can(TEXT("She let her in. Liv's the one who pulled the card.")));
		S.Say(TEXT("Go on."));
		S.Say(TEXT("That's all."));
		TestEqual(TEXT("Disgraced"), S.Talk(TEXT("DA_Dialogue_SombreOdette")), FName(TEXT("after_disgraced")));
		TestFalse(TEXT("Disgraced: no keeper offer"), S.Can(TEXT("Will you keep it? By hand.")));
	}

	// Keepers exclude each other, and a light on the Line needs none.
	{
		FSombre S;
		S.ReachKnows();
		S.Set(TEXT("sombre.dell_confessed"));
		TestEqual(TEXT("Odette at knows"), S.Talk(TEXT("DA_Dialogue_SombreOdette")), FName(TEXT("knows")));
		TestTrue(TEXT("Odette keeps it"), S.Say(TEXT("Will you keep it? By hand. Oil and the old clockwork.")));
		TestEqual(TEXT("Dell confessed"), S.Talk(TEXT("DA_Dialogue_SombreDell")), FName(TEXT("confessed")));
		TestFalse(TEXT("Dell not offered once Odette keeps it"), S.Can(TEXT("Keep the real light instead.")));
	}
	{
		FSombre S;
		S.ReachKnows();
		S.Set(TEXT("sombre.dell_confessed"));
		S.Talk(TEXT("DA_Dialogue_SombreDell"));
		TestTrue(TEXT("Offer Dell"), S.Say(TEXT("Keep the real light instead.")));
		TestTrue(TEXT("Dell accepts"), S.Say(TEXT("It stops.")));
		TestTrue(TEXT("keeper_dell"), S.Flag(TEXT("sombre.keeper_dell")));
		TestEqual(TEXT("Dell keeps it"), S.Talk(TEXT("DA_Dialogue_SombreDell")), FName(TEXT("keeper")));
		S.Talk(TEXT("DA_Dialogue_SombreOdette"));
		TestFalse(TEXT("Odette not offered once Dell keeps it"), S.Can(TEXT("Will you keep it? By hand. Oil and the old clockwork.")));
	}
	{
		FSombre S;
		S.ReachKnows();
		S.Set(TEXT("sombre.dell_confessed"));
		S.Set(TEXT("sombre.light_line"));
		S.Talk(TEXT("DA_Dialogue_SombreOdette"));
		TestFalse(TEXT("On the Line: no Odette offer"), S.Can(TEXT("Will you keep it? By hand. Oil and the old clockwork.")));
		S.Talk(TEXT("DA_Dialogue_SombreDell"));
		TestFalse(TEXT("On the Line: no Dell offer"), S.Can(TEXT("Keep the real light instead.")));
	}

	// Dell's confession (§8.2 #1): Persuasion 2 or Survival 2; the reworded clause, and no rain.
	{
		const FString Boots = TEXT("Your boots are wet to the knee. Nobody wades the reef for fish.");
		FSombre S;
		S.Talk(TEXT("DA_Dialogue_SombreDell"));
		TestEqual(TEXT("Zero build: only goodbye"), S.Visible().Num(), 1);
		S.Skill(TEXT("Skill.Survival"), 2);
		S.Talk(TEXT("DA_Dialogue_SombreDell"));
		TestTrue(TEXT("Survival 2 reads the boots"), S.Can(Boots));
		TestFalse(TEXT("Survival alone is not Persuasion"), S.Can(TEXT("That isn't all.")));
		TestTrue(TEXT("Wet boots"), S.Say(Boots));
		TestTrue(TEXT("Confessed by Survival"), S.Flag(TEXT("sombre.dell_confessed")));

		FSombre P;
		P.Skill(TEXT("Skill.Persuasion"), 2);
		P.Talk(TEXT("DA_Dialogue_SombreDell"));
		TestTrue(TEXT("Persuasion 2"), P.Say(TEXT("That isn't all.")));
		TestTrue(TEXT("Confessed by Persuasion"), P.Flag(TEXT("sombre.dell_confessed")));

		for (const FDCDialogueNode& Node : LoadDialogue(TEXT("DA_Dialogue_SombreDell"))->Nodes)
		{
			for (const FDCDialogueChoice& Choice : Node.Choices)
			{
				TestFalse(TEXT("No rain clause left"), Choice.Text.ToString().Contains(TEXT("rained")));
			}
		}
	}

	// Zero investment: no skills, every step still possible (the cells' world acts stood in for by their flags).
	{
		FSombre S;
		S.Talk(TEXT("DA_Dialogue_SombreMara"));
		S.Say(TEXT("Does it matter?"));
		S.Say(TEXT("..."));
		S.Say(TEXT("Hold on to something."));
		S.Set(TEXT("sombre.reef_struck"));
		S.Set(TEXT("sombre.storm"));
		S.Give(TEXT("liv_letter"));
		S.Talk(TEXT("DA_Dialogue_SombreVarga"));
		S.Say(TEXT("What do you need?"));
		S.WorldState->DiscoverLocation(TEXT("sombre.light"));
		S.Set(TEXT("sombre.cable_cut_found"));
		S.Give(TEXT("cut_cable_end"));
		TestEqual(TEXT("Zero: vault stage"), S.Stage(Characteristic), FName(TEXT("vault")));

		// In by Odette's key with no tie at all ("She's why I came."), or by Sigrun with the cable end.
		S.Talk(TEXT("DA_Dialogue_SombreOdette"));
		S.Say(TEXT("A listener came through here four months ago."));
		TestTrue(TEXT("Zero, no tie: the key is still offered"), S.Can(TEXT("She's why I came.")));
		TestEqual(TEXT("Zero: Sigrun first"), S.Talk(TEXT("DA_Dialogue_SombreSigrun")), FName(TEXT("first")));
		TestTrue(TEXT("Zero: confront Sigrun"), S.Say(TEXT("I found the lamp's feed cut. Clean. Shears, not a saw.")));
		TestTrue(TEXT("Zero: Sigrun gets you in"), S.Say(TEXT("Get me into the vault.")));
		TestTrue(TEXT("Zero: vault_opened by Sigrun"), S.Flag(TEXT("sombre.vault_opened")));
		TestEqual(TEXT("Zero: inside"), S.Stage(Characteristic), FName(TEXT("vault_inside")));
		S.Set(TEXT("sombre.panel_read"));
		S.Set(TEXT("sombre.liv_note_found"));
		S.Give(TEXT("liv_note"));
		TestEqual(TEXT("Zero: knows"), S.Stage(Characteristic), FName(TEXT("knows")));

		// Hale: his crew splices the feed, and he gives the card and oil, all with no skill.
		TestEqual(TEXT("Zero: Hale first"), S.Talk(TEXT("DA_Dialogue_SombreHale")), FName(TEXT("first")));
		S.Say(TEXT("Section Fourteen. It's trying to come back."));
		TestTrue(TEXT("hale_met"), S.Flag(TEXT("sombre.hale_met")));
		S.Say(TEXT("Go on."));
		TestTrue(TEXT("Zero: the card"), S.Say(TEXT("Give me the card.")));
		TestEqual(TEXT("Zero: card given"), S.Has(TEXT("section_key_compact")), 1);
		S.Say(TEXT("Go on."));
		TestTrue(TEXT("Zero: the crew"), S.Say(TEXT("Send your crew up to splice the feed.")));
		TestTrue(TEXT("hale_crew_up"), S.Flag(TEXT("sombre.hale_crew_up")));
		S.Say(TEXT("Go on."));
		TestTrue(TEXT("Zero: oil"), S.Say(TEXT("I need oil for a hand light.")));
		TestEqual(TEXT("Zero: oil given"), S.Has(TEXT("lamp_oil")), 1);
		S.Say(TEXT("Go on."));
		TestFalse(TEXT("Card once while carried"), S.Can(TEXT("Give me the card.")));
		TestFalse(TEXT("Crew once"), S.Can(TEXT("Send your crew up to splice the feed.")));
		TestTrue(TEXT("Copper buyer known"), S.Can(TEXT("The copper buyer.")));

		// False Light by the evidence approach, named to Varga.
		S.WorldState->DiscoverLocation(TEXT("sombre.headland"));
		S.Give(TEXT("false_lantern"));
		TestEqual(TEXT("Zero: Varga waiting"), S.Talk(TEXT("DA_Dialogue_SombreVarga")), FName(TEXT("waiting")));
		TestFalse(TEXT("Without a confession, no confession line"), S.Can(TEXT("It was the Pruitts. Tem and Dell hold the lantern.")));
		TestTrue(TEXT("Zero: name them with the lantern"), S.Say(TEXT("It was the Pruitts. Here's their lantern.")));
		TestEqual(TEXT("Zero: False Light done"), S.Stage(FalseLight), FName(TEXT("done")));
		TestEqual(TEXT("Shown, not handed over"), S.Has(TEXT("false_lantern")), 1);

		// Decide (the seat-the-card act, by the cell), call, meet, and end on the Line.
		S.Set(TEXT("sombre.feed_spliced"));
		S.Inventory->RemoveItem(UDCItemDefinition::FindByItemId(TEXT("section_key_compact")), 1);
		S.Set(TEXT("sombre.light_line"));
		S.Set(TEXT("sombre.light_decided"));
		S.OpenMeeting();
		TestEqual(TEXT("Zero: the meeting"), S.Node(), FName(TEXT("evidence")));
		S.Say(TEXT("That's all."));
		TestEqual(TEXT("Zero: done_line"), S.Stage(Characteristic), FName(TEXT("done_line")));
		TestEqual(TEXT("Zero: Varga after the Line"), S.Talk(TEXT("DA_Dialogue_SombreVarga")), FName(TEXT("after_line")));
		TestTrue(TEXT("Where next"), S.Say(TEXT("Where next?")));
		TestTrue(TEXT("The default end-card trigger"), S.Flag(TEXT("sombre.slice_end")));
		TestEqual(TEXT("Hale after the Line"), S.Talk(TEXT("DA_Dialogue_SombreHale")), FName(TEXT("after_line")));
		TestEqual(TEXT("Jonas after the Line"), S.Talk(TEXT("DA_Dialogue_SombreJonas")), FName(TEXT("after_line")));
		TestEqual(TEXT("Mara after the Line"), S.Talk(TEXT("DA_Dialogue_SombreMara")), FName(TEXT("after_line")));
	}

	// Sigrun's help, and the sea cock's helper is refused once the light is on the Line.
	{
		FSombre S;
		S.Give(TEXT("cut_cable_end"));
		S.Talk(TEXT("DA_Dialogue_SombreSigrun"));
		S.Say(TEXT("I found the lamp's feed cut. Clean. Shears, not a saw."));
		TestTrue(TEXT("Ask about the vault"), S.Say(TEXT("What would you do with the vault?")));
		TestTrue(TEXT("Do it with me"), S.Say(TEXT("Do it with me.")));
		TestTrue(TEXT("sigrun_helping"), S.Flag(TEXT("sombre.sigrun_helping")));
		TestEqual(TEXT("Helping"), S.Talk(TEXT("DA_Dialogue_SombreSigrun")), FName(TEXT("helping")));

		FSombre L;
		L.Set(TEXT("sombre.sigrun_revealed"));
		L.Set(TEXT("sombre.light_line"));
		L.Set(TEXT("sombre.vault_opened"));
		TestEqual(TEXT("Revealed"), L.Talk(TEXT("DA_Dialogue_SombreSigrun")), FName(TEXT("revealed")));
		TestFalse(TEXT("Vault already open: no unjam"), L.Can(TEXT("Get me into the vault.")));
		L.Say(TEXT("About the vault."));
		TestFalse(TEXT("On the Line: Sigrun won't flood it"), L.Can(TEXT("Do it with me.")));
	}
	return true;
}

#endif
