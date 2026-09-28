#include "AI/DCFriendlyNPC.h"
#include "Animation/AnimInstance.h"
#include "Character/DCPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "Dialogue/DCDialogueAsset.h"
#include "Dialogue/DCDialogueComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Save/DCPersistentIdComponent.h"
#include "UI/DCHUD.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "DCFriendlyNPC"

ADCFriendlyNPC::ADCFriendlyNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;

	PersistentIdComponent = CreateDefaultSubobject<UDCPersistentIdComponent>(TEXT("PersistentId"));

	GetCapsuleComponent()->SetCapsuleSize(42.0f, 92.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
	if (MeshFinder.Succeeded())
	{
		GetMesh()->SetSkeletalMeshAsset(MeshFinder.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimFinder(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (AnimFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimFinder.Class);
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = false;
	Movement->bUseControllerDesiredRotation = false;
	Movement->MaxWalkSpeed = 0.0f;

	DisplayName = LOCTEXT("DefaultName", "Mara");
	Greeting = LOCTEXT("DefaultGreeting",
		"Keep your voice down. There's a scavenger past those plates.");
	Dialogue = TSoftObjectPtr<UDCDialogueAsset>(
		FSoftObjectPath(TEXT("/Game/Dialogue/DA_Dialogue_MaraIntro.DA_Dialogue_MaraIntro")));
}

void ADCFriendlyNPC::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (FVector::Dist2D(GetActorLocation(), Player->GetActorLocation()) <= NoticeRange)
		{
			FaceActor(Player, DeltaSeconds);
		}
	}
}

void ADCFriendlyNPC::FaceActor(const AActor* Target, float DeltaSeconds)
{
	if (!Target)
	{
		return;
	}

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.0f;
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	FRotator TargetRotation = ToTarget.ToOrientationRotator();
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;

	FRotator Current = GetActorRotation();
	Current.Pitch = 0.0f;
	Current.Roll = 0.0f;
	SetActorRotation(FMath::RInterpTo(Current, TargetRotation, DeltaSeconds, TurnSpeed));
}

bool ADCFriendlyNPC::CanInteract_Implementation(AActor* Interactor) const
{
	if (!Interactor)
	{
		return false;
	}

	if (const ADCPlayerCharacter* Character = Cast<ADCPlayerCharacter>(Interactor))
	{
		if (Character->GetDialogueComponent() && Character->GetDialogueComponent()->IsInDialogue())
		{
			return false;
		}
	}

	return Dialogue.ToSoftObjectPath().IsValid() || !Greeting.IsEmpty();
}

FDCInteractionPrompt ADCFriendlyNPC::GetInteractionPrompt_Implementation(AActor* Interactor) const
{
	return { LOCTEXT("TalkAction", "Talk"), DisplayName };
}

FGameplayTag ADCFriendlyNPC::GetInteractionType_Implementation() const
{
	return DCTags::Interaction_Talk;
}

void ADCFriendlyNPC::Interact_Implementation(AActor* Interactor)
{
	FaceActor(Interactor, 1.0f);

	UDCDialogueComponent* DialogueComp = Interactor
		? Interactor->FindComponentByClass<UDCDialogueComponent>()
		: nullptr;
	if (DialogueComp)
	{
		if (const UDCDialogueAsset* Asset = Dialogue.LoadSynchronous())
		{
			if (DialogueComp->StartDialogue(Asset, this))
			{
				return;
			}
		}
	}

	if (!Greeting.IsEmpty())
	{
		ADCHUD::ShowMessageFor(Interactor, Greeting, GreetingDuration);
	}
}

FName ADCFriendlyNPC::GetPersistentId_Implementation() const
{
	return UDCPersistentIdComponent::GetIdOnActor(this);
}

void ADCFriendlyNPC::CapturePersistentState_Implementation(FDCPersistentActorState& OutState) const
{
	OutState.PersistentId = UDCPersistentIdComponent::GetIdOnActor(this);
	OutState.bExists = true;
	OutState.bAlive = true;
}

void ADCFriendlyNPC::ApplyPersistentState_Implementation(const FDCPersistentActorState& State)
{
	(void)State;
}

#undef LOCTEXT_NAMESPACE
