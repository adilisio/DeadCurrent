#include "Character/DCCharacterProgressionComponent.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCTestHelpers.h"
#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Interaction/DCInteractable.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "World/DCInspectableActor.h"
#include "UObject/UnrealType.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace DCProgressionTest
{
	FGameplayTag Attr(const TCHAR* Name)
	{
		return UDCCharacterProgressionComponent::AttributeTag(Name);
	}

	FGameplayTag Skill(const TCHAR* Name)
	{
		return UDCCharacterProgressionComponent::SkillTag(Name);
	}

	FGameplayTag Perk(const TCHAR* Name)
	{
		return UDCCharacterProgressionComponent::PerkTag(Name);
	}

	FDCGameplayCondition AtLeast(EDCConditionType Type, const TCHAR* Id, int32 Quantity)
	{
		FDCGameplayCondition Condition;
		Condition.Type = Type;
		Condition.Id = Id;
		Condition.Quantity = Quantity;
		return Condition;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCProgressionLookupTest, "DeadCurrent.Progression.Lookup",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCProgressionLookupTest::RunTest(const FString& Parameters)
{
	using namespace DCProgressionTest;

	UDCCharacterProgressionComponent* Build = NewObject<UDCCharacterProgressionComponent>();
	const FGameplayTag Grasp = Attr(TEXT("Attribute.Grasp"));
	const FGameplayTag Engineering = Skill(TEXT("Skill.Engineering"));
	const FGameplayTag Survival = Skill(TEXT("Skill.Survival"));
	const FGameplayTag Eye = Perk(TEXT("Perk.SchematicEye"));
	const FGameplayTag Bogus = FGameplayTag::RequestGameplayTag(TEXT("Skill.NotASkill"), false);

	TestTrue(TEXT("Grasp is a known attribute"), Grasp.IsValid());
	TestTrue(TEXT("Engineering is a known skill"), Engineering.IsValid());
	TestFalse(TEXT("Unknown skill tag is rejected"), UDCCharacterProgressionComponent::IsKnownSkill(TEXT("Skill.NotASkill")));
	TestFalse(TEXT("Unknown attribute cannot be raised"), Build->TryRaiseAttribute(Bogus));
	TestFalse(TEXT("Unknown perk cannot be taken"), Build->TryTakePerk(Bogus));
	TestEqual(TEXT("Unknown attribute reads as 0"), Build->GetAttribute(Bogus), 0);
	TestEqual(TEXT("Unknown skill reads as 0"), Build->GetEffectiveSkill(Bogus), 0);
	TestFalse(TEXT("Unknown perk is not owned"), Build->HasPerk(Bogus));

	TestEqual(TEXT("Fresh attribute points"), Build->GetUnspentAttributePoints(), UDCCharacterProgressionComponent::AttributePointPool);
	TestEqual(TEXT("Fresh skill points"), Build->GetUnspentSkillPoints(), UDCCharacterProgressionComponent::SkillPointPool);
	TestEqual(TEXT("Fresh perk point"), Build->GetUnspentPerkPoints(), UDCCharacterProgressionComponent::PerkPointPool);

	TestTrue(TEXT("Raise Grasp"), Build->TryRaiseAttribute(Grasp));
	TestTrue(TEXT("Raise Grasp again"), Build->TryRaiseAttribute(Grasp));
	TestFalse(TEXT("Grasp caps at 2"), Build->TryRaiseAttribute(Grasp));
	TestEqual(TEXT("Grasp is 2"), Build->GetAttribute(Grasp), 2);
	TestEqual(TEXT("One attribute point left"), Build->GetUnspentAttributePoints(), 1);

	TestEqual(TEXT("Engineering with no ranks is just the attribute bonus"), Build->GetEffectiveSkill(Engineering), 1);
	TestTrue(TEXT("One rank of Engineering"), Build->TryRaiseSkill(Engineering));
	TestEqual(TEXT("Effective Engineering is ranks plus bonus"), Build->GetEffectiveSkill(Engineering), 2);
	TestTrue(TEXT("Second rank of Engineering spends the pool"), Build->TryRaiseSkill(Engineering));
	TestFalse(TEXT("No skill points left"), Build->TryRaiseSkill(Survival));
	TestEqual(TEXT("Effective Engineering at the cap plus bonus"), Build->GetEffectiveSkill(Engineering), 3);

	TestTrue(TEXT("Take Schematic Eye"), Build->TryTakePerk(Eye));
	TestTrue(TEXT("Schematic Eye is owned"), Build->HasPerk(Eye));
	TestFalse(TEXT("Perk point is spent"), Build->TryTakePerk(Perk(TEXT("Perk.PulseRead"))));

	Build->ResetAllocation();
	TestEqual(TEXT("Reset clears Grasp"), Build->GetAttribute(Grasp), 0);
	TestEqual(TEXT("Reset clears Engineering"), Build->GetSkill(Engineering), 0);
	TestFalse(TEXT("Reset clears the perk"), Build->HasPerk(Eye));
	TestEqual(TEXT("Reset refunds attribute points"), Build->GetUnspentAttributePoints(), UDCCharacterProgressionComponent::AttributePointPool);

	TestEqual(TEXT("Engineering's attribute is Grasp"), UDCCharacterProgressionComponent::LinkedAttribute(Engineering), Grasp);
	TestEqual(TEXT("Display name"), UDCCharacterProgressionComponent::GetSkillName(Engineering).ToString(), FString(TEXT("Engineering")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCProgressionRulesTest, "DeadCurrent.Progression.Rules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCProgressionRulesTest::RunTest(const FString& Parameters)
{
	using namespace DCProgressionTest;

	FDCTestWorld World;
	AActor* Player = World.SpawnActor();
	UDCCharacterProgressionComponent* Build = FDCTestWorld::AddComponent<UDCCharacterProgressionComponent>(Player);
	const FDCRuleContext Context = FDCRuleContext::ForActor(Player);
	TestNotNull(TEXT("Context finds progression"), Context.Progression);

	const FDCGameplayCondition Engineering = AtLeast(EDCConditionType::SkillAtLeast, TEXT("Skill.Engineering"), 2);
	const FDCGameplayCondition Fieldcraft = AtLeast(EDCConditionType::AttributeAtLeast, TEXT("Attribute.Fieldcraft"), 2);
	const FDCGameplayCondition Eye = AtLeast(EDCConditionType::HasPerk, TEXT("Perk.SchematicEye"), 1);
	FDCGameplayCondition NotEngineering = Engineering;
	NotEngineering.bNegate = true;

	TestFalse(TEXT("Unspent build fails Engineering 2"), UDCGameplayRules::CheckCondition(Engineering, Context));
	TestTrue(TEXT("Negate passes while the check fails"), UDCGameplayRules::CheckCondition(NotEngineering, Context));
	TestFalse(TEXT("No perk yet"), UDCGameplayRules::CheckCondition(Eye, Context));

	Build->SetAttributeValue(Attr(TEXT("Attribute.Grasp")), 2);
	Build->SetSkillValue(Skill(TEXT("Skill.Engineering")), 1);
	TestTrue(TEXT("Attribute bonus plus one rank passes Engineering 2"), UDCGameplayRules::CheckCondition(Engineering, Context));
	TestFalse(TEXT("Fieldcraft was not raised"), UDCGameplayRules::CheckCondition(Fieldcraft, Context));

	Build->SetAttributeValue(Attr(TEXT("Attribute.Fieldcraft")), 2);
	TestTrue(TEXT("Fieldcraft 2 passes"), UDCGameplayRules::CheckCondition(Fieldcraft, Context));

	Build->SetPerkOwned(Perk(TEXT("Perk.SchematicEye")), true);
	TestTrue(TEXT("HasPerk passes"), UDCGameplayRules::CheckCondition(Eye, Context));

	FDCGameplayCondition Unknown;
	Unknown.Type = EDCConditionType::SkillAtLeast;
	Unknown.Id = TEXT("Skill.NotASkill");
	Unknown.Quantity = 1;
	TestFalse(TEXT("Unknown skill fails closed"), UDCGameplayRules::CheckCondition(Unknown, Context));

	FDCRuleContext Empty;
	TestFalse(TEXT("Missing progression fails the check"), UDCGameplayRules::CheckCondition(Engineering, Empty));

	TestEqual(TEXT("Label"), UDCGameplayRules::FormatCheckLabels({ Engineering, Eye }),
		FString(TEXT("[Engineering 2] [Schematic Eye]")));

	TArray<FString> Problems;
	UDCGameplayRules::ValidateReferences(Engineering, Problems);
	UDCGameplayRules::ValidateReferences(Fieldcraft, Problems);
	UDCGameplayRules::ValidateReferences(Eye, Problems);
	TestEqual(TEXT("Catalog ids validate"), Problems.Num(), 0);
	UDCGameplayRules::ValidateReferences(Unknown, Problems);
	TestEqual(TEXT("Unknown skill id is a validation error"), Problems.Num(), 1);

	FDCGameplayCondition NoId;
	NoId.Type = EDCConditionType::HasPerk;
	UDCGameplayRules::ValidateReferences(NoId, Problems);
	TestTrue(TEXT("Perk with no id is a validation error"), Problems.Num() >= 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCProgressionContentChecksTest, "DeadCurrent.Progression.ContentChecks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCProgressionContentChecksTest::RunTest(const FString& Parameters)
{
	using namespace DCProgressionTest;

	FDCTestWorld World;
	AActor* PlayerActor = World.SpawnActor();
	UDCCharacterProgressionComponent* Build = FDCTestWorld::AddComponent<UDCCharacterProgressionComponent>(PlayerActor);
	UDCDialogueComponent* Dialogue = FDCTestWorld::AddComponent<UDCDialogueComponent>(PlayerActor);

	UDCDialogueAsset* Asset = NewObject<UDCDialogueAsset>();
	Asset->DialogueId = TEXT("test.progression");
	Asset->EntryNodeId = TEXT("storm");

	FDCDialogueChoice Press;
	Press.Text = FText::FromString(TEXT("You're leaving something out."));
	Press.NextNodeId = TEXT("pressed");
	Press.Conditions = { AtLeast(EDCConditionType::SkillAtLeast, TEXT("Skill.Persuasion"), 2) };

	FDCDialogueChoice Bye;
	Bye.Text = FText::FromString(TEXT("Goodbye."));

	FDCDialogueNode Storm;
	Storm.NodeId = TEXT("storm");
	Storm.Speaker = FText::FromString(TEXT("Mara"));
	Storm.Line = FText::FromString(TEXT("It's always a storm."));
	Storm.Choices = { Press, Bye };

	FDCDialogueNode Pressed;
	Pressed.NodeId = TEXT("pressed");
	Pressed.Line = FText::FromString(TEXT("I didn't follow."));
	Pressed.Choices = { Bye };

	Asset->Nodes = { Storm, Pressed };
	TestTrue(TEXT("Dialogue starts"), Dialogue->StartDialogue(Asset, nullptr));
	TestEqual(TEXT("Persuasion choice is hidden"), Dialogue->GetVisibleChoiceIndices().Num(), 1);

	Build->SetAttributeValue(Attr(TEXT("Attribute.Bearing")), 2);
	Build->SetSkillValue(Skill(TEXT("Skill.Persuasion")), 1);
	TestEqual(TEXT("Persuasion choice appears at effective 2"), Dialogue->GetVisibleChoiceIndices().Num(), 2);
	TestTrue(TEXT("The visible choice is the press"), Dialogue->SelectChoice(0));
	TestEqual(TEXT("Press advances"), Dialogue->GetCurrentNodeId(), FName(TEXT("pressed")));

	ADCInspectableActor* Panel = World.Get()->SpawnActor<ADCInspectableActor>();
	FDCInspectVariant Skilled;
	Skilled.Action = FText::FromString(TEXT("Inspect"));
	Skilled.Description = FText::FromString(TEXT("The mast lamp is not on these lugs."));
	Skilled.Conditions = { AtLeast(EDCConditionType::SkillAtLeast, TEXT("Skill.Engineering"), 2) };
	FDCInspectVariant PerkRead;
	PerkRead.Description = FText::FromString(TEXT("NOT A SHOAL"));
	PerkRead.Conditions = { AtLeast(EDCConditionType::HasPerk, TEXT("Perk.SchematicEye"), 1) };

	FArrayProperty* VariantsProperty = CastField<FArrayProperty>(ADCInspectableActor::StaticClass()->FindPropertyByName(TEXT("Variants")));
	if (!TestNotNull(TEXT("Variants property"), VariantsProperty))
	{
		return false;
	}

	FTextProperty* DescriptionProperty = CastField<FTextProperty>(ADCInspectableActor::StaticClass()->FindPropertyByName(TEXT("Description")));
	if (DescriptionProperty)
	{
		DescriptionProperty->SetPropertyValue_InContainer(Panel, FText::FromString(TEXT("Gutted panel.")));
	}
	*VariantsProperty->ContainerPtrToValuePtr<TArray<FDCInspectVariant>>(Panel) = { PerkRead, Skilled };

	const FString Unskilled = IDCInteractable::Execute_GetInteractionPrompt(Panel, PlayerActor).Action.ToString();
	TestFalse(TEXT("Unskilled prompt has no engineering label"), Unskilled.Contains(TEXT("Engineering")));
	Build->SetSkillValue(Skill(TEXT("Skill.Engineering")), 2);
	const FString SkilledPrompt = IDCInteractable::Execute_GetInteractionPrompt(Panel, PlayerActor).Action.ToString();
	TestTrue(TEXT("Skilled prompt names the check"), SkilledPrompt.Contains(TEXT("[Engineering 2]")));

	Build->SetPerkOwned(Perk(TEXT("Perk.SchematicEye")), true);
	const FString PerkPrompt = IDCInteractable::Execute_GetInteractionPrompt(Panel, PlayerActor).Action.ToString();
	TestTrue(TEXT("Perk variant wins and labels the perk"), PerkPrompt.Contains(TEXT("[Schematic Eye]")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCProgressionSaveTest, "DeadCurrent.Progression.Save",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCProgressionSaveTest::RunTest(const FString& Parameters)
{
	using namespace DCProgressionTest;

	UDCCharacterProgressionComponent* Build = NewObject<UDCCharacterProgressionComponent>();
	TestEqual(TEXT("New build is unspent"), Build->GetUnspentSkillPoints(), UDCCharacterProgressionComponent::SkillPointPool);

	Build->TryRaiseAttribute(Attr(TEXT("Attribute.Grasp")));
	Build->TryRaiseAttribute(Attr(TEXT("Attribute.Grasp")));
	Build->TryRaiseSkill(Skill(TEXT("Skill.Engineering")));
	Build->TryTakePerk(Perk(TEXT("Perk.SchematicEye")));

	UDCSaveGame* Save = NewObject<UDCSaveGame>();
	Save->SaveVersion = UDCSaveGame::CurrentVersion;
	UDCSaveSubsystem::CaptureBuild(Save, Build);

	Build->ResetAllocation();
	Build->TryRaiseSkill(Skill(TEXT("Skill.Persuasion")));
	TestEqual(TEXT("Mutated away from the save"), Build->GetSkill(Skill(TEXT("Skill.Engineering"))), 0);

	UDCSaveSubsystem::ApplyBuild(Save, Build);
	TestEqual(TEXT("Loaded Grasp"), Build->GetAttribute(Attr(TEXT("Attribute.Grasp"))), 2);
	TestEqual(TEXT("Loaded Engineering"), Build->GetSkill(Skill(TEXT("Skill.Engineering"))), 1);
	TestEqual(TEXT("Loaded effective Engineering"), Build->GetEffectiveSkill(Skill(TEXT("Skill.Engineering"))), 2);
	TestTrue(TEXT("Loaded perk"), Build->HasPerk(Perk(TEXT("Perk.SchematicEye"))));
	TestEqual(TEXT("Persuasion was not in the save"), Build->GetSkill(Skill(TEXT("Skill.Persuasion"))), 0);

	UDCSaveGame* Old = NewObject<UDCSaveGame>();
	Old->SaveVersion = 4;
	UDCSaveSubsystem::ApplyBuild(Old, Build);
	TestEqual(TEXT("Pre-RPG save restores an unspent build"), Build->GetAttribute(Attr(TEXT("Attribute.Grasp"))), 0);
	TestFalse(TEXT("Pre-RPG save has no perk"), Build->HasPerk(Perk(TEXT("Perk.SchematicEye"))));
	TestEqual(TEXT("Pre-RPG save refunds the pools"), Build->GetUnspentAttributePoints(), UDCCharacterProgressionComponent::AttributePointPool);
	return true;
}

#endif
