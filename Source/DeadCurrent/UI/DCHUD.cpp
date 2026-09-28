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
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"

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

	if (bShowInventory)
	{
		DrawInventory();
	}
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

void ADCHUD::DrawInventory()
{
	const APawn* Pawn = GetOwningPawn();
	const UDCInventoryComponent* Inventory = Pawn ? Pawn->FindComponentByClass<UDCInventoryComponent>() : nullptr;
	if (!Inventory)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	const float Padding = 12.0f;
	const float PanelWidth = 340.0f;
	const TArray<FDCItemStack>& Stacks = Inventory->GetStacks();
	const int32 Rows = FMath::Max(1, Stacks.Num());
	const float PanelHeight = Padding * 2.0f + LineHeight * (Rows + 3);
	const float X = 40.0f;
	float Y = 60.0f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f), X, Y, PanelWidth, PanelHeight);

	const float Left = X + Padding;
	const float Right = X + PanelWidth - Padding;
	Y += Padding;

	DrawText(TEXT("INVENTORY"), TextColor, Left, Y, Font);
	Y += LineHeight * 1.5f;

	auto DrawRightAligned = [this, Font](const FString& Text, float RightEdge, float RowY)
	{
		float Width, Height;
		GetTextSize(Text, Width, Height, Font);
		DrawText(Text, TextColor, RightEdge - Width, RowY, Font);
	};

	if (Stacks.IsEmpty())
	{
		DrawText(TEXT("(empty)"), TextColor * 0.7f, Left, Y, Font);
		Y += LineHeight;
	}

	for (const FDCItemStack& Stack : Stacks)
	{
		if (!Stack.Item)
		{
			continue;
		}

		FString Name = Stack.Item->DisplayName.ToString();
		if (Stack.Quantity > 1)
		{
			Name += FString::Printf(TEXT("  x%d"), Stack.Quantity);
		}
		DrawText(Name, TextColor, Left, Y, Font);
		DrawRightAligned(FString::Printf(TEXT("%.2f kg"), Stack.Item->Weight * Stack.Quantity), Right, Y);
		Y += LineHeight;
	}

	Y += LineHeight * 0.5f;
	DrawText(TEXT("Total"), TextColor, Left, Y, Font);
	DrawRightAligned(FString::Printf(TEXT("%.2f kg"), Inventory->GetTotalWeight()), Right, Y);
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
