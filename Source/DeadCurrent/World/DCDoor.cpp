#include "World/DCDoor.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "Save/DCPersistentIdComponent.h"

#define LOCTEXT_NAMESPACE "DCDoor"

ADCDoor::ADCDoor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	SetRootComponent(Hinge);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Hinge);
	DoorMesh->SetMobility(EComponentMobility::Movable);

	PersistentIdComponent = CreateDefaultSubobject<UDCPersistentIdComponent>(TEXT("PersistentId"));

	DisplayName = LOCTEXT("DefaultName", "Door");
}

void ADCDoor::BeginPlay()
{
	Super::BeginPlay();

	ClosedYaw = DoorMesh->GetRelativeRotation().Yaw;
	CurrentYaw = ClosedYaw;
	TargetYaw = ClosedYaw;
}

void ADCDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CurrentYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaSeconds, SwingSpeed);

	FRotator Rotation = DoorMesh->GetRelativeRotation();
	Rotation.Yaw = CurrentYaw;
	DoorMesh->SetRelativeRotation(Rotation);

	if (FMath::IsNearlyEqual(CurrentYaw, TargetYaw))
	{
		SetActorTickEnabled(false);
	}
}

bool ADCDoor::CanInteract_Implementation(AActor* Interactor) const
{
	return true;
}

FDCInteractionPrompt ADCDoor::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return { bIsOpen ? LOCTEXT("CloseAction", "Close") : LOCTEXT("OpenAction", "Open"), DisplayName };
}

FGameplayTag ADCDoor::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Use;
}

void ADCDoor::Interact_Implementation(AActor* Interactor)
{
	bIsOpen = !bIsOpen;

	if (bIsOpen)
	{
		// The leaf extends along +Y, so positive yaw swings it toward -X. Swing toward the side the interactor is not on.
		const FVector LocalInteractor = GetActorTransform().InverseTransformPosition(Interactor ? Interactor->GetActorLocation() : GetActorLocation());
		TargetYaw = ClosedYaw + (LocalInteractor.X < 0.0f ? -OpenAngle : OpenAngle);
	}
	else
	{
		TargetYaw = ClosedYaw;
	}

	SetActorTickEnabled(true);
}

FName ADCDoor::GetPersistentId_Implementation() const
{
	return UDCPersistentIdComponent::GetIdOnActor(this);
}

void ADCDoor::CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const
{
	OutState.PersistentId = UDCPersistentIdComponent::GetIdOnActor(this);
	OutState.bExists = true;
	OutState.bAlive = true;
}

void ADCDoor::ApplyPersistentState_Implementation(const FDCPersistentActorState& State)
{
	(void)State;
}

#undef LOCTEXT_NAMESPACE
