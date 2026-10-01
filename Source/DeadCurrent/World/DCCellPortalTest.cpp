#include "Core/DCTestHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Save/DCPersistent.h"
#include "Save/DCPersistentIdComponent.h"
#include "TimerManager.h"
#include "World/DCCellPortal.h"
#include "World/DCConditionalPresence.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  ADCCellPortal (Phase 6, VS-03): locked text, first-match variants and their prompts, consequences applied by a use,
 *  arrival at the destination, the scene cut that snaps conditional presence (and is not a restore), the timed
 *  transition, and that a portal saves nothing. No map and no HUD: TryUse reports what a use did, and a plain
 *  ACharacter stands in for the player.
 */
namespace DCCellPortalTest
{
	const FName Key = TEXT("test.portal_key");
	const FName Unlocked = TEXT("test.portal_unlocked");
	const FName OpenUsed = TEXT("test.portal_open_used");
	const FName Moved = TEXT("test.portal_presence_moved");

	const FVector Start(0.0, 0.0, 100.0);
	const FVector Arrival(20000.0, 5000.0, 300.0);
	const float ArrivalYaw = 135.0f;

	template <class T>
	void SetProperty(UObject* Object, const TCHAR* Name, const T& Value)
	{
		if (FProperty* Property = Object->GetClass()->FindPropertyByName(Name))
		{
			*Property->ContainerPtrToValuePtr<T>(Object) = Value;
		}
	}

	FDCGameplayCondition Flag(FName Id)
	{
		FDCGameplayCondition Condition;
		Condition.Type = EDCConditionType::WorldFlag;
		Condition.Id = Id;
		return Condition;
	}

	FDCGameplayConsequence SetFlag(FName Id)
	{
		FDCGameplayConsequence Consequence;
		Consequence.Type = EDCConsequenceType::SetWorldFlag;
		Consequence.Id = Id;
		return Consequence;
	}

	FDCPortalVariant Variant(const TCHAR* Id, const TCHAR* Verb, TArray<FDCGameplayCondition> Conditions, TArray<FDCGameplayConsequence> Consequences)
	{
		FDCPortalVariant Result;
		Result.VariantId = Id;
		Result.Verb = FText::FromString(Verb);
		Result.Conditions = MoveTemp(Conditions);
		Result.Consequences = MoveTemp(Consequences);
		return Result;
	}

