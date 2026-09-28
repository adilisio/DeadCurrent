#include "Save/DCPersistentIdComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCPersistentIdTest, "DeadCurrent.Save.PersistentId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCPersistentIdTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Missing actor has no id"), UDCPersistentIdComponent::GetIdOnActor(nullptr).IsNone());

	AActor* Actor = NewObject<AActor>();
	UDCPersistentIdComponent* Comp = NewObject<UDCPersistentIdComponent>(Actor, TEXT("PersistentId"));
	TestTrue(TEXT("Unset id is none"), Comp->GetPersistentId().IsNone());

	Comp->SetPersistentId(TEXT("gym.scavenger"));
	TestEqual(TEXT("Component stores id"), Comp->GetPersistentId(), FName(TEXT("gym.scavenger")));
	TestEqual(TEXT("Lookup from actor"), UDCPersistentIdComponent::GetIdOnActor(Actor), FName(TEXT("gym.scavenger")));

	UDCPersistentRegistry* Registry = NewObject<UDCPersistentRegistry>();
	TestFalse(TEXT("Empty id is rejected"), Registry->RegisterActor(Actor, NAME_None));
	TestTrue(TEXT("First register succeeds"), Registry->RegisterActor(Actor, TEXT("gym.scavenger")));
	TestEqual(TEXT("Find returns actor"), Registry->FindActor(TEXT("gym.scavenger")), Actor);

	AActor* Other = NewObject<AActor>();
	TestFalse(TEXT("Duplicate id is rejected"), Registry->RegisterActor(Other, TEXT("gym.scavenger")));
	TestEqual(TEXT("Original kept"), Registry->FindActor(TEXT("gym.scavenger")), Actor);

	Registry->UnregisterActor(Actor);
	TestNull(TEXT("Unregistered is gone"), Registry->FindActor(TEXT("gym.scavenger")));
	TestTrue(TEXT("Id can be reused"), Registry->RegisterActor(Other, TEXT("gym.scavenger")));

	return true;
}

#endif
