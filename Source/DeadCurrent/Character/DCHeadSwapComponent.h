#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DCHeadSwapComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

/**
 *  Presentation only: gives a character a different head. It hides everything the character's skeletal mesh
 *  skins to AttachBone (the head, its hair, and its eyes) and attaches a static mesh at that bone, so the head
 *  follows the animation. A costume pack ships one head; this is how a named character stops looking like every
 *  other character in it. It changes no collision, health, AI, or dialogue. With no HeadMesh it does nothing.
 */
UCLASS(ClassGroup=(Presentation), meta=(BlueprintSpawnableComponent))
class DEADCURRENT_API UDCHeadSwapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDCHeadSwapComponent();

	/** True once a head was attached. */
	UFUNCTION(BlueprintPure, Category="Head")
	bool IsSwapped() const { return HeadComponent != nullptr; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head")
	TSoftObjectPtr<UStaticMesh> HeadMesh;

	/** Bone the head is attached to and whose skinned vertices are hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head")
	FName AttachBone = TEXT("head");

	/** In the bone's space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head")
	FVector Offset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Head", meta=(ClampMin="0.01"))
	float Scale = 1.0f;

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> HeadComponent;
};
