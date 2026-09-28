#include "AI/DCFriendlyNPC.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/DCHUD.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "DCFriendlyNPC"

ADCFriendlyNPC::ADCFriendlyNPC()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;

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
		"Keep your voice down. There's a scavenger past those plates. Come talk when you want a real conversation.");
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
	return Interactor != nullptr && !Greeting.IsEmpty();
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
	ADCHUD::ShowMessageFor(Interactor, Greeting, GreetingDuration);
}

#undef LOCTEXT_NAMESPACE
