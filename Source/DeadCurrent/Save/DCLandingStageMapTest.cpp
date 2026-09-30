#include "AI/DCScavengerCharacter.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCHealthComponent.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Interaction/DCInteractable.h"
#include "Inventory/DCInventoryComponent.h"
#include "HAL/FileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveGame.h"
#include "Save/DCSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UI/DCHUD.h"
#include "World/DCConditionalPresence.h"
#include "World/DCFlickerLight.h"
#include "World/DCInspectableActor.h"
#include "World/DCLootContainer.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  Phase 5: the Landing Stage (Design/POIs/shore.landing_stage.md) in the real Lvl_Boathouse. Both Shore Watch
 *  routes are played through the real pickup, kill, and Mara's real turn-ins; the stage and Mara's placement are
 *  read back from the presence rules and the actors they own, then through save, diverge, and a real F9.
 *  Helpers are local on purpose, so the accepted map tests' file stays unchanged. Scratch save slot.
 */
namespace DCLandingStageTest
{
	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_Boathouse");
	const TCHAR* TestSlot = TEXT("DeadCurrent_LandingTest");
	const FName Quest = TEXT("shore.watch");
	const FName Location = TEXT("shore.landing_stage");
	const FName Tackle = TEXT("landing.tackle");

	const FVector Lookout(3100.0, 1280.0, 0.0);        // Mara's authored spot
	const FVector MaraOnStage(960.0, -990.0, 0.0);     // her coil-route placement
	const FVector SkiffMoored(765.0, -985.0, 0.0);      // alongside the deck's west side
	const FVector SkiffAdrift(1720.0, -2080.0, 0.0);
	const FVector OnGangway(1010.0, -700.0, 110.0);    // inside the discovery volume, 3 m from the deck
	const FVector NearMara(3100.0, 1120.0, 100.0);     // talking range
	const FVector FarAway(-1100.0, -250.0, 100.0);     // the Survey Launch beach: far from the lookout and the stage

	UWorld* GameWorld() { return AutomationCommon::GetAnyGameWorld(); }

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

	UDCWorldStateSubsystem* WorldState() { return GameWorld() ? GameWorld()->GetSubsystem<UDCWorldStateSubsystem>() : nullptr; }

	/** A presence rule by its tag (build_landing_stage.py tags each Presence_<Name>). */
	ADCConditionalPresence* Rule(const TCHAR* Name)
	{
		const FName Tag(*FString::Printf(TEXT("Presence_%s"), Name));
		for (TActorIterator<ADCConditionalPresence> It(GameWorld()); It; ++It)
		{
			if (It->Tags.Contains(Tag))
			{
				return *It;
			}
		}
		return nullptr;
	}

	FName StateOf(const TCHAR* Name)
	{
		const ADCConditionalPresence* R = Rule(Name);
		return R ? R->GetActiveStateId() : FName(TEXT("<missing rule>"));
	}

	AActor* FirstTarget(const TCHAR* Name)
	{
		const ADCConditionalPresence* R = Rule(Name);
		return R && R->GetTargets().Num() > 0 ? R->GetTargets()[0].Get() : nullptr;
	}

	bool TargetsShown(const TCHAR* Name)
	{
		const ADCConditionalPresence* R = Rule(Name);
		if (!R || R->GetTargets().IsEmpty())
		{
			return false;
		}
		for (const TObjectPtr<AActor>& Target : R->GetTargets())
		{
			if (!Target || Target->IsHidden() || (!Target->GetActorEnableCollision() && Target->IsA<ADCInspectableActor>()))
			{
				return false;
			}
		}
		return true;
	}

	bool TargetsHidden(const TCHAR* Name)
	{
		const ADCConditionalPresence* R = Rule(Name);
		if (!R || R->GetTargets().IsEmpty())
		{
			return false;
		}
		for (const TObjectPtr<AActor>& Target : R->GetTargets())
		{
			if (!Target || !Target->IsHidden() || Target->GetActorEnableCollision())
			{
				return false;
			}
		}
		return true;
	}

