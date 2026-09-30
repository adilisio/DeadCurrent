#include "World/DCWorldStateSubsystem.h"
#include "DeadCurrent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UDCWorldStateSubsystem* UDCWorldStateSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull)
		: nullptr;
	return World ? World->GetSubsystem<UDCWorldStateSubsystem>() : nullptr;
}

void UDCWorldStateSubsystem::SetFlag(FName Flag)
{
	if (Flag.IsNone() || Flags.Contains(Flag))
	{
		return;
	}

	Flags.Add(Flag);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCWORLD] flag set %s"), *Flag.ToString());
	NotifyChanged();
}

void UDCWorldStateSubsystem::ClearFlag(FName Flag)
{
	if (Flags.Remove(Flag) > 0)
	{
		UE_LOG(LogDeadCurrent, Log, TEXT("[DCWORLD] flag cleared %s"), *Flag.ToString());
		NotifyChanged();
	}
}

void UDCWorldStateSubsystem::ReplaceFlags(const TArray<FName>& SavedFlags)
{
	Flags.Reset();
	for (const FName Flag : SavedFlags)
	{
		if (!Flag.IsNone())
		{
			Flags.AddUnique(Flag);
		}
	}
}

bool UDCWorldStateSubsystem::DiscoverLocation(FName LocationId)
{
	if (LocationId.IsNone() || DiscoveredLocations.Contains(LocationId))
	{
		return false;
	}

	DiscoveredLocations.Add(LocationId);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCWORLD] location discovered %s"), *LocationId.ToString());
	OnLocationDiscovered.Broadcast(LocationId);
	NotifyChanged();
	return true;
}

void UDCWorldStateSubsystem::ReplaceDiscoveredLocations(const TArray<FName>& SavedLocations)
{
	DiscoveredLocations.Reset();
	for (const FName LocationId : SavedLocations)
	{
		if (!LocationId.IsNone())
		{
			DiscoveredLocations.AddUnique(LocationId);
		}
	}
}

void UDCWorldStateSubsystem::NotifyChanged()
{
	OnChanged.Broadcast();
}

void UDCWorldStateSubsystem::NotifyChanged(const UObject* WorldContextObject)
{
	if (UDCWorldStateSubsystem* WorldState = Get(WorldContextObject))
	{
		WorldState->NotifyChanged();
	}
}

void UDCWorldStateSubsystem::NotifyRestored()
{
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCWORLD] state restored"));
	OnRestored.Broadcast();
}
