#include "AI/DCFriendlyNPC.h"
#include "AI/DCScavengerCharacter.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCHealthComponent.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Interaction/DCInteractable.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UI/DCHUD.h"
#include "UnrealClient.h"
#include "World/DCDamageVolume.h"
#include "World/DCFlickerLight.h"
#include "World/DCInspectableActor.h"
#include "World/DCLocationVolume.h"
#include "World/DCLootContainer.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  Plays Shore Watch in the real Lvl_Boathouse (game context: run with -game, see Tools/RunTests.bat -map):
 *  placed actors, real interactions, and real F9 loads that reopen the map. Uses a scratch save slot.
 */
namespace DCBoathouseTest
{
	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_Boathouse");
	const TCHAR* TestSlot = TEXT("DeadCurrent_MapTest");
	const FName Quest = TEXT("shore.watch");

	UWorld* GameWorld()
	{
		return AutomationCommon::GetAnyGameWorld();
	}

	ADCPlayerCharacter* Player()
	{
		UWorld* World = GameWorld();
		return World ? Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)) : nullptr;
	}

	AActor* Find(FName PersistentId)
	{
		UWorld* World = GameWorld();
		const UDCPersistentRegistry* Registry = World ? World->GetSubsystem<UDCPersistentRegistry>() : nullptr;
		return Registry ? Registry->FindActor(PersistentId) : nullptr;
	}

	UDCSaveSubsystem* Saves()
	{
		UWorld* World = GameWorld();
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UDCSaveSubsystem>() : nullptr;
	}

	UDCWorldStateSubsystem* WorldState()
	{
		return GameWorld() ? GameWorld()->GetSubsystem<UDCWorldStateSubsystem>() : nullptr;
	}

	/** The relay rig at the scavenger camp (inspectables have no persistent id). */
	ADCInspectableActor* RelayRig()
	{
		ADCInspectableActor* Best = nullptr;
		double BestDist = 150.0;
		for (TActorIterator<ADCInspectableActor> It(GameWorld()); It; ++It)
		{
			const double Dist = FVector::Dist2D(It->GetActorLocation(), FVector(2480.0, -220.0, 0.0));
			if (Dist < BestDist)
			{
				Best = *It;
				BestDist = Dist;
			}
		}
		return Best;
	}

	bool Say(const FString& Text)
	{
		UDCDialogueComponent* Dialogue = Player() ? Player()->GetDialogueComponent() : nullptr;
		const FDCDialogueNode* Node = Dialogue ? Dialogue->GetCurrentNode() : nullptr;
		if (!Node)
		{
			return false;
		}
		const TArray<int32> Visible = Dialogue->GetVisibleChoiceIndices();
		for (int32 Index = 0; Index < Visible.Num(); ++Index)
		{
			if (Node->Choices[Visible[Index]].Text.ToString() == Text)
			{
				return Dialogue->SelectChoice(Index);
			}
		}
		return false;
	}

	FName TalkToMara()
	{
		ADCPlayerCharacter* P = Player();
		AActor* Mara = Find(TEXT("boat.mara"));
		if (!P || !Mara)
		{
			return NAME_None;
		}
		P->GetDialogueComponent()->EndDialogue();
		IDCInteractable::Execute_Interact(Mara, P);
		return P->GetDialogueComponent()->GetCurrentNodeId();
	}

	int32 Count(FName ItemId)
	{
		return Player() ? Player()->GetInventoryComponent()->GetQuantityByItemId(ItemId) : -1;
	}

	FName Stage()
	{
		return Player() ? Player()->GetQuestComponent()->GetStage(Quest) : NAME_None;
	}

	/** Waits until a different world than Previous has begun play with a player in it. */
	class FWaitForReload : public IAutomationLatentCommand
	{
	public:
		explicit FWaitForReload(TSharedRef<TWeakObjectPtr<UWorld>> InPrevious) : Previous(InPrevious) {}

		virtual bool Update() override
		{
			UWorld* World = GameWorld();
			if (!World || World == Previous->Get() || !World->HasBegunPlay() || !Player())
			{
				return FPlatformTime::Seconds() - StartTime > 30.0;
			}
			return true;
		}

	private:
		TSharedRef<TWeakObjectPtr<UWorld>> Previous;
	};

	/** Presses F9: remembers the current world, loads, then waits for the reopened map. */
	void QueueLoad(FAutomationTestBase* Test)
	{
		TSharedRef<TWeakObjectPtr<UWorld>> Previous = MakeShared<TWeakObjectPtr<UWorld>>();
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Test, Previous]()
		{
			*Previous = GameWorld();
			Test->TestTrue(TEXT("Load requested"), Saves() && Saves()->LoadCurrentGame());
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForReload(Previous));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	}

	void QueueFreshMap()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
		{
			if (UDCSaveSubsystem* S = Saves())
			{
				S->SetSlotName(TestSlot);
			}
			UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
			GEngine->Exec(GameWorld(), *FString::Printf(TEXT("Open %s"), MapPath));
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	}

	void QueueCleanup()
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([]()
		{
			UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
			if (UDCSaveSubsystem* S = Saves())
			{
				S->SetSlotName(UDCSaveSubsystem::DefaultSlotName);
			}
			return true;
		}));
	}

	// Exploration Loop: the Wrecked Survey Launch west of the boathouse (see build_boathouse.py).
	const FName WreckLocation = TEXT("shore.survey_launch");
	const FName Locker = TEXT("boat.wreck_locker");
	const FName Tender = TEXT("boat.wreck_tender");
	const FVector Beach(-1100.0, -250.0, 100.0);        // inside the discovery volume, dry
	const FVector LiveWater(-1250.0, -1300.0, 100.0);   // in the water off the stern
	const FVector StartArea(300.0, 0.0, 100.0);         // inside the boathouse, far from the POI

	TArray<FString> VisibleChoiceTexts()
	{
		TArray<FString> Texts;
		UDCDialogueComponent* Dialogue = Player() ? Player()->GetDialogueComponent() : nullptr;
		if (const FDCDialogueNode* Node = Dialogue ? Dialogue->GetCurrentNode() : nullptr)
		{
			for (const int32 Index : Dialogue->GetVisibleChoiceIndices())
			{
				Texts.Add(Node->Choices[Index].Text.ToString());
			}
		}
		return Texts;
	}

	ADCInspectableActor* Inspectable(const TCHAR* DisplayName)
	{
		for (TActorIterator<ADCInspectableActor> It(GameWorld()); It; ++It)
		{
			if (It->GetDisplayName().ToString() == DisplayName)
			{
				return *It;
			}
		}
		return nullptr;
	}

	/** Nearest actor of class T whose bounds center (greybox boxes pivot at a corner) is within MaxDistance. */
	template <class T>
	T* Nearest(const FVector& Where, double MaxDistance = 400.0)
	{
		T* Best = nullptr;
		for (TActorIterator<T> It(GameWorld()); It; ++It)
		{
			FVector Center, Extent;
			It->GetActorBounds(false, Center, Extent);
			const double Dist = FVector::Dist2D(Center, Where);
			if (Dist < MaxDistance)
			{
				Best = *It;
				MaxDistance = Dist;
			}
		}
		return Best;
	}

	ADCLocationVolume* WreckVolume()
	{
		for (TActorIterator<ADCLocationVolume> It(GameWorld()); It; ++It)
		{
			if (It->GetLocationId() == WreckLocation)
			{
				return *It;
			}
		}
		return nullptr;
	}

	ADCDamageVolume* LiveWaterHazard() { return Nearest<ADCDamageVolume>(FVector(-1500.0, -1390.0, 0.0)); }

	ADCFlickerLight* Sparks() { return Nearest<ADCFlickerLight>(FVector(-1230.0, -1470.0, 0.0)); }

	ADCLootContainer* Container(FName Id) { return Cast<ADCLootContainer>(Find(Id)); }

	/** Every POI actor the tests touch exists (so later steps can use them without null checks). */
	bool SurveyLaunchPlaced(FAutomationTestBase* Test)
	{
		return Test->TestNotNull(TEXT("Player"), Player())
			&& Test->TestNotNull(TEXT("Discovery volume placed"), WreckVolume())
			&& Test->TestNotNull(TEXT("Survey locker placed"), Container(Locker))
			&& Test->TestNotNull(TEXT("Tender placed"), Container(Tender))
			&& Test->TestNotNull(TEXT("Live water placed"), LiveWaterHazard())
			&& Test->TestNotNull(TEXT("Sparks placed"), Sparks())
			&& Test->TestNotNull(TEXT("Battery placed"), Inspectable(TEXT("Battery bank")));
	}

	int32 StacksIn(FName Id)
	{
		const ADCLootContainer* C = Container(Id);
		return C ? C->GetInventoryComponent()->GetStacks().Num() : -1;
	}

	bool Discovered()
	{
		return WorldState() && WorldState()->IsLocationDiscovered(WreckLocation);
	}

	void Use(AActor* Target)
	{
		if (Target && Player())
		{
			IDCInteractable::Execute_Interact(Target, Player());
		}
	}

	void Use(const TCHAR* InspectableName) { Use(Inspectable(InspectableName)); }

	void Teleport(const FVector& Where)
	{
		if (ADCPlayerCharacter* P = Player())
		{
			P->GetDialogueComponent()->EndDialogue();
			P->SetActorLocation(Where, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	ADCHUD* HUD()
	{
		const APlayerController* PC = Player() ? Cast<APlayerController>(Player()->GetController()) : nullptr;
		return PC ? PC->GetHUD<ADCHUD>() : nullptr;
	}

	FString Message() { return HUD() ? HUD()->GetActiveMessage().ToString() : FString(); }

	FString Banner() { return HUD() ? HUD()->GetActiveBannerSubtitle().ToString() : FString(); }

	float Health() { return Player() ? Player()->GetHealthComponent()->GetHealth() : -1.0f; }

	void SwitchSlot(const FString& Slot)
	{
		if (UDCSaveSubsystem* S = Saves())
		{
			S->SetSlotName(Slot);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseCoilRouteTest, "DeadCurrent.Map.Boathouse.CoilRoute",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseCoilRouteTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	QueueFreshMap();

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Relay rig placed"), RelayRig())
			|| !TestNotNull(TEXT("Mara placed"), Find(TEXT("boat.mara"))) || !TestNotNull(TEXT("Coil placed"), Find(TEXT("boat.pickup_coil"))))
		{
			return true;
		}

		// Clue from the real relay rig variant.
		IDCInteractable::Execute_Interact(RelayRig(), Player());
		TestTrue(TEXT("Rig clue flag"), WorldState()->HasFlag(TEXT("shore.relay_inspected")));

		// Accept from Mara.
		TestEqual(TEXT("Greeting"), TalkToMara(), FName(TEXT("greeting")));
		TestTrue(TEXT("Ask"), Say(TEXT("You keep looking toward his camp.")));
		TestTrue(TEXT("Accept quietly"), Say(TEXT("I'll pull the coil out of his rig. No shooting.")));
		Say(TEXT("Goodbye."));
		TestEqual(TEXT("Accepted"), Stage(), FName(TEXT("accepted")));

		// Take the coil through the real pickup.
		IDCInteractable::Execute_Interact(Find(TEXT("boat.pickup_coil")), Player());
		TestEqual(TEXT("Coil in inventory"), Count(TEXT("radio_coil")), 1);
		TestEqual(TEXT("Quest advanced by pickup"), Stage(), FName(TEXT("return_coil")));

		// With rendering (not -nullrhi), capture the objective line and quest journal for review:
		// Saved/Screenshots/<platform>/DC_QuestHUD.png
		if (FApp::CanEverRender())
		{
			const APlayerController* PC = Cast<APlayerController>(Player()->GetController());
			if (ADCHUD* HUD = PC ? PC->GetHUD<ADCHUD>() : nullptr)
			{
				HUD->ToggleInventory();
			}
			FScreenshotRequest::RequestScreenshot(TEXT("DC_QuestHUD"), true, false);
		}

		TestTrue(TEXT("Save (ready to turn in)"), Saves()->SaveCurrentGame());
		return true;
	}));

	// Give a rendered run time to draw the HUD before the screenshot frame.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		// Diverge after the save: hand the coil over.
		TestEqual(TEXT("Turn-in node"), TalkToMara(), FName(TEXT("turnin_coil")));
		TestTrue(TEXT("Hand over"), Say(TEXT("Here. It's yours.")));
		TestEqual(TEXT("Done"), Stage(), FName(TEXT("done_coil")));
		return true;
	}));

	// F9: the reopened map must be back at "ready to turn in", coil in hand, pickup gone.
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player after load"), Player()))
		{
			return true;
		}
		TestEqual(TEXT("Load: stage"), Stage(), FName(TEXT("return_coil")));
		TestEqual(TEXT("Load: coil"), Count(TEXT("radio_coil")), 1);
		TestEqual(TEXT("Load: no dressings yet"), Count(TEXT("field_dressing")), 0);
		TestNull(TEXT("Load: coil pickup stays taken"), Find(TEXT("boat.pickup_coil")));
		TestTrue(TEXT("Load: clue flag"), WorldState()->HasFlag(TEXT("shore.relay_inspected")));
		TestFalse(TEXT("Load: outcome flag not set"), WorldState()->HasFlag(TEXT("shore.relay_recovered")));

		TestEqual(TEXT("Load: turn-in node"), TalkToMara(), FName(TEXT("turnin_coil")));
		TestTrue(TEXT("Load: hand over"), Say(TEXT("Here. It's yours.")));
		TestEqual(TEXT("Coil outcome"), Stage(), FName(TEXT("done_coil")));
		TestEqual(TEXT("Dressings"), Count(TEXT("field_dressing")), 2);
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		TestTrue(TEXT("Scavenger alive on the coil route"), Scav && !Scav->GetHealthComponent()->IsDead());
		TestTrue(TEXT("Save (complete)"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestEqual(TEXT("Reload: complete"), Stage(), FName(TEXT("done_coil")));
		TestTrue(TEXT("Reload: outcome flag"), WorldState() && WorldState()->HasFlag(TEXT("shore.relay_recovered")));
		TestEqual(TEXT("Reload: coil gone"), Count(TEXT("radio_coil")), 0);
		TestEqual(TEXT("Reload: epilogue"), TalkToMara(), FName(TEXT("done_coil")));
		return true;
	}));

	QueueCleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseCombatRouteTest, "DeadCurrent.Map.Boathouse.CombatRoute",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseCombatRouteTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	QueueFreshMap();

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Scavenger placed"), Scav))
		{
			return true;
		}

		TestTrue(TEXT("Save before the quest"), Saves()->SaveCurrentGame());

		TalkToMara();
		Say(TEXT("You keep looking toward his camp."));
		Say(TEXT("I'll put him down."));
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		UDCHealthComponent::ApplyDamageToActor(Scav, Damage);
		TestEqual(TEXT("First kill advanced quest"), Stage(), FName(TEXT("return_killed")));
		return true;
	}));

	// Loading the pre-quest save must bring the scavenger back and forget the quest.
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Scavenger after pre-quest load"), Scav) || !TestNotNull(TEXT("Player after pre-quest load"), Player()))
		{
			return true;
		}
		TestFalse(TEXT("Pre-quest load: scavenger alive"), Scav->GetHealthComponent()->IsDead());
		TestFalse(TEXT("Pre-quest load: quest not started"), Player()->GetQuestComponent()->HasQuest(Quest));
		TestEqual(TEXT("Pre-quest load: greeting"), TalkToMara(), FName(TEXT("greeting")));
		Player()->GetDialogueComponent()->EndDialogue();

		TalkToMara();
		Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Accept"), Say(TEXT("I'll put him down.")));
		Say(TEXT("Goodbye."));

		// Kill him through the real damage path.
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		Damage.Instigator = Player();
		UDCHealthComponent::ApplyDamageToActor(Scav, Damage);
		TestTrue(TEXT("Scavenger dead"), Scav->GetHealthComponent()->IsDead());
		TestEqual(TEXT("Kill advanced quest"), Stage(), FName(TEXT("return_killed")));

		// Loot one stack so the save carries a partly looted corpse.
		IDCInteractable::Execute_Interact(Scav, Player());
		TestTrue(TEXT("Save (ready to turn in)"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Scavenger after load"), Scav) || !TestNotNull(TEXT("Player after load"), Player()))
		{
			return true;
		}
		TestTrue(TEXT("Load: scavenger dead (health)"), Scav->GetHealthComponent()->IsDead());
		TestTrue(TEXT("Load: corpse still lootable"), IDCInteractable::Execute_CanInteract(const_cast<ADCScavengerCharacter*>(Scav), Player()));
		TestEqual(TEXT("Load: corpse has two stacks left"), Scav->GetInventoryComponent()->GetStacks().Num(), 2);
		TestEqual(TEXT("Load: stage"), Stage(), FName(TEXT("return_killed")));

		const int32 AmmoBefore = Count(TEXT("ammo_9mm"));
		TestEqual(TEXT("Load: turn-in node"), TalkToMara(), FName(TEXT("turnin_kill")));
		TestTrue(TEXT("Turn in"), Say(TEXT("He's dead. His relay has no one to tend it.")));
		TestEqual(TEXT("Kill outcome"), Stage(), FName(TEXT("done_killed")));
		TestEqual(TEXT("Ammo reward"), Count(TEXT("ammo_9mm")) - AmmoBefore, 24);
		TestTrue(TEXT("Save (complete)"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestEqual(TEXT("Reload: complete"), Stage(), FName(TEXT("done_killed")));
		TestTrue(TEXT("Reload: outcome flag"), WorldState() && WorldState()->HasFlag(TEXT("shore.path_cleared")));
		TestEqual(TEXT("Reload: epilogue"), TalkToMara(), FName(TEXT("done_killed")));
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		TestTrue(TEXT("Reload: scavenger still dead"), Scav && Scav->GetHealthComponent()->IsDead());
		return true;
	}));

	QueueCleanup();
	return true;
}

