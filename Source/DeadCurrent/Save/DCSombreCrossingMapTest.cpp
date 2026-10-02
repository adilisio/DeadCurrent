#include "Core/DCMapTestHelpers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "World/DCCellPortal.h"
#include "World/DCConditionalPresence.h"
#include "World/DCFlickerLight.h"

/**
 *  Phase 6 VS-10: the crossing (B0) and the harbor arrival (B1) on the real map (game context; Tools\RunTests.bat
 *  Map.Sombre.Crossing). Owned by the crossing and harbor cells (Design/POIs/sombre.crossing.md, sombre.harbor.md);
 *  there is no Map.Sombre.Harbor (ledger §12).
 *
 *    zero investment   read nothing, talk to no one, use the door: the strike still happens, Mara and Varga are on the quay
 *    the crossing      a new game on the deck in the storm; the letter gives liv_letter once; the chart; Mara's tie
 *                      question sets one tie flag and is asked once; her watch reveal; F5 on the deck, diverge, F9
 *    the strike        the door sets sombre.reef_struck and sombre.storm; the player on the quay facing up the island;
 *                      Mara and the Ida already there; Varga aboard; the door gone; the crossing's false light out
 *    the harbor        sombre.harbor discovered once; Varga's first reply starts both quests; she waits after
 */
namespace DCSombreCrossingTest
{
	using namespace DCMapTest;

	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_PointeSombre");
	const TCHAR* TestSlot = TEXT("DeadCurrent_SombreCrossingTest");

	const FName ReefStruck = TEXT("sombre.reef_struck");
	const FName Storm = TEXT("sombre.storm");
	const FName LetterRead = TEXT("watch.mara_travelling");
	const FName TieAsked = TEXT("sombre.mara_tie_asked");
	const FName Sister = TEXT("player.tie.sister");
	const FName Partner = TEXT("player.tie.partner");
	const FName TookIn = TEXT("player.tie.took_in");
	const FName Harbor = TEXT("sombre.harbor");
	const FName Characteristic = TEXT("sombre.characteristic");
	const FName FalseLight = TEXT("sombre.false_light");

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

	inline AActor* Labelled(const TCHAR* Label)
	{
		for (TActorIterator<AActor> It(GameWorld()); It; ++It)
		{
			if (It->GetActorNameOrLabel() == Label)
			{
				return *It;
			}
		}
		return nullptr;
	}

	inline FVector At(const TCHAR* Tag)
	{
		const AActor* Actor = Tagged(Tag);
		return Actor ? Actor->GetActorLocation() : FVector(NAN);
	}

	inline ADCConditionalPresence* Rule(const TCHAR* Tag) { return Cast<ADCConditionalPresence>(Tagged(Tag)); }

	inline FName StateOf(const TCHAR* Tag)
	{
		const ADCConditionalPresence* R = Rule(Tag);
		return R ? R->GetActiveStateId() : NAME_None;
	}

	inline ADCCellPortal* Door() { return Cast<ADCCellPortal>(Tagged(TEXT("Crossing:WheelhouseDoor"))); }

	inline bool LanternLit()
	{
		const ADCFlickerLight* Lantern = Cast<ADCFlickerLight>(Labelled(TEXT("Lantern_Crossing")));
		return Lantern && Lantern->IsLightActive();
	}

	/** The new-game look: every target tagged Atmosphere:storm in place and shown (the core's VS-04 rule). */
	inline bool StormLook()
	{
		int32 Shown = 0;
		for (TActorIterator<AActor> It(GameWorld()); It; ++It)
		{
			if (It->ActorHasTag(TEXT("Atmosphere:storm")))
			{
				if (It->IsHidden() || It->GetActorLocation().Z < -100000.0)
				{
					return false;
				}
				++Shown;
			}
		}
		return Shown > 0;
	}

	inline int32 TieFlags()
	{
		return (WorldState()->HasFlag(Sister) ? 1 : 0) + (WorldState()->HasFlag(Partner) ? 1 : 0) + (WorldState()->HasFlag(TookIn) ? 1 : 0);
	}

