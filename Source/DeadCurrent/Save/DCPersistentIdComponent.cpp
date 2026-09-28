#include "Save/DCPersistentIdComponent.h"
#include "DeadCurrent.h"
#include "DrawDebugHelpers.h"
#include "Save/DCPersistentRegistry.h"

UDCPersistentIdComponent::UDCPersistentIdComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UDCPersistentIdComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UDCPersistentRegistry* Registry = World->GetSubsystem<UDCPersistentRegistry>())
		{
			if (!Registry->RegisterActor(GetOwner(), PersistentId))
			{
				UE_LOG(LogDeadCurrent, Warning, TEXT("[DCPERSIST] %s failed to register id '%s'"),
					*GetNameSafe(GetOwner()), *PersistentId.ToString());
			}
		}
	}
}

void UDCPersistentIdComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (UDCPersistentRegistry* Registry = World->GetSubsystem<UDCPersistentRegistry>())
		{
			Registry->UnregisterActor(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UDCPersistentIdComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bDrawId || PersistentId.IsNone() || !GetOwner())
	{
		return;
	}

	const FVector Above = GetOwner()->GetActorLocation() + FVector(0.0f, 0.0f, 145.0f);
	DrawDebugString(GetWorld(), Above, PersistentId.ToString(), nullptr, FColor::Cyan, 0.0f, true);
}

FName UDCPersistentIdComponent::GetIdOnActor(const AActor* Actor)
{
	const UDCPersistentIdComponent* Comp = Actor
		? Actor->FindComponentByClass<UDCPersistentIdComponent>()
		: nullptr;
	return Comp ? Comp->GetPersistentId() : NAME_None;
}
