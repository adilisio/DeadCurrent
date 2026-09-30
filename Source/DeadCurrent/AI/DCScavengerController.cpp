#include "AI/DCScavengerController.h"
#include "AI/DCScavengerCharacter.h"
#include "Character/DCPlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Combat/DCHealthComponent.h"
#include "DeadCurrent.h"
#include "DrawDebugHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ADCScavengerController::ADCScavengerController()
{
	PrimaryActorTick.bCanEverTick = true;

	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SetPerceptionComponent(*Perception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1800.0f;
	SightConfig->LoseSightRadius = 2400.0f;
	SightConfig->PeripheralVisionAngleDegrees = 75.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->SetMaxAge(6.0f);
	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(UAISense_Sight::StaticClass());
}

void ADCScavengerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Scavenger = Cast<ADCScavengerCharacter>(InPawn);
	Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ADCScavengerController::OnPerceptionUpdated);
	SetState(EDCScavengerState::Patrol);
}

void ADCScavengerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Scavenger || State == EDCScavengerState::Dead)
	{
		return;
	}

	if (Scavenger->GetHealthComponent() && Scavenger->GetHealthComponent()->IsDead())
	{
		SetState(EDCScavengerState::Dead);
		return;
	}

	TryNotice();

	switch (State)
	{
	case EDCScavengerState::Patrol:      TickPatrol(); break;
	case EDCScavengerState::Investigate: TickInvestigate(); break;
	case EDCScavengerState::Chase:       TickChase(); break;
	case EDCScavengerState::Attack:      TickAttack(); break;
	default: break;
	}

	if (bDrawState)
	{
		const FVector Above = Scavenger->GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
		DrawDebugString(GetWorld(), Above, GetStateName(), nullptr, FColor::White, 0.0f, true);
	}
}

void ADCScavengerController::NotifyDamagedBy(AActor* InstigatorActor)
{
	if (State == EDCScavengerState::Dead || !InstigatorActor || InstigatorActor == GetPawn())
	{
		return;
	}

	Target = InstigatorActor;
	LastKnownLocation = InstigatorActor->GetActorLocation();
	SetState(EDCScavengerState::Chase);
}

void ADCScavengerController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (State == EDCScavengerState::Dead || !Cast<ADCPlayerCharacter>(Actor))
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		bSeeingTarget = true;
		Target = Actor;
		LastKnownLocation = Actor->GetActorLocation();
		LoseTimer = 0.0f;
		TryNotice();
	}
	else if (Target.Get() == Actor)
	{
		bSeeingTarget = false;
		LastKnownLocation = Stimulus.StimulusLocation;
	}
}

bool ADCScavengerController::CanNoticeAt(bool bTargetCrouched, float Distance, float AngleFromFacingDegrees,
	float CrouchedRadius, float CrouchedHalfAngleDegrees)
{
	return !bTargetCrouched || (Distance <= CrouchedRadius && AngleFromFacingDegrees <= CrouchedHalfAngleDegrees);
}

bool ADCScavengerController::CanNotice(const AActor* Actor) const
{
	const APawn* Self = GetPawn();
	const ACharacter* TargetCharacter = Cast<ACharacter>(Actor);
	if (!Self || !Actor)
	{
		return false;
	}
	const FVector ToTarget = Actor->GetActorLocation() - Self->GetActorLocation();
	const FVector Facing = Self->GetActorForwardVector();
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(
		FVector::DotProduct(Facing.GetSafeNormal2D(), ToTarget.GetSafeNormal2D()), -1.0f, 1.0f)));
	return CanNoticeAt(TargetCharacter && TargetCharacter->bIsCrouched, ToTarget.Size2D(), Angle, CrouchedSightRadius,
		CrouchedPeripheralDegrees);
}

void ADCScavengerController::TryNotice()
{
	if ((State != EDCScavengerState::Patrol && State != EDCScavengerState::Investigate) || !bSeeingTarget)
	{
		return;
	}
	AActor* Seen = Target.Get();
	if (Seen && CanNotice(Seen))
	{
		SetState(EDCScavengerState::Chase);
	}
}

void ADCScavengerController::SetState(EDCScavengerState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	PauseTimer = 0.0f;
	LoseTimer = 0.0f;
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCAI] %s -> %s"), *GetNameSafe(GetPawn()), *GetStateName());

	if (State == EDCScavengerState::Patrol || State == EDCScavengerState::Dead)
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		StopMovement();
	}
	else if (AActor* Focus = Target.Get())
	{
		SetFocus(Focus);
	}
}

