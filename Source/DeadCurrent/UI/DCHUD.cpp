#include "UI/DCHUD.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCFirearm.h"
#include "Combat/DCHealthComponent.h"
#include "Dialogue/DCDialogueComponent.h"
#include "Dialogue/DCDialogueTypes.h"
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
	DrawDialogue();

	if (bShowInventory)
	{
		DrawInventory();
	}

	DrawWeapon();
	DrawHealth();
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
	const ADCPlayerCharacter* Character = Cast<ADCPlayerCharacter>(Pawn);
	if (Character && Character->GetDialogueComponent() && Character->GetDialogueComponent()->IsInDialogue())
	{
		return;
	}

	const UDCInteractorComponent* Interactor = Pawn ? Pawn->FindComponentByClass<UDCInteractorComponent>() : nullptr;

	FDCInteractionPrompt Prompt;
	if (!Interactor || !Interactor->GetFocusedPrompt(Prompt))
	{
		return;
	}

	FString KeyLabel;
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

void ADCHUD::DrawWeapon()
{
	const ADCPlayerCharacter* Character = Cast<ADCPlayerCharacter>(GetOwningPawn());
	const ADCFirearm* Firearm = Character ? Character->GetEquippedFirearm() : nullptr;
	if (!Firearm || Firearm->IsHolstered())
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	FString AmmoText;
	if (Firearm->IsReloading())
	{
		AmmoText = TEXT("Reloading");
	}
	else
	{
		AmmoText = FString::Printf(TEXT("%d  |  %d"), Firearm->GetRoundsInMagazine(), Firearm->GetReserveAmmo());
	}

	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(AmmoText, Width, Height, Font);
	const float X = Canvas->ClipX - Width - 40.0f;
	const float Y = Canvas->ClipY - Height - 36.0f;
	DrawText(AmmoText, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), X + 1.0f, Y + 1.0f, Font);
	DrawText(AmmoText, TextColor, X, Y, Font);

	if (Firearm->GetRoundsInMagazine() == 0 && Firearm->GetReserveAmmo() > 0 && !Firearm->IsReloading())
	{
		FString ReloadHint;
		const ULocalPlayer* LocalPlayer = GetOwningPlayerController() ? GetOwningPlayerController()->GetLocalPlayer() : nullptr;
		if (const UEnhancedInputLocalPlayerSubsystem* Input = LocalPlayer ? LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>() : nullptr)
		{
			if (Character->GetReloadAction())
			{
				const TArray<FKey> Keys = Input->QueryKeysMappedToAction(Character->GetReloadAction());
				if (Keys.Num() > 0)
				{
					ReloadHint = FString::Printf(TEXT("[%s] Reload"), *Keys[0].GetDisplayName().ToString());
				}
			}
		}
		if (ReloadHint.IsEmpty())
		{
			ReloadHint = TEXT("Reload");
		}

		float HintWidth = 0.0f;
		float HintHeight = 0.0f;
		GetTextSize(ReloadHint, HintWidth, HintHeight, Font);
		DrawText(ReloadHint, TextColor, Canvas->ClipX - HintWidth - 40.0f, Y - HintHeight - 6.0f, Font);
	}
}

void ADCHUD::DrawHealth()
{
	const APawn* Pawn = GetOwningPawn();
	const UDCHealthComponent* Health = Pawn ? Pawn->FindComponentByClass<UDCHealthComponent>() : nullptr;
	if (!Health)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float BarWidth = 180.0f;
	const float BarHeight = 12.0f;
	const float X = 40.0f;
	const float Y = Canvas->ClipY - 48.0f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f), X, Y, BarWidth, BarHeight);

	const float Pct = FMath::Clamp(Health->GetHealthPercent(), 0.0f, 1.0f);
	const FLinearColor Fill = Health->IsDead()
		? FLinearColor(0.4f, 0.05f, 0.05f, 0.9f)
		: FLinearColor(0.75f, 0.12f, 0.12f, 0.9f);
	DrawRect(Fill, X, Y, BarWidth * Pct, BarHeight);

	const FString Label = FString::Printf(TEXT("%d"), FMath::RoundToInt(Health->GetHealth()));
	DrawText(Label, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), X + 1.0f, Y - 20.0f, Font);
	DrawText(Label, TextColor, X, Y - 20.0f, Font);

	if (Health->IsDead())
	{
		DrawCenteredText(TEXT("YOU DIED"), Canvas->ClipY * 0.42f, GEngine->GetLargeFont(), FLinearColor(0.85f, 0.1f, 0.1f, 1.0f));
	}
}

void ADCHUD::DrawDialogue()
{
	const ADCPlayerCharacter* Character = Cast<ADCPlayerCharacter>(GetOwningPawn());
	const UDCDialogueComponent* Dialogue = Character ? Character->GetDialogueComponent() : nullptr;
	const FDCDialogueNode* Node = Dialogue ? Dialogue->GetCurrentNode() : nullptr;
	if (!Node)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	const float Padding = 16.0f;
	const float PanelWidth = FMath::Min(720.0f, Canvas->ClipX - 80.0f);
	const int32 ChoiceRows = FMath::Max(1, Node->Choices.Num());
	const float PanelHeight = Padding * 2.0f + LineHeight * (4 + ChoiceRows);
	const float X = (Canvas->ClipX - PanelWidth) * 0.5f;
	float Y = Canvas->ClipY * 0.58f;

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f), X, Y, PanelWidth, PanelHeight);

	const float Left = X + Padding;
	Y += Padding;

	const FString Speaker = Node->Speaker.IsEmpty() ? TEXT("???") : Node->Speaker.ToString();
	DrawText(Speaker, FLinearColor(0.85f, 0.75f, 0.45f, 1.0f), Left, Y, Font);
	Y += LineHeight * 1.25f;

	DrawText(Node->Line.ToString(), TextColor, Left, Y, Font);
	Y += LineHeight * 1.5f;

	for (int32 Index = 0; Index < Node->Choices.Num(); ++Index)
	{
		const FString Choice = FString::Printf(TEXT("[%d]  %s"), Index + 1, *Node->Choices[Index].Text.ToString());
		DrawText(Choice, TextColor, Left, Y, Font);
		Y += LineHeight;
	}
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
