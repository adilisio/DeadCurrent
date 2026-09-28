#include "Core/DCContentSubsystem.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "DeadCurrent.h"
#include "Items/DCItemDefinition.h"
#include "Quest/DCQuestDefinition.h"

const TArray<FString>& UDCContentSubsystem::GetContentPaths()
{
	static const TArray<FString> Paths = { TEXT("/Game/Items"), TEXT("/Game/Quests") };
	return Paths;
}

void UDCContentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	TArray<UObject*> Definitions;
	LoadAllDefinitions(Definitions);
	LoadedDefinitions.Append(Definitions);
	UE_LOG(LogDeadCurrent, Log, TEXT("[DCCONTENT] loaded %d item and quest definitions"), LoadedDefinitions.Num());
}

void UDCContentSubsystem::LoadAllDefinitions(TArray<UObject*>& OutDefinitions)
{
	IAssetRegistry& Registry = IAssetRegistry::GetChecked();
#if WITH_EDITOR
	// Uncooked runs (editor, -game from the editor build) may still be scanning in the background.
	Registry.ScanPathsSynchronous(GetContentPaths(), false);
#endif

	FARFilter Filter;
	for (const FString& Path : GetContentPaths())
	{
		Filter.PackagePaths.Add(FName(*Path));
	}
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UDCItemDefinition::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UDCQuestDefinition::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> Assets;
	Registry.GetAssets(Filter, Assets);
	for (const FAssetData& Asset : Assets)
	{
		if (UObject* Loaded = Asset.GetAsset())
		{
			OutDefinitions.Add(Loaded);
		}
	}
}
