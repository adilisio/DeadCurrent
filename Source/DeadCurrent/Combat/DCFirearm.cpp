#include "Combat/DCFirearm.h"
#include "Combat/DCDamageable.h"
#include "Combat/DCHealthComponent.h"
#include "Combat/DCImpactMark.h"
#include "CollisionQueryParams.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayTags.h"
#include "Engine/HitResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "UI/DCHUD.h"

#define LOCTEXT_NAMESPACE "DCFirearm"

ADCFirearm::ADCFirearm()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCastShadow(false);
	Mesh->SetOnlyOwnerSee(true);
	Mesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;

	MuzzleLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleLight"));
	MuzzleLight->SetupAttachment(Mesh);
	MuzzleLight->SetVisibility(false);
	MuzzleLight->SetIntensity(8000.0f);
	MuzzleLight->SetAttenuationRadius(120.0f);
	MuzzleLight->SetCastShadows(false);
	MuzzleLight->SetLightColor(FLinearColor(1.0f, 0.82f, 0.45f));
}

void ADCFirearm::SetDefinition(const UDCItemDefinition* NewDefinition)
{
	Definition = NewDefinition;
	RoundsInMagazine = 0;
	bReloading = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimer);
	}

	if (Definition)
	{
		UStaticMesh* EquippedMesh = Definition->WorldMesh.LoadSynchronous();
		Mesh->SetStaticMesh(EquippedMesh);
		Mesh->SetRelativeScale3D(Definition->WorldMeshScale);
		ApplyEquippedTransform();
	}

	NotifyStateChanged();
}

void ADCFirearm::ApplyEquippedTransform()
{
	if (!Definition)
	{
		return;
	}

	SetActorRelativeTransform(FTransform(Definition->EquippedRotation, Definition->EquippedOffset));
	MuzzleLight->SetRelativeLocation(FVector(0.0f, 21.0f, 5.0f));
}

void ADCFirearm::SetHolstered(bool bInHolstered)
{
	bHolstered = bInHolstered;
	SetActorHiddenInGame(bHolstered);
	if (bHolstered)
	{
		GetWorldTimerManager().ClearTimer(ReloadTimer);
		bReloading = false;
		HideMuzzleLight();
	}
	NotifyStateChanged();
}

int32 ADCFirearm::GetMagazineSize() const
{
	return Definition ? Definition->MagazineSize : 0;
}

int32 ADCFirearm::GetReserveAmmo() const
{
	const UDCInventoryComponent* Inventory = FindOwnerInventory();
	const UDCItemDefinition* Ammo = Definition ? Definition->AmmoItem.LoadSynchronous() : nullptr;
	return (Inventory && Ammo) ? Inventory->GetQuantity(Ammo) : 0;
}

UDCInventoryComponent* ADCFirearm::FindOwnerInventory() const
{
	const AActor* InventoryOwner = GetOwner() ? GetOwner() : GetInstigator();
	return InventoryOwner ? InventoryOwner->FindComponentByClass<UDCInventoryComponent>() : nullptr;
}

bool ADCFirearm::CanFire() const
{
	if (!Definition || bHolstered || bReloading)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return (World->GetTimeSeconds() - LastFireTime) >= Definition->FireInterval;
}

bool ADCFirearm::CanReload() const
{
	return Definition && !bHolstered && !bReloading
		&& RoundsInMagazine < GetMagazineSize()
		&& GetReserveAmmo() > 0;
}

int32 ADCFirearm::ConsumeRound()
{
	if (RoundsInMagazine <= 0)
	{
		return 0;
	}

	--RoundsInMagazine;
	NotifyStateChanged();
	return 1;
}

int32 ADCFirearm::LoadRoundsFromInventory(UDCInventoryComponent* Inventory)
{
	if (!Definition || !Inventory)
	{
		return 0;
	}

	UDCItemDefinition* Ammo = Definition->AmmoItem.LoadSynchronous();
	if (!Ammo)
	{
		return 0;
	}

	const int32 Need = GetMagazineSize() - RoundsInMagazine;
	if (Need <= 0)
	{
		return 0;
	}

	const int32 Taken = Inventory->RemoveItem(Ammo, Need);
	RoundsInMagazine += Taken;
	if (Taken > 0)
	{
		NotifyStateChanged();
	}
	return Taken;
}

bool ADCFirearm::Fire()
{
	if (!CanFire())
	{
		return false;
	}

	APawn* Shooter = GetInstigator();
	UWorld* World = GetWorld();
	if (!World || !Shooter)
	{
		return false;
	}

	if (RoundsInMagazine <= 0)
	{
		if (USoundBase* Dry = Definition->DryFireSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Dry, GetActorLocation());
		}

		const FText Message = GetReserveAmmo() > 0
			? LOCTEXT("ReloadHint", "Reload")
			: LOCTEXT("NoAmmo", "No ammo");
		ADCHUD::ShowMessageFor(Shooter, Message, 1.2f);
		LastFireTime = World->GetTimeSeconds();
		return false;
	}

	ConsumeRound();
	LastFireTime = World->GetTimeSeconds();

	if (USoundBase* FireSnd = Definition->FireSound.LoadSynchronous())
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSnd, GetActorLocation());
	}

	MuzzleLight->SetVisibility(true);
	GetWorldTimerManager().SetTimer(MuzzleTimer, this, &ADCFirearm::HideMuzzleLight, 0.04f, false);

	FVector ViewLocation;
	FRotator ViewRotation;
	if (const APlayerController* PC = Cast<APlayerController>(Shooter->GetController()))
	{
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		Shooter->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	const FVector TraceEnd = ViewLocation + ViewRotation.Vector() * Definition->Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(DCFirearm), false, Shooter);
	Params.AddIgnoredActor(this);

	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_WorldStatic);
	Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
	Objects.AddObjectTypesToQuery(ECC_Pawn);

	FHitResult Hit;
	if (World->LineTraceSingleByObjectType(Hit, ViewLocation, TraceEnd, Objects, Params))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<ADCImpactMark>(Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), SpawnParams);

		if (AActor* HitActor = Hit.GetActor())
		{
			FDCDamageInfo Damage;
			Damage.Amount = Definition->Damage;
			Damage.DamageType = DCTags::Damage_Ballistic;
			Damage.Instigator = Shooter;
			Damage.Causer = this;
			Damage.Hit = Hit;
			UDCHealthComponent::ApplyDamageToActor(HitActor, Damage);
		}
	}

	return true;
}

bool ADCFirearm::StartReload()
{
	if (!CanReload())
	{
		return false;
	}

	bReloading = true;
	GetWorldTimerManager().SetTimer(ReloadTimer, this, &ADCFirearm::FinishReload, Definition->ReloadDuration, false);
	NotifyStateChanged();
	return true;
}

void ADCFirearm::FinishReload()
{
	bReloading = false;
	LoadRoundsFromInventory(FindOwnerInventory());
	NotifyStateChanged();
}

void ADCFirearm::HideMuzzleLight()
{
	MuzzleLight->SetVisibility(false);
}

void ADCFirearm::NotifyStateChanged() const
{
	// HUD polls each frame; kept as a hook for audio/anim later.
}

#undef LOCTEXT_NAMESPACE