	ACharacter* SpawnWalker(const FDCTestWorld& World)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World.Get()->SpawnActor<ACharacter>(ACharacter::StaticClass(), FTransform(Start), Params);
	}

	ATargetPoint* SpawnDestination(const FDCTestWorld& World)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World.Get()->SpawnActor<ATargetPoint>(ATargetPoint::StaticClass(),
			FTransform(FRotator(0.0, ArrivalYaw, 0.0), Arrival), Params);
	}

	/** A portal whose first variant needs the key and sets Unlocked, and whose second always passes and sets OpenUsed. */
	ADCCellPortal* SpawnPortal(const FDCTestWorld& World, AActor* Destination, float FadeOut, float Hold, float FadeIn,
		bool bWithOpenVariant = true)
	{
		ADCCellPortal* Portal = World.Get()->SpawnActorDeferred<ADCCellPortal>(ADCCellPortal::StaticClass(),
			FTransform(FVector(100.0, 0.0, 0.0)), nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		TArray<FDCPortalVariant> Variants;
		Variants.Add(Variant(TEXT("key"), TEXT("Unlock"), { Flag(Key) }, { SetFlag(Unlocked) }));
		if (bWithOpenVariant)
		{
			Variants.Add(Variant(TEXT("open"), TEXT("Go down"), {}, { SetFlag(OpenUsed) }));
		}
		SetProperty(Portal, TEXT("Variants"), Variants);
		SetProperty(Portal, TEXT("DisplayName"), FText::FromString(TEXT("Vault hatch")));
		SetProperty(Portal, TEXT("LockedText"), FText::FromString(TEXT("The hatch is locked.")));
		SetProperty(Portal, TEXT("Destination"), TObjectPtr<AActor>(Destination));
		SetProperty(Portal, TEXT("FadeOutSeconds"), FadeOut);
		SetProperty(Portal, TEXT("HoldSeconds"), Hold);
		SetProperty(Portal, TEXT("FadeInSeconds"), FadeIn);
		Portal->FinishSpawning(FTransform(FVector(100.0, 0.0, 0.0)));
		return Portal;
	}

	/** A deferring presence rule (pivot at the arrival point) that moves its one target when Moved is set. */
	ADCConditionalPresence* SpawnPresence(const FDCTestWorld& World, AActor*& OutTarget)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Target = World.Get()->SpawnActor<AStaticMeshActor>(Arrival + FVector(200.0, 0.0, 0.0), FRotator::ZeroRotator, Params);
		Target->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
		OutTarget = Target;

		ADCConditionalPresence* Rule = World.Get()->SpawnActorDeferred<ADCConditionalPresence>(
			ADCConditionalPresence::StaticClass(), FTransform(Arrival), nullptr, nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		FDCPresenceState Elsewhere;
		Elsewhere.StateId = TEXT("moved");
		Elsewhere.Conditions = { Flag(Moved) };
		Elsewhere.bMove = true;
		Elsewhere.Placement = FTransform(Arrival + FVector(0.0, 600.0, 0.0));
		SetProperty(Rule, TEXT("Targets"), TArray<TObjectPtr<AActor>>{ Target });
		SetProperty(Rule, TEXT("States"), TArray<FDCPresenceState>{ Elsewhere });
		Rule->FinishSpawning(FTransform(Arrival));
		return Rule;
	}

	/**
	 *  Drives the world's timers, which is what a timed portal runs on. FTimerManager::Tick runs at most once per engine
	 *  frame (it compares GFrameCounter), so each step advances the frame counter as a real frame would.
	 */
	void Advance(const FDCTestWorld& World, float Seconds)
	{
		const int32 Steps = FMath::Max(1, FMath::CeilToInt(Seconds / 0.05f));
		for (int32 Step = 0; Step < Steps; ++Step)
		{
			++GFrameCounter;
			World.Get()->GetTimerManager().Tick(Seconds / Steps);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCCellPortalTest, "DeadCurrent.World.CellPortal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCCellPortalTest::RunTest(const FString& Parameters)
{
	using namespace DCCellPortalTest;

	// --- Locked, prompts, first match, consequences, arrival (instant mode) ---
	{
		FDCTestWorld World;
		UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		ACharacter* Walker = SpawnWalker(World);
		ADCCellPortal* Locked = SpawnPortal(World, SpawnDestination(World), 0.0f, 0.0f, 0.0f, /*bWithOpenVariant*/ false);

		TestTrue(TEXT("A portal with every duration zero is instant"), Locked->IsInstant());
		TestTrue(TEXT("A locked portal can still be used (to read why it is locked)"), IDCInteractable::Execute_CanInteract(Locked, Walker));
		const FDCInteractionPrompt LockedPrompt = IDCInteractable::Execute_GetInteractionPrompt(Locked, Walker);
		TestEqual(TEXT("Locked prompt verb"), LockedPrompt.Action.ToString(), FString(TEXT("Try")));
		TestEqual(TEXT("Prompt target is the display name"), LockedPrompt.TargetName.ToString(), FString(TEXT("Vault hatch")));
		TestEqual(TEXT("No variant passes: locked"), Locked->TryUse(Walker), EDCPortalUse::Locked);
		TestTrue(TEXT("Locked: the player does not move"), Walker->GetActorLocation().Equals(Start, 1.0));
		TestFalse(TEXT("Locked: no consequence runs"), WorldState->HasFlag(Unlocked));

		ADCCellPortal* Portal = SpawnPortal(World, SpawnDestination(World), 0.0f, 0.0f, 0.0f);
		TestEqual(TEXT("Without the key the open variant is used"),
			IDCInteractable::Execute_GetInteractionPrompt(Portal, Walker).Action.ToString(), FString(TEXT("Go down")));

		WorldState->SetFlag(Key);
		TestEqual(TEXT("First passing variant wins (the key variant is listed first)"),
			IDCInteractable::Execute_GetInteractionPrompt(Portal, Walker).Action.ToString(), FString(TEXT("Unlock")));

		Walker->GetCharacterMovement()->Velocity = FVector(600.0, 0.0, 0.0);
		TestEqual(TEXT("Instant use passes"), Portal->TryUse(Walker), EDCPortalUse::Passed);
		TestTrue(TEXT("The chosen variant's consequence ran"), WorldState->HasFlag(Unlocked));
		TestFalse(TEXT("Only the chosen variant's consequences run"), WorldState->HasFlag(OpenUsed));
		TestFalse(TEXT("An instant use leaves no transition running"), Portal->IsTransitioning());
		TestTrue(TEXT("Arrived at the destination"), Walker->GetActorLocation().Equals(Arrival, 1.0));
		TestEqual(TEXT("Facing the destination's yaw"), static_cast<int32>(FMath::RoundToInt(Walker->GetActorRotation().Yaw)), static_cast<int32>(FMath::RoundToInt(ArrivalYaw)));
		TestTrue(TEXT("Arrives at rest"), Walker->GetCharacterMovement()->Velocity.IsNearlyZero());

		// A second use applies the consequences again: each use is one use.
		WorldState->ClearFlag(Unlocked);
		Walker->SetActorLocation(Start);
		TestEqual(TEXT("Second use passes"), Portal->TryUse(Walker), EDCPortalUse::Passed);
		TestTrue(TEXT("Consequences run once per use"), WorldState->HasFlag(Unlocked));

		// Saves nothing: no persistent id, not a persistent actor.
		TestNull(TEXT("No persistent id component"), Portal->FindComponentByClass<UDCPersistentIdComponent>());
		TestFalse(TEXT("Not IDCPersistent"), Portal->GetClass()->ImplementsInterface(UDCPersistent::StaticClass()));

		// No destination: refused, with an error in the log.
		AddExpectedError(TEXT("has no Destination"), EAutomationExpectedErrorFlags::Contains, 1);
		ADCCellPortal* Nowhere = SpawnPortal(World, nullptr, 0.0f, 0.0f, 0.0f);
		Walker->SetActorLocation(Start);
		TestEqual(TEXT("A portal with no destination does nothing"), Nowhere->TryUse(Walker), EDCPortalUse::Ignored);
		TestTrue(TEXT("...and does not move the player"), Walker->GetActorLocation().Equals(Start, 1.0));
	}

	// --- The scene cut snaps conditional presence, and is not a restore ---
	{
		FDCTestWorld World;
		UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		ACharacter* Walker = SpawnWalker(World);
		ADCCellPortal* Portal = SpawnPortal(World, SpawnDestination(World), 0.0f, 0.0f, 0.0f);

		int32 Cuts = 0;
		int32 Restores = 0;
		WorldState->OnSceneCut.AddLambda([&Cuts]() { ++Cuts; });
		WorldState->OnRestored.AddLambda([&Restores]() { ++Restores; });

		AActor* Target = nullptr;
		ADCConditionalPresence* Rule = SpawnPresence(World, Target);
		Rule->SetObserverOverride(Arrival); // the player is standing where the change would happen

		WorldState->SetFlag(Moved);
		TestTrue(TEXT("A flag change alone defers while observed"), Rule->HasPendingChange());
		TestEqual(TEXT("...and the targets have not moved yet"), Rule->GetActiveStateId(), FName());

		TestEqual(TEXT("Use the portal"), Portal->TryUse(Walker), EDCPortalUse::Passed);
		TestEqual(TEXT("The portal signalled one scene cut"), Cuts, 1);
		TestEqual(TEXT("A portal use is not a restore"), Restores, 0);
		TestFalse(TEXT("The scene cut applied the pending change"), Rule->HasPendingChange());
		TestEqual(TEXT("Presence is in its new state after the cut"), Rule->GetActiveStateId(), FName(TEXT("moved")));
		TestTrue(TEXT("The target moved with it"), Target->GetActorLocation().Equals(Arrival + FVector(200.0, 600.0, 0.0), 1.0));

		WorldState->NotifyRestored();
		TestEqual(TEXT("A restore is not a scene cut"), Cuts, 1);
		TestEqual(TEXT("The restore signal fired once"), Restores, 1);
	}

	// --- A timed use: dark first, then the move, then the fade back; uses in between are ignored ---
	{
		FDCTestWorld World;
		UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();
		ACharacter* Walker = SpawnWalker(World);
		ADCCellPortal* Portal = SpawnPortal(World, SpawnDestination(World), 0.3f, 0.2f, 0.3f);
		int32 Cuts = 0;
		WorldState->OnSceneCut.AddLambda([&Cuts]() { ++Cuts; });

		TestFalse(TEXT("Timed portal"), Portal->IsInstant());
		TestEqual(TEXT("A timed use starts"), Portal->TryUse(Walker), EDCPortalUse::Passed);
		TestTrue(TEXT("Transition running"), Portal->IsTransitioning());
		TestFalse(TEXT("A running transition cannot be used again"), IDCInteractable::Execute_CanInteract(Portal, Walker));
		TestEqual(TEXT("A second use while fading is ignored"), Portal->TryUse(Walker), EDCPortalUse::Ignored);
		TestTrue(TEXT("Not moved before the screen is dark"), Walker->GetActorLocation().Equals(Start, 1.0));
		TestFalse(TEXT("No consequence before the screen is dark"), WorldState->HasFlag(OpenUsed));
		TestEqual(TEXT("No scene cut before the screen is dark"), Cuts, 0);

		// A timer set during a frame starts counting at the next tick, so allow one step beyond the fade-out.
		Advance(World, 0.45f);
		TestTrue(TEXT("At black: moved"), Walker->GetActorLocation().Equals(Arrival, 1.0));
		TestTrue(TEXT("At black: the consequence ran"), WorldState->HasFlag(OpenUsed));
		TestEqual(TEXT("At black: one scene cut"), Cuts, 1);
		TestTrue(TEXT("Still transitioning while black and fading in"), Portal->IsTransitioning());

		Advance(World, 1.0f);
		TestFalse(TEXT("Transition over after the fade in"), Portal->IsTransitioning());
		TestTrue(TEXT("Usable again"), IDCInteractable::Execute_CanInteract(Portal, Walker));
		TestEqual(TEXT("Still exactly one scene cut"), Cuts, 1);
	}

	// --- While one portal's transition runs, every portal refuses (VS-04: the way back out stands at the arrival) ---
	{
		FDCTestWorld World;
		ACharacter* Walker = SpawnWalker(World);
		ADCCellPortal* In = SpawnPortal(World, SpawnDestination(World), 0.3f, 0.2f, 0.3f);
		ADCCellPortal* Back = SpawnPortal(World, SpawnDestination(World), 0.3f, 0.2f, 0.3f);

		TestEqual(TEXT("The first portal starts"), In->TryUse(Walker), EDCPortalUse::Passed);
		TestFalse(TEXT("Another portal offers no use while a transition runs"), IDCInteractable::Execute_CanInteract(Back, Walker));
		TestEqual(TEXT("Another portal is ignored while fading out"), Back->TryUse(Walker), EDCPortalUse::Ignored);
		Advance(World, 0.45f);
		TestTrue(TEXT("Moved by the first portal"), Walker->GetActorLocation().Equals(Arrival, 1.0));
		TestEqual(TEXT("Another portal is ignored while black and fading in"), Back->TryUse(Walker), EDCPortalUse::Ignored);
		TestFalse(TEXT("The other portal never started"), Back->IsTransitioning());
		Advance(World, 1.0f);
		TestFalse(TEXT("The first transition is over"), In->IsTransitioning());
		TestTrue(TEXT("The other portal is usable afterwards"), IDCInteractable::Execute_CanInteract(Back, Walker));
		TestEqual(TEXT("And passes"), Back->TryUse(Walker), EDCPortalUse::Passed);
	}

	return true;
}

#endif
