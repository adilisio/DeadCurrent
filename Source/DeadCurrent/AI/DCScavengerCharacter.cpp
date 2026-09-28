#include "AI/DCScavengerCharacter.h"
#include "AI/DCScavengerController.h"
#include "Animation/AnimInstance.h"
#include "Combat/DCHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Inventory/DCInventoryComponent.h"
#include "UI/DCHUD.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "DCScavenger"

ADCScavengerCharacter::ADCScavengerCharacter()
{
	AIControllerClass = ADCScavengerController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	HealthComponent = CreateDefaultSubobject<UDCHealthComponent>(TEXT("Health"));
	InventoryComponent = CreateDefaultSubobject<UDCInventoryComponent>(TEXT("Inventory"));

	GetCapsuleComponent()->SetCapsuleSize(42.0f, 92.0f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"));
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
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 480.0f, 0.0f);
	Movement->MaxWalkSpeed = 320.0f;
	Movement->MaxWalkSpeedCrouched = 160.0f;
	Movement->bUseControllerDesiredRotation = true;

	DisplayName = LOCTEXT("DefaultName", "Scavenger");
}

void ADCScavengerCharacter::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDied.AddDynamic(this, &ADCScavengerCharacter::HandleDied);
}

void ADCScavengerCharacter::ApplyDamage_Implementation(const FDCDamageInfo& Damage)
{
	if (HealthComponent->IsDead())
	{
		return;
	}

	if (ADCScavengerController* AIC = Cast<ADCScavengerController>(GetController()))
	{
		AIC->NotifyDamagedBy(Damage.Instigator.Get());
	}

	if (APawn* Pawn = Cast<APawn>(Damage.Instigator.Get()))
	{
		const FText Message = FText::Format(
			LOCTEXT("HitHealth", "{0}  {1}"),
			DisplayName,
			FText::AsNumber(FMath::RoundToInt(HealthComponent->GetHealth())));
		ADCHUD::ShowMessageFor(Pawn, Message, 1.2f);
	}
}

bool ADCScavengerCharacter::TryMelee(AActor* Target)
{
	if (!Target || HealthComponent->IsDead())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetTimeSeconds() < NextMeleeTime)
	{
		return false;
	}

	if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) > MeleeRange)
	{
		return false;
	}

	NextMeleeTime = World->GetTimeSeconds() + MeleeCooldown;

	FDCDamageInfo Damage;
	Damage.Amount = MeleeDamage;
	Damage.DamageType = DCTags::Damage_Melee;
	Damage.Instigator = this;
	Damage.Causer = this;
	const float Applied = UDCHealthComponent::ApplyDamageToActor(Target, Damage);
	if (Applied > 0.0f)
	{
		ADCHUD::ShowMessageFor(Cast<APawn>(Target), LOCTEXT("HitBy", "The scavenger hits you."), 1.2f);
	}
	return Applied > 0.0f;
}

void ADCScavengerCharacter::HandleDied(UDCHealthComponent* Health, const FDCDamageInfo& Damage)
{
	if (APawn* Pawn = Cast<APawn>(Damage.Instigator.Get()))
	{
		ADCHUD::ShowMessageFor(Pawn, LOCTEXT("Down", "Scavenger down."), 2.0f);
	}
	Die();
}

void ADCScavengerCharacter::Die()
{
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesSimulatePhysics(true);

	DetachFromControllerPendingDestroy();
}

#undef LOCTEXT_NAMESPACE
