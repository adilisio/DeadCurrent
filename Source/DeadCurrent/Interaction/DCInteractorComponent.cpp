#include "Interaction/DCInteractorComponent.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UDCInteractorComponent::UDCInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UDCInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetFocus(FindInteractable());
}

bool UDCInteractorComponent::TryInteract()
{
	AActor* Target = FocusedActor.Get();
	if (!Target || !IDCInteractable::Execute_CanInteract(Target, GetOwner()))
	{
		return false;
	}

	IDCInteractable::Execute_Interact(Target, GetOwner());
	return true;
}

bool UDCInteractorComponent::GetFocusedPrompt(FDCInteractionPrompt& OutPrompt) const
{
	AActor* Target = FocusedActor.Get();
	if (!Target)
	{
		return false;
	}

	OutPrompt = IDCInteractable::Execute_GetInteractionPrompt(Target, GetOwner());
	return true;
}

AActor* UDCInteractorComponent::FindInteractable() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC)
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * InteractionRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(DCInteractionTrace), false, Pawn);

	FHitResult Hit;
	if (!GetWorld()->SweepSingleByChannel(Hit, ViewLocation, TraceEnd, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(TraceRadius), Params))
	{
		return nullptr;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->Implements<UDCInteractable>())
	{
		return nullptr;
	}

	return IDCInteractable::Execute_CanInteract(HitActor, GetOwner()) ? HitActor : nullptr;
}

void UDCInteractorComponent::SetFocus(AActor* NewFocus)
{
	AActor* OldFocus = FocusedActor.Get();
	if (NewFocus == OldFocus)
	{
		return;
	}

	FocusedActor = NewFocus;
	OnFocusChanged.Broadcast(NewFocus, OldFocus);
}
