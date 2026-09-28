#include "Character/DCPlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "Combat/DCFirearm.h"
#include "Combat/DCHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DeadCurrent.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Quest/DCQuestComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/DCInteractorComponent.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "TimerManager.h"
#include "UI/DCHUD.h"

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

	InteractorComponent = CreateDefaultSubobject<UDCInteractorComponent>(TEXT("Interactor"));
	InventoryComponent = CreateDefaultSubobject<UDCInventoryComponent>(TEXT("Inventory"));
	HealthComponent = CreateDefaultSubobject<UDCHealthComponent>(TEXT("Health"));
	StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	DialogueComponent = CreateDefaultSubobject<UDCDialogueComponent>(TEXT("Dialogue"));
	QuestComponent = CreateDefaultSubobject<UDCQuestComponent>(TEXT("Quest"));

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

	InventoryComponent->OnInventoryChanged.AddDynamic(this, &ADCPlayerCharacter::HandleInventoryChanged);
	HealthComponent->OnDied.AddDynamic(this, &ADCPlayerCharacter::HandleDied);
	QuestComponent->OnQuestUpdated.AddDynamic(this, &ADCPlayerCharacter::HandleQuestUpdated);

	if (StimuliSource)
	{
		StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
	}
}

void ADCPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateSprint();
	UpdateCrouchEyeHeight(DeltaSeconds);
	UpdateRecoilRecovery(DeltaSeconds);
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

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoInteract);

		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoToggleInventory);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoFire);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoReload);
		EnhancedInputComponent->BindAction(EquipWeaponAction, ETriggerEvent::Started, this, &ADCPlayerCharacter::DoToggleWeapon);
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

void ADCPlayerCharacter::DoInteract()
{
	if (DialogueComponent && DialogueComponent->IsInDialogue())
	{
		return;
	}
	InteractorComponent->TryInteract();
}

void ADCPlayerCharacter::DoToggleInventory()
{
	if (DialogueComponent && DialogueComponent->IsInDialogue())
	{
		return;
	}

	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (ADCHUD* HUD = PC ? PC->GetHUD<ADCHUD>() : nullptr)
	{
		HUD->ToggleInventory();
	}
}

void ADCPlayerCharacter::DoFire()
{
	if (DialogueComponent && DialogueComponent->IsInDialogue())
	{
		return;
	}

	if (HealthComponent->IsDead() || !EquippedFirearm || EquippedFirearm->IsHolstered())
	{
		return;
	}

	if (!EquippedFirearm->Fire())
	{
		return;
	}

	const UDCItemDefinition* Def = EquippedFirearm->GetDefinition();
	if (!Def)
	{
		return;
	}

	const float Yaw = FMath::FRandRange(-Def->RecoilYawVariance, Def->RecoilYawVariance);
	AddControllerPitchInput(-Def->RecoilPitch);
	AddControllerYawInput(Yaw);
	RecoilToRecover += Def->RecoilPitch;
	RecoilRecoverySpeed = Def->RecoilRecoverySpeed;
}

void ADCPlayerCharacter::DoReload()
{
	if (!EquippedFirearm || EquippedFirearm->IsHolstered())
	{
		return;
	}

	if (EquippedFirearm->StartReload())
	{
		return;
	}

	if (EquippedFirearm->GetReserveAmmo() <= 0
		&& EquippedFirearm->GetRoundsInMagazine() < EquippedFirearm->GetMagazineSize())
	{
		ADCHUD::ShowMessageFor(this, NSLOCTEXT("DCPlayerCharacter", "NoAmmo", "No ammo"), 1.2f);
	}
}

void ADCPlayerCharacter::DoToggleWeapon()
{
	if (EquippedFirearm)
	{
		EquippedFirearm->SetHolstered(!EquippedFirearm->IsHolstered());
		return;
	}

	if (const UDCItemDefinition* Def = FindFirearmInInventory())
	{
		SpawnAndEquip(Def);
	}
}

void ADCPlayerCharacter::HandleInventoryChanged(UDCInventoryComponent* Inventory)
{
	if (!Inventory)
	{
		return;
	}

	if (!EquippedFirearm)
	{
		if (const UDCItemDefinition* Def = FindFirearmInInventory())
		{
			SpawnAndEquip(Def, !bRestoringSave);
		}
		return;
	}

	if (!bRestoringSave
		&& !EquippedFirearm->IsHolstered()
		&& EquippedFirearm->GetRoundsInMagazine() == 0
		&& EquippedFirearm->CanReload())
	{
		EquippedFirearm->StartReload();
	}
}

void ADCPlayerCharacter::HandleDied(UDCHealthComponent* Health, const FDCDamageInfo& Damage)
{
	DisableInput(Cast<APlayerController>(GetController()));
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	ADCHUD::ShowMessageFor(this, NSLOCTEXT("DCPlayerCharacter", "Died", "You died."), 4.0f);

	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ADCPlayerCharacter::Respawn, 4.0f, false);
}

void ADCPlayerCharacter::HandleQuestUpdated(FName QuestId, FName StageId)
{
	if (bRestoringSave)
	{
		return;
	}

	const FText Objective = QuestComponent->GetObjectiveText();
	if (!Objective.IsEmpty())
	{
		ADCHUD::ShowMessageFor(this, Objective, 4.0f);
		return;
	}

	if (QuestComponent->IsComplete(QuestId))
	{
		ADCHUD::ShowMessageFor(this, NSLOCTEXT("DCPlayerCharacter", "QuestDone", "Quest complete."), 3.0f);
	}

	(void)StageId;
}

void ADCPlayerCharacter::Respawn()
{
	HealthComponent->ResetHealth();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	EnableInput(Cast<APlayerController>(GetController()));

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		SetActorLocationAndRotation(It->GetActorLocation(), It->GetActorRotation());
		if (AController* PC = GetController())
		{
			PC->SetControlRotation(It->GetActorRotation());
		}
		break;
	}
}

const UDCItemDefinition* ADCPlayerCharacter::FindFirearmInInventory() const
{
	for (const FDCItemStack& Stack : InventoryComponent->GetStacks())
	{
		if (Stack.Item && Stack.Item->IsFirearm())
		{
			return Stack.Item;
		}
	}
	return nullptr;
}

void ADCPlayerCharacter::ClearEquippedFirearm()
{
	if (EquippedFirearm)
	{
		EquippedFirearm->Destroy();
		EquippedFirearm = nullptr;
	}
}

void ADCPlayerCharacter::SpawnAndEquip(const UDCItemDefinition* Definition, bool bAutoReload)
{
	if (!Definition || !GetWorld())
	{
		return;
	}

	ClearEquippedFirearm();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedFirearm = GetWorld()->SpawnActor<ADCFirearm>(Params);
	if (!EquippedFirearm)
	{
		return;
	}

	EquippedFirearm->AttachToComponent(FirstPersonCameraComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	EquippedFirearm->SetDefinition(Definition);
	EquippedFirearm->SetHolstered(false);
	if (bAutoReload)
	{
		EquippedFirearm->StartReload();
	}
}

void ADCPlayerCharacter::UpdateRecoilRecovery(float DeltaSeconds)
{
	if (RecoilToRecover <= KINDA_SMALL_NUMBER)
	{
		RecoilToRecover = 0.0f;
		return;
	}

	const float Step = FMath::Min(RecoilToRecover, RecoilRecoverySpeed * DeltaSeconds);
	AddControllerPitchInput(Step);
	RecoilToRecover -= Step;
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
