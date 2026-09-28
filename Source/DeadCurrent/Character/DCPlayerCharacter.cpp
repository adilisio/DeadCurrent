#include "Character/DCPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DeadCurrent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"

ADCPlayerCharacter::ADCPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Subobject names must stay stable: Blueprint subclasses store overrides against them.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 450.0f;
	Movement->MaxWalkSpeedCrouched = 200.0f;
	Movement->BrakingDecelerationFalling = 1500.0f;
	Movement->AirControl = 0.25f;
	Movement->GetNavAgentPropertiesRef().bCanCrouch = true;
	Movement->SetCrouchedHalfHeight(60.0f);
}

void ADCPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	BaseFirstPersonMeshLocation = FirstPersonMesh->GetRelativeLocation();
}

void ADCPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateSprint();
	UpdateCrouchEyeHeight(DeltaSeconds);
}

void ADCPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ADCPlayerCharacter::DoJumpEnd);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADCPlayerCharacter::MoveInput);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADCPlayerCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ADCPlayerCharacter::LookInput);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ADCPlayerCharacter::DoSprintEnd);

		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoCrouchToggle);
	}
	else
	{
		UE_LOG(LogDeadCurrent, Error, TEXT("'%s' Failed to find an Enhanced Input Component!"), *GetNameSafe(this));
	}
}

void ADCPlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ADCPlayerCharacter::LookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void ADCPlayerCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ADCPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ADCPlayerCharacter::DoJumpStart()
{
	Jump();
}

void ADCPlayerCharacter::DoJumpEnd()
{
	StopJumping();
}

void ADCPlayerCharacter::DoSprintStart()
{
	bSprintHeld = true;

	if (GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
	}
}

void ADCPlayerCharacter::DoSprintEnd()
{
	bSprintHeld = false;
}

void ADCPlayerCharacter::DoCrouchToggle()
{
	// Standing up under a low ceiling is deferred by the movement component until there is room.
	if (GetCharacterMovement()->bWantsToCrouch)
	{
		UnCrouch();
	}
	else
	{
		bSprintHeld = false;
		Crouch();
	}
}

void ADCPlayerCharacter::UpdateSprint()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();

	const float ForwardInput = FVector::DotProduct(GetLastMovementInputVector(), GetActorForwardVector());
	const bool bShouldSprint = bSprintHeld && !bIsCrouched && ForwardInput >= SprintMinForwardInput;

	if (bShouldSprint != bIsSprinting)
	{
		bIsSprinting = bShouldSprint;
		Movement->MaxWalkSpeed = bIsSprinting ? SprintSpeed : BaseWalkSpeed;
	}
}

void ADCPlayerCharacter::UpdateCrouchEyeHeight(float DeltaSeconds)
{
	// The camera follows the arms mesh's head bone, which does not animate into a crouch,
	// so the crouched view height is applied as an offset on the arms mesh.
	const float TargetOffset = bIsCrouched ? -CrouchEyeDrop : 0.0f;
	if (FMath::IsNearlyEqual(CurrentCrouchEyeOffset, TargetOffset, 0.01f))
	{
		return;
	}

	CurrentCrouchEyeOffset = FMath::FInterpTo(CurrentCrouchEyeOffset, TargetOffset, DeltaSeconds, CrouchEyeInterpSpeed);
	FirstPersonMesh->SetRelativeLocation(BaseFirstPersonMeshLocation + FVector(0.0f, 0.0f, CurrentCrouchEyeOffset));
}
