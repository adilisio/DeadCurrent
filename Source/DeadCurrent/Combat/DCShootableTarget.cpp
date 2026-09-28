#include "Combat/DCShootableTarget.h"
#include "Combat/DCHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "TimerManager.h"
#include "UI/DCHUD.h"

#define LOCTEXT_NAMESPACE "DCShootableTarget"

ADCShootableTarget::ADCShootableTarget()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	SetRootComponent(Mesh);

	HealthComponent = CreateDefaultSubobject<UDCHealthComponent>(TEXT("Health"));
}

void ADCShootableTarget::BeginPlay()
{
	Super::BeginPlay();

	BaseScale = Mesh->GetRelativeScale3D();
	HealthComponent->OnDied.AddDynamic(this, &ADCShootableTarget::HandleDied);
}

void ADCShootableTarget::ApplyDamage_Implementation(const FDCDamageInfo& Damage)
{
	if (HealthComponent->IsDead())
	{
		return;
	}

	Mesh->SetRelativeScale3D(BaseScale * 1.1f);
	GetWorldTimerManager().SetTimer(FlashTimer, this, &ADCShootableTarget::EndFlash, FlashDuration, false);

	if (APawn* Pawn = Cast<APawn>(Damage.Instigator.Get()))
	{
		const FText Message = FText::Format(
			LOCTEXT("HitHealth", "{0}  {1}"),
			DisplayName,
			FText::AsNumber(FMath::RoundToInt(HealthComponent->GetHealth())));
		ADCHUD::ShowMessageFor(Pawn, Message, 1.5f);
	}
}

void ADCShootableTarget::HandleDied(UDCHealthComponent* Health, const FDCDamageInfo& Damage)
{
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(BaseScale);

	FRotator Fallen = GetActorRotation();
	Fallen.Pitch += 80.0f;
	SetActorRotation(Fallen);

	if (APawn* Pawn = Cast<APawn>(Damage.Instigator.Get()))
	{
		ADCHUD::ShowMessageFor(Pawn, FText::Format(LOCTEXT("Destroyed", "{0} down"), DisplayName), 2.0f);
	}
}

void ADCShootableTarget::EndFlash()
{
	if (!HealthComponent->IsDead())
	{
		Mesh->SetRelativeScale3D(BaseScale);
	}
}

#undef LOCTEXT_NAMESPACE
