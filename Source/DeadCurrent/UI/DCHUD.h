#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DCHUD.generated.h"

class UFont;

/**
 *  Minimal canvas HUD for the first playable: crosshair dot, interaction prompt, and timed messages.
 */
UCLASS()
class DEADCURRENT_API ADCHUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	/** Shows a message in the lower part of the screen, replacing any current message */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowMessage(const FText& Message, float Duration = 4.0f);

	/** Convenience for gameplay code: shows a message on the HUD of the player controlling Actor, if any */
	static void ShowMessageFor(const AActor* Actor, const FText& Message, float Duration = 4.0f);

	/** Large two-line announcement in the upper part of the screen, e.g. LOCATION DISCOVERED / Wrecked Survey Launch */
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowBanner(const FText& Title, const FText& Subtitle, float Duration = 4.0f);

	static void ShowBannerFor(const AActor* Actor, const FText& Title, const FText& Subtitle, float Duration = 4.0f);

	/** Subtitle of the banner on screen now, or empty. */
	FText GetActiveBannerSubtitle() const;

	/** Message on screen now, or empty. */
	FText GetActiveMessage() const;

	UFUNCTION(BlueprintCallable, Category="HUD")
	void ToggleInventory() { bShowInventory = !bShowInventory; }

	UFUNCTION(BlueprintPure, Category="HUD")
	bool IsInventoryShown() const { return bShowInventory; }

	UFUNCTION(BlueprintCallable, Category="HUD")
	void ToggleBuild() { bShowBuild = !bShowBuild; }

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetBuildShown(bool bShown) { bShowBuild = bShown; }

	UFUNCTION(BlueprintPure, Category="HUD")
	bool IsBuildShown() const { return bShowBuild; }

protected:

	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor TextColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, Category="HUD", meta=(ClampMin="0"))
	float CrosshairSize = 4.0f;

private:

	void DrawCrosshair();

	void DrawInteractionPrompt();

	void DrawMessage();

	void DrawBanner();

	void DrawInventory();

	void DrawWeapon();

	void DrawHealth();

	void DrawDialogue();

	void DrawObjective();

	/** Journal beside the inventory panel: each quest with its status and objective or outcome, then discovered places. */
	void DrawQuestLog(float X, float Top);

	void DrawBuild();

	static ADCHUD* FindFor(const AActor* Actor);

	void DrawCenteredText(const FString& Text, float Y, UFont* Font, const FLinearColor& Color);

	/** Dark plate behind centered lines, so light text stays readable on a pale wall or sky. */
	void DrawBackedCenteredLines(const TArray<FString>& Lines, float TopY, UFont* Font, const FLinearColor& Color);

	void WrapTextToWidth(const FString& Text, UFont* Font, float MaxWidth, TArray<FString>& OutLines);

	float DrawWrappedLines(const TArray<FString>& Lines, float X, float Y, UFont* Font, const FLinearColor& Color);

	FText CurrentMessage;

	bool bShowInventory = false;

	bool bShowBuild = false;

	double MessageExpireTime = 0.0;

	FText BannerTitle;

	FText BannerSubtitle;

	double BannerStartTime = 0.0;

	double BannerExpireTime = 0.0;
};
