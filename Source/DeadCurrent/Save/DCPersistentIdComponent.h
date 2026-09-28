#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DCPersistentIdComponent.generated.h"

/**
 *  Authored stable ID for a placed world actor. The registry indexes these at BeginPlay.
 */
UCLASS(ClassGroup=(DeadCurrent), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCPersistentIdComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDCPersistentIdComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category="Save")
	FName GetPersistentId() const { return PersistentId; }

	UFUNCTION(BlueprintCallable, Category="Save")
	void SetPersistentId(FName NewId) { PersistentId = NewId; }

	static FName GetIdOnActor(const AActor* Actor);

protected:

	/** Unique within a map. Example: gym.scavenger */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Save")
	FName PersistentId;

	UPROPERTY(EditAnywhere, Category="Save")
	bool bDrawId = false;
};
