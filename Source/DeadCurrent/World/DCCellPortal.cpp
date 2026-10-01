#include "World/DCCellPortal.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/DCPlayerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCGameplayTags.h"
#include "DeadCurrent.h"
#include "EngineUtils.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UI/DCHUD.h"
#include "World/DCWorldStateSubsystem.h"

#define LOCTEXT_NAMESPACE "DCCellPortal"

ADCCellPortal::ADCCellPortal()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	DisplayName = LOCTEXT("DefaultName", "Door");
	LockedVerb = LOCTEXT("LockedVerb", "Try");
	LockedText = LOCTEXT("LockedText", "It won't open.");
}

bool ADCCellPortal::IsInstant() const
{
	return FadeOutSeconds <= 0.0f && HoldSeconds <= 0.0f && FadeInSeconds <= 0.0f;
}

int32 ADCCellPortal::FindVariant(AActor* Interactor) const
{
	const FDCRuleContext Context = FDCRuleContext::ForActor(Interactor);
	return Variants.IndexOfByPredicate([&Context](const FDCPortalVariant& Variant)
	{
		return UDCGameplayRules::CheckConditions(Variant.Conditions, Context);
	});
}

bool ADCCellPortal::IsAnyTransitionRunning(const UWorld* World)
{
	for (TActorIterator<ADCCellPortal> It(World); It; ++It)
	{
		if (It->IsTransitioning())
		{
			return true;
		}
	}
	return false;
}

bool ADCCellPortal::CanInteract_Implementation(AActor* Interactor) const
{
	// Locked portals stay usable: using one is how the player reads why it is locked. While any portal's transition
	// runs, none is: the way back out often stands right at the arrival point (VS-04).
	return !IsAnyTransitionRunning(GetWorld());
}

FDCInteractionPrompt ADCCellPortal::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	const int32 Index = FindVariant(Interactor);
	if (!Variants.IsValidIndex(Index))
	{
		return { LockedVerb, DisplayName };
	}

	const FDCPortalVariant& Variant = Variants[Index];
	FText Verb = Variant.Verb;
	const FString Label = UDCGameplayRules::FormatCheckLabels(Variant.Conditions);
	if (!Label.IsEmpty())
	{
		Verb = FText::FromString(Label + TEXT("  ") + Verb.ToString());
	}
	return { Verb, DisplayName };
}

FGameplayTag ADCCellPortal::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Use;
}

void ADCCellPortal::Interact_Implementation(AActor* Interactor)
{
	TryUse(Interactor);
}

EDCPortalUse ADCCellPortal::TryUse(AActor* Interactor)
{
	if (!Interactor || IsAnyTransitionRunning(GetWorld()))
	{
		return EDCPortalUse::Ignored;
	}

	const int32 Index = FindVariant(Interactor);
	if (!Variants.IsValidIndex(Index))
	{
		ADCHUD::ShowMessageFor(Interactor, LockedText, 4.0f);
		UE_LOG(LogDeadCurrent, Log, TEXT("[DCPORTAL] %s: locked for %s"), *GetName(), *Interactor->GetName());
		return EDCPortalUse::Locked;
	}

	if (!Destination)
	{
		UE_LOG(LogDeadCurrent, Error, TEXT("[DCPORTAL] %s has no Destination; nothing happens"), *GetName());
		return EDCPortalUse::Ignored;
	}

	UE_LOG(LogDeadCurrent, Log, TEXT("[DCPORTAL] %s: %s passes for %s (%s)"), *GetName(),
		*Variants[Index].VariantId.ToString(), *Interactor->GetName(), IsInstant() ? TEXT("instant") : TEXT("timed"));

	if (IsInstant())
	{
		Cut(Interactor, Index);
		return EDCPortalUse::Passed;
	}

	bTransitioning = true;
	PendingInteractor = Interactor;
	PendingVariant = Index;
	SetPlayerLocked(Interactor, true);
	Fade(Interactor, 0.0f, 1.0f, FadeOutSeconds, true);

	FTimerDelegate AtBlack = FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		AActor* Who = PendingInteractor.Get();
		if (Who)
		{
			Cut(Who, PendingVariant);
		}
		if (HoldSeconds > 0.0f)
		{
			GetWorldTimerManager().SetTimer(StepTimer, this, &ADCCellPortal::BeginFadeIn, HoldSeconds, false);
		}
		else
		{
			BeginFadeIn();
		}
	});
	if (FadeOutSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(StepTimer, AtBlack, FadeOutSeconds, false);
	}
	else
	{
		AtBlack.Execute();
	}
	return EDCPortalUse::Passed;
}

