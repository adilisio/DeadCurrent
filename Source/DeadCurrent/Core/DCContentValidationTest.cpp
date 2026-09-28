#include "AssetRegistry/IAssetRegistry.h"
#include "Core/DCContentSubsystem.h"
#include "Core/DCGameplayRules.h"
#include "Dialogue/DCDialogueAsset.h"
#include "Engine/AssetManager.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestDefinition.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/DataValidation.h"

/**
 *  Every dialogue and quest asset in the project: graph checks (IsDataValid) plus every condition
 *  and consequence resolving to real quests, stages and items. Catches content typos before a playtest.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCContentValidationTest, "DeadCurrent.Content.Validate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDCContentValidationTest::RunTest(const FString& Parameters)
{
	TArray<UObject*> Definitions;
	UDCContentSubsystem::LoadAllDefinitions(Definitions);

	IAssetRegistry& Registry = IAssetRegistry::GetChecked();
	Registry.ScanPathsSynchronous({ TEXT("/Game/Dialogue") }, false);
	TArray<FAssetData> DialogueAssets;
	Registry.GetAssetsByClass(UDCDialogueAsset::StaticClass()->GetClassPathName(), DialogueAssets, true);

	int32 QuestCount = 0;
	for (UObject* Object : Definitions)
	{
		const UDCQuestDefinition* Quest = Cast<UDCQuestDefinition>(Object);
		if (!Quest)
		{
			continue;
		}
		++QuestCount;

		FDataValidationContext Context;
		if (Quest->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			for (const FDataValidationContext::FIssue& Issue : Context.GetIssues())
			{
				AddError(FString::Printf(TEXT("%s: %s"), *Quest->GetName(), *Issue.Message.ToString()));
			}
		}

		TArray<FString> Problems;
		for (const FDCQuestStage& Stage : Quest->Stages)
		{
			for (const FDCGameplayConsequence& Consequence : Stage.OnEnter)
			{
				UDCGameplayRules::ValidateReferences(Consequence, Problems);
			}
			for (const FDCQuestTransition& Transition : Stage.Transitions)
			{
				for (const FDCGameplayCondition& Condition : Transition.Conditions)
				{
					UDCGameplayRules::ValidateReferences(Condition, Problems);
				}
			}
		}
		for (const FString& Problem : Problems)
		{
			AddError(FString::Printf(TEXT("%s: %s"), *Quest->GetName(), *Problem));
		}
	}

	for (const FAssetData& AssetData : DialogueAssets)
	{
		const UDCDialogueAsset* Dialogue = Cast<UDCDialogueAsset>(AssetData.GetAsset());
		if (!Dialogue)
		{
			continue;
		}

		FDataValidationContext Context;
		if (Dialogue->IsDataValid(Context) == EDataValidationResult::Invalid)
		{
			for (const FDataValidationContext::FIssue& Issue : Context.GetIssues())
			{
				AddError(FString::Printf(TEXT("%s: %s"), *Dialogue->GetName(), *Issue.Message.ToString()));
			}
		}

		TArray<FString> Problems;
		for (const FDCDialogueEntry& Entry : Dialogue->Entries)
		{
			for (const FDCGameplayCondition& Condition : Entry.Conditions)
			{
				UDCGameplayRules::ValidateReferences(Condition, Problems);
			}
		}
		for (const FDCDialogueNode& Node : Dialogue->Nodes)
		{
			for (const FDCDialogueChoice& Choice : Node.Choices)
			{
				for (const FDCGameplayCondition& Condition : Choice.Conditions)
				{
					UDCGameplayRules::ValidateReferences(Condition, Problems);
				}
				for (const FDCGameplayConsequence& Consequence : Choice.Consequences)
				{
					UDCGameplayRules::ValidateReferences(Consequence, Problems);
				}
			}
		}
		for (const FString& Problem : Problems)
		{
			AddError(FString::Printf(TEXT("%s: %s"), *Dialogue->GetName(), *Problem));
		}
	}

	TestTrue(TEXT("Found quest definitions"), QuestCount > 0);

	// Quests, items and dialogue are found by id at runtime, so only the Asset Manager rules in
	// DefaultGame.ini get them cooked into packaged builds. Every asset must be registered there.
	auto CountRegistered = [](const TCHAR* Type)
	{
		TArray<FPrimaryAssetId> Ids;
		UAssetManager::Get().GetPrimaryAssetIdList(FPrimaryAssetType(Type), Ids);
		return Ids.Num();
	};
	const int32 ItemCount = Definitions.FilterByPredicate([](const UObject* O) { return O->IsA<UDCItemDefinition>(); }).Num();
	TestEqual(TEXT("Asset Manager registers every quest"), CountRegistered(TEXT("DCQuestDefinition")), QuestCount);
	TestEqual(TEXT("Asset Manager registers every item"), CountRegistered(TEXT("DCItemDefinition")), ItemCount);
	TestEqual(TEXT("Asset Manager registers every dialogue"), CountRegistered(TEXT("DCDialogueAsset")), DialogueAssets.Num());
	TestTrue(TEXT("Found dialogue assets"), DialogueAssets.Num() > 0);
	AddInfo(FString::Printf(TEXT("Validated %d quests and %d dialogues"), QuestCount, DialogueAssets.Num()));
	return true;
}

#endif