	double Dist2D(const AActor* Actor, const FVector& Where)
	{
		if (!Actor)
		{
			return 1.0e9;
		}
		FVector Center, Extent;
		Actor->GetActorBounds(false, Center, Extent);
		return FVector::Dist2D(Center, Where);
	}

	AActor* Mara() { return Find(TEXT("boat.mara")); }

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

	ADCFlickerLight* Bulb()
	{
		for (TActorIterator<ADCFlickerLight> It(GameWorld()); It; ++It)
		{
			if (It->Tags.Contains(TEXT("LandingStage")) && FVector::Dist2D(It->GetActorLocation(), FVector(1272.0, -978.0, 0.0)) < 20.0)
			{
				return *It;
			}
		}
		return nullptr;
	}

	ADCHUD* HUD()
	{
		const APlayerController* PC = Player() ? Cast<APlayerController>(Player()->GetController()) : nullptr;
		return PC ? PC->GetHUD<ADCHUD>() : nullptr;
	}

	FString Message() { return HUD() ? HUD()->GetActiveMessage().ToString() : FString(); }

	FString Banner() { return HUD() ? HUD()->GetActiveBannerSubtitle().ToString() : FString(); }

	FString Read(const TCHAR* DisplayName)
	{
		ADCInspectableActor* Target = Inspectable(DisplayName);
		if (!Target || !Player())
		{
			return FString();
		}
		IDCInteractable::Execute_Interact(Target, Player());
		return Message();
	}

	void Teleport(const FVector& Where)
	{
		if (ADCPlayerCharacter* P = Player())
		{
			P->GetDialogueComponent()->EndDialogue();
			P->SetActorLocation(Where, false, nullptr, ETeleportType::TeleportPhysics);
		}
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
		if (!P || !Mara())
		{
			return NAME_None;
		}
		P->GetDialogueComponent()->EndDialogue();
		IDCInteractable::Execute_Interact(Mara(), P);
		return P->GetDialogueComponent()->GetCurrentNodeId();
	}

	FName Stage() { return Player() ? Player()->GetQuestComponent()->GetStage(Quest) : NAME_None; }

	int32 StacksIn(FName Id)
	{
		const ADCLootContainer* C = Cast<ADCLootContainer>(Find(Id));
		return C ? C->GetInventoryComponent()->GetStacks().Num() : -1;
	}

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

	/** F9, then wait only until the reopened map has a player: the checks that follow run before any rule tick. */
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

	/** Every rule and actor the tests read is in the map. */
	bool StagePlaced(FAutomationTestBase* Test)
	{
		bool bOk = Test->TestNotNull(TEXT("Player"), Player()) && Test->TestNotNull(TEXT("Mara"), Mara());
		for (const TCHAR* Name : { TEXT("OpenLanding"), TEXT("CrateLid"), TEXT("CrateContents"), TEXT("Packed"),
			TEXT("CutLine"), TEXT("Skiff"), TEXT("Mara"), TEXT("MaraPack") })
		{
			bOk &= Test->TestNotNull(*FString::Printf(TEXT("Rule %s placed"), Name), Rule(Name));
		}
		bOk &= Test->TestNotNull(TEXT("Crate placed"), Inspectable(TEXT("Crate")));
		bOk &= Test->TestNotNull(TEXT("Bulb placed"), Bulb());
		bOk &= Test->TestNotNull(TEXT("Tackle box placed"), Find(Tackle));
		return bOk;
	}

