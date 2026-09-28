#pragma once

#include "CoreMinimal.h"
#include "Core/DCGameplayTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DCGameplayRules.generated.h"

class UDCInventoryComponent;
class UDCQuestComponent;
class UDCWorldStateSubsystem;
class UDCPersistentRegistry;

/**
 *  Everything a condition can read and a consequence can change. Built from the instigating
 *  actor (the player for dialogue, quests and interactions). Fields may be null; a condition
 *  whose data is missing fails, and a consequence whose target is missing does nothing.
 */
struct DEADCURRENT_API FDCRuleContext
{
	AActor* Instigator = nullptr;
	UDCInventoryComponent* Inventory = nullptr;
	UDCQuestComponent* Quests = nullptr;
	UDCWorldStateSubsystem* WorldState = nullptr;
	UDCPersistentRegistry* Registry = nullptr;

	/** Fills the context from the actor's components and its world's subsystems. */
	static FDCRuleContext ForActor(AActor* Instigator);
};

/**
 *  The shared RPG rule language: evaluates FDCGameplayCondition and applies FDCGameplayConsequence.
 *  Dialogue, quests, interactables (and later terminals and world events) all call through here.
 */
UCLASS()
class DEADCURRENT_API UDCGameplayRules : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static bool CheckCondition(const FDCGameplayCondition& Condition, const FDCRuleContext& Context);

	/** True when every condition passes. An empty list passes. */
	static bool CheckConditions(const TArray<FDCGameplayCondition>& Conditions, const FDCRuleContext& Context);

	static void ApplyConsequence(const FDCGameplayConsequence& Consequence, const FDCRuleContext& Context);

	static void ApplyConsequences(const TArray<FDCGameplayConsequence>& Consequences, const FDCRuleContext& Context);

	/** Blueprint entry: check conditions for Instigator (usually the player). */
	UFUNCTION(BlueprintCallable, Category="Rules", meta=(DefaultToSelf="Instigator"))
	static bool CheckConditionsFor(AActor* Instigator, const TArray<FDCGameplayCondition>& Conditions);

	/** Blueprint entry: apply consequences for Instigator (usually the player). */
	UFUNCTION(BlueprintCallable, Category="Rules", meta=(DefaultToSelf="Instigator"))
	static void ApplyConsequencesFor(AActor* Instigator, const TArray<FDCGameplayConsequence>& Consequences);

	/** Short readable form for logs and validation messages. */
	static FString Describe(const FDCGameplayCondition& Condition);

	static FString Describe(const FDCGameplayConsequence& Consequence);
};
