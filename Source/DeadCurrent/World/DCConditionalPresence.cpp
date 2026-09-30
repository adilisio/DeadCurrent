#include "World/DCConditionalPresence.h"
#include "Components/SceneComponent.h"
#include "Core/DCGameplayRules.h"
#include "DeadCurrent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "World/DCWorldStateSubsystem.h"

namespace DCConditionalPresence
{
	/** A target drawn on screen this recently counts as being looked at. */
	constexpr float SeenToleranceSeconds = 0.5f;
}

ADCConditionalPresence::ADCConditionalPresence()
{
	PrimaryActorTick.bCanEverTick = true;

	Pivot = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	Pivot->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Pivot);
}

void ADCConditionalPresence::BeginPlay()
{
	Super::BeginPlay();

	AuthoredPivot = GetActorTransform();
	Authored.Reset();
	for (AActor* Target : Targets)
	{
		if (!Target || Target == this)
		{
			continue;
		}
		FAuthored& Entry = Authored.AddDefaulted_GetRef();
		Entry.Actor = Target;
		Entry.Offset = Target->GetActorTransform().GetRelativeTransform(AuthoredPivot);
		Entry.bCollisionEnabled = Target->GetActorEnableCollision();

		const USceneComponent* Root = Target->GetRootComponent();
		if (Root && Root->Mobility != EComponentMobility::Movable && States.ContainsByPredicate([](const FDCPresenceState& S) { return S.bMove; }))
		{
			UE_LOG(LogDeadCurrent, Warning, TEXT("[DCPRESENCE] %s: target %s is not Movable and cannot be placed"),
				*GetName(), *Target->GetName());
		}
	}

	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		ChangedHandle = WorldState->OnChanged.AddUObject(this, &ADCConditionalPresence::HandleWorldChanged);
		RestoredHandle = WorldState->OnRestored.AddUObject(this, &ADCConditionalPresence::Snap);
	}

	SetActorTickInterval(CheckInterval);
	Snap();
}

void ADCConditionalPresence::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		WorldState->OnChanged.Remove(ChangedHandle);
		WorldState->OnRestored.Remove(RestoredHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ADCConditionalPresence::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Evaluate();
}

void ADCConditionalPresence::HandleWorldChanged()
{
	Evaluate();
}

int32 ADCConditionalPresence::FindPassingState() const
{
	AActor* Context = const_cast<ADCConditionalPresence*>(this);
	if (const UWorld* World = GetWorld())
	{
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			Context = Pawn;
		}
	}

	const FDCRuleContext Rules = FDCRuleContext::ForActor(Context);
	for (int32 Index = 0; Index < States.Num(); ++Index)
	{
		if (UDCGameplayRules::CheckConditions(States[Index].Conditions, Rules))
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

FName ADCConditionalPresence::GetActiveStateId() const
{
	return States.IsValidIndex(ActiveState) ? States[ActiveState].StateId : NAME_None;
}

FTransform ADCConditionalPresence::PivotFor(int32 StateIndex) const
{
	return States.IsValidIndex(StateIndex) && States[StateIndex].bMove ? States[StateIndex].Placement : AuthoredPivot;
}

void ADCConditionalPresence::Evaluate()
{
	const int32 Next = FindPassingState();
	if (bApplied && Next == ActiveState)
	{
		bHasPending = false;
		return;
	}
	if (bApplied && IsObserved(Next))
	{
		bHasPending = true;
		return;
	}
	Apply(Next);
}

void ADCConditionalPresence::Snap()
{
	Apply(FindPassingState());
}

bool ADCConditionalPresence::IsObserved(int32 NextStateIndex) const
{
	if (!bDeferWhileObserved)
	{
		return false;
	}

	TOptional<FVector> Observer = ObserverOverride;
	if (!Observer.IsSet())
	{
		if (const APawn* Pawn = GetWorld() ? UGameplayStatics::GetPlayerPawn(GetWorld(), 0) : nullptr)
		{
			Observer = Pawn->GetActorLocation();
		}
	}

	if (Observer.IsSet())
	{
		const FVector Here = PivotFor(ActiveState).GetLocation();
		const FVector There = PivotFor(NextStateIndex).GetLocation();
		if (FVector::Dist(*Observer, Here) <= ObservedDistance || FVector::Dist(*Observer, There) <= ObservedDistance)
		{
			return true;
		}
	}

	// On screen recently. A world that never rendered (tests, a null-RHI run) reports 0, which is not a render.
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	for (const FAuthored& Entry : Authored)
	{
		const AActor* Target = Entry.Actor.Get();
		const float LastSeen = Target ? Target->GetLastRenderTime() : 0.0f;
		if (Target && !Target->IsHidden() && LastSeen > 0.0f && Now - LastSeen <= DCConditionalPresence::SeenToleranceSeconds)
		{
			return true;
		}
	}
	return false;
}

void ADCConditionalPresence::Apply(int32 StateIndex)
{
	const bool bChanged = !bApplied || StateIndex != ActiveState;
	const FTransform NewPivot = PivotFor(StateIndex);
	const bool bPresent = !States.IsValidIndex(StateIndex) || States[StateIndex].bPresent;

	SetActorTransform(NewPivot, false, nullptr, ETeleportType::TeleportPhysics);
	for (const FAuthored& Entry : Authored)
	{
		AActor* Target = Entry.Actor.Get();
		if (!Target)
		{
			continue; // destroyed (a pickup taken): nothing to place
		}
		Target->SetActorTransform(Entry.Offset * NewPivot, false, nullptr, ETeleportType::TeleportPhysics);
		Target->SetActorHiddenInGame(!bPresent);
		Target->SetActorEnableCollision(bPresent && Entry.bCollisionEnabled);
	}

	ActiveState = StateIndex;
	bApplied = true;
	bHasPending = false;
	if (bChanged)
	{
		UE_LOG(LogDeadCurrent, Log, TEXT("[DCPRESENCE] %s -> %s (%s, %d targets)"), *GetName(),
			*(StateIndex == INDEX_NONE ? FString(TEXT("default")) : GetActiveStateId().ToString()),
			bPresent ? TEXT("present") : TEXT("hidden"), Authored.Num());
	}
}
