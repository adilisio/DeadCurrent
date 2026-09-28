#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DCImpactMark.generated.h"

class UStaticMeshComponent;

/** Short-lived mark at a bullet impact, so hits on walls and floors are visible. */
UCLASS()
class DEADCURRENT_API ADCImpactMark : public AActor
{
	GENERATED_BODY()

public:
	ADCImpactMark();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
