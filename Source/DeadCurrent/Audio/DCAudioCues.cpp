#include "Audio/DCAudioCues.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/SoftObjectPath.h"

void DCAudioCues::PlayUI(const UObject* WorldContext, const TCHAR* AssetPath, float Volume)
{
	if (!WorldContext || !AssetPath)
	{
		return;
	}

	// Not LoadObject: a missing cue is normal (no audio content) and must not log an error.
	if (USoundBase* Cue = Cast<USoundBase>(FSoftObjectPath(AssetPath).TryLoad()))
	{
		UGameplayStatics::PlaySound2D(WorldContext, Cue, Volume);
	}
}