	inline APlayerController* PC() { return Player() ? Cast<APlayerController>(Player()->GetController()) : nullptr; }

	/** Uses the wheelhouse door as play does (the timed fade) and waits until the player stands on the quay. */
	inline void QueueStrike(FAutomationTestBase* Test)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Test]()
		{
			ADCCellPortal* Strike = Door();
			if (Test->TestNotNull(TEXT("The wheelhouse door"), Strike))
			{
				Test->TestEqual(TEXT("The door's verb"), IDCInteractable::Execute_GetInteractionPrompt(Strike, Player()).Action.ToString(),
					FString(TEXT("Tell Varga about the light")));
				Test->TestEqual(TEXT("The door opens"), Strike->TryUse(Player()), EDCPortalUse::Passed);
			}
			return true;
		}));
		QueueWaitUntil(Test, []()
		{
			return Player() && FVector::Dist2D(Player()->GetActorLocation(), At(TEXT("Anchor:Anchor_CrossingExit_Quay"))) < 100.0
				&& PC() && !PC()->IsMoveInputIgnored();
		}, TEXT("the strike's arrival on the quay"), 8.0);
	}

	/** After the strike: the arrival, the people, and the world state the crossing hands to the harbor. */
	inline void CheckArrival(FAutomationTestBase* Test)
	{
		Test->TestTrue(TEXT("The strike set sombre.reef_struck"), WorldState()->HasFlag(ReefStruck));
		Test->TestTrue(TEXT("The strike set sombre.storm (ledger §3.1)"), WorldState()->HasFlag(Storm));
		Test->TestTrue(TEXT("The look is still the storm (the look does not read sombre.storm)"), StormLook());
		Test->TestTrue(TEXT("Facing up the island (the arrival's yaw, 40)"),
			FMath::Abs(FRotator::NormalizeAxis(PC()->GetControlRotation().Yaw - 40.0)) < 2.0);
		Test->TestEqual(TEXT("The Ida is at her berth"), StateOf(TEXT("Presence_Ida")), FName(TEXT("berth")));
		Test->TestEqual(TEXT("The wheelhouse door is gone"), StateOf(TEXT("Presence_IdaDoor")), FName(TEXT("gone")));
		Test->TestTrue(TEXT("The door cannot be used again"), !Door() || Door()->IsHidden());
		Test->TestEqual(TEXT("Mara is on the quay"), StateOf(TEXT("Presence_Mara")), FName(TEXT("quay")));
		const AActor* Mara = Find(TEXT("sombre.mara"));
		if (Test->TestNotNull(TEXT("Mara"), Mara))
		{
			Test->TestFalse(TEXT("Mara is shown"), Mara->IsHidden());
			const double Dist = FVector::Dist2D(Mara->GetActorLocation(), Player()->GetActorLocation());
			Test->AddInfo(FString::Printf(TEXT("Mara stands %.0f cm from the arrival"), Dist));
			Test->TestTrue(TEXT("Mara is already there, near the arrival"), Dist < 600.0);
		}
		Test->TestEqual(TEXT("Varga is aboard (not at the wheel)"), StateOf(TEXT("Presence_Varga")), NAME_None);
		const AActor* Varga = Find(TEXT("sombre.varga"));
		if (Test->TestNotNull(TEXT("Varga"), Varga))
		{
			Test->TestFalse(TEXT("Varga is shown"), Varga->IsHidden());
			Test->TestTrue(TEXT("Varga is within talking distance of the quay"),
				FVector::Dist2D(Varga->GetActorLocation(), Player()->GetActorLocation()) < 900.0);
		}
		Test->TestFalse(TEXT("The crossing's false light is out after the strike"), LanternLit());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreCrossingTest, "DeadCurrent.Map.Sombre.Crossing",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreCrossingTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreCrossingTest;

	// ---- Zero investment: read nothing, talk to no one, use the door.
	QueueFreshMap(MapPath, TestSlot);
	QueueStrike(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		CheckArrival(this);
		TestEqual(TEXT("Zero investment: no letter"), Count(TEXT("liv_letter")), 0);
		TestEqual(TEXT("Zero investment: no tie"), TieFlags(), 0);
		TestFalse(TEXT("Zero investment: the tie was never asked"), WorldState()->HasFlag(TieAsked));
		return true;
	}));
	QueueWaitUntil(this, []() { return WorldState()->IsLocationDiscovered(Harbor); }, TEXT("the harbor's discovery"), 5.0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		// Mara met first on the quay: the crossing is over, so no "Before we get there" (VS-10 finding #17).
		TestEqual(TEXT("Zero investment: Mara on the quay has her island line"), TalkTo(TEXT("sombre.mara")), FName(TEXT("town")));
		Say(TEXT("Goodbye."));
		TestEqual(TEXT("No tie asked on the quay"), TieFlags(), 0);
		return true;
	}));

	// ---- The crossing, played.
	QueueFreshMap(MapPath, TestSlot);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("The door"), Door()))
		{
			return true;
		}
		TestTrue(TEXT("A new game starts on the deck"),
			FVector::Dist2D(Player()->GetActorLocation(), At(TEXT("Anchor:Anchor_NewGame_Deck"))) < 150.0);
		TestTrue(TEXT("A new game is in the storm look"), StormLook());
		TestFalse(TEXT("No storm flag before the strike (the strike sets it)"), WorldState()->HasFlag(Storm));
		TestFalse(TEXT("Not struck yet"), WorldState()->HasFlag(ReefStruck));
		TestTrue(TEXT("The false light burns ahead"), LanternLit());
		TestNull(TEXT("The greybox's glow box is retired"), Labelled(TEXT("FalseLight_Lantern")));
		TestEqual(TEXT("Mara at the rail (her default)"), StateOf(TEXT("Presence_Mara")), NAME_None);
		const AActor* Mara = Find(TEXT("sombre.mara"));
		if (TestNotNull(TEXT("Mara"), Mara))
		{
			TestTrue(TEXT("Mara is at the rail"), FVector::Dist2D(Mara->GetActorLocation(), At(TEXT("Anchor:Anchor_Mara_Rail"))) < 150.0);
		}
		TestEqual(TEXT("Varga is at the wheel"), StateOf(TEXT("Presence_Varga")), FName(TEXT("at_the_wheel")));
		const AActor* Varga = Find(TEXT("sombre.varga"));
		TestTrue(TEXT("Varga is not on deck"), Varga && Varga->IsHidden());

		// Liv's letter: the first read gives it, once.
		ADCInspectableActor* Letter = Inspectable(TEXT("Liv's letter"));
		if (TestNotNull(TEXT("Liv's letter"), Letter))
		{
			TestEqual(TEXT("The letter's verb"), IDCInteractable::Execute_GetInteractionPrompt(Letter, Player()).Action.ToString(), FString(TEXT("Read")));
			Use(Letter);
			TestEqual(TEXT("Reading gives liv_letter"), Count(TEXT("liv_letter")), 1);
			TestTrue(TEXT("Reading sets watch.mara_travelling"), WorldState()->HasFlag(LetterRead));
			TestTrue(TEXT("The letter's text"), Message().StartsWith(TEXT("Posted from the old Authority landing")));
			Use(Letter);
			TestEqual(TEXT("A second read gives nothing more"), Count(TEXT("liv_letter")), 1);
			TestTrue(TEXT("The re-read line"), Message().StartsWith(TEXT("Liv's letter. You know it by heart.")));
		}
		ADCInspectableActor* Chart = Inspectable(TEXT("Chart"));
		if (TestNotNull(TEXT("The recap chart"), Chart))
		{
			Use(Chart);
			TestTrue(TEXT("The chart names the run"), Message().Contains(TEXT("Pointe Sombre")));
		}

		// Mara's tie question: one tie flag, asked once; her watch reveal.
		TestEqual(TEXT("Mara asks at the rail"), TalkTo(TEXT("sombre.mara")), FName(TEXT("crossing_tie")));
		TestTrue(TEXT("Answer: partner"), Say(TEXT("My partner.")));
		TestEqual(TEXT("Exactly one tie flag"), TieFlags(), 1);
		TestTrue(TEXT("The partner flag"), WorldState()->HasFlag(Partner));
		TestTrue(TEXT("Asked"), WorldState()->HasFlag(TieAsked));
		TestEqual(TEXT("Then the wrong light"), Player()->GetDialogueComponent()->GetCurrentNodeId(), FName(TEXT("crossing")));
		TestTrue(TEXT("Ask about the shore"), Say(TEXT("What is it you do, on that shore?")));
		TestTrue(TEXT("The watch reveal"), WorldState()->HasFlag(TEXT("watch.revealed")));
		TestTrue(TEXT("And the third"), Say(TEXT("And the third?")));
		TestTrue(TEXT("Hold on"), Say(TEXT("Hold on to something.")));
		TestEqual(TEXT("She does not ask again"), TalkTo(TEXT("sombre.mara")), FName(TEXT("crossing")));
		TestFalse(TEXT("The watch is told once"), VisibleChoiceTexts().Contains(TEXT("What is it you do, on that shore?")));
		Say(TEXT("Hold on to something."));

		// F5 on the deck, then diverge.
		TestTrue(TEXT("Save on the deck"), Saves()->SaveCurrentGame());
		WorldState()->ClearFlag(LetterRead);
		WorldState()->ClearFlag(Partner);
		Teleport(At(TEXT("Anchor:Anchor_CrossingExit_Quay")));
		return true;
	}));
	QueueLoad(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player after F9"), Player()))
		{
			return true;
		}
		TestTrue(TEXT("F9: back on the deck"),
			FVector::Dist2D(Player()->GetActorLocation(), At(TEXT("Anchor:Anchor_NewGame_Deck"))) < 1200.0
			&& Player()->GetActorLocation().Z > 250.0);
		TestTrue(TEXT("F9: the letter was read"), WorldState()->HasFlag(LetterRead));
		TestEqual(TEXT("F9: one letter"), Count(TEXT("liv_letter")), 1);
		TestTrue(TEXT("F9: the tie"), WorldState()->HasFlag(Partner));
		TestFalse(TEXT("F9: still at sea"), WorldState()->HasFlag(ReefStruck));
		TestEqual(TEXT("F9: Mara at the rail"), StateOf(TEXT("Presence_Mara")), NAME_None);
		ADCInspectableActor* Letter = Inspectable(TEXT("Liv's letter"));
		if (Letter)
		{
			Use(Letter);
			TestEqual(TEXT("F9: the letter is not given again"), Count(TEXT("liv_letter")), 1);
		}
		return true;
	}));

	// ---- The strike, and the harbor.
	QueueStrike(this);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]() { CheckArrival(this); return true; }));
	QueueWaitUntil(this, []() { return WorldState()->IsLocationDiscovered(Harbor); }, TEXT("the harbor's discovery"), 5.0);
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		TestFalse(TEXT("The harbor is discovered once"), WorldState()->DiscoverLocation(Harbor));
		TestEqual(TEXT("No quest before Varga"), Stage(Characteristic), NAME_None);
		TestEqual(TEXT("Varga's first conversation"), TalkTo(TEXT("sombre.varga")), FName(TEXT("first")));
		TestTrue(TEXT("Ask what she needs"), Say(TEXT("What do you need?")));
		TestEqual(TEXT("The Wrong Characteristic starts"), Stage(Characteristic), FName(TEXT("arrived")));
		TestEqual(TEXT("False Light starts"), Stage(FalseLight), FName(TEXT("asked")));
		TestTrue(TEXT("Her lead"), Say(TEXT("Liv came through here.")));
		TestEqual(TEXT("Then she waits"), TalkTo(TEXT("sombre.varga")), FName(TEXT("waiting")));
		TestEqual(TEXT("Mara on the quay, once the quests run"), TalkTo(TEXT("sombre.mara")), FName(TEXT("town")));
		Say(TEXT("Goodbye."));
		TestFalse(TEXT("The HUD shows an objective"), Player()->GetQuestComponent()->GetObjectiveText().IsEmpty());
		return true;
	}));

	QueueCleanup(TestSlot);
	return true;
}

#endif
