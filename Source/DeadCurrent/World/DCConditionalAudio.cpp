#include "World/DCConditionalAudio.h"
#include "World/DCWorldStateSubsystem.h"
#include "Components/AudioComponent.h"
#include "Core/DCGameplayRules.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

ADCConditionalAudio::ADCConditionalAudio()
{
	PrimaryActorTick.bCanEverTick = true;

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->bAutoActivate = false;
	Audio->bAllowSpatialization = true;
	SetRootComponent(Audio);
}

void ADCConditionalAudio::BeginPlay()
{
	Super::BeginPlay();

	if (USoundBase* Loaded = Sound.LoadSynchronous())
	{
		Audio->SetSound(Loaded);
	}
	if (USoundAttenuation* Falloff = Attenuation.LoadSynchronous())
	{
		Audio->bOverrideAttenuation = false;
		Audio->AttenuationSettings = Falloff;
	}
	Audio->SetVolumeMultiplier(VolumeMultiplier);

	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		RestoredHandle = WorldState->OnRestored.AddUObject(this, &ADCConditionalAudio::Snap);
	}

	TimeToCheck = 0.0f;
	Evaluate();
}

void ADCConditionalAudio::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this))
	{
		WorldState->OnRestored.Remove(RestoredHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ADCConditionalAudio::Snap()
{
	const bool bPass = ConditionsPass();
	bPrimed = true;
	bAudible = bPass;
	if (Mode == EDCConditionalAudioMode::OnceWhenTrue)
	{
		return; // re-primed: the next rising edge plays, the restored state does not
	}
	bFadingOut = false;
	if (!bPass && Audio->IsPlaying())
	{
		Audio->Stop();
	}
	else if (bPass && !Audio->IsPlaying() && Audio->GetSound())
	{
		Audio->Play();
	}
}

bool ADCConditionalAudio::ConditionsPass() const
{
	if (Conditions.IsEmpty())
	{
		return true;
	}

	AActor* Context = const_cast<ADCConditionalAudio*>(this);
	if (const UWorld* World = GetWorld())
	{
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
		{
			Context = Pawn;
		}
	}
	return UDCGameplayRules::CheckConditions(Conditions, FDCRuleContext::ForActor(Context));
}

void ADCConditionalAudio::Evaluate()
{
	const bool bPass = ConditionsPass();

	if (Mode == EDCConditionalAudioMode::OnceWhenTrue)
	{
		// The first evaluation only records the starting state, so a load that already has the flag stays silent.
		const bool bRose = bPrimed && bPass && !bAudible;
		bPrimed = true;
		bAudible = bPass;
		TriggerCount += bRose ? 1 : 0;
		if (bRose && Audio->GetSound())
		{
			Audio->Play();
		}
		return;
	}

	bPrimed = true;
	bAudible = bPass;
	if (bPass)
	{
		bFadingOut = false;
		if (!Audio->IsPlaying() && Audio->GetSound())
		{
			Audio->Play();
			UE_LOG(LogTemp, Log, TEXT("ConditionalAudio %s playing %s at volume %.2f (attenuation %s)"), *GetName(),
				*GetNameSafe(Audio->GetSound()), VolumeMultiplier, *GetNameSafe(Audio->AttenuationSettings));
		}
	}
	else if (!bFadingOut && Audio->IsPlaying())
	{
		bFadingOut = true;
		Audio->FadeOut(FadeOutSeconds, 0.0f);
	}
}

void ADCConditionalAudio::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeToCheck -= DeltaSeconds;
	if (TimeToCheck <= 0.0f)
	{
		TimeToCheck = CheckInterval;
		Evaluate();
	}
}
