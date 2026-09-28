#include "UI/DCHUD.h"
#include "Character/DCPlayerCharacter.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/DCInteractorComponent.h"

void ADCHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	DrawCrosshair();
	DrawInteractionPrompt();
	DrawMessage();
}

void ADCHUD::ShowMessage(const FText& Message, float Duration)
{
	CurrentMessage = Message;
	MessageExpireTime = GetWorld()->GetTimeSeconds() + Duration;
}

void ADCHUD::ShowMessageFor(const AActor* Actor, const FText& Message, float Duration)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (ADCHUD* HUD = PC ? PC->GetHUD<ADCHUD>() : nullptr)
	{
		HUD->ShowMessage(Message, Duration);
	}
}

void ADCHUD::DrawCrosshair()
{
	const float Half = CrosshairSize * 0.5f;
	DrawRect(FLinearColor(1.0f, 1.0f, 1.0f, 0.6f), Canvas->ClipX * 0.5f - Half, Canvas->ClipY * 0.5f - Half, CrosshairSize, CrosshairSize);
}

void ADCHUD::DrawInteractionPrompt()
{
	const APawn* Pawn = GetOwningPawn();
	const UDCInteractorComponent* Interactor = Pawn ? Pawn->FindComponentByClass<UDCInteractorComponent>() : nullptr;

	FDCInteractionPrompt Prompt;
	if (!Interactor || !Interactor->GetFocusedPrompt(Prompt))
	{
		return;
	}

	FString KeyLabel;
	const ADCPlayerCharacter* Character = Cast<ADCPlayerCharacter>(Pawn);
	const ULocalPlayer* LocalPlayer = GetOwningPlayerController() ? GetOwningPlayerController()->GetLocalPlayer() : nullptr;
	if (const UEnhancedInputLocalPlayerSubsystem* Input = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr)
	{
		if (Character && Character->GetInteractAction())
		{
			const TArray<FKey> Keys = Input->QueryKeysMappedToAction(Character->GetInteractAction());
			if (Keys.Num() > 0)
			{
				KeyLabel = FString::Printf(TEXT("[%s] "), *Keys[0].GetDisplayName().ToString());
			}
		}
	}

	FString Text = KeyLabel + Prompt.Action.ToString();
	if (!Prompt.TargetName.IsEmpty())
	{
		Text += TEXT(" ") + Prompt.TargetName.ToString();
	}

	DrawCenteredText(Text, Canvas->ClipY * 0.5f + 40.0f, GEngine->GetMediumFont(), TextColor);
}

void ADCHUD::DrawMessage()
{
	if (CurrentMessage.IsEmpty() || GetWorld()->GetTimeSeconds() > MessageExpireTime)
	{
		return;
	}

	DrawCenteredText(CurrentMessage.ToString(), Canvas->ClipY * 0.75f, GEngine->GetMediumFont(), TextColor);
}

void ADCHUD::DrawCenteredText(const FString& Text, float Y, UFont* Font, const FLinearColor& Color)
{
	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(Text, Width, Height, Font);

	const float X = (Canvas->ClipX - Width) * 0.5f;
	DrawText(Text, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), X + 1.0f, Y + 1.0f, Font);
	DrawText(Text, Color, X, Y, Font);
}
