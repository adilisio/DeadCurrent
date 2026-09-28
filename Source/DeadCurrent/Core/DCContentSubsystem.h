#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DCContentSubsystem.generated.h"

/**
 *  Loads every item and quest definition when the game starts and keeps them loaded, so gameplay
 *  can look them up by stable id (UDCItemDefinition::FindByItemId, UDCQuestDefinition::FindByQuestId)
 *  without hard-coded asset paths and without a hitch on first use. New content in /Game/Items or
 *  /Game/Quests is picked up automatically.
 */
UCLASS()
class DEADCURRENT_API UDCContentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Content folders scanned for definitions. */
	static const TArray<FString>& GetContentPaths();

private:

	UPROPERTY()
	TArray<TObjectPtr<UObject>> LoadedDefinitions;
};
