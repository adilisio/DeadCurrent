#include "Core/DCMapTestHelpers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Combat/DCDamageable.h"
#include "Core/DCGameplayTags.h"
#include "Components/LightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PostProcessVolume.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "HighResScreenshot.h"
#include "Interaction/DCInteractorComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Save/DCSaveGame.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "World/DCCellPortal.h"
#include "World/DCConditionalPresence.h"
#include "World/DCFlickerLight.h"

/**
 *  Phase 6 VS-04: the map architecture of Lvl_PointeSombre, proved on the real map (game context, run with
 *  Tools\RunTests.bat Map.Sombre). Integrator-owned. These tests use the architecture fixture (pointe_sombre/arch_test.py:
 *  a test door by the quay into an abstracted interior cell) and the core script's own actors, never slice content,
 *  so they hold however the cells grow.
 *
 *    Architecture   walk to a portal; the real interact path; fade, input lock, scene-cut snap, arrival facing;
 *                   F5 inside the cell, diverge, F9 back into it; a locked portal; the way back; the interior
 *                   lighting rule; the terrain under seeded probe points
 *    CrossMapLoad   a shore save loaded from the slice, a slice save loaded from the shore, a Phase 5 save from the slice
 *    Respawn        the player start follows the story: deck, quay after the reef strike, tower base once the vault opens
 *    Atmosphere     exactly one of the three looks for every flag combination; a load restores it
 */
namespace DCSombreArchTest
{
	using namespace DCMapTest;

	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_PointeSombre");
	const TCHAR* ShorePath = TEXT("/Game/Maps/Lvl_Boathouse");
	const TCHAR* TestSlot = TEXT("DeadCurrent_SombreArchTest");

	const FName Moved = TEXT("dev.arch_moved");
	const FName Unlocked = TEXT("dev.arch_unlocked");
	const FName ReefStruck = TEXT("sombre.reef_struck");
	const FName VaultOpened = TEXT("sombre.vault_opened");
	const FName HaleArrived = TEXT("sombre.hale_arrived");
	const FName MeetingDone = TEXT("sombre.meeting_done");
	const FString LockedText = TEXT("The back door is locked. (Architecture test door: dev.arch_unlocked opens it.)");

	/** The interior slots are 2.5 km east of the island (build_pointe_sombre.py INTERIOR_SLOTS). */
	constexpr double InteriorSlotY = 200000.0;

	inline AActor* Tagged(FName Tag)
	{
		for (TActorIterator<AActor> It(GameWorld()); It; ++It)
		{
			if (It->ActorHasTag(Tag))
			{
				return *It;
			}
		}
		return nullptr;
	}

	inline TArray<AActor*> AllTagged(FName Tag)
	{
		TArray<AActor*> Found;
		for (TActorIterator<AActor> It(GameWorld()); It; ++It)
		{
			if (It->ActorHasTag(Tag))
			{
				Found.Add(*It);
			}
		}
		return Found;
	}

	inline ADCCellPortal* Portal(const TCHAR* Tag) { return Cast<ADCCellPortal>(Tagged(Tag)); }

	inline ADCConditionalPresence* Rule(const TCHAR* Tag) { return Cast<ADCConditionalPresence>(Tagged(Tag)); }

	inline APlayerController* PC()
	{
		return Player() ? Cast<APlayerController>(Player()->GetController()) : nullptr;
	}

	inline float FadeAmount()
	{
		const APlayerController* Controller = PC();
		const APlayerCameraManager* Camera = Controller ? Controller->PlayerCameraManager.Get() : nullptr;
		return Camera && Camera->bEnableFading ? Camera->FadeAmount : 0.0f;
	}

	inline FVector At(const TCHAR* Tag)
	{
		const AActor* Actor = Tagged(Tag);
		return Actor ? Actor->GetActorLocation() : FVector(NAN);
	}

	inline FString Level()
	{
		return GameWorld() ? UGameplayStatics::GetCurrentLevelName(GameWorld(), true) : FString();
	}

	inline APlayerStart* Start()
	{
		TActorIterator<APlayerStart> It(GameWorld());
		return It ? *It : nullptr;
	}

