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
	/** Phase 5 playtest: he has spotted the player and warns them off before attacking. */
	Warn,
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

	/**
	 *  Whether a target his sight sense already reports can actually be noticed. Standing, the sense decides
	 *  (SightRadius, PeripheralVisionAngleDegrees). Crouched, the target must also be within CrouchedSightRadius
	 *  and CrouchedPeripheralDegrees of where he faces: staying low is how the quiet route gets past him.
	 */
	static bool CanNoticeAt(bool bTargetCrouched, float Distance, float AngleFromFacingDegrees,
		float CrouchedRadius, float CrouchedHalfAngleDegrees);

protected:

	/** A crouched player is noticed only this close (cm). Standing uses the sight sense's 12 m. */
	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="cm"))
	float CrouchedSightRadius = 800.0f;

	/** While warning, he attacks if the player comes this close (cm), or shoots him. */
	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="cm"))
	float WarnAttackRadius = 600.0f;

	/** What he shouts when he first spots the player. PROVISIONAL. */
	UPROPERTY(EditAnywhere, Category="AI")
	FText WarningLine = NSLOCTEXT("DCScavenger", "Warning", "Scavenger: \"This stretch is mine. Turn around.\"");

	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="s"))
	float WarningSeconds = 3.5f;

	/** Half-angle of the cone in which a crouched player is noticed. Standing uses the sense's 75. */
	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", ClampMax="180", Units="deg"))
	float CrouchedPeripheralDegrees = 45.0f;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	TObjectPtr<UAIPerceptionComponent> Perception;

	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="s"))
	float LoseSightTime = 4.0f;

	UPROPERTY(EditAnywhere, Category="AI", meta=(ClampMin="0", Units="s"))
	float PatrolPause = 1.2f;

	UPROPERTY(EditAnywhere, Category="AI")
	bool bDrawState = false;

private:

	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void SetState(EDCScavengerState NewState);

	void TickPatrol();
	void TickWarn();
	void TickInvestigate();
	void TickChase();
	void TickAttack();

	bool IsTargetDead() const;

	/** CanNoticeAt for the current target, from this pawn's position and facing. */
	bool CanNotice(const AActor* Actor) const;

	/** Patrol or Investigate: chase once the seen target can be noticed (re-checked every tick, because the sight
	 *  sense reports a target only when it first comes into view). */
	void TryNotice();

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
