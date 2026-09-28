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

	UFUNCTION(BlueprintCallable, Category="HUD")
	void ToggleInventory() { bShowInventory = !bShowInventory; }

	UFUNCTION(BlueprintPure, Category="HUD")
	bool IsInventoryShown() const { return bShowInventory; }

protected:

	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor TextColor = FLinearColor(0.9f, 0.9f, 0.85f, 1.0f);

	UPROPERTY(EditAnywhere, Category="HUD", meta=(ClampMin="0"))
	float CrosshairSize = 4.0f;

private:

	void DrawCrosshair();

	void DrawInteractionPrompt();

	void DrawMessage();

	void DrawInventory();

	void DrawWeapon();

	void DrawCenteredText(const FString& Text, float Y, UFont* Font, const FLinearColor& Color);

	FText CurrentMessage;

	bool bShowInventory = false;

	double MessageExpireTime = 0.0;
};
