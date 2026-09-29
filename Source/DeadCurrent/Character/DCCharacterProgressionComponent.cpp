#include "Character/DCCharacterProgressionComponent.h"
#include "NativeGameplayTags.h"

namespace DCProgressionTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Grasp, "Attribute.Grasp", "Reading made things: panels, housings, wiring");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Fieldcraft, "Attribute.Fieldcraft", "Reading ground, animals, and how people moved");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Bearing, "Attribute.Bearing", "Holding a conversation and pressing for what was withheld");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill_Engineering, "Skill.Engineering", "Working out how a made thing was put together and what it is doing");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill_Survival, "Skill.Survival", "Reading a place for what passed through it and what will hurt");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Skill_Persuasion, "Skill.Persuasion", "Getting someone to say the part they were leaving out");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Perk_SchematicEye, "Perk.SchematicEye", "Margin marks and pinouts that other people file as noise");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Perk_PulseRead, "Perk.PulseRead", "Telling one shock from a slow death");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Perk_RelayEar, "Perk.RelayEar", "Hearing whether a housing was seated by someone who knew it");
}

#define LOCTEXT_NAMESPACE "DCCharacterProgression"

namespace DCProgressionCatalog
{
	struct FAttribute
	{
		FGameplayTag Tag;
		FText Name;
	};

	struct FSkill
	{
		FGameplayTag Tag;
		FText Name;
		FGameplayTag LinkedAttribute;
	};

	struct FPerk
	{
		FGameplayTag Tag;
		FText Name;
		FText Description;
	};

	static const TArray<FAttribute>& Attributes()
	{
		static const TArray<FAttribute> Rows = {
			{ DCProgressionTags::Attribute_Grasp, LOCTEXT("Grasp", "Grasp") },
			{ DCProgressionTags::Attribute_Fieldcraft, LOCTEXT("Fieldcraft", "Fieldcraft") },
			{ DCProgressionTags::Attribute_Bearing, LOCTEXT("Bearing", "Bearing") },
		};
		return Rows;
	}

	static const TArray<FSkill>& Skills()
	{
		static const TArray<FSkill> Rows = {
			{ DCProgressionTags::Skill_Engineering, LOCTEXT("Engineering", "Engineering"), DCProgressionTags::Attribute_Grasp },
			{ DCProgressionTags::Skill_Survival, LOCTEXT("Survival", "Survival"), DCProgressionTags::Attribute_Fieldcraft },
			{ DCProgressionTags::Skill_Persuasion, LOCTEXT("Persuasion", "Persuasion"), DCProgressionTags::Attribute_Bearing },
		};
		return Rows;
	}

	static const TArray<FPerk>& Perks()
	{
		static const TArray<FPerk> Rows = {
			{ DCProgressionTags::Perk_SchematicEye, LOCTEXT("SchematicEye", "Schematic Eye"),
				LOCTEXT("SchematicEyeDesc", "Read margin marks on a chart or a housing that other people file as noise.") },
			{ DCProgressionTags::Perk_PulseRead, LOCTEXT("PulseRead", "Pulse Read"),
				LOCTEXT("PulseReadDesc", "Tell a single shock from something that died slowly.") },
			{ DCProgressionTags::Perk_RelayEar, LOCTEXT("RelayEar", "Relay Ear"),
				LOCTEXT("RelayEarDesc", "Hear whether a relay housing was seated by someone who knew the pinout.") },
		};
		return Rows;
	}

	template <typename RowType>
	static const RowType* Find(const TArray<RowType>& Rows, FGameplayTag Id)
	{
		return Rows.FindByPredicate([Id](const RowType& Row) { return Row.Tag == Id; });
	}

	static FGameplayTag TagNamed(FName Id)
	{
		return Id.IsNone() ? FGameplayTag() : FGameplayTag::RequestGameplayTag(Id, false);
	}
}

#undef LOCTEXT_NAMESPACE

UDCCharacterProgressionComponent::UDCCharacterProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

int32 UDCCharacterProgressionComponent::GetAttribute(FGameplayTag Id) const
{
	const int32* Found = Attributes.Find(Id);
	return Found ? *Found : 0;
}

int32 UDCCharacterProgressionComponent::GetSkill(FGameplayTag Id) const
{
	const int32* Found = Skills.Find(Id);
	return Found ? *Found : 0;
}

int32 UDCCharacterProgressionComponent::GetEffectiveSkill(FGameplayTag Id) const
{
	if (!DCProgressionCatalog::Find(DCProgressionCatalog::Skills(), Id))
	{
		return 0;
	}

	int32 Value = GetSkill(Id);
	if (GetAttribute(LinkedAttribute(Id)) >= AttributeBonusAt)
	{
		Value += AttributeSkillBonus;
	}
	return Value;
}