FString ADCScavengerController::GetStateName() const
{
	switch (State)
	{
	case EDCScavengerState::Patrol:      return TEXT("Patrol");
	case EDCScavengerState::Investigate: return TEXT("Investigate");
	case EDCScavengerState::Chase:       return TEXT("Chase");
	case EDCScavengerState::Attack:      return TEXT("Attack");
	case EDCScavengerState::Dead:        return TEXT("Dead");
	default: return TEXT("?");
	}
}

bool ADCScavengerController::IsTargetDead() const
{
	const AActor* Actor = Target.Get();
	const UDCHealthComponent* Health = Actor ? Actor->FindComponentByClass<UDCHealthComponent>() : nullptr;
	return !Actor || (Health && Health->IsDead());
}

void ADCScavengerController::TickPatrol()
{
	if (PauseTimer > 0.0f)
	{
		PauseTimer -= GetWorld()->GetDeltaSeconds();
		return;
	}

	const TArray<FVector>& Points = Scavenger->GetPatrolPoints();
	if (Points.Num() == 0)
	{
		return;
	}

	PatrolIndex = FMath::Clamp(PatrolIndex, 0, Points.Num() - 1);
	if (GetMoveStatus() != EPathFollowingStatus::Moving)
	{
		const FVector Goal = Points[PatrolIndex];
		const EPathFollowingRequestResult::Type Result = MoveToLocation(Goal, 80.0f);
		if (Result == EPathFollowingRequestResult::AlreadyAtGoal || GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			if (FVector::Dist2D(Scavenger->GetActorLocation(), Goal) < 120.0f)
			{
				PatrolIndex = (PatrolIndex + 1) % Points.Num();
				PauseTimer = PatrolPause;
			}
		}
	}
}

void ADCScavengerController::TickInvestigate()
{
	// Seeing the target again is handled by TryNotice, which applies the crouch rule.
	if (GetMoveStatus() != EPathFollowingStatus::Moving)
	{
		if (FVector::Dist2D(Scavenger->GetActorLocation(), LastKnownLocation) > 120.0f)
		{
			MoveToLocation(LastKnownLocation, 80.0f);
		}
		else
		{
			PauseTimer += GetWorld()->GetDeltaSeconds();
			if (PauseTimer >= 2.0f)
			{
				Target = nullptr;
				SetState(EDCScavengerState::Patrol);
			}
		}
	}
}

void ADCScavengerController::TickChase()
{
	if (IsTargetDead())
	{
		Target = nullptr;
		bSeeingTarget = false;
		SetState(EDCScavengerState::Patrol);
		return;
	}

	AActor* Actor = Target.Get();
	if (bSeeingTarget && Actor)
	{
		LastKnownLocation = Actor->GetActorLocation();
		LoseTimer = 0.0f;
	}
	else
	{
		LoseTimer += GetWorld()->GetDeltaSeconds();
		if (LoseTimer >= LoseSightTime)
		{
			SetState(EDCScavengerState::Investigate);
			return;
		}
	}

	if (Actor && FVector::Dist(Scavenger->GetActorLocation(), Actor->GetActorLocation()) <= Scavenger->GetMeleeRange())
	{
		SetState(EDCScavengerState::Attack);
		return;
	}

	if (Actor)
	{
		MoveToActor(Actor, Scavenger->GetMeleeRange() * 0.45f);
	}
	else
	{
		MoveToLocation(LastKnownLocation, 80.0f);
	}
}

void ADCScavengerController::TickAttack()
{
	if (IsTargetDead())
	{
		Target = nullptr;
		bSeeingTarget = false;
		SetState(EDCScavengerState::Patrol);
		return;
	}

	AActor* Actor = Target.Get();
	if (!Actor)
	{
		SetState(EDCScavengerState::Investigate);
		return;
	}

	if (!bSeeingTarget)
	{
		LoseTimer += GetWorld()->GetDeltaSeconds();
		if (LoseTimer >= LoseSightTime)
		{
			SetState(EDCScavengerState::Investigate);
			return;
		}
	}
	else
	{
		LoseTimer = 0.0f;
	}

	const float Dist = FVector::Dist(Scavenger->GetActorLocation(), Actor->GetActorLocation());
	if (Dist > Scavenger->GetMeleeRange() * 1.2f)
	{
		SetState(EDCScavengerState::Chase);
		return;
	}

	StopMovement();
	SetFocus(Actor);
	Scavenger->TryMelee(Actor);
}
