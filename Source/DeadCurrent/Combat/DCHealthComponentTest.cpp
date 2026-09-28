#include "Combat/DCHealthComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCHealthDamageTest, "DeadCurrent.Combat.Health",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCHealthDamageTest::RunTest(const FString& Parameters)
{
	UDCHealthComponent* Health = NewObject<UDCHealthComponent>();
	Health->ResetHealth();

	TestEqual(TEXT("Starts at max"), Health->GetHealth(), 100.0f);
	TestFalse(TEXT("Alive"), Health->IsDead());

	FDCDamageInfo Hit;
	Hit.Amount = 25.0f;
	TestEqual(TEXT("First shot"), Health->ApplyDamage(Hit), 25.0f);
	TestEqual(TEXT("75 left"), Health->GetHealth(), 75.0f);

	Health->ApplyDamage(Hit);
	Health->ApplyDamage(Hit);
	TestEqual(TEXT("Killing blow applies only remaining health"), Health->ApplyDamage(Hit), 25.0f);
	TestTrue(TEXT("Dead at zero"), Health->IsDead());
	TestEqual(TEXT("Health clamped"), Health->GetHealth(), 0.0f);
	TestEqual(TEXT("Dead ignores further damage"), Health->ApplyDamage(Hit), 0.0f);
	TestEqual(TEXT("Dead ignores heal"), Health->Heal(50.0f), 0.0f);

	Health->ResetHealth();
	TestFalse(TEXT("Reset clears death"), Health->IsDead());
	TestEqual(TEXT("Reset restores max"), Health->GetHealth(), 100.0f);
	TestEqual(TEXT("Heal does not exceed max"), Health->Heal(40.0f), 0.0f);

	Health->ApplyDamage(Hit);
	TestEqual(TEXT("Partial heal"), Health->Heal(10.0f), 10.0f);
	TestEqual(TEXT("Health after heal"), Health->GetHealth(), 85.0f);

	Health->ApplyLoadedState(40.0f, false);
	TestEqual(TEXT("Loaded health"), Health->GetHealth(), 40.0f);
	TestFalse(TEXT("Loaded alive"), Health->IsDead());
	Health->ApplyLoadedState(0.0f, true);
	TestTrue(TEXT("Loaded dead without Reset"), Health->IsDead());
	TestEqual(TEXT("Loaded dead health"), Health->GetHealth(), 0.0f);

	AActor* Nobody = nullptr;
	TestEqual(TEXT("Null target is rejected"), UDCHealthComponent::ApplyDamageToActor(Nobody, Hit), 0.0f);

	return true;
}

#endif