	/** Opens a map without touching the save slot (QueueFreshMap empties it), and waits until it has loaded. */
	inline void QueueOpen(const TCHAR* Path)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Path]()
		{
			GEngine->Exec(GameWorld(), *FString::Printf(TEXT("Open %s"), Path));
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	}

	/** With rendering, a screenshot for the review record (Saved/Screenshots/<platform>/<Name>.png). */
	inline void Shot(const TCHAR* Name)
	{
		if (FApp::CanEverRender())
		{
			FScreenshotRequest::RequestScreenshot(Name, false, false);
		}
	}

	/** Each look's five targets (sun, fill light, sky, fog, post-process volume) are tagged Atmosphere:<look>. */
	constexpr int32 LookTargets = 5;

	inline bool LookActive(const TCHAR* Look)
	{
		const TArray<AActor*> Targets = AllTagged(*FString::Printf(TEXT("Atmosphere:%s"), Look));
		if (Targets.Num() != LookTargets)
		{
			return false;
		}
		for (const AActor* Target : Targets)
		{
			if (Target->IsHidden() || Target->GetActorLocation().Z < -100000.0)
			{
				return false;
			}
		}
		return true;
	}

	inline bool LookParked(const TCHAR* Look)
	{
		const TArray<AActor*> Targets = AllTagged(*FString::Printf(TEXT("Atmosphere:%s"), Look));
		for (const AActor* Target : Targets)
		{
			if (!Target->IsHidden() || Target->GetActorLocation().Z > -100000.0)
			{
				return false;
			}
		}
		return Targets.Num() == LookTargets;
	}

	inline FString ActiveLooks()
	{
		TArray<FString> Active;
		for (const TCHAR* Look : { TEXT("storm"), TEXT("calm"), TEXT("night") })
		{
			if (LookActive(Look))
			{
				Active.Add(Look);
			}
			else if (!LookParked(Look))
			{
				Active.Add(FString(Look) + TEXT("(half)"));
			}
		}
		return FString::Join(Active, TEXT(","));
	}

	/** The lightning of a look (ADCFlickerLight tagged Lightning:<look>) is flashing, not switched off. */
	inline bool LightningActive(const TCHAR* Look)
	{
		const ADCFlickerLight* Flicker = Cast<ADCFlickerLight>(Tagged(*FString::Printf(TEXT("Lightning:%s"), Look)));
		return Flicker && Flicker->IsLightActive();
	}

	/** The terrain under (X, Y): a downward trace that looks through anything that is not a terrain tile. */
	inline bool TraceTerrain(double X, double Y, FHitResult& OutHit)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(SombreProbe), true, Player());
		for (int32 Attempt = 0; Attempt < 6; ++Attempt)
		{
			if (!GameWorld()->LineTraceSingleByChannel(OutHit, FVector(X, Y, 20000.0), FVector(X, Y, -3000.0), ECC_Visibility, Params))
			{
				return false;
			}
			AActor* HitActor = OutHit.GetActor();
			if (HitActor && HitActor->ActorHasTag(TEXT("SombreTerrain")))
			{
				return true;
			}
			Params.AddIgnoredActor(HitActor);
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreArchitectureTest, "DeadCurrent.Map.Sombre.Architecture",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreArchitectureTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreArchTest;
	QueueFreshMap(MapPath, TestSlot);

	// The new game: on the crossing deck, in the storm. The fixture is where the core script put it.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Door in"), Portal(TEXT("ArchTest:DoorIn")))
			|| !TestNotNull(TEXT("Door out"), Portal(TEXT("ArchTest:DoorOut")))
			|| !TestNotNull(TEXT("Back door"), Portal(TEXT("ArchTest:BackDoor")))
			|| !TestNotNull(TEXT("Crate rule"), Rule(TEXT("Presence_TestCrate"))))
		{
			return true;
		}
		TestEqual(TEXT("The map is Lvl_PointeSombre"), Level(), FString(TEXT("Lvl_PointeSombre")));
		TestTrue(TEXT("New game starts on the crossing deck"),
			FVector::Dist2D(Player()->GetActorLocation(), At(TEXT("Anchor:Anchor_NewGame_Deck"))) < 150.0);
		TestTrue(TEXT("New game is in the storm"), LookActive(TEXT("storm")));

		// Interior placement rule: the fixed parts of an interior cell are on lighting channel 1 only, out of the
		// exterior sun's reach, and drawn only from nearby. The exterior sun is channel 0 only.
		int32 InteriorPrimitives = 0;
		for (AActor* Actor : AllTagged(TEXT("Cell:arch_test")))
		{
			if (Actor->GetActorLocation().Y < InteriorSlotY || Actor->IsA<ADCConditionalPresence>())
			{
				continue;
			}
			TInlineComponentArray<UPrimitiveComponent*> Primitives(Actor);
			for (const UPrimitiveComponent* Primitive : Primitives)
			{
				if (Primitive->IsA<UStaticMeshComponent>())
				{
					++InteriorPrimitives;
					TestFalse(*FString::Printf(TEXT("%s is off channel 0"), *Actor->GetName()), Primitive->LightingChannels.bChannel0);
					TestTrue(*FString::Printf(TEXT("%s is on channel 1"), *Actor->GetName()), Primitive->LightingChannels.bChannel1);
					TestTrue(*FString::Printf(TEXT("%s has a draw distance"), *Actor->GetName()), Primitive->LDMaxDrawDistance > 0.0f);
				}
			}
		}
		TestTrue(TEXT("The interior cell has geometry to check"), InteriorPrimitives >= 6);
		for (const AActor* Target : AllTagged(TEXT("Atmosphere:storm")))
		{
			if (const ADirectionalLight* Sun = Cast<ADirectionalLight>(Target))
			{
				const ULightComponent* Light = Sun->GetLightComponent();
				TestTrue(TEXT("The sun lights channel 0"), Light->LightingChannels.bChannel0);
				TestFalse(TEXT("The sun does not light channel 1"), Light->LightingChannels.bChannel1);
			}
		}

		// Terrain: traces at seeded grid vertices meet the surface at its height, facing up.
		FString ProbeText;
		const FString ProbePath = FPaths::ProjectDir() / TEXT("Tools/PointeSombre/out/terrain_probe.json");
		TSharedPtr<FJsonObject> Probe;
		if (TestTrue(TEXT("Terrain probe file"), FFileHelper::LoadFileToString(ProbeText, *ProbePath))
			&& TestTrue(TEXT("Terrain probe parses"), FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(ProbeText), Probe) && Probe.IsValid()))
		{
			int32 Points = 0, OnTerrain = 0, FacingUp = 0;
			double Worst = 0.0;
			for (const TSharedPtr<FJsonValue>& Value : Probe->GetArrayField(TEXT("points")))
			{
				const TArray<TSharedPtr<FJsonValue>> P = Value->AsArray();
				++Points;
				FHitResult Hit;
				if (!TraceTerrain(P[0]->AsNumber(), P[1]->AsNumber(), Hit))
				{
					continue;
				}
				++OnTerrain;
				FacingUp += Hit.ImpactNormal.Z > 0.0 ? 1 : 0;
				Worst = FMath::Max(Worst, FMath::Abs(Hit.ImpactPoint.Z - P[2]->AsNumber()));
			}
			AddInfo(FString::Printf(TEXT("Terrain probe: %d points, %d on terrain, %d facing up, worst height error %.1f cm"),
				Points, OnTerrain, FacingUp, Worst));
			TestTrue(TEXT("Every probe point is solid ground"), Points > 100 && OnTerrain == Points);
			TestEqual(TEXT("The terrain faces up everywhere"), FacingUp, OnTerrain);
			TestTrue(TEXT("The surface is where the heightfield says (2 cm)"), Worst <= 2.0);
		}
		return true;
	}));

	// Walk up to the test door: start 6 m east of it, facing it, and walk until the interaction trace finds it.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		const FVector Door = Portal(TEXT("ArchTest:DoorIn"))->GetActorLocation();
		FVector Outside = At(TEXT("ArchTest:Outside"));
		Teleport(FVector(Outside.X, Outside.Y + 450.0, Outside.Z + 20.0));
		const FRotator Facing = (FVector(Door.X, Door.Y, 0.0) - FVector(Outside.X, Outside.Y + 450.0, 0.0)).Rotation();
		PC()->SetControlRotation(FRotator(0.0, Facing.Yaw, 0.0));
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	TSharedRef<FVector> WalkStart = MakeShared<FVector>(FVector::ZeroVector);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([WalkStart]()
	{
		*WalkStart = Player()->GetActorLocation();
		return true;
	}));
	QueueWaitUntil(this, []()
	{
		ADCPlayerCharacter* P = Player();
		ADCCellPortal* Door = Portal(TEXT("ArchTest:DoorIn"));
		if (!P || !Door)
		{
			return false;
		}
		if (P->GetInteractorComponent()->GetFocusedActor() == Door)
		{
			return true;
		}
		P->AddMovementInput(P->GetControlRotation().Vector(), 1.0f);
		return false;
	}, TEXT("walking up to the test door until the interaction trace focuses it"), 10.0);

	// A presence change made while the player stands beside the crate waits (they would see it vanish).
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, WalkStart]()
	{
		const double Walked = FVector::Dist2D(*WalkStart, Player()->GetActorLocation());
		AddInfo(FString::Printf(TEXT("Walked %.0f cm on the terrain to the door"), Walked));
		TestTrue(TEXT("The player walked (not teleported) to the door"), Walked > 150.0);
		FDCInteractionPrompt Prompt;
		TestTrue(TEXT("The door shows a prompt"), Player()->GetInteractorComponent()->GetFocusedPrompt(Prompt));
		TestEqual(TEXT("Prompt verb"), Prompt.Action.ToString(), FString(TEXT("Go in")));
		TestEqual(TEXT("Prompt target"), Prompt.TargetName.ToString(), FString(TEXT("Test door")));
		WorldState()->SetFlag(Moved);
		Rule(TEXT("Presence_TestCrate"))->Evaluate();
		TestTrue(TEXT("The crate's move waits while the player is beside it"), Rule(TEXT("Presence_TestCrate"))->HasPendingChange());
		Shot(TEXT("SombreArch_1_AtDoor"));
		TestTrue(TEXT("E on the focused door"), Player()->GetInteractorComponent()->TryInteract());
		ADCCellPortal* Door = Portal(TEXT("ArchTest:DoorIn"));
		TestTrue(TEXT("The transition is running"), Door->IsTransitioning());
		TestTrue(TEXT("Movement input is off"), PC()->IsMoveInputIgnored());
		TestTrue(TEXT("Look input is off"), PC()->IsLookInputIgnored());
		return true;
	}));

	// The fade-out, sampled every frame until the player is moved: it starts clear, only darkens, passes through
	// partly dark (game time, not wall time: a rendered run hitches), and is black at the move.
	TSharedRef<TArray<float>> Fades = MakeShared<TArray<float>>();
	QueueWaitUntil(this, [Fades]()
	{
		if (!Player())
		{
			return false;
		}
		if (Player()->GetActorLocation().Y > InteriorSlotY)
		{
			return true;
		}
		Fades->Add(FadeAmount());
		if (Fades->Num() == 3)
		{
			Shot(TEXT("SombreArch_2_FadingOut"));
		}
		return false;
	}, TEXT("the move into the cell"), 3.0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Fades]()
	{
		bool bRising = true, bPartly = false;
		for (int32 Index = 0; Index < Fades->Num(); ++Index)
		{
			bRising &= Index == 0 || (*Fades)[Index] + 0.001f >= (*Fades)[Index - 1];
			bPartly |= (*Fades)[Index] > 0.02f && (*Fades)[Index] < 0.98f;
		}
		TArray<FString> Shown;
		for (const float Fade : *Fades)
		{
			Shown.Add(FString::Printf(TEXT("%.2f"), Fade));
		}
		AddInfo(FString::Printf(TEXT("Fade-out over %d frames before the move: %s"), Fades->Num(), *FString::Join(Shown, TEXT(" "))));
		TestTrue(TEXT("The fade-out starts clear"), Fades->Num() > 0 && (*Fades)[0] < 0.5f);
		TestTrue(TEXT("The fade-out only darkens"), bRising);
		TestTrue(TEXT("The screen was partly dark on the way to black"), bPartly);
		return true;
	}));

	// At black: moved into the cell, the crate snapped in on the scene cut, input still off.
	TSharedRef<FVector> Arrived = MakeShared<FVector>(FVector::ZeroVector);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Arrived]()
	{
		*Arrived = Player()->GetActorLocation();
		TestTrue(TEXT("At black when moved"), FadeAmount() > 0.95f);
		TestTrue(TEXT("Arrived at the cell's arrival point"), FVector::Dist(*Arrived, At(TEXT("ArchTest:Inside"))) < 60.0);
		TestTrue(TEXT("Arrived at rest"), Player()->GetVelocity().Size2D() < 1.0);
		ADCConditionalPresence* Crate = Rule(TEXT("Presence_TestCrate"));
		TestFalse(TEXT("The scene cut applied the waiting change"), Crate->HasPendingChange());
		TestEqual(TEXT("The crate is inside"), Crate->GetActiveStateId(), FName(TEXT("inside")));
		TestTrue(TEXT("Input still off at black"), PC()->IsMoveInputIgnored() && PC()->IsLookInputIgnored());
		Shot(TEXT("SombreArch_3_Arrived"));

		// Other inputs during the transition must not corrupt it. The way back out stands at the arrival point:
		// it is refused (no second transition, no return on the same press); E and jump change nothing lasting.
		ADCCellPortal* Out = Portal(TEXT("ArchTest:DoorOut"));
		TestFalse(TEXT("The way back offers no use mid-transition"), IDCInteractable::Execute_CanInteract(Out, Player()));
		TestEqual(TEXT("The way back is refused mid-transition"), Out->TryUse(Player()), EDCPortalUse::Ignored);
		Player()->GetInteractorComponent()->TryInteract();
		Player()->Jump();
		TestFalse(TEXT("No second transition started"), Out->IsTransitioning());
		return true;
	}));

	// While the screen is dark, movement input does nothing.
	QueueWaitUntil(this, [Arrived]()
	{
		ADCCellPortal* Door = Portal(TEXT("ArchTest:DoorIn"));
		if (Door && Door->IsTransitioning())
		{
			Player()->AddMovementInput(FVector(1.0, 0.0, 0.0), 1.0f);
			return false;
		}
		return true;
	}, TEXT("the transition to finish"), 4.0);
	// Let a jump made during the transition land before judging where the player stands.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Arrived]()
	{
		TestTrue(TEXT("Movement input during the transition did not move the player"),
			FVector::Dist2D(*Arrived, Player()->GetActorLocation()) < 5.0);
		TestFalse(TEXT("The way back did not run"), Portal(TEXT("ArchTest:DoorOut"))->IsTransitioning());

		// Standing cleanly on the cell floor: on the ground, the capsule centre a half-height above it, and the
		// capsule (a hair smaller) overlapping nothing. Destination markers are authored at standing capsule-centre
		// height because the portal puts the capsule centre on the marker.
		const UCapsuleComponent* Capsule = Player()->GetCapsuleComponent();
		TestTrue(TEXT("On the ground after arrival"), Player()->GetCharacterMovement()->IsMovingOnGround());
		FHitResult Floor;
		const FVector Feet = Player()->GetActorLocation();
		FCollisionQueryParams FloorParams(SCENE_QUERY_STAT(SombreFloor), false, Player());
		if (TestTrue(TEXT("A floor under the arrival"), GameWorld()->LineTraceSingleByChannel(Floor, Feet, Feet - FVector(0.0, 0.0, 400.0), ECC_Visibility, FloorParams)))
		{
			const double Gap = Feet.Z - Capsule->GetScaledCapsuleHalfHeight() - Floor.ImpactPoint.Z;
			AddInfo(FString::Printf(TEXT("Arrival: capsule bottom %.1f cm above the floor"), Gap));
			TestTrue(TEXT("Standing on the floor (within 3 cm)"), FMath::Abs(Gap) < 3.0);
		}
		FCollisionQueryParams OverlapParams(SCENE_QUERY_STAT(SombreOverlap), false, Player());
		TestFalse(TEXT("The capsule overlaps nothing"), GameWorld()->OverlapBlockingTestByChannel(Feet, Capsule->GetComponentQuat(),
			ECC_Pawn, FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius() - 2.0f, Capsule->GetScaledCapsuleHalfHeight() - 2.0f), OverlapParams));
		TestFalse(TEXT("Movement input back after the fade in"), PC()->IsMoveInputIgnored());
		TestFalse(TEXT("Look input back after the fade in"), PC()->IsLookInputIgnored());
		TestTrue(TEXT("The screen is clear"), FadeAmount() < 0.05f);
		const FRotator Facing = PC()->GetControlRotation();
		TestTrue(TEXT("Facing the arrival point's yaw (90)"), FMath::Abs(FRotator::NormalizeAxis(Facing.Yaw - 90.0)) < 1.0);
		TestTrue(TEXT("Pitch levelled on arrival"), FMath::Abs(FRotator::NormalizeAxis(Facing.Pitch)) < 1.0);
		Shot(TEXT("SombreArch_4_Inside"));

		// F5 inside the cell, then diverge: back outside, the crate's flag cleared.
		TestTrue(TEXT("Save inside the cell"), Saves()->SaveCurrentGame());
		WorldState()->ClearFlag(Moved);
		Teleport(At(TEXT("Anchor:Anchor_CrossingExit_Quay")));
		return true;
	}));

	QueueLoad(this);

	// F9 reopened the map with the player inside the cell and the crate inside.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player after F9"), Player()))
		{
			return true;
		}
		TestTrue(TEXT("F9 put the player back inside the cell"),
			FVector::Dist(Player()->GetActorLocation(), At(TEXT("ArchTest:Inside"))) < 80.0);
		TestTrue(TEXT("F9 restored the flag"), WorldState()->HasFlag(Moved));
		TestEqual(TEXT("F9: the crate is inside"), Rule(TEXT("Presence_TestCrate"))->GetActiveStateId(), FName(TEXT("inside")));

		// A locked portal refuses, shows its text, and leaves the player where they are.
		ADCCellPortal* Back = Portal(TEXT("ArchTest:BackDoor"));
		const FVector Before = Player()->GetActorLocation();
		TestEqual(TEXT("Locked prompt verb"), IDCInteractable::Execute_GetInteractionPrompt(Back, Player()).Action.ToString(), FString(TEXT("Try")));
		TestEqual(TEXT("Locked back door refuses"), Back->TryUse(Player()), EDCPortalUse::Locked);
		TestEqual(TEXT("Locked text shown"), Message(), LockedText);
		TestFalse(TEXT("Locked: no transition"), Back->IsTransitioning());
		TestTrue(TEXT("Locked: not moved"), FVector::Dist(Before, Player()->GetActorLocation()) < 1.0);
		WorldState()->SetFlag(Unlocked);
		TestEqual(TEXT("Unlocked prompt verb"), IDCInteractable::Execute_GetInteractionPrompt(Back, Player()).Action.ToString(), FString(TEXT("Go through")));
		WorldState()->ClearFlag(Unlocked);

		// The way back out, timed like in play.
		TestEqual(TEXT("Door out passes"), Portal(TEXT("ArchTest:DoorOut"))->TryUse(Player()), EDCPortalUse::Passed);
		return true;
	}));
	QueueWaitUntil(this, []() { return Portal(TEXT("ArchTest:DoorOut")) && !Portal(TEXT("ArchTest:DoorOut"))->IsTransitioning(); },
		TEXT("the way back out"), 4.0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestTrue(TEXT("Back outside at the door"), FVector::Dist(Player()->GetActorLocation(), At(TEXT("ArchTest:Outside"))) < 60.0);
		TestTrue(TEXT("Facing away from the door (yaw 90)"),
			FMath::Abs(FRotator::NormalizeAxis(PC()->GetControlRotation().Yaw - 90.0)) < 1.0);
		TestFalse(TEXT("Input back"), PC()->IsMoveInputIgnored());
		TestTrue(TEXT("Still the storm outside"), LookActive(TEXT("storm")));
		Shot(TEXT("SombreArch_5_Outside"));
		return true;
	}));

	QueueCleanup(TestSlot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreCrossMapLoadTest, "DeadCurrent.Map.Sombre.CrossMapLoad",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreCrossMapLoadTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreArchTest;
	const FName ShoreFlag = TEXT("dev.crossmap_shore");
	const FName SliceFlag = TEXT("dev.crossmap_slice");
	const FVector InBoathouse(300.0, 0.0, 100.0);
	TSharedRef<FVector> Quay = MakeShared<FVector>(FVector::ZeroVector);

	// 1. A shore save, loaded from the slice, opens the shore with its state.
	QueueFreshMap(ShorePath, TestSlot);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, ShoreFlag, InBoathouse]()
	{
		TestEqual(TEXT("On the shore"), Level(), FString(TEXT("Lvl_Boathouse")));
		WorldState()->SetFlag(ShoreFlag);
		Teleport(InBoathouse);
		TestTrue(TEXT("Shore save"), Saves()->SaveCurrentGame());
		return true;
	}));
	QueueOpen(MapPath);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, ShoreFlag]()
	{
		TestEqual(TEXT("In the slice"), Level(), FString(TEXT("Lvl_PointeSombre")));
		TestFalse(TEXT("The slice starts without the shore's flag"), WorldState()->HasFlag(ShoreFlag));
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, ShoreFlag, InBoathouse]()
	{
		TestEqual(TEXT("F9 of a shore save from the slice opens the shore"), Level(), FString(TEXT("Lvl_Boathouse")));
		TestTrue(TEXT("Shore state restored"), WorldState() && WorldState()->HasFlag(ShoreFlag));
		TestTrue(TEXT("Shore position restored"), Player() && FVector::Dist2D(Player()->GetActorLocation(), InBoathouse) < 50.0);
		return true;
	}));

	// 2. A slice save, loaded from the shore, opens the slice with its state.
	QueueOpen(MapPath);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SliceFlag, Quay]()
	{
		TestEqual(TEXT("In the slice again"), Level(), FString(TEXT("Lvl_PointeSombre")));
		*Quay = At(TEXT("Anchor:Anchor_CrossingExit_Quay"));
		WorldState()->SetFlag(SliceFlag);
		WorldState()->SetFlag(ReefStruck);
		Teleport(*Quay);
		TestTrue(TEXT("Slice save"), Saves()->SaveCurrentGame());
		return true;
	}));
	QueueOpen(ShorePath);
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SliceFlag, Quay]()
	{
		TestEqual(TEXT("F9 of a slice save from the shore opens the slice"), Level(), FString(TEXT("Lvl_PointeSombre")));
		TestTrue(TEXT("Slice state restored"), WorldState() && WorldState()->HasFlag(SliceFlag) && WorldState()->HasFlag(ReefStruck));
		const FVector Here = Player() ? Player()->GetActorLocation() : FVector::ZeroVector;
		TestTrue(*FString::Printf(TEXT("Slice position restored (the quay): %.0f cm away at (%.0f, %.0f, %.0f)"),
			FVector::Dist2D(Here, *Quay), Here.X, Here.Y, Here.Z), Player() && FVector::Dist2D(Here, *Quay) < 50.0);
		return true;
	}));

	// 3. A Phase 5 (version 5) shore save, loaded from the slice, still loads on the shore.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, InBoathouse]()
	{
		UDCSaveGame* Old = NewObject<UDCSaveGame>();
		Old->SaveVersion = 5;
		Old->MapName = TEXT("Lvl_Boathouse");
		Old->MapPackage = ShorePath;
		Old->PlayerLocation = InBoathouse;
		Old->PlayerHealth = 100.0f;
		Old->Quests.Add({ TEXT("shore.watch"), TEXT("done_coil") });
		Old->WorldFlags.Add(TEXT("shore.relay_recovered"));
		TestTrue(TEXT("Wrote a Phase 5 save"), UGameplayStatics::SaveGameToSlot(Old, TestSlot, 0));
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, InBoathouse]()
	{
		TestEqual(TEXT("A Phase 5 save from the slice opens the shore"), Level(), FString(TEXT("Lvl_Boathouse")));
		TestEqual(TEXT("Phase 5 save: Shore Watch outcome"), Stage(TEXT("shore.watch")), FName(TEXT("done_coil")));
		TestTrue(TEXT("Phase 5 save: flag"), WorldState() && WorldState()->HasFlag(TEXT("shore.relay_recovered")));
		TestTrue(TEXT("Phase 5 save: position"), Player() && FVector::Dist2D(Player()->GetActorLocation(), InBoathouse) < 50.0);
		TestNotNull(TEXT("Phase 5 save: Mara is there"), Find(TEXT("boat.mara")));
		return true;
	}));

	QueueCleanup(TestSlot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreRespawnTest, "DeadCurrent.Map.Sombre.Respawn",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreRespawnTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreArchTest;

	auto Kill = []()
	{
		FDCDamageInfo Damage;
		Damage.Amount = 10000.0f;
		Damage.DamageType = DCTags::Damage_Environmental;
		UDCHealthComponent::ApplyDamageToActor(Player(), Damage);
	};
	auto QueueDeathAndRespawn = [this, Kill](const TCHAR* Anchor, const TCHAR* What)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Kill, What]()
		{
			Teleport(At(TEXT("ArchTest:Outside")));
			Kill();
			TestTrue(*FString::Printf(TEXT("%s: dead"), What), Health() <= 0.0f);
			return true;
		}));
		QueueWaitUntil(this, []() { return Health() > 0.0f; }, What, 8.0);
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Anchor, What]()
		{
			const double Dist = FVector::Dist2D(Player()->GetActorLocation(), At(Anchor));
			AddInfo(FString::Printf(TEXT("%s: respawned %.0f cm from %s"), What, Dist, Anchor));
			TestTrue(*FString::Printf(TEXT("%s: respawned at %s"), What, Anchor), Dist < 150.0);
			return true;
		}));
	};

	QueueFreshMap(MapPath, TestSlot);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player start"), Start()) || !TestNotNull(TEXT("Respawn rule"), Rule(TEXT("Presence_Respawn"))))
		{
			return true;
		}
		TestTrue(TEXT("A new game's start is on the deck"),
			FVector::Dist2D(Start()->GetActorLocation(), At(TEXT("Anchor:Anchor_NewGame_Deck"))) < 10.0);
		return true;
	}));
	QueueDeathAndRespawn(TEXT("Anchor:Anchor_NewGame_Deck"), TEXT("Before the reef strike"));

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		WorldState()->SetFlag(ReefStruck);
		TestEqual(TEXT("After the reef strike the start is on the quay"), Rule(TEXT("Presence_Respawn"))->GetActiveStateId(), FName(TEXT("harbor")));
		return true;
	}));
	QueueDeathAndRespawn(TEXT("Anchor:Anchor_CrossingExit_Quay"), TEXT("After the reef strike"));

	// A save after the strike: the start is on the quay again after F9 (presence snaps on the restore).
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestTrue(TEXT("Save after the strike"), Saves()->SaveCurrentGame());
		WorldState()->ClearFlag(ReefStruck);
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestTrue(TEXT("F9: the start is on the quay"),
			Start() && FVector::Dist2D(Start()->GetActorLocation(), At(TEXT("Anchor:Anchor_CrossingExit_Quay"))) < 10.0);
		WorldState()->SetFlag(VaultOpened);
		TestEqual(TEXT("Once the vault is open the start is at the tower base"), Rule(TEXT("Presence_Respawn"))->GetActiveStateId(), FName(TEXT("tower_base")));
		return true;
	}));
	QueueDeathAndRespawn(TEXT("Anchor:Anchor_Respawn_TowerBase"), TEXT("After the vault opens"));

	QueueCleanup(TestSlot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreAtmosphereTest, "DeadCurrent.Map.Sombre.Atmosphere",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreAtmosphereTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreArchTest;
	QueueFreshMap(MapPath, TestSlot);

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		struct FCase { bool bHale; bool bMeeting; const TCHAR* Look; };
		const FCase Cases[] = {
			{ false, false, TEXT("storm") }, { true, false, TEXT("calm") }, { true, true, TEXT("night") }, { false, true, TEXT("night") },
		};
		for (const FCase& Case : Cases)
		{
			Case.bHale ? WorldState()->SetFlag(HaleArrived) : WorldState()->ClearFlag(HaleArrived);
			Case.bMeeting ? WorldState()->SetFlag(MeetingDone) : WorldState()->ClearFlag(MeetingDone);
			const FString Label = FString::Printf(TEXT("hale=%d meeting=%d"), Case.bHale, Case.bMeeting);
			TestEqual(*FString::Printf(TEXT("%s: exactly the %s look"), *Label, Case.Look), ActiveLooks(), FString(Case.Look));
			TestEqual(*FString::Printf(TEXT("%s: storm lightning"), *Label), LightningActive(TEXT("storm")), FCString::Strcmp(Case.Look, TEXT("storm")) == 0);
			TestEqual(*FString::Printf(TEXT("%s: night lightning"), *Label), LightningActive(TEXT("night")), FCString::Strcmp(Case.Look, TEXT("night")) == 0);
		}
		// Night, saved; diverge to the storm; a load brings the night back.
		TestTrue(TEXT("Save at night"), Saves()->SaveCurrentGame());
		WorldState()->ClearFlag(HaleArrived);
		WorldState()->ClearFlag(MeetingDone);
		TestEqual(TEXT("Diverged to the storm"), ActiveLooks(), FString(TEXT("storm")));
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestEqual(TEXT("F9 restores the night"), ActiveLooks(), FString(TEXT("night")));
		return true;
	}));

	QueueCleanup(TestSlot);
	return true;
}

#endif
