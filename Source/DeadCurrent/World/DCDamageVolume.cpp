#include "World/DCDamageVolume.h"
#include "Combat/DCHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayRules.h"
#include "Core/DCGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "UI/DCHUD.h"

ADCDamageVolume::ADCDamageVolume()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCanEverAffectNavigation(false);
}

bool ADCDamageVolume::IsHazardActive() const
{
	return ActiveConditions.IsEmpty()
		|| UDCGameplayRules::CheckConditions(ActiveConditions, FDCRuleContext::ForActor(const_cast<ADCDamageVolume*>(this)));
}

void ADCDamageVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Checked every tick rather than on change events: a save restore replaces world flags silently.
	const bool bActive = IsHazardActive();
	if (bActive != bShownActive)
	{
		bShownActive = bActive;
		Mesh->SetVisibility(bActive);
	}

	if (!bActive || DamagePerSecond <= 0.0f)
	{
		return;
	}

	TArray<AActor*> Overlapping;
	Mesh->GetOverlappingActors(Overlapping, APawn::StaticClass());

	FDCDamageInfo Damage;
	Damage.Amount = DamagePerSecond * DeltaSeconds;
	Damage.DamageType = DCTags::Damage_Environmental;
	Damage.Instigator = this;
	Damage.Causer = this;

	for (AActor* Actor : Overlapping)
	{
		if (!Actor)
		{
			continue;
		}

		const UDCHealthComponent* Health = Actor->FindComponentByClass<UDCHealthComponent>();
		if (Health && Health->IsDead())
		{
			continue;
		}

		UDCHealthComponent::ApplyDamageToActor(Actor, Damage);

		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			if (!WarnedActors.Contains(Actor))
			{
				WarnedActors.Add(Actor);
				if (!DisplayName.IsEmpty())
				{
					ADCHUD::ShowMessageFor(Pawn, DisplayName, 2.0f);
				}
			}
		}
	}
}