bool UDCCharacterProgressionComponent::HasPerk(FGameplayTag Id) const
{
	return Perks.Contains(Id);
}

int32 UDCCharacterProgressionComponent::SumValues(const TMap<FGameplayTag, int32>& Ranks) const
{
	int32 Total = 0;
	for (const TPair<FGameplayTag, int32>& Pair : Ranks)
	{
		Total += Pair.Value;
	}
	return Total;
}

int32 UDCCharacterProgressionComponent::GetUnspentAttributePoints() const
{
	return AttributePointPool - SumValues(Attributes);
}

int32 UDCCharacterProgressionComponent::GetUnspentSkillPoints() const
{
	return SkillPointPool - SumValues(Skills);
}

int32 UDCCharacterProgressionComponent::GetUnspentPerkPoints() const
{
	return PerkPointPool - Perks.Num();
}

bool UDCCharacterProgressionComponent::TryRaiseAttribute(FGameplayTag Id)
{
	if (!IsKnownAttribute(Id.GetTagName()) || GetAttribute(Id) >= MaxAttribute || GetUnspentAttributePoints() <= 0)
	{
		return false;
	}

	Attributes.FindOrAdd(Id) += 1;
	return true;
}

bool UDCCharacterProgressionComponent::TryRaiseSkill(FGameplayTag Id)
{
	if (!IsKnownSkill(Id.GetTagName()) || GetSkill(Id) >= MaxSkillRank || GetUnspentSkillPoints() <= 0)
	{
		return false;
	}

	Skills.FindOrAdd(Id) += 1;
	return true;
}

bool UDCCharacterProgressionComponent::TryTakePerk(FGameplayTag Id)
{
	if (!IsKnownPerk(Id.GetTagName()) || HasPerk(Id) || GetUnspentPerkPoints() <= 0)
	{
		return false;
	}

	Perks.Add(Id);
	return true;
}

void UDCCharacterProgressionComponent::ResetAllocation()
{
	Attributes.Reset();
	Skills.Reset();
	Perks.Reset();
}

void UDCCharacterProgressionComponent::SetAttributeValue(FGameplayTag Id, int32 Value)
{
	if (!IsKnownAttribute(Id.GetTagName()))
	{
		return;
	}

	Value = FMath::Clamp(Value, 0, MaxAttribute);
	if (Value <= 0)
	{
		Attributes.Remove(Id);
	}
	else
	{
		Attributes.Add(Id, Value);
	}
}

void UDCCharacterProgressionComponent::SetSkillValue(FGameplayTag Id, int32 Value)
{
	if (!IsKnownSkill(Id.GetTagName()))
	{
		return;
	}

	Value = FMath::Clamp(Value, 0, MaxSkillRank);
	if (Value <= 0)
	{
		Skills.Remove(Id);
	}
	else
	{
		Skills.Add(Id, Value);
	}
}

void UDCCharacterProgressionComponent::SetPerkOwned(FGameplayTag Id, bool bOwned)
{
	if (!IsKnownPerk(Id.GetTagName()))
	{
		return;
	}

	if (bOwned)
	{
		Perks.AddUnique(Id);
	}
	else
	{
		Perks.Remove(Id);
	}
}

void UDCCharacterProgressionComponent::CaptureState(TArray<FDCSavedRank>& OutAttributes, TArray<FDCSavedRank>& OutSkills, TArray<FName>& OutPerks) const
{
	OutAttributes.Reset();
	OutSkills.Reset();
	OutPerks.Reset();

	for (const DCProgressionCatalog::FAttribute& Row : DCProgressionCatalog::Attributes())
	{
		const int32 Value = GetAttribute(Row.Tag);
		if (Value > 0)
		{
			FDCSavedRank Rank;
			Rank.Id = Row.Tag.GetTagName();
			Rank.Value = Value;
			OutAttributes.Add(Rank);
		}
	}
	for (const DCProgressionCatalog::FSkill& Row : DCProgressionCatalog::Skills())
	{
		const int32 Value = GetSkill(Row.Tag);
		if (Value > 0)
		{
			FDCSavedRank Rank;
			Rank.Id = Row.Tag.GetTagName();
			Rank.Value = Value;
			OutSkills.Add(Rank);
		}
	}
	for (const FGameplayTag& Perk : Perks)
	{
		OutPerks.Add(Perk.GetTagName());
	}
}

void UDCCharacterProgressionComponent::RestoreState(const TArray<FDCSavedRank>& InAttributes, const TArray<FDCSavedRank>& InSkills, const TArray<FName>& InPerks)
{
	ResetAllocation();
	for (const FDCSavedRank& Rank : InAttributes)
	{
		SetAttributeValue(DCProgressionCatalog::TagNamed(Rank.Id), Rank.Value);
	}
	for (const FDCSavedRank& Rank : InSkills)
	{
		SetSkillValue(DCProgressionCatalog::TagNamed(Rank.Id), Rank.Value);
	}
	for (const FName& PerkId : InPerks)
	{
		SetPerkOwned(DCProgressionCatalog::TagNamed(PerkId), true);
	}
}