	/** The before-resolution picture. */
	void ExpectDefault(FAutomationTestBase* Test, const FString& When)
	{
		auto Label = [&When](const TCHAR* What) { return FString::Printf(TEXT("%s: %s"), *When, What); };
		Test->TestTrue(*Label(TEXT("lantern lit, card up, line tied")), TargetsShown(TEXT("OpenLanding")));
		Test->TestEqual(*Label(TEXT("lid leaning (default)")), StateOf(TEXT("CrateLid")), FName());
		Test->TestTrue(*Label(TEXT("crate half packed")), TargetsShown(TEXT("CrateContents")));
		Test->TestTrue(*Label(TEXT("no lashing, no cargo")), TargetsHidden(TEXT("Packed")));
		Test->TestTrue(*Label(TEXT("no cut line")), TargetsHidden(TEXT("CutLine")));
		Test->TestTrue(*Label(TEXT("skiff moored")), Dist2D(FirstTarget(TEXT("Skiff")), SkiffMoored) < 60.0);
		Test->TestTrue(*Label(TEXT("Mara at her lookout")), Dist2D(Mara(), Lookout) < 80.0);
		Test->TestTrue(*Label(TEXT("pack at the lookout")), Dist2D(FirstTarget(TEXT("MaraPack")), Lookout) < 300.0);
	}

	void ExpectCoil(FAutomationTestBase* Test, const FString& When)
	{
		auto Label = [&When](const TCHAR* What) { return FString::Printf(TEXT("%s: %s"), *When, What); };
		Test->TestTrue(*Label(TEXT("lantern lit")), TargetsShown(TEXT("OpenLanding")));
		Test->TestEqual(*Label(TEXT("lid shut")), StateOf(TEXT("CrateLid")), FName(TEXT("coil")));
		Test->TestTrue(*Label(TEXT("contents packed away")), TargetsHidden(TEXT("CrateContents")));
		Test->TestTrue(*Label(TEXT("lashed and loaded")), TargetsShown(TEXT("Packed")));
		Test->TestTrue(*Label(TEXT("skiff moored")), Dist2D(FirstTarget(TEXT("Skiff")), SkiffMoored) < 60.0);
		Test->TestEqual(*Label(TEXT("Mara rule")), StateOf(TEXT("Mara")), FName(TEXT("coil")));
		Test->TestTrue(*Label(TEXT("Mara on the stage")), Dist2D(Mara(), MaraOnStage) < 80.0);
		Test->TestTrue(*Label(TEXT("pack on the stage")), Dist2D(FirstTarget(TEXT("MaraPack")), MaraOnStage) < 300.0);
	}

