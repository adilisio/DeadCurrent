#include "Save/DCPersistentRegistry.h"
#include "DeadCurrent.h"
#include "GameFramework/Actor.h"

bool UDCPersistentRegistry::RegisterActor(AActor* Actor, FName Id)
{
	if (!Actor || Id.IsNone())
	{
		return false;
	}

	if (TWeakObjectPtr<AActor>* Existing = ById.Find(Id))
	{
		if (Existing->IsValid() && Existing->Get() != Actor)
		{
			UE_LOG(LogDeadCurrent, Warning, TEXT("[DCPERSIST] Duplicate id '%s' on %s (already %s)"),
				*Id.ToString(), *GetNameSafe(Actor), *GetNameSafe(Existing->Get()));
			return false;
		}
	}

	ById.Add(Id, Actor);
	return true;
}

void UDCPersistentRegistry::UnregisterActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	for (auto It = ById.CreateIterator(); It; ++It)
	{
		if (It.Value().Get() == Actor)
		{
			It.RemoveCurrent();
			return;
		}
	}
}

AActor* UDCPersistentRegistry::FindActor(FName Id) const
{
	if (const TWeakObjectPtr<AActor>* Found = ById.Find(Id))
	{
		return Found->Get();
	}
	return nullptr;
}

TArray<FName> UDCPersistentRegistry::GetRegisteredIds() const
{
	TArray<FName> Ids;
	ById.GetKeys(Ids);
	return Ids;
}
