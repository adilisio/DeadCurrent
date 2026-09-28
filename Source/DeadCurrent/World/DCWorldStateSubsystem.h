#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DCWorldStateSubsystem.generated.h"

/**
 *  Named world facts ("shore.path_cleared") that conditions read and consequences write.
 *  Also the "something changed" signal that quests listen to for re-checking their conditions.
 *  Saved and restored by UDCSaveSubsystem.
 */
UCLASS()
class DEADCURRENT_API UDCWorldStateSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	static UDCWorldStateSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="World State")
	void SetFlag(FName Flag);

	UFUNCTION(BlueprintCallable, Category="World State")
	void ClearFlag(FName Flag);

	UFUNCTION(BlueprintPure, Category="World State")
	bool HasFlag(FName Flag) const { return !Flag.IsNone() && Flags.Contains(Flag); }

	const TArray<FName>& GetFlags() const { return Flags; }

	/** Replaces every flag (save restore). Does not broadcast. */
	void ReplaceFlags(const TArray<FName>& SavedFlags);

	/** Tell listeners that state conditions can read has changed (flags, deaths). */
	void NotifyChanged();

	/** Static helper for gameplay code that may run without a world (tests). */
	static void NotifyChanged(const UObject* WorldContextObject);

	FSimpleMulticastDelegate OnChanged;

private:

	/** Ordered so saves are stable. */
	UPROPERTY()
	TArray<FName> Flags;
};
