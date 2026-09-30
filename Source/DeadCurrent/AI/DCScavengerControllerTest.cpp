#include "AI/DCScavengerController.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  The scavenger's notice rule (Phase 5 playtest): standing, his sight sense decides alone; crouched, he notices the
 *  player only close and roughly in front of him. This is what makes Mara's "stay low, time his walk" true.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCScavengerNoticeTest, "DeadCurrent.AI.ScavengerNotice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCScavengerNoticeTest::RunTest(const FString& Parameters)
{
	const float Radius = 800.0f;
	const float HalfAngle = 45.0f;
	auto Notice = [&](bool bCrouched, float Distance, float Angle)
	{
		return ADCScavengerController::CanNoticeAt(bCrouched, Distance, Angle, Radius, HalfAngle);
	};

	TestTrue(TEXT("Standing, anything the sense reports is noticed"), Notice(false, 1700.0f, 70.0f));
	TestTrue(TEXT("Crouched, close and in front: noticed"), Notice(true, 500.0f, 20.0f));
	TestFalse(TEXT("Crouched at 12 m in front: not noticed"), Notice(true, 1200.0f, 0.0f));
	TestFalse(TEXT("Crouched close but off to the side: not noticed"), Notice(true, 500.0f, 60.0f));
	TestTrue(TEXT("Crouched at the edge of both limits: noticed"), Notice(true, 800.0f, 45.0f));
	return true;
}

#endif
