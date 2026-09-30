#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DCWorldStateSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDCLocationDiscovered, FName /*LocationId*/);

/**
 *  Named world facts ("shore.path_cleared") that conditions read and consequences write, and the
 *  locations the player has discovered. Also the "something changed" signal that quests listen to
 *  for re-checking their conditions. Saved and restored by UDCSaveSubsystem.
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

	/**
	 *  Marks a location discovered. Returns true only the first time, then fires OnLocationDiscovered
	 *  and OnChanged. Location ids are stable, like persistent ids ("shore.survey_launch").
	 */
	UFUNCTION(BlueprintCallable, Category="World State")
	bool DiscoverLocation(FName LocationId);

	UFUNCTION(BlueprintPure, Category="World State")
	bool IsLocationDiscovered(FName LocationId) const { return !LocationId.IsNone() && DiscoveredLocations.Contains(LocationId); }

	/** In discovery order. */
	const TArray<FName>& GetDiscoveredLocations() const { return DiscoveredLocations; }

	/** Replaces every discovered location (save restore). Does not broadcast. */
	void ReplaceDiscoveredLocations(const TArray<FName>& SavedLocations);

	/** Tell listeners that state conditions can read has changed (flags, deaths, discoveries). */
	void NotifyChanged();

	/** Static helper for gameplay code that may run without a world (tests). */
	static void NotifyChanged(const UObject* WorldContextObject);

	FSimpleMulticastDelegate OnChanged;

	/**
	 *  Tell listeners that state was replaced wholesale (a save was applied, or a review tool set up a
	 *  scene), so presentation that normally changes out of sight should snap to the new state now.
	 *  Not a change signal: nothing re-runs consequences on it.
	 */
	void NotifyRestored();

	FSimpleMulticastDelegate OnRestored;

	/** A location was discovered for the first time (not fired on save restore). */
	FDCLocationDiscovered OnLocationDiscovered;

private:

	/** Ordered so saves are stable. */
	UPROPERTY()
	TArray<FName> Flags;

	UPROPERTY()
	TArray<FName> DiscoveredLocations;
};