void ADCCellPortal::Cut(AActor* Interactor, int32 VariantIndex)
{
	if (!Interactor || !Variants.IsValidIndex(VariantIndex))
	{
		return;
	}

	if (const ADCPlayerCharacter* Player = Cast<ADCPlayerCharacter>(Interactor))
	{
		if (UDCDialogueComponent* Dialogue = Player->GetDialogueComponent())
		{
			Dialogue->EndDialogue();
		}
	}

	UDCGameplayRules::ApplyConsequences(Variants[VariantIndex].Consequences, FDCRuleContext::ForActor(Interactor));

	if (Destination)
	{
		const FVector Arrival = Destination->GetActorLocation();
		const float Yaw = Destination->GetActorRotation().Yaw;
		if (bUseDestinationYaw)
		{
			Interactor->SetActorLocationAndRotation(Arrival, FRotator(0.0f, Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
		}
		else
		{
			Interactor->SetActorLocation(Arrival, false, nullptr, ETeleportType::TeleportPhysics);
		}

		if (const ACharacter* Character = Cast<ACharacter>(Interactor))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->StopMovementImmediately();
			}
		}
		if (const APawn* Pawn = Cast<APawn>(Interactor); Pawn && bUseDestinationYaw)
		{
			if (AController* Controller = Pawn->GetController())
			{
				Controller->SetControlRotation(FRotator(0.0f, Yaw, 0.0f));
			}
		}
	}

	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		WorldState->NotifySceneCut();
	}

	if (!CardText.IsEmpty())
	{
		ADCHUD::ShowMessageFor(Interactor, CardText, CardSeconds);
	}
}

void ADCCellPortal::BeginFadeIn()
{
	AActor* Who = PendingInteractor.Get();
	Fade(Who, 1.0f, 0.0f, FadeInSeconds, false);
	if (FadeInSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(StepTimer, this, &ADCCellPortal::FinishTransition, FadeInSeconds, false);
	}
	else
	{
		FinishTransition();
	}
}

void ADCCellPortal::FinishTransition()
{
	SetPlayerLocked(PendingInteractor.Get(), false);
	PendingInteractor.Reset();
	PendingVariant = INDEX_NONE;
	bTransitioning = false;
}

void ADCCellPortal::SetPlayerLocked(AActor* Interactor, bool bLocked) const
{
	const APawn* Pawn = Cast<APawn>(Interactor);
	if (APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr)
	{
		PC->SetIgnoreMoveInput(bLocked);
		PC->SetIgnoreLookInput(bLocked);
	}
}

void ADCCellPortal::Fade(AActor* Interactor, float From, float To, float Seconds, bool bHoldWhenFinished) const
{
	const APawn* Pawn = Cast<APawn>(Interactor);
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->StartCameraFade(From, To, FMath::Max(Seconds, 0.01f), FLinearColor::Black, false, bHoldWhenFinished);
	}
}

void ADCCellPortal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bTransitioning)
	{
		// The level is going away mid-fade (a load, quitting): give input back and drop the pending step.
		GetWorldTimerManager().ClearTimer(StepTimer);
		SetPlayerLocked(PendingInteractor.Get(), false);
		bTransitioning = false;
	}
	Super::EndPlay(EndPlayReason);
}

#undef LOCTEXT_NAMESPACE
