#pragma once

#include "CoreMinimal.h"
#include "Character/DCProgressionTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DCCharacterProgressionComponent.generated.h"

/**
 *  The player's attributes, skills and perks.
 *
 *  PROVISIONAL model: three attributes, each linked to one skill. Effective skill is the
 *  invested ranks plus one when the linked attribute is at least AttributeBonusAt.
 *  Point pools are small on purpose. This is the proof that a build changes options,
 *  not the final progression economy. There is no XP and no level.
 *
 *  Lookup is by Gameplay Tag. Unknown tags read as 0 / not owned and cannot be spent.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCCharacterProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Points available on a new game. Unspent = pool minus what is invested. */
	static constexpr int32 AttributePointPool = 3;
	static constexpr int32 SkillPointPool = 2;
	static constexpr int32 PerkPointPool = 1;

	static constexpr int32 MaxAttribute = 2;
	static constexpr int32 MaxSkillRank = 2;

	/** A linked attribute at or above this adds AttributeSkillBonus to that skill's effective rank. */
	static constexpr int32 AttributeBonusAt = 2;
	static constexpr int32 AttributeSkillBonus = 1;

	UDCCharacterProgressionComponent();

	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetAttribute(FGameplayTag Id) const;

	/** Invested ranks, before the attribute bonus. */
	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetSkill(FGameplayTag Id) const;

	/** Ranks plus the linked-attribute bonus. Skill checks use this. */
	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetEffectiveSkill(FGameplayTag Id) const;

	UFUNCTION(BlueprintPure, Category="Progression")
	bool HasPerk(FGameplayTag Id) const;

	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetUnspentAttributePoints() const;

	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetUnspentSkillPoints() const;

	UFUNCTION(BlueprintPure, Category="Progression")
	int32 GetUnspentPerkPoints() const;

	/** Spend one point. False if the tag is unknown, the rank is already at max, or no points remain. */
	UFUNCTION(BlueprintCallable, Category="Progression")
	bool TryRaiseAttribute(FGameplayTag Id);

	UFUNCTION(BlueprintCallable, Category="Progression")
	bool TryRaiseSkill(FGameplayTag Id);

	/** Spend the perk point. False if it is already spent, the tag is unknown, or the perk is owned. */
	UFUNCTION(BlueprintCallable, Category="Progression")
	bool TryTakePerk(FGameplayTag Id);

	/**
	 *  Puts every point back. Prototype only, so a playtest can try each orientation.
	 *  This is not a respec feature.
	 */
	UFUNCTION(BlueprintCallable, Category="Progression")
	void ResetAllocation();

	/** Direct write used by save/load and tests. Unknown tags are ignored. Values clamp to the max. */
	void SetAttributeValue(FGameplayTag Id, int32 Value);
	void SetSkillValue(FGameplayTag Id, int32 Value);
	void SetPerkOwned(FGameplayTag Id, bool bOwned);

	void CaptureState(TArray<FDCSavedRank>& OutAttributes, TArray<FDCSavedRank>& OutSkills, TArray<FName>& OutPerks) const;

	/** Replaces the whole build. Empty arrays are the unspent default (a save from before the RPG layer). */
	void RestoreState(const TArray<FDCSavedRank>& InAttributes, const TArray<FDCSavedRank>& InSkills, const TArray<FName>& InPerks);

	UFUNCTION(BlueprintPure, Category="Progression")
	static FText GetAttributeName(FGameplayTag Id);

	UFUNCTION(BlueprintPure, Category="Progression")
	static FText GetSkillName(FGameplayTag Id);

	UFUNCTION(BlueprintPure, Category="Progression")
	static FText GetPerkName(FGameplayTag Id);

	UFUNCTION(BlueprintPure, Category="Progression")
	static FText GetPerkDescription(FGameplayTag Id);

	static bool IsKnownAttribute(FName Id);
	static bool IsKnownSkill(FName Id);
	static bool IsKnownPerk(FName Id);

	static FGameplayTag AttributeTag(FName Id);
	static FGameplayTag SkillTag(FName Id);
	static FGameplayTag PerkTag(FName Id);

	static const TArray<FGameplayTag>& AllAttributes();
	static const TArray<FGameplayTag>& AllSkills();
	static const TArray<FGameplayTag>& AllPerks();

	/** The attribute that feeds this skill, or an empty tag. */
	static FGameplayTag LinkedAttribute(FGameplayTag SkillId);

private:

	int32 SumValues(const TMap<FGameplayTag, int32>& Ranks) const;

	TMap<FGameplayTag, int32> Attributes;
	TMap<FGameplayTag, int32> Skills;
	TArray<FGameplayTag> Perks;
};
