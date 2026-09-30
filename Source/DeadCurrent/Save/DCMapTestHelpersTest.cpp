#include "Core/DCMapTestHelpers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  Phase 6 (VS-02): proves the shared in-map test harness (Core/DCMapTestHelpers.h) on the real Lvl_Boathouse, and is
 *  the smallest worked example of a map test: fresh map, find and talk, teleport, F5, diverge, F9, wait for a condition,
 *  clean up. Game context (run with -game, see Tools/RunTests.bat). Uses a scratch save slot of its own.
 */
namespace DCMapTestHelpersTest
{
	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_Boathouse");
	const TCHAR* TestSlot = TEXT("DeadCurrent_HelpersTest");
	const FName Flag = TEXT("helpers.test_flag");
	const FVector InsideBoathouse(300.0, 0.0, 100.0);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCMapTestHelpersTest, "DeadCurrent.Map.Boathouse.TestHelpers",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCMapTestHelpersTest::RunTest(const FString& Parameters)
{
	using namespace DCMapTest;
	using namespace DCMapTestHelpersTest;
	QueueFreshMap(MapPath, TestSlot);

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player"), Player()) || !TestNotNull(TEXT("Mara found by persistent id"), Find(TEXT("boat.mara")))
			|| !TestNotNull(TEXT("Inspectable found by display name"), Inspectable(TEXT("Battery bank")))
			|| !TestNotNull(TEXT("World state"), WorldState()) || !TestNotNull(TEXT("HUD"), HUD()))
		{
			return true;
		}
		TestEqual(TEXT("Fresh inventory"), Count(TEXT("ammo_9mm")), 0);
		TestEqual(TEXT("No quest yet"), Stage(TEXT("shore.watch")), NAME_None);
		TestTrue(TEXT("Alive"), Health() > 0.0f);

		// Conversation: entry node, visible replies, pick one by text, a reply that is not there is refused.
		TestEqual(TEXT("TalkTo starts the NPC's conversation"), TalkTo(TEXT("boat.mara")), FName(TEXT("greeting")));
		TestTrue(TEXT("A visible reply"), VisibleChoiceTexts().Contains(TEXT("Who are you?")));
		TestTrue(TEXT("Say picks it"), Say(TEXT("Who are you?")));
		TestFalse(TEXT("Say refuses a reply that is not offered"), Say(TEXT("No such reply.")));
		TestEqual(TEXT("Conversation moved on"), Player()->GetDialogueComponent()->GetCurrentNodeId(), FName(TEXT("who")));
		TestEqual(TEXT("TalkTo to a missing id does nothing"), TalkTo(TEXT("no.such.npc")), NAME_None);

		// Teleport ends the conversation and moves the player.
		Teleport(InsideBoathouse);
		TestNull(TEXT("Teleport ends the conversation"), Player()->GetDialogueComponent()->GetCurrentNode());
		TestTrue(TEXT("Teleported"), FVector::Dist2D(Player()->GetActorLocation(), InsideBoathouse) < 5.0);

		// F5, then diverge.
		WorldState()->SetFlag(Flag);
		TestTrue(TEXT("Save"), Saves()->SaveCurrentGame());
		WorldState()->ClearFlag(Flag);
		TestFalse(TEXT("Diverged"), WorldState()->HasFlag(Flag));
		Teleport(FVector(2000.0, -400.0, 100.0));
		return true;
	}));

	QueueLoad(this);

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		if (!TestNotNull(TEXT("Player after F9"), Player()) || !TestNotNull(TEXT("World state after F9"), WorldState()))
		{
			return true;
		}
		TestTrue(TEXT("F9 restored the flag"), WorldState()->HasFlag(Flag));
		TestTrue(TEXT("F9 restored the position"), FVector::Dist2D(Player()->GetActorLocation(), InsideBoathouse) < 50.0);
		TestNotNull(TEXT("Helpers still find actors in the reopened map"), Find(TEXT("boat.mara")));
		return true;
	}));

	// QueueWaitUntil polls each frame and moves on the moment the condition holds.
	TSharedRef<int32> Polls = MakeShared<int32>(0);
	QueueWaitUntil(this, [Polls]() { return ++*Polls >= 5; }, TEXT("five polls"), 5.0);

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Polls]()
	{
		TestEqual(TEXT("QueueWaitUntil stopped polling once the condition held"), *Polls, 5);
		return true;
	}));

	QueueCleanup(TestSlot);
	return true;
}

#endif
