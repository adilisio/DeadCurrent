#pragma once

#include "CoreMinimal.h"
#include "DCProgressionTypes.generated.h"

/** One attribute or skill rank in a save. Id is the Gameplay Tag name, e.g. Skill.Engineering. */
USTRUCT(BlueprintType)
struct DEADCURRENT_API FDCSavedRank
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Save")
	int32 Value = 0;
};
