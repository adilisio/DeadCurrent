#include "Core/DCTestHelpers.h"
#include "World/DCConditionalAudio.h"
#include "World/DCWorldStateSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  ADCConditionalAudio follows the shared rule language and only listens. The sound assets are not
 *  needed: the actor's audibility and trigger count are what presentation reads.
 */
namespace DCConditionalAudioTest
{
	ADCConditionalAudio* Spawn(const FDCTestWorld& World, FName Flag, bool bNegate, EDCConditionalAudioMode Mode)
	{
		ADCConditionalAudio* Actor = World.Get()->SpawnActor<ADCConditionalAudio>();
		FDCGameplayCondition Condition;
		Condition.Type = EDCConditionType::WorldFlag;
		Condition.Id = Flag;
		Condition.bNegate = bNegate;

		if (FArrayProperty* Property = CastField<FArrayProperty>(Actor->GetClass()->FindPropertyByName(TEXT("Conditions"))))
		{
			*Property->ContainerPtrToValuePtr<TArray<FDCGameplayCondition>>(Actor) = { Condition };
		}
		if (FEnumProperty* ModeProperty = CastField<FEnumProperty>(Actor->GetClass()->FindPropertyByName(TEXT("Mode"))))
		{
			ModeProperty->GetUnderlyingProperty()->SetIntPropertyValue(ModeProperty->ContainerPtrToValuePtr<void>(Actor), static_cast<int64>(Mode));
		}
		return Actor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCConditionalAudioTest, "DeadCurrent.Presentation.ConditionalAudio",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCConditionalAudioTest::RunTest(const FString& Parameters)
{
	using namespace DCConditionalAudioTest;
	FDCTestWorld World;
	UDCWorldStateSubsystem* WorldState = World.Get()->GetSubsystem<UDCWorldStateSubsystem>();

	// "Hums until the power is cut": the pattern the live water uses.
	ADCConditionalAudio* Hum = Spawn(World, TEXT("test.cut"), true, EDCConditionalAudioMode::WhileTrue);
	// "Plays once when the power is cut": the breaker throw.
	ADCConditionalAudio* Throw = Spawn(World, TEXT("test.cut"), false, EDCConditionalAudioMode::OnceWhenTrue);
	// Passing at level start stays silent: a load that already has the flag.
	WorldState->SetFlag(TEXT("test.already"));
	ADCConditionalAudio* Loaded = Spawn(World, TEXT("test.already"), false, EDCConditionalAudioMode::OnceWhenTrue);
	ADCConditionalAudio* Bed = World.Get()->SpawnActor<ADCConditionalAudio>();

	for (ADCConditionalAudio* Actor : { Hum, Throw, Loaded, Bed })
	{
		Actor->Evaluate();
	}
	TestTrue(TEXT("No conditions: always audible"), Bed->IsAudible());
	TestTrue(TEXT("Hum audible before the flag"), Hum->IsAudible());
	TestFalse(TEXT("Throw waits for the flag"), Throw->IsAudible());
	TestEqual(TEXT("Throw has not fired"), Throw->GetTriggerCount(), 0);
	TestEqual(TEXT("A flag already set at start does not fire"), Loaded->GetTriggerCount(), 0);

	WorldState->SetFlag(TEXT("test.cut"));
	for (ADCConditionalAudio* Actor : { Hum, Throw, Loaded, Bed })
	{
		Actor->Evaluate();
	}
	TestFalse(TEXT("Hum stops with the flag"), Hum->IsAudible());
	TestTrue(TEXT("Throw fires when the flag rises"), Throw->IsAudible());
	TestEqual(TEXT("Throw fired once"), Throw->GetTriggerCount(), 1);

	Throw->Evaluate();
	Throw->Evaluate();
	TestEqual(TEXT("Throw does not repeat while the flag stays set"), Throw->GetTriggerCount(), 1);

	// A save restore replaces flags without broadcasting; the periodic re-check follows it.
	WorldState->ReplaceFlags({});
	Hum->Evaluate();
	Throw->Evaluate();
	TestTrue(TEXT("Hum returns when a restore drops the flag"), Hum->IsAudible());
	TestFalse(TEXT("Throw re-arms"), Throw->IsAudible());

	// Presentation only: none of this wrote world state.
	TestFalse(TEXT("The actors never set test.cut"), WorldState->HasFlag(TEXT("test.cut")));
	TestFalse(TEXT("Nor invented any other flag"), WorldState->HasFlag(TEXT("test.already")));
	return true;
}

#endif