	void ExpectCombat(FAutomationTestBase* Test, const FString& When)
	{
		auto Label = [&When](const TCHAR* What) { return FString::Printf(TEXT("%s: %s"), *When, What); };
		Test->TestTrue(*Label(TEXT("lantern out, card gone, line gone")), TargetsHidden(TEXT("OpenLanding")));
		Test->TestEqual(*Label(TEXT("lid thrown down")), StateOf(TEXT("CrateLid")), FName(TEXT("combat")));
		Test->TestTrue(*Label(TEXT("crate emptied")), TargetsHidden(TEXT("CrateContents")));
		Test->TestTrue(*Label(TEXT("no lashing, no cargo")), TargetsHidden(TEXT("Packed")));
		Test->TestTrue(*Label(TEXT("cut line on the cleat")), TargetsShown(TEXT("CutLine")));
		Test->TestTrue(*Label(TEXT("skiff adrift")), Dist2D(FirstTarget(TEXT("Skiff")), SkiffAdrift) < 60.0);
		Test->TestTrue(*Label(TEXT("Mara stays at her lookout")), Dist2D(Mara(), Lookout) < 80.0);
		Test->TestTrue(*Label(TEXT("pack stays at the lookout")), Dist2D(FirstTarget(TEXT("MaraPack")), Lookout) < 300.0);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCLandingStageTest, "DeadCurrent.Map.Boathouse.LandingStage",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCLandingStageTest::RunTest(const FString& Parameters)
{
	using namespace DCLandingStageTest;
	QueueFreshMap();

	// Before Shore Watch: the default picture, discovery once, the bulbs and their power.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		ExpectDefault(this, TEXT("Fresh map"));
		TestTrue(TEXT("Bulbs lit while the wreck's battery is live"), Bulb()->IsLightActive());

		// The art-level dressing (dress_landing.py) is loaded with the map and never collides.
		int32 Dressing = 0;
		int32 Colliding = 0;
		for (TActorIterator<AActor> It(GameWorld()); It; ++It)
		{
			if (It->Tags.Contains(TEXT("LandingDress")))
			{
				++Dressing;
				Colliding += It->GetActorEnableCollision() ? 1 : 0;
			}
		}
		TestTrue(TEXT("Landing dressing loaded from the art level"), Dressing >= 5);
		TestEqual(TEXT("Landing dressing has no collision"), Colliding, 0);
		TestTrue(TEXT("Crate reads half packed"), Read(TEXT("Crate")).Contains(TEXT("half packed")));
		TestTrue(TEXT("Card readable"), Read(TEXT("Card")).Contains(TEXT("UNLESS THE LAMP IS LIT")));
		TestFalse(TEXT("Stage not yet discovered"), WorldState()->IsLocationDiscovered(Location));
		Teleport(OnGangway);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.6f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestTrue(TEXT("Walking onto the gangway discovers the stage"), WorldState() && WorldState()->IsLocationDiscovered(Location));
		TestEqual(TEXT("Banner"), Banner(), FString(TEXT("Landing Stage")));

		// The tackle box: take one stack; the save below must remember it.
		IDCInteractable::Execute_Interact(Find(Tackle), Player());
		TestEqual(TEXT("One stack left in the tackle box"), StacksIn(Tackle), 1);

		// The wreck's power, cut elsewhere, reaches the stage's bulbs.
		WorldState()->SetFlag(TEXT("wreck.power_cut"));
		TestFalse(TEXT("Power cut: bulbs dark"), Bulb()->IsLightActive());
		TestTrue(TEXT("Power cut: bulbs read dark"), Read(TEXT("Bulbs")).Contains(TEXT("dark")));
		WorldState()->ClearFlag(TEXT("wreck.power_cut"));

		// Coil route, through the real pickup and Mara's real turn-in.
		IDCInteractable::Execute_Interact(Find(TEXT("boat.pickup_coil")), Player());
		Teleport(NearMara);
		TestEqual(TEXT("Mara greets"), TalkToMara(), FName(TEXT("greeting")));
		Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Coil shortcut"), Say(TEXT("This coil? I already pulled it.")));
		TestTrue(TEXT("Hand over"), Say(TEXT("Here. It's yours.")));
		TestEqual(TEXT("Coil outcome"), Stage(), FName(TEXT("done_coil")));

		// She does not vanish in front of the player.
		TestTrue(TEXT("Mara's move waits while the player is with her"), Rule(TEXT("Mara"))->HasPendingChange());
		TestTrue(TEXT("Mara still at the lookout mid-conversation"), Dist2D(Mara(), Lookout) < 80.0);
		Teleport(FarAway);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		ExpectCoil(this, TEXT("Coil route, player away"));
		TestTrue(TEXT("Crate reads packed"), Read(TEXT("Crate")).Contains(TEXT("lashed")));
		TestEqual(TEXT("Mara talks from the stage"), TalkToMara(), FName(TEXT("done_coil")));

		// Save standing on the stage, so only the restore snap (not a later tick) can put Mara there on load.
		Teleport(OnGangway);
		TestTrue(TEXT("Save on the stage (coil)"), Saves()->SaveCurrentGame());

		// Diverge: the flag goes, but the player is right here, so nothing moves under them.
		WorldState()->ClearFlag(TEXT("shore.relay_recovered"));
		TestTrue(TEXT("Diverged change waits while the player watches"), Rule(TEXT("Mara"))->HasPendingChange());
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		ExpectCoil(this, TEXT("F9 (coil), at once"));
		TestTrue(TEXT("F9: still discovered"), WorldState()->IsLocationDiscovered(Location));
		TestNotEqual(TEXT("F9: no second banner"), Banner(), FString(TEXT("Landing Stage")));
		TestEqual(TEXT("F9: tackle box stays one stack"), StacksIn(Tackle), 1);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		ExpectCoil(this, TEXT("F9 (coil), a second later"));
		return true;
	}));

	// Combat route on a fresh map: the real kill and turn-in.
	QueueFreshMap();
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		if (!TestNotNull(TEXT("Scavenger"), Scav))
		{
			return true;
		}
		Teleport(NearMara);
		TalkToMara();
		Say(TEXT("You keep looking toward his camp."));
		TestTrue(TEXT("Accept"), Say(TEXT("I'll put him down.")));
		FDCDamageInfo Damage;
		Damage.Amount = 1000.0f;
		Damage.Instigator = Player();
		UDCHealthComponent::ApplyDamageToActor(Scav, Damage);
		TestEqual(TEXT("Kill advanced the quest"), Stage(), FName(TEXT("return_killed")));
		ExpectDefault(this, TEXT("Killed, not yet told"));

		TestEqual(TEXT("Turn-in node"), TalkToMara(), FName(TEXT("turnin_kill")));
		TestTrue(TEXT("Turn in"), Say(TEXT("He's dead. His relay has no one to tend it.")));
		TestEqual(TEXT("Kill outcome"), Stage(), FName(TEXT("done_killed")));
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.6f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		// The stage is 30 m away behind the ridge: it changed while the player stood with Mara.
		ExpectCombat(this, TEXT("Combat route"));
		TestTrue(TEXT("Crate reads emptied"), Read(TEXT("Crate")).Contains(TEXT("empty")));
		TestTrue(TEXT("Lantern reads cold"), Read(TEXT("Storm lantern")).Contains(TEXT("cold")));

		// Handing over a coil after the kill does not bring the skiff back.
		WorldState()->SetFlag(TEXT("shore.relay_recovered"));
		ExpectCombat(this, TEXT("Kill, then coil"));
		WorldState()->ClearFlag(TEXT("shore.relay_recovered"));

		Teleport(OnGangway);
		TestTrue(TEXT("Save on the stage (combat)"), Saves()->SaveCurrentGame());
		WorldState()->ClearFlag(TEXT("shore.path_cleared"));
		return true;
	}));

	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		ExpectCombat(this, TEXT("F9 (combat), at once"));
		TestEqual(TEXT("F9 (combat): Mara's epilogue"), TalkToMara(), FName(TEXT("done_killed")));
		return true;
	}));

	QueueCleanup();
	return true;
}

