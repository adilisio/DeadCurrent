#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "DCScavengerController.generated.h"

class ADCPlayerCharacter;
class ADCScavengerCharacter;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

UENUM(BlueprintType)
enum class EDCScavengerState : uint8
{
	Patrol,
	Investigate,
	Chase,
	Attack,
	Dead
};

/**
 *  Sight, patrol, chase, melee, and giving up when the player is lost.
 */
UCLASS()
class DEADCURRENT_API ADCScavengerController : public AAIController
{
	GENERATED_BODY()

public:
	ADCScavengerController();

	void NotifyDamagedBy(AActor* InstigatorActor);

	EDCScavengerState GetState() const { return State; }

	FString GetStateName() const;

protected:

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	TObjectPtr<UAIPerceptionComponent> Perception;

	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="s"))
	float LoseSightTime = 4.0f;

	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="s"))
	float PatrolPause = 1.2f;

	UPROPERTY(EditAnywhere, Category="AI")
	bool bDrawState = true;

private:

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetState(EDCScavengerState NewState);

	void TickPatrol();
	void TickInvestigate();
	void TickChase();
	void TickAttack();

	bool IsTargetDead() const;

	TObjectPtr<ADCScavengerCharacter> Scavenger;

	TWeakObjectPtr<AActor> Target;

	FVector LastKnownLocation = FVector::ZeroVector;

	EDCScavengerState State = EDCScavengerState::Patrol;

	int32 PatrolIndex = 0;

	float LoseTimer = 0.0f;

	float PauseTimer = 0.0f;

	bool bSeeingTarget = false;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
};
