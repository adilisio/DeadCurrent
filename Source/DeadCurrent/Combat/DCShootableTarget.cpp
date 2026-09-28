#include "Combat/DCShootableTarget.h"
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
}

void ADCShootableTarget::BeginPlay()
{
	Super::BeginPlay();

	BaseScale = Mesh->GetRelativeScale3D();
}

void ADCShootableTarget::ApplyDamage_Implementation(const FDCDamageInfo& Damage)
{
	HitCount++;

	Mesh->SetRelativeScale3D(BaseScale * 1.1f);
	GetWorldTimerManager().SetTimer(FlashTimer, this, &ADCShootableTarget::EndFlash, FlashDuration, false);

	if (APawn* Pawn = Cast<APawn>(Damage.Instigator.Get()))
	{
		const FText Message = FText::Format(LOCTEXT("Hit", "Hit {0} ({1})"), DisplayName, HitCount);
		ADCHUD::ShowMessageFor(Pawn, Message, 1.5f);
	}
}

void ADCShootableTarget::EndFlash()
{
	Mesh->SetRelativeScale3D(BaseScale);
}

#undef LOCTEXT_NAMESPACE
