#include "World/DCLocationVolume.h"
#include "Components/BoxComponent.h"
#include "DeadCurrent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "UI/DCHUD.h"
#include "World/DCWorldStateSubsystem.h"

#define LOCTEXT_NAMESPACE "DCLocationVolume"

ADCLocationVolume::ADCLocationVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Bounds->SetBoxExtent(FVector(500.0f, 500.0f, 300.0f));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetGenerateOverlapEvents(false);
	Bounds->SetCanEverAffectNavigation(false);
	Bounds->ShapeColor = FColor(80, 200, 255);
	Bounds->SetHiddenInGame(true);
	SetRootComponent(Bounds);
}

void ADCLocationVolume::BeginPlay()
{
	Super::BeginPlay();

	if (LocationId.IsNone())
	{
		UE_LOG(LogDeadCurrent, Warning, TEXT("[DCLOCATION] %s has no LocationId and will never be discovered"), *GetName());
		SetActorTickEnabled(false);
	}
}

void ADCLocationVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Discovered earlier, or restored from a save: nothing left to do.
	if (IsDiscovered())
	{
		SetActorTickEnabled(false);
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APawn* Pawn = It->IsValid() ? (*It)->GetPawn() : nullptr;
		if (Pawn && ContainsPoint(Pawn->GetActorLocation()) && TryDiscover(Pawn))
		{
			SetActorTickEnabled(false);
			return;
		}
	}
}

bool ADCLocationVolume::TryDiscover(AActor* Discoverer)
{
	UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this);
	if (!WorldState || !WorldState->DiscoverLocation(LocationId))
	{
		return false;
	}

	ADCHUD::ShowBannerFor(Discoverer, LOCTEXT("Discovered", "LOCATION DISCOVERED"), GetDisplayName(), BannerDuration);
	return true;
}

bool ADCLocationVolume::ContainsPoint(const FVector& WorldLocation) const
{
	const FVector Local = Bounds->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = Bounds->GetUnscaledBoxExtent();
	return FMath::Abs(Local.X) <= Extent.X && FMath::Abs(Local.Y) <= Extent.Y && FMath::Abs(Local.Z) <= Extent.Z;
}

bool ADCLocationVolume::IsDiscovered() const
{
	const UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this);
	return WorldState && WorldState->IsLocationDiscovered(LocationId);
}

FText ADCLocationVolume::GetDisplayName() const
{
	return DisplayName.IsEmpty() ? FText::FromName(LocationId) : DisplayName;
}

FText ADCLocationVolume::FindDisplayName(const UWorld* World, FName LocationId)
{
	if (World)
	{
		for (TActorIterator<ADCLocationVolume> It(const_cast<UWorld*>(World)); It; ++It)
		{
			if (It->GetLocationId() == LocationId)
			{
				return It->GetDisplayName();
			}
		}
	}
	return FText::FromName(LocationId);
}

#undef LOCTEXT_NAMESPACE
