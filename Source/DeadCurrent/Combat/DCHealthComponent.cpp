#include "Combat/DCHealthComponent.h"
#include "GameFramework/Actor.h"

UDCHealthComponent::UDCHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDCHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bDead = false;
}

float UDCHealthComponent::ApplyDamage(const FDCDamageInfo& Damage)
{
	if (bDead || Damage.Amount <= 0.0f)
	{
		return 0.0f;
	}

	const float Applied = FMath::Min(CurrentHealth, Damage.Amount);
	CurrentHealth -= Applied;
	OnHealthChanged.Broadcast(this, CurrentHealth, -Applied);

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		bDead = true;
		OnDied.Broadcast(this, Damage);
	}

	return Applied;
}

float UDCHealthComponent::Heal(float Amount)
{
	if (bDead || Amount <= 0.0f)
	{
		return 0.0f;
	}

	const float Applied = FMath::Min(MaxHealth - CurrentHealth, Amount);
	CurrentHealth += Applied;
	if (Applied > 0.0f)
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, Applied);
	}
	return Applied;
}

void UDCHealthComponent::ResetHealth()
{
	const float Delta = MaxHealth - CurrentHealth;
	CurrentHealth = MaxHealth;
	bDead = false;
	OnHealthChanged.Broadcast(this, CurrentHealth, Delta);
}

void UDCHealthComponent::ApplyLoadedState(float NewHealth, bool bIsDead)
{
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	bDead = bIsDead;
	OnHealthChanged.Broadcast(this, CurrentHealth, 0.0f);
}

float UDCHealthComponent::ApplyDamageToActor(AActor* Target, const FDCDamageInfo& Damage)
{
	if (!Target || Damage.Amount <= 0.0f)
	{
		return 0.0f;
	}

	float Applied = 0.0f;
	if (UDCHealthComponent* Health = Target->FindComponentByClass<UDCHealthComponent>())
	{
		Applied = Health->ApplyDamage(Damage);
	}

	if (Target->Implements<UDCDamageable>())
	{
		IDCDamageable::Execute_ApplyDamage(Target, Damage);
		if (Applied <= 0.0f)
		{
			Applied = Damage.Amount;
		}
	}

	return Applied;
}
