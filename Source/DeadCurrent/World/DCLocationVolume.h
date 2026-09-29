#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DCLocationVolume.generated.h"

class UBoxComponent;

/**
 *  A named place the player discovers by walking into it ("Wrecked Survey Launch").
 *  Discovery is recorded once in UDCWorldStateSubsystem (saved with the game, readable by the
 *  LocationDiscovered condition) and announced with a HUD banner.
 *
 *  The volume checks player positions a few times a second instead of using collision, so it never
 *  blocks traces or bullets, and a save restored inside it is not re-announced (world state is
 *  restored before the next check). Once its location is discovered it stops checking.
 */
UCLASS()
class DEADCURRENT_API ADCLocationVolume : public AActor
{
	GENERATED_BODY()

public:
	ADCLocationVolume();

	virtual void Tick(float DeltaSeconds) override;

	/** Discovers this location on behalf of Discoverer (banner on their HUD). True only if newly discovered. */
	UFUNCTION(BlueprintCallable, Category="Location")
	bool TryDiscover(AActor* Discoverer);

	/** Whether WorldLocation is inside the volume's box (rotation and scale respected). */
	UFUNCTION(BlueprintPure, Category="Location")
	bool ContainsPoint(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category="Location")
	bool IsDiscovered() const;

	UFUNCTION(BlueprintPure, Category="Location")
	FName GetLocationId() const { return LocationId; }

	UFUNCTION(BlueprintPure, Category="Location")
	FText GetDisplayName() const;

	/** For spawned volumes (tests, tools). Placed volumes author these in the details panel. */
	void SetLocation(FName InLocationId, const FText& InDisplayName) { LocationId = InLocationId; DisplayName = InDisplayName; }

	UBoxComponent* GetBounds() const { return Bounds; }

	/** Display name of a location placed in World, or the id itself when no volume there carries it. */
	static FText FindDisplayName(const UWorld* World, FName LocationId);

protected:

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> Bounds;

	/** Stable id saved with the game ("shore.survey_launch"). Never change it once a location has shipped. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Location")
	FName LocationId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Location")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Location", meta=(ClampMin="0", Units="s"))
	float BannerDuration = 4.5f;
};
