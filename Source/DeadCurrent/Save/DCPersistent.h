#pragma once

#include "CoreMinimal.h"
#include "Save/DCPersistentTypes.h"
#include "UObject/Interface.h"
#include "DCPersistent.generated.h"

UINTERFACE(BlueprintType)
class DEADCURRENT_API UDCPersistent : public UInterface
{
	GENERATED_BODY()
};

/**
 *  World actors whose identity and state survive save/load. IDs are authored on
 *  UDCPersistentIdComponent; this interface is how the save system talks to them.
 */
class DEADCURRENT_API IDCPersistent
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	FName GetPersistentId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	void CapturePersistentState(FDCPersistentActorState& OutState) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Save")
	void ApplyPersistentState(const FDCPersistentActorState& State);
};
