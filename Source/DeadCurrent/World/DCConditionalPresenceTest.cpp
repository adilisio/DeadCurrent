#include "Core/DCTestHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Save/DCPersistentIdComponent.h"
#include "World/DCConditionalPresence.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  ADCConditionalPresence places, hides, and restores its targets from world state alone. No map, no player:
 *  "observed" is driven through the observer override, the same check the player pawn feeds in play.
 */
namespace DCConditionalPresenceTest
{
	const FName Gone = TEXT("test.presence_gone");
	const FName Moved = TEXT("test.presence_moved");

	const FVector PivotAt(1000.0, 0.0, 0.0);
	const FVector PlacementAt(5000.0, 0.0, 0.0);

	AStaticMeshActor* SpawnTarget(const FDCTestWorld& World, const FVector& Location, const FRotator& Rotation)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Actor = World.Get()->SpawnActor<AStaticMeshActor>(Location, Rotation, Params);
		Actor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		Actor->SetActorLocationAndRotation(Location, Rotation);
		return Actor;
	}

	FDCGameplayCondition Flag(FName Id)
	{
		FDCGameplayCondition Condition;
		Condition.Type = EDCConditionType::WorldFlag;
		Condition.Id = Id;
		return Condition;
	}

	template <class T>
	void SetProperty(UObject* Object, const TCHAR* Name, const T& Value)
	{
		if (FProperty* Property = Object->GetClass()->FindPropertyByName(Name))
		{
			*Property->ContainerPtrToValuePtr<T>(Object) = Value;
		}
	}

	/** Spawned deferred so Targets and States are set before BeginPlay captures the authored transforms. */
	ADCConditionalPresence* SpawnRule(const FDCTestWorld& World, const TArray<AActor*>& Targets)
	{
		ADCConditionalPresence* Rule = World.Get()->SpawnActorDeferred<ADCConditionalPresence>(
			ADCConditionalPresence::StaticClass(), FTransform(PivotAt), nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FDCPresenceState Hidden;
		Hidden.StateId = TEXT("gone");
		Hidden.Conditions = { Flag(Gone) };
		Hidden.bPresent = false;

		FDCPresenceState Elsewhere;
		Elsewhere.StateId = TEXT("moved");
		Elsewhere.Conditions = { Flag(Moved) };
		Elsewhere.bMove = true;
		Elsewhere.Placement = FTransform(FRotator(0.0, 90.0, 0.0), PlacementAt);

		TArray<TObjectPtr<AActor>> TargetRefs;
		for (AActor* Target : Targets)
		{
			TargetRefs.Add(Target);
		}
		SetProperty(Rule, TEXT("Targets"), TargetRefs);
		SetProperty(Rule, TEXT("States"), TArray<FDCPresenceState>{ Hidden, Elsewhere });
		Rule->FinishSpawning(FTransform(PivotAt));
		return Rule;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCConditionalPresenceTest, "DeadCurrent.World.ConditionalPresence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCConditionalPresenceTest::RunTest(const FString& Parameters)
{
	using namespace DCConditionalPresenceTest;
	FDCTestWorld World;
	UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();

	// Two targets around the pivot: A 1 m ahead of it, B 2 m to its side and turned.
	AStaticMeshActor* A = SpawnTarget(World, PivotAt + FVector(100.0, 0.0, 0.0), FRotator::ZeroRotator);
	AStaticMeshActor* B = SpawnTarget(World, PivotAt + FVector(0.0, 200.0, 0.0), FRotator(0.0, 30.0, 0.0));
	ADCConditionalPresence* Rule = SpawnRule(World, { A, B });
	Rule->SetObserverOverride(FVector(-100000.0, 0.0, 0.0)); // far away until a step says otherwise

	// No state passes: authored placement, present.
	TestEqual(TEXT("Default state has no id"), Rule->GetActiveStateId(), FName());
	TestTrue(TEXT("A at its authored spot"), A->GetActorLocation().Equals(PivotAt + FVector(100.0, 0.0, 0.0), 0.1));
	TestFalse(TEXT("A shown"), A->IsHidden());
	TestTrue(TEXT("A collides"), A->GetActorEnableCollision());

	// A flag change is picked up through the world-state signal, with no tick.
	WorldState->SetFlag(Moved);
	TestEqual(TEXT("Moved state applies when the flag is set"), Rule->GetActiveStateId(), FName(TEXT("moved")));
	TestTrue(TEXT("A keeps its offset, turned with the placement"),
		A->GetActorLocation().Equals(PlacementAt + FVector(0.0, 100.0, 0.0), 0.1));
	TestTrue(TEXT("B keeps its offset, turned with the placement"),
		B->GetActorLocation().Equals(PlacementAt + FVector(-200.0, 0.0, 0.0), 0.1));
	TestEqual(TEXT("B's own yaw is added to the placement's"), static_cast<int32>(FMath::RoundToInt(B->GetActorRotation().Yaw)), 120);
	TestTrue(TEXT("The rule actor stands at the placement"), Rule->GetActorLocation().Equals(PlacementAt, 0.1));

	// First passing state wins: hidden is listed first.
	WorldState->SetFlag(Gone);
	TestEqual(TEXT("First passing state wins"), Rule->GetActiveStateId(), FName(TEXT("gone")));
	TestTrue(TEXT("Hidden"), A->IsHidden() && B->IsHidden());
	TestFalse(TEXT("No collision while hidden, so traces and walking pass through"), A->GetActorEnableCollision());
	TestTrue(TEXT("A hidden state that does not move keeps the authored spot"),
		A->GetActorLocation().Equals(PivotAt + FVector(100.0, 0.0, 0.0), 0.1));

	WorldState->ClearFlag(Gone);
	TestEqual(TEXT("Back to the next passing state"), Rule->GetActiveStateId(), FName(TEXT("moved")));
	TestFalse(TEXT("Shown again"), A->IsHidden());
	TestTrue(TEXT("Collision restored as authored"), A->GetActorEnableCollision());

	// Observed: the player is at the placement. Clearing the flag must not snap the targets away under them.
	Rule->SetObserverOverride(PlacementAt + FVector(300.0, 0.0, 0.0));
	WorldState->ClearFlag(Moved);
	TestTrue(TEXT("Change waits while observed"), Rule->HasPendingChange());
	TestEqual(TEXT("Still in the old state"), Rule->GetActiveStateId(), FName(TEXT("moved")));
	Rule->Evaluate();
	TestTrue(TEXT("Still waiting on a re-check"), Rule->HasPendingChange());

	Rule->SetObserverOverride(FVector(-100000.0, 0.0, 0.0));
	Rule->Evaluate();
	TestFalse(TEXT("Applies once the player is away"), Rule->HasPendingChange());
	TestEqual(TEXT("Default again"), Rule->GetActiveStateId(), FName());
	TestTrue(TEXT("A back at its authored spot"), A->GetActorLocation().Equals(PivotAt + FVector(100.0, 0.0, 0.0), 0.1));
	TestEqual(TEXT("B back at its authored yaw"), static_cast<int32>(FMath::RoundToInt(B->GetActorRotation().Yaw)), 30);

	// Observed at the destination counts too: nothing appears at the player's feet.
	Rule->SetObserverOverride(PlacementAt + FVector(300.0, 0.0, 0.0));
	WorldState->SetFlag(Moved);
	TestTrue(TEXT("Waits while the player is near the new place"), Rule->HasPendingChange());
	TestEqual(TEXT("Nothing moved yet"), Rule->GetActiveStateId(), FName());

	// A restore (a load, a review setup) snaps regardless of the observer, and says so silently.
	WorldState->NotifyRestored();
	TestFalse(TEXT("A restore snaps"), Rule->HasPendingChange());
	TestEqual(TEXT("Restored to the passing state"), Rule->GetActiveStateId(), FName(TEXT("moved")));

	// Restores replace flags without broadcasting; the restore signal alone is enough to follow them.
	WorldState->ReplaceFlags({ Gone });
	WorldState->NotifyRestored();
	TestEqual(TEXT("Follows replaced flags on restore"), Rule->GetActiveStateId(), FName(TEXT("gone")));
	TestTrue(TEXT("Hidden after restore"), A->IsHidden());

	// A destroyed target (a pickup taken) is skipped, not an error.
	WorldState->ReplaceFlags({ Moved });
	B->Destroy();
	Rule->Snap();
	TestEqual(TEXT("Still applies with a target gone"), Rule->GetActiveStateId(), FName(TEXT("moved")));
	TestFalse(TEXT("The survivor is shown"), A->IsHidden());

	// It only listens: no flag written, nothing of its own to save.
	TestEqual(TEXT("Only the test's flag is set"), WorldState->GetFlags().Num(), 1);
	TestNull(TEXT("The rule has no persistent id"), Rule->FindComponentByClass<UDCPersistentIdComponent>());
	return true;
}

#endif