/**
 *  Saves written before the stage existed (version 5, the last format) load with the stage showing what their
 *  flags imply, the stage undiscovered, and its container full.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCLandingStageSavesTest, "DeadCurrent.Map.Boathouse.LandingStageSaves",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCLandingStageSavesTest::RunTest(const FString& Parameters)
{
	using namespace DCLandingStageTest;

	auto WriteOldSave = [this](FName OutcomeStage, FName OutcomeFlag, bool bScavengerDead)
	{
		UDCSaveGame* Old = NewObject<UDCSaveGame>();
		Old->SaveVersion = 5;
		Old->MapName = TEXT("Lvl_Boathouse");
		Old->MapPackage = MapPath;
		Old->PlayerLocation = FVector(300.0, 0.0, 100.0);
		Old->PlayerHealth = 100.0f;
		Old->Quests.Add({ Quest, OutcomeStage });
		Old->WorldFlags.Add(OutcomeFlag);
		if (bScavengerDead)
		{
			FDCPersistentActorState Scav;
			Scav.PersistentId = TEXT("boat.scavenger");
			Scav.bAlive = false;
			Old->WorldActors.Add(Scav);
		}
		TestTrue(TEXT("Wrote a pre-Phase-5 save"), UGameplayStatics::SaveGameToSlot(Old, TestSlot, 0));
	};

	QueueFreshMap();
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, WriteOldSave]()
	{
		WriteOldSave(TEXT("done_coil"), TEXT("shore.relay_recovered"), false);
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		ExpectCoil(this, TEXT("Old coil save"));
		TestFalse(TEXT("Old coil save: stage undiscovered"), WorldState()->IsLocationDiscovered(Location));
		TestEqual(TEXT("Old coil save: tackle box full"), StacksIn(Tackle), 2);
		return true;
	}));

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, WriteOldSave]()
	{
		WriteOldSave(TEXT("done_killed"), TEXT("shore.path_cleared"), true);
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		ExpectCombat(this, TEXT("Old combat save"));
		TestFalse(TEXT("Old combat save: stage undiscovered"), WorldState()->IsLocationDiscovered(Location));
		const ADCScavengerCharacter* Scav = Cast<ADCScavengerCharacter>(Find(TEXT("boat.scavenger")));
		TestTrue(TEXT("Old combat save: scavenger dead"), Scav && Scav->GetHealthComponent()->IsDead());
		return true;
	}));

	QueueCleanup();
	return true;
}

/**
 *  The player's own save, if there is one (Saved/SaveGames/DeadCurrent.sav), copied to a scratch slot and loaded:
 *  whatever it holds, the stage must show what its flags imply and the stage must be undiscovered unless the save
 *  says otherwise. The player's slot is only read. With no save on this machine the test logs that and passes.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCLandingStagePlayerSaveTest, "DeadCurrent.Map.Boathouse.LandingStagePlayerSave",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCLandingStagePlayerSaveTest::RunTest(const FString& Parameters)
{
	using namespace DCLandingStageTest;
	const FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
	const FString PlayerSave = SaveDir / TEXT("DeadCurrent.sav");
	if (!FPaths::FileExists(PlayerSave))
	{
		AddInfo(TEXT("No player save on this machine; nothing to check."));
		return true;
	}

	QueueFreshMap();
	TSharedRef<TArray<FName>> Flags = MakeShared<TArray<FName>>();
	TSharedRef<bool> bSavedDiscovered = MakeShared<bool>(false);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, PlayerSave, SaveDir, Flags, bSavedDiscovered]()
	{
		TestTrue(TEXT("Copied the player's save to the scratch slot"),
			IFileManager::Get().Copy(*(SaveDir / FString(TestSlot) + TEXT(".sav")), *PlayerSave) == COPY_OK);
		if (const UDCSaveGame* Save = Cast<UDCSaveGame>(UGameplayStatics::LoadGameFromSlot(TestSlot, 0)))
		{
			*Flags = Save->WorldFlags;
			*bSavedDiscovered = Save->DiscoveredLocations.Contains(Location);
			AddInfo(FString::Printf(TEXT("Player save: version %d, %d flags, %d quests"), Save->SaveVersion,
				Save->WorldFlags.Num(), Save->Quests.Num()));
		}
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Flags, bSavedDiscovered]()
	{
		if (!StagePlaced(this))
		{
			return true;
		}
		if (Flags->Contains(TEXT("shore.path_cleared")))
		{
			ExpectCombat(this, TEXT("Player save (combat flags)"));
		}
		else if (Flags->Contains(TEXT("shore.relay_recovered")))
		{
			ExpectCoil(this, TEXT("Player save (coil flags)"));
		}
		else
		{
			ExpectDefault(this, TEXT("Player save (no outcome flag)"));
		}
		TestEqual(TEXT("Player save: stage discovered only if the save says so"),
			WorldState()->IsLocationDiscovered(Location), *bSavedDiscovered);
		return true;
	}));
	QueueCleanup();
	return true;
}

#endif