FText UDCCharacterProgressionComponent::GetAttributeName(FGameplayTag Id)
{
	if (const DCProgressionCatalog::FAttribute* Row = DCProgressionCatalog::Find(DCProgressionCatalog::Attributes(), Id))
	{
		return Row->Name;
	}
	return FText::FromName(Id.GetTagName());
}

FText UDCCharacterProgressionComponent::GetSkillName(FGameplayTag Id)
{
	if (const DCProgressionCatalog::FSkill* Row = DCProgressionCatalog::Find(DCProgressionCatalog::Skills(), Id))
	{
		return Row->Name;
	}
	return FText::FromName(Id.GetTagName());
}

FText UDCCharacterProgressionComponent::GetPerkName(FGameplayTag Id)
{
	if (const DCProgressionCatalog::FPerk* Row = DCProgressionCatalog::Find(DCProgressionCatalog::Perks(), Id))
	{
		return Row->Name;
	}
	return FText::FromName(Id.GetTagName());
}

FText UDCCharacterProgressionComponent::GetPerkDescription(FGameplayTag Id)
{
	if (const DCProgressionCatalog::FPerk* Row = DCProgressionCatalog::Find(DCProgressionCatalog::Perks(), Id))
	{
		return Row->Description;
	}
	return FText::GetEmpty();
}

bool UDCCharacterProgressionComponent::IsKnownAttribute(FName Id)
{
	const FGameplayTag Tag = DCProgressionCatalog::TagNamed(Id);
	return DCProgressionCatalog::Find(DCProgressionCatalog::Attributes(), Tag) != nullptr;
}

bool UDCCharacterProgressionComponent::IsKnownSkill(FName Id)
{
	const FGameplayTag Tag = DCProgressionCatalog::TagNamed(Id);
	return DCProgressionCatalog::Find(DCProgressionCatalog::Skills(), Tag) != nullptr;
}

bool UDCCharacterProgressionComponent::IsKnownPerk(FName Id)
{
	const FGameplayTag Tag = DCProgressionCatalog::TagNamed(Id);
	return DCProgressionCatalog::Find(DCProgressionCatalog::Perks(), Tag) != nullptr;
}

FGameplayTag UDCCharacterProgressionComponent::AttributeTag(FName Id)
{
	const FGameplayTag Tag = DCProgressionCatalog::TagNamed(Id);
	return IsKnownAttribute(Id) ? Tag : FGameplayTag();
}

FGameplayTag UDCCharacterProgressionComponent::SkillTag(FName Id)
{
	const FGameplayTag Tag = DCProgressionCatalog::TagNamed(Id);
	return IsKnownSkill(Id) ? Tag : FGameplayTag();
}

FGameplayTag UDCCharacterProgressionComponent::PerkTag(FName Id)
{
	const FGameplayTag Tag = DCProgressionCatalog::TagNamed(Id);
	return IsKnownPerk(Id) ? Tag : FGameplayTag();
}

const TArray<FGameplayTag>& UDCCharacterProgressionComponent::AllAttributes()
{
	static TArray<FGameplayTag> Ids;
	if (Ids.IsEmpty())
	{
		for (const DCProgressionCatalog::FAttribute& Row : DCProgressionCatalog::Attributes())
		{
			Ids.Add(Row.Tag);
		}
	}
	return Ids;
}

const TArray<FGameplayTag>& UDCCharacterProgressionComponent::AllSkills()
{
	static TArray<FGameplayTag> Ids;
	if (Ids.IsEmpty())
	{
		for (const DCProgressionCatalog::FSkill& Row : DCProgressionCatalog::Skills())
		{
			Ids.Add(Row.Tag);
		}
	}
	return Ids;
}

const TArray<FGameplayTag>& UDCCharacterProgressionComponent::AllPerks()
{
	static TArray<FGameplayTag> Ids;
	if (Ids.IsEmpty())
	{
		for (const DCProgressionCatalog::FPerk& Row : DCProgressionCatalog::Perks())
		{
			Ids.Add(Row.Tag);
		}
	}
	return Ids;
}

FGameplayTag UDCCharacterProgressionComponent::LinkedAttribute(FGameplayTag SkillId)
{
	if (const DCProgressionCatalog::FSkill* Row = DCProgressionCatalog::Find(DCProgressionCatalog::Skills(), SkillId))
	{
		return Row->LinkedAttribute;
	}
	return FGameplayTag();
}