/**
 *  The Exploration Loop in the real map: find the wreck, get the discovery once, read it, get hurt by
 *  the live water, loot, cut the power, take the hidden kit, save, diverge, F9, and find it all as it was.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseSurveyLaunchTest, "DeadCurrent.Map.Boathouse.SurveyLaunch",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseSurveyLaunchTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	QueueFreshMap();
	TSharedRef<int32> Announcements = MakeShared<int32>(0);
	TSharedRef<float> HealthBefore = MakeShared<float>(0.0f);

	// The POI is placed and untouched, and nothing leads the player to it.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Announcements]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		for (const TCHAR* Clue : { TEXT("Survey log"), TEXT("Depth sounder"), TEXT("Breaker panel"), TEXT("Battery bank"),
			TEXT("Emergency beacon"), TEXT("Name board"), TEXT("Life jackets"), TEXT("Dead fish"), TEXT("West window") })
		{
			TestNotNull(*FString::Printf(TEXT("Clue placed: %s"), Clue), Inspectable(Clue));
		}
		TestEqual(TEXT("Display name"), WreckVolume()->GetDisplayName().ToString(), FString(TEXT("Wrecked Survey Launch")));
		TestFalse(TEXT("Start: not discovered"), Discovered());
		TestFalse(TEXT("Start: spawn is outside the POI"), WreckVolume()->ContainsPoint(Player()->GetActorLocation()));
		TestEqual(TEXT("Start: locker full"), StacksIn(Locker), 3);
		TestEqual(TEXT("Start: tender full"), StacksIn(Tender), 3);
		TestTrue(TEXT("Start: water live"), LiveWaterHazard()->IsHazardActive());
		TestTrue(TEXT("Start: no quest involved"), Player()->GetQuestComponent()->GetQuestLog().IsEmpty());

		WorldState()->OnLocationDiscovered.AddLambda([Announcements](FName) { ++*Announcements; });
		Teleport(Beach);
		return true;
	}));

	// Walking in discovers it (the volume polls a few times a second).
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Announcements, HealthBefore]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		TestTrue(TEXT("Discovered by walking in"), Discovered());
		TestEqual(TEXT("Banner"), Banner(), FString(TEXT("Wrecked Survey Launch")));
		TestEqual(TEXT("Announced once"), *Announcements, 1);

		// Step into the live water.
		*HealthBefore = Health();
		Teleport(LiveWater);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Announcements, HealthBefore]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		TestTrue(TEXT("Live water hurts"), Health() < *HealthBefore - 5.0f);
		TestTrue(TEXT("Live water warns"), Message().Contains(TEXT("live")));
		TestTrue(TEXT("Still alive after a second"), Health() > 0.0f);
		Teleport(Beach);
		TestEqual(TEXT("Leaving and re-entering does not rediscover"), *Announcements, 1);

		// Read the place.
		Use(TEXT("Survey log"));
		TestTrue(TEXT("Log read"), WorldState()->HasFlag(TEXT("wreck.log_read")));
		TestTrue(TEXT("Log points at the tender"), Message().Contains(TEXT("tender")));
		Use(TEXT("Breaker panel"));
		TestTrue(TEXT("Panel reads differently after the log"), Message().Contains(TEXT("The log says")));
		Use(TEXT("Emergency beacon"));
		TestTrue(TEXT("Beacon, power still on"), Message().Contains(TEXT("dead for decades")));

		// Loot the obvious locker, but not all of it.
		Use(Container(Locker));
		TestEqual(TEXT("Took the rounds"), Count(TEXT("ammo_9mm")), 12);
		TestEqual(TEXT("Locker partly looted"), StacksIn(Locker), 2);

		// Inspect, then pull, the battery leads.
		ADCInspectableActor* Battery = Inspectable(TEXT("Battery bank"));
		TestEqual(TEXT("Battery verb before"), IDCInteractable::Execute_GetInteractionPrompt(Battery, Player()).Action.ToString(), FString(TEXT("Inspect")));
		Use(Battery);
		TestEqual(TEXT("Battery verb after inspecting"), IDCInteractable::Execute_GetInteractionPrompt(Battery, Player()).Action.ToString(), FString(TEXT("Pull the leads")));
		TestTrue(TEXT("Water still live until pulled"), LiveWaterHazard()->IsHazardActive());
		Use(Battery);
		TestTrue(TEXT("Power cut"), WorldState()->HasFlag(TEXT("wreck.power_cut")));
		TestFalse(TEXT("Water dead"), LiveWaterHazard()->IsHazardActive());
		TestFalse(TEXT("Sparks out"), Sparks()->IsLightActive());
		Use(TEXT("Emergency beacon"));
		TestTrue(TEXT("Beacon notices the lamp"), Message().Contains(TEXT("still flickering")));

		*HealthBefore = Health();
		Teleport(LiveWater);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, HealthBefore]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		TestEqual(TEXT("Dead water does not hurt"), Health(), *HealthBefore);

		// The hidden kit in the tender.
		Use(Container(Tender));
		Use(Container(Tender));
		Use(Container(Tender));
		TestTrue(TEXT("Tender emptied"), Container(Tender)->IsEmpty());
		TestEqual(TEXT("Kit dressings"), Count(TEXT("field_dressing")), 2);
		TestEqual(TEXT("Kit chart (the novel item)"), Count(TEXT("survey_chart")), 1);
		TestEqual(TEXT("All the rounds"), Count(TEXT("ammo_9mm")), 30);

		Teleport(Beach);
		TestTrue(TEXT("Save at the wreck"), Saves()->SaveCurrentGame());

		// Diverge after the save.
		Use(Container(Locker));
		Use(Container(Locker));
		TestTrue(TEXT("Diverged: locker empty"), Container(Locker)->IsEmpty());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		if (!TestNotNull(TEXT("Player after load"), Player()) || !TestNotNull(TEXT("Locker after load"), Container(Locker)))
		{
			return true;
		}
		TestTrue(TEXT("Load: still discovered"), Discovered());
		TestTrue(TEXT("Load: loaded inside the POI"), WreckVolume()->ContainsPoint(Player()->GetActorLocation()));
		TestEqual(TEXT("Load: not re-announced"), Banner(), FString());
		for (const TCHAR* Flag : { TEXT("wreck.log_read"), TEXT("wreck.battery_seen"), TEXT("wreck.power_cut") })
		{
			TestTrue(*FString::Printf(TEXT("Load: %s"), Flag), WorldState()->HasFlag(Flag));
		}
		TestEqual(TEXT("Load: locker keeps exactly what was left"), StacksIn(Locker), 2);
		const UDCInventoryComponent* LockerInventory = Container(Locker)->GetInventoryComponent();
		TestEqual(TEXT("Load: locker wiring"), LockerInventory->GetQuantityByItemId(TEXT("salvage_wiring")), 3);
		TestEqual(TEXT("Load: locker dressing"), LockerInventory->GetQuantityByItemId(TEXT("field_dressing")), 1);
		TestEqual(TEXT("Load: locker rounds stay taken"), LockerInventory->GetQuantityByItemId(TEXT("ammo_9mm")), 0);
		TestTrue(TEXT("Load: tender stays empty"), Container(Tender)->IsEmpty());
		TestEqual(TEXT("Load: player rounds"), Count(TEXT("ammo_9mm")), 30);
		TestEqual(TEXT("Load: player dressings"), Count(TEXT("field_dressing")), 2);
		TestFalse(TEXT("Load: water stays dead"), LiveWaterHazard()->IsHazardActive());
		TestFalse(TEXT("Load: sparks stay out"), Sparks()->IsLightActive());
		TestEqual(TEXT("Load: battery remembers"), IDCInteractable::Execute_GetInteractionPrompt(Inspectable(TEXT("Battery bank")), Player()).Action.ToString(), FString(TEXT("Inspect")));
		Use(TEXT("West window"));
		TestTrue(TEXT("Load: the boathouse window knows"), Message().Contains(TEXT("nothing left to power it")));
		return true;
	}));

	QueueCleanup();
	return true;
}

/**
 *  Save/load combinations of the POI with Shore Watch at the same time, across several slots:
 *  undiscovered + quest active, discovered + partial tender + quest active, and both complete.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseSurveyLaunchSavesTest, "DeadCurrent.Map.Boathouse.SurveyLaunchSaves",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseSurveyLaunchSavesTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	const FString SlotA = FString(TestSlot) + TEXT("_A");
	const FString SlotB = FString(TestSlot) + TEXT("_B");
	const FString SlotC = FString(TestSlot) + TEXT("_C");
	QueueFreshMap();

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SlotA]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Tender"), Container(Tender)))
		{
			return true;
		}
		// A: Shore Watch accepted, the wreck never seen.
		TalkToMara();
		Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Accept"), Say(TEXT("I'll put him down.")));
		Player()->GetDialogueComponent()->EndDialogue();
		SwitchSlot(SlotA);
		TestTrue(TEXT("Save A"), Saves()->SaveCurrentGame());

		Teleport(Beach);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SlotB]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		// B: discovered, log read, tender half looted, locker untouched, quest still active.
		TestTrue(TEXT("Discovered"), Discovered());
		Use(TEXT("Survey log"));
		Use(Container(Tender));
		TestEqual(TEXT("Tender partly looted"), StacksIn(Tender), 2);
		SwitchSlot(SlotB);
		TestTrue(TEXT("Save B"), Saves()->SaveCurrentGame());

		// Diverge: finish Shore Watch.
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		Damage.Instigator = Player();
		UDCHealthComponent::ApplyDamageToActor(Find(TEXT("boat.scavenger")), Damage);
		TestEqual(TEXT("Diverged: quest advanced"), Stage(), FName(TEXT("return_killed")));
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SlotC]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Player after B"), Player()) || !TestNotNull(TEXT("Scavenger after B"), Scav))
		{
			return true;
		}
		TestEqual(TEXT("B: quest still accepted"), Stage(), FName(TEXT("accepted")));
		TestFalse(TEXT("B: scavenger alive"), Scav->GetHealthComponent()->IsDead());
		TestTrue(TEXT("B: discovered"), Discovered());
		TestTrue(TEXT("B: log read"), WorldState()->HasFlag(TEXT("wreck.log_read")));
		TestEqual(TEXT("B: tender keeps its other stacks"), StacksIn(Tender), 2);
		TestEqual(TEXT("B: that stack is the rounds"), Container(Tender)->GetInventoryComponent()->GetQuantityByItemId(TEXT("ammo_9mm")), 18);
		TestEqual(TEXT("B: locker untouched"), StacksIn(Locker), 3);
		TestTrue(TEXT("B: water still live"), LiveWaterHazard()->IsHazardActive());

		// C: finish Shore Watch and tell Mara about the wreck.
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		Damage.Instigator = Player();
		UDCHealthComponent::ApplyDamageToActor(Find(TEXT("boat.scavenger")), Damage);
		TestEqual(TEXT("Turn-in"), TalkToMara(), FName(TEXT("turnin_kill")));
		TestTrue(TEXT("Turn in"), Say(TEXT("He's dead. His relay has no one to tend it.")));
		TestEqual(TEXT("Epilogue"), TalkToMara(), FName(TEXT("done_killed")));
		TestTrue(TEXT("Tell Mara about the wreck"), Say(TEXT("There's a wrecked survey launch west of the boathouse. I read her log.")));
		TestTrue(TEXT("Ask"), Say(TEXT("Was it a storm?")));
		Player()->GetDialogueComponent()->EndDialogue();
		TestTrue(TEXT("Mara told"), WorldState()->HasFlag(TEXT("wreck.mara_told")));
		SwitchSlot(SlotC);
		TestTrue(TEXT("Save C"), Saves()->SaveCurrentGame());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SlotA]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		if (!TestNotNull(TEXT("Player after C"), Player()))
		{
			return true;
		}
		TestEqual(TEXT("C: Shore Watch complete"), Stage(), FName(TEXT("done_killed")));
		TestTrue(TEXT("C: path cleared"), WorldState()->HasFlag(TEXT("shore.path_cleared")));
		TestTrue(TEXT("C: discovered"), Discovered());
		TestEqual(TEXT("C: tender still partial"), StacksIn(Tender), 2);
		TalkToMara();
		TestFalse(TEXT("C: wreck line used up"), VisibleChoiceTexts().Contains(TEXT("There's a wrecked survey launch west of the boathouse. I read her log.")));
		Player()->GetDialogueComponent()->EndDialogue();

		// Back to A: before any of it.
		SwitchSlot(SlotA);
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		if (!TestNotNull(TEXT("Player after A"), Player()))
		{
			return true;
		}
		TestEqual(TEXT("A: quest accepted"), Stage(), FName(TEXT("accepted")));
		TestFalse(TEXT("A: not discovered"), Discovered());
		TestEqual(TEXT("A: tender full again"), StacksIn(Tender), 3);
		TestEqual(TEXT("A: locker full"), StacksIn(Locker), 3);
		TestFalse(TEXT("A: log unread"), WorldState()->HasFlag(TEXT("wreck.log_read")));
		TestFalse(TEXT("A: loaded outside the POI"), WreckVolume()->ContainsPoint(Player()->GetActorLocation()));
		Teleport(Beach);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, SlotA, SlotB, SlotC]()
	{
		if (!SurveyLaunchPlaced(this))
		{
			return true;
		}
		TestTrue(TEXT("A: can be discovered again"), Discovered());
		TestEqual(TEXT("A: with its banner"), Banner(), FString(TEXT("Wrecked Survey Launch")));
		for (const FString& Slot : { SlotA, SlotB, SlotC })
		{
			UGameplayStatics::DeleteGameInSlot(Slot, 0);
		}
		return true;
	}));

	QueueCleanup();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCBoathouseLegacySaveTest, "DeadCurrent.Map.Boathouse.LegacySave",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCBoathouseLegacySaveTest::RunTest(const FString& Parameters)
{
	using namespace DCBoathouseTest;
	QueueFreshMap();

	// A first-playable (version 0/1) save: short map name only, the old Shore Watch stage ids,
	// the old completion flag, and the coil already taken.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		UDCSaveGame* Legacy = NewObject<UDCSaveGame>();
		Legacy->MapName = TEXT("Lvl_Boathouse");
		Legacy->PlayerLocation = FVector(3000.0, 1100.0, 100.0);
		Legacy->PlayerHealth = 80.0f;
		Legacy->PlayerInventory.Add({ TEXT("radio_coil"), 1, FString() });
		Legacy->Quests.Add({ Quest, TEXT("return") });
		Legacy->WorldFlags.Add(TEXT("shore.cleared"));
		for (const TCHAR* Id : { TEXT("boat.scavenger"), TEXT("boat.mara"), TEXT("boat.door"), TEXT("boat.pickup_pistol"),
			TEXT("boat.pickup_ammo"), TEXT("boat.pickup_dressing") })
		{
			FDCPersistentActorState State;
			State.PersistentId = Id;
			Legacy->WorldActors.Add(State);
		}
		TestTrue(TEXT("Wrote legacy save"), UGameplayStatics::SaveGameToSlot(Legacy, TestSlot, 0));
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player after legacy load"), Player()))
		{
			return true;
		}
		TestTrue(TEXT("Legacy: player moved to saved spot"), Player()->GetActorLocation().Equals(FVector(3000.0, 1100.0, 100.0), 150.0));
		TestEqual(TEXT("Legacy: coil restored"), Count(TEXT("radio_coil")), 1);
		TestNull(TEXT("Legacy: coil pickup gone"), Find(TEXT("boat.pickup_coil")));
		TestFalse(TEXT("Legacy: unknown stage dropped"), Player()->GetQuestComponent()->HasQuest(Quest));

		// The Exploration Loop POI postdates this save: it keeps its authored state.
		TestFalse(TEXT("Legacy: wreck undiscovered"), Discovered());
		TestEqual(TEXT("Legacy: locker full"), StacksIn(Locker), 3);
		TestEqual(TEXT("Legacy: tender full"), StacksIn(Tender), 3);
		TestTrue(TEXT("Legacy: water live"), LiveWaterHazard() && LiveWaterHazard()->IsHazardActive());

		// The quest can be picked up again, and the coil shortcut applies.
		TestEqual(TEXT("Legacy: greeting"), TalkToMara(), FName(TEXT("greeting")));
		Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Legacy: coil shortcut"), Say(TEXT("This coil? I already pulled it.")));
		TestEqual(TEXT("Legacy: straight to turn-in"), Stage(), FName(TEXT("return_coil")));
		return true;
	}));

	QueueCleanup();
	return true;
}

#endif
