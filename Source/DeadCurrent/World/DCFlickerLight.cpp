#include "World/DCFlickerLight.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/DCGameplayRules.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

ADCFlickerLight::ADCFlickerLight()
{
	PrimaryActorTick.bCanEverTick = true;

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetMobility(EComponentMobility::Movable);
	Light->SetCastShadows(false);
	SetRootComponent(Light);

	Glow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glow"));
	Glow->SetupAttachment(Light);
	Glow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Glow->SetCastShadow(false);
	Glow->SetCanEverAffectNavigation(false);
}

void ADCFlickerLight::BeginPlay()
{
	Super::BeginPlay();

	BaseIntensity = Light->Intensity;
	if (Glow->GetStaticMesh() && Glow->GetMaterial(0))
	{
		GlowMaterial = Glow->CreateDynamicMaterialInstance(0);
		if (GlowMaterial)
		{
			GlowMaterial->GetVectorParameterValue(FHashedMaterialParameterInfo(GlowColorParameter), BaseGlowColor);
		}
	}
}

bool ADCFlickerLight::IsLightActive() const
{
	return ActiveConditions.IsEmpty()
		|| UDCGameplayRules::CheckConditions(ActiveConditions, FDCRuleContext::ForActor(const_cast<ADCFlickerLight*>(this)));
}

void ADCFlickerLight::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Re-checked every tick, like hazards, so a save restore (which replaces flags silently) takes effect.
	if (!IsLightActive())
	{
		if (Brightness != 0.0f)
		{
			ApplyBrightness(0.0f);
		}
		return;
	}

	TimeToNextChange -= DeltaSeconds;
	if (TimeToNextChange > 0.0f)
	{
		return;
	}

	TimeToNextChange = FMath::FRandRange(MinInterval, FMath::Max(MinInterval, MaxInterval));
	ApplyBrightness(FMath::FRand() < DropoutChance ? 0.0f : FMath::FRandRange(MinBrightness * MaxBrightness, MaxBrightness));
}

void ADCFlickerLight::ApplyBrightness(float NewBrightness)
{
	const bool bFlashedOn = Brightness <= 0.0f && NewBrightness > 0.0f;
	Brightness = NewBrightness;
	if (bFlashedOn && !FlashSounds.IsEmpty() && FMath::FRand() < FlashSoundChance)
	{
		if (USoundBase* Snap = FlashSounds[FMath::RandRange(0, FlashSounds.Num() - 1)].LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Snap, GetActorLocation(), FlashVolume, 1.0f, 0.0f,
				FlashAttenuation.LoadSynchronous());
		}
	}
	Light->SetIntensity(BaseIntensity * Brightness);
	Glow->SetVisibility(Brightness > 0.0f);
	if (GlowMaterial)
	{
		FLinearColor Color = BaseGlowColor * Brightness;
		Color.A = BaseGlowColor.A;
		GlowMaterial->SetVectorParameterValue(GlowColorParameter, Color);
	}
}
