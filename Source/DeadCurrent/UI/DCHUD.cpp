#include "UI/DCHUD.h"
#include "Character/DCCharacterProgressionComponent.h"
#include "Character/DCPlayerCharacter.h"
#include "Combat/DCFirearm.h"
#include "Core/DCGameplayRules.h"
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
#include "Quest/DCQuestComponent.h"
#include "Quest/DCQuestDefinition.h"
#include "World/DCLocationVolume.h"
#include "World/DCWorldStateSubsystem.h"

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
	DrawBanner();
	DrawDialogue();
	DrawObjective();

	if (bShowInventory)
	{
		DrawInventory();
	}

	if (bShowBuild)
	{
		DrawBuild();
	}

	DrawWeapon();
	DrawHealth();
}

void ADCHUD::ShowMessage(const FText& Message, float Duration)
{
	CurrentMessage = Message;
	MessageExpireTime = GetWorld()->GetTimeSeconds() + Duration;
}

ADCHUD* ADCHUD::FindFor(const AActor* Actor)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	const APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	return PC ? PC->GetHUD<ADCHUD>() : nullptr;
}

void ADCHUD::ShowMessageFor(const AActor* Actor, const FText& Message, float Duration)
{
	if (ADCHUD* HUD = FindFor(Actor))
	{
		HUD->ShowMessage(Message, Duration);
	}
}

void ADCHUD::ShowBanner(const FText& Title, const FText& Subtitle, float Duration)
{
	BannerTitle = Title;
	BannerSubtitle = Subtitle;
	BannerStartTime = GetWorld()->GetTimeSeconds();
	BannerExpireTime = BannerStartTime + Duration;
}

void ADCHUD::ShowBannerFor(const AActor* Actor, const FText& Title, const FText& Subtitle, float Duration)
{
	if (ADCHUD* HUD = FindFor(Actor))
	{
		HUD->ShowBanner(Title, Subtitle, Duration);
	}
}

FText ADCHUD::GetActiveBannerSubtitle() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() <= BannerExpireTime ? BannerSubtitle : FText::GetEmpty();
}

FText ADCHUD::GetActiveMessage() const
{
	return GetWorld() && GetWorld()->GetTimeSeconds() <= MessageExpireTime ? CurrentMessage : FText::GetEmpty();
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

	UFont* Font = GEngine->GetMediumFont();
	const float MaxWidth = FMath::Max(120.0f, Canvas->ClipX - 80.0f);
	TArray<FString> Lines;
	WrapTextToWidth(CurrentMessage.ToString(), Font, MaxWidth, Lines);
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	float Y = Canvas->ClipY * 0.75f;
	for (const FString& Line : Lines)
	{
		DrawCenteredText(Line, Y, Font, TextColor);
		Y += LineHeight;
	}
}

void ADCHUD::DrawBanner()
{
	const double Now = GetWorld()->GetTimeSeconds();
	if (BannerSubtitle.IsEmpty() || Now > BannerExpireTime)
	{
		return;
	}

	// Quick fade in, slow fade out.
	const float FadeIn = FMath::Clamp(static_cast<float>((Now - BannerStartTime) / 0.3), 0.0f, 1.0f);
	const float FadeOut = FMath::Clamp(static_cast<float>((BannerExpireTime - Now) / 1.0), 0.0f, 1.0f);
	const float Alpha = FMath::Min(FadeIn, FadeOut);

	float Y = Canvas->ClipY * 0.2f;
	if (!BannerTitle.IsEmpty())
	{
		UFont* TitleFont = GEngine->GetMediumFont();
		DrawCenteredText(BannerTitle.ToString(), Y, TitleFont, FLinearColor(0.85f, 0.75f, 0.45f, Alpha));
		Y += TitleFont->GetMaxCharHeight() + 6.0f;
	}
	DrawCenteredText(BannerSubtitle.ToString(), Y, GEngine->GetLargeFont(), FLinearColor(TextColor.R, TextColor.G, TextColor.B, Alpha));
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

	DrawQuestLog(X + PanelWidth + 20.0f, 60.0f);
}

void ADCHUD::DrawQuestLog(float X, float Top)
{
	const APawn* Pawn = GetOwningPawn();
	const UDCQuestComponent* Quests = Pawn ? Pawn->FindComponentByClass<UDCQuestComponent>() : nullptr;
	if (!Quests)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	const float Padding = 12.0f;
	const float PanelWidth = FMath::Clamp(Canvas->ClipX - X - 40.0f, 240.0f, 460.0f);
	const float TextWidth = PanelWidth - Padding * 2.0f;

	// Lay out first so the panel fits its text.
	struct FRow { FString Text; FLinearColor Color; };
	TArray<FRow> Rows;
	for (const FDCQuestProgress& Progress : Quests->GetQuestLog())
	{
		const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(Progress.QuestId);
		const FString Name = Definition ? Definition->DisplayName.ToString() : Progress.QuestId.ToString();
		const bool bComplete = Quests->IsComplete(Progress.QuestId);
		Rows.Add({ FString::Printf(TEXT("%s  (%s)"), *Name, bComplete ? TEXT("complete") : TEXT("active")),
			bComplete ? TextColor * 0.8f : FLinearColor(0.85f, 0.75f, 0.45f, 1.0f) });

		TArray<FString> Lines;
		WrapTextToWidth(Quests->GetStageText(Progress.QuestId).ToString(), Font, TextWidth, Lines);
		for (const FString& Line : Lines)
		{
			Rows.Add({ Line, TextColor * 0.75f });
		}
	}
	if (Rows.IsEmpty())
	{
		Rows.Add({ TEXT("(none)"), TextColor * 0.7f });
	}

	// Discovered places, so a load can be checked at a glance.
	const UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(this);
	const TArray<FName> NoPlaces;
	const TArray<FName>& Places = WorldState ? WorldState->GetDiscoveredLocations() : NoPlaces;
	const int32 PlacesHeaderRow = Rows.Num();
	Rows.Add({ FString(), TextColor });
	Rows.Add({ TEXT("PLACES"), TextColor });
	for (const FName Place : Places)
	{
		Rows.Add({ ADCLocationVolume::FindDisplayName(GetWorld(), Place).ToString(), TextColor * 0.8f });
	}
	if (Places.IsEmpty())
	{
		Rows.Add({ TEXT("(none)"), TextColor * 0.7f });
	}

	const UDCCharacterProgressionComponent* Build = Pawn->FindComponentByClass<UDCCharacterProgressionComponent>();
	Rows.Add({ FString(), TextColor });
	Rows.Add({ TEXT("CHARACTER"), TextColor });
	if (!Build)
	{
		Rows.Add({ TEXT("(unavailable)"), TextColor * 0.7f });
	}
	else
	{
		Rows.Add({ FString::Printf(TEXT("Attributes  (%d left)"), Build->GetUnspentAttributePoints()), TextColor * 0.8f });
		for (const FGameplayTag& Id : UDCCharacterProgressionComponent::AllAttributes())
		{
			Rows.Add({ FString::Printf(TEXT("%s  %d"), *UDCCharacterProgressionComponent::GetAttributeName(Id).ToString(), Build->GetAttribute(Id)), TextColor });
		}
		Rows.Add({ FString::Printf(TEXT("Skills  (%d left)"), Build->GetUnspentSkillPoints()), TextColor * 0.8f });
		for (const FGameplayTag& Id : UDCCharacterProgressionComponent::AllSkills())
		{
			const int32 Raw = Build->GetSkill(Id);
			const int32 Effective = Build->GetEffectiveSkill(Id);
			FString Line = FString::Printf(TEXT("%s  %d"), *UDCCharacterProgressionComponent::GetSkillName(Id).ToString(), Raw);
			if (Effective != Raw)
			{
				Line += FString::Printf(TEXT("  (effective %d)"), Effective);
			}
			Rows.Add({ Line, TextColor });
		}
		FString PerkLine = FString::Printf(TEXT("Perks  (%d left)"), Build->GetUnspentPerkPoints());
		bool bAnyPerk = false;
		for (const FGameplayTag& Id : UDCCharacterProgressionComponent::AllPerks())
		{
			if (Build->HasPerk(Id))
			{
				PerkLine += TEXT("  ") + UDCCharacterProgressionComponent::GetPerkName(Id).ToString();
				bAnyPerk = true;
			}
		}
		if (!bAnyPerk)
		{
			PerkLine += TEXT("  (none)");
		}
		Rows.Add({ PerkLine, TextColor });
		Rows.Add({ TEXT("[B] Change build"), TextColor * 0.7f });
	}

	// Rows, plus half a line of spacing under each of the two headers.
	const float PanelHeight = Padding * 2.0f + LineHeight * (Rows.Num() + 2.0f);
	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f), X, Top, PanelWidth, PanelHeight);

	float Y = Top + Padding;
	DrawText(TEXT("QUESTS"), TextColor, X + Padding, Y, Font);
	Y += LineHeight * 1.5f;
	for (int32 Index = 0; Index < Rows.Num(); ++Index)
	{
		DrawText(Rows[Index].Text, Rows[Index].Color, X + Padding, Y, Font);
		Y += Index == PlacesHeaderRow + 1 ? LineHeight * 1.5f : LineHeight;
	}
}

void ADCHUD::DrawBuild()
{
	const APawn* Pawn = GetOwningPawn();
	const UDCCharacterProgressionComponent* Build = Pawn ? Pawn->FindComponentByClass<UDCCharacterProgressionComponent>() : nullptr;
	if (!Build)
	{
		return;
	}

	UFont* Font = GEngine->GetMediumFont();
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	const float Padding = 12.0f;
	const float PanelWidth = 520.0f;

	struct FRow { FString Text; FLinearColor Color; };
	TArray<FRow> Rows;
	Rows.Add({ FString::Printf(TEXT("ATTRIBUTES  (%d left)"), Build->GetUnspentAttributePoints()), FLinearColor(0.85f, 0.75f, 0.45f, 1.0f) });
	const TCHAR* AttributeKeys[] = { TEXT("F1"), TEXT("F2"), TEXT("F3") };
	const TArray<FGameplayTag>& Attributes = UDCCharacterProgressionComponent::AllAttributes();
	for (int32 Index = 0; Index < Attributes.Num(); ++Index)
	{
		const FGameplayTag& Id = Attributes[Index];
		const TCHAR* Key = Index < UE_ARRAY_COUNT(AttributeKeys) ? AttributeKeys[Index] : TEXT("?");
		Rows.Add({ FString::Printf(TEXT("[%s]  %s  %d"), Key, *UDCCharacterProgressionComponent::GetAttributeName(Id).ToString(), Build->GetAttribute(Id)), TextColor });
	}
	Rows.Add({ FString::Printf(TEXT("SKILLS  (%d left)"), Build->GetUnspentSkillPoints()), FLinearColor(0.85f, 0.75f, 0.45f, 1.0f) });
	const TCHAR* SkillKeys[] = { TEXT("F4"), TEXT("F5"), TEXT("F6") };
	const TArray<FGameplayTag>& Skills = UDCCharacterProgressionComponent::AllSkills();
	for (int32 Index = 0; Index < Skills.Num(); ++Index)
	{
		const FGameplayTag& Id = Skills[Index];
		const TCHAR* Key = Index < UE_ARRAY_COUNT(SkillKeys) ? SkillKeys[Index] : TEXT("?");
		const int32 Raw = Build->GetSkill(Id);
		const int32 Effective = Build->GetEffectiveSkill(Id);
		FString Line = FString::Printf(TEXT("[%s]  %s  %d"), Key, *UDCCharacterProgressionComponent::GetSkillName(Id).ToString(), Raw);
		if (Effective != Raw)
		{
			Line += FString::Printf(TEXT("  (effective %d)"), Effective);
		}
		Rows.Add({ Line, TextColor });
	}
	Rows.Add({ FString::Printf(TEXT("PERK  (%d left)"), Build->GetUnspentPerkPoints()), FLinearColor(0.85f, 0.75f, 0.45f, 1.0f) });
	const TCHAR* PerkKeys[] = { TEXT("F7"), TEXT("F8"), TEXT("F9") };
	const TArray<FGameplayTag>& Perks = UDCCharacterProgressionComponent::AllPerks();
	for (int32 Index = 0; Index < Perks.Num(); ++Index)
	{
		const FGameplayTag& Id = Perks[Index];
		const TCHAR* Key = Index < UE_ARRAY_COUNT(PerkKeys) ? PerkKeys[Index] : TEXT("?");
		const bool bOwned = Build->HasPerk(Id);
		Rows.Add({ FString::Printf(TEXT("[%s]  %s%s"), Key, *UDCCharacterProgressionComponent::GetPerkName(Id).ToString(), bOwned ? TEXT("  (taken)") : TEXT("")),
			bOwned ? TextColor : TextColor * 0.85f });
		Rows.Add({ FString(TEXT("      ")) + UDCCharacterProgressionComponent::GetPerkDescription(Id).ToString(), TextColor * 0.7f });
	}
	Rows.Add({ TEXT("[F10]  Reset allocation (prototype)"), TextColor * 0.7f });
	Rows.Add({ TEXT("Close this panel before F5 save or F9 load."), TextColor * 0.7f });
	Rows.Add({ TEXT("[B]  Close"), TextColor * 0.7f });

	const float PanelHeight = Padding * 2.0f + LineHeight * (Rows.Num() + 1.4f);
	const float X = 40.0f;
	const float Top = 60.0f;
	DrawRect(FLinearColor(0.02f, 0.03f, 0.04f, 0.82f), X, Top, PanelWidth, PanelHeight);
	float Y = Top + Padding;
	DrawText(TEXT("BUILD"), TextColor, X + Padding, Y, Font);
	Y += LineHeight * 1.4f;
	for (const FRow& Row : Rows)
	{
		DrawText(Row.Text, Row.Color, X + Padding, Y, Font);
		Y += LineHeight;
	}
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

void ADCHUD::DrawObjective()
{
	const ADCPlayerCharacter* Character = Cast<ADCPlayerCharacter>(GetOwningPawn());
	if (!Character || !Character->GetQuestComponent())
	{
		return;
	}

	if (Character->GetDialogueComponent() && Character->GetDialogueComponent()->IsInDialogue())
	{
		return;
	}

	const UDCQuestComponent* Quests = Character->GetQuestComponent();
	const FName QuestId = Quests->GetTrackedQuestId();
	if (QuestId.IsNone())
	{
		return;
	}

	const UDCQuestDefinition* Definition = UDCQuestDefinition::FindByQuestId(QuestId);
	const FString Name = Definition ? Definition->DisplayName.ToString() : QuestId.ToString();
	const FString Objective = FString::Printf(TEXT("%s: %s"), *Name, *Quests->GetStageText(QuestId).ToString());

	UFont* Font = GEngine->GetMediumFont();
	const float MaxWidth = FMath::Max(120.0f, Canvas->ClipX - 80.0f);
	TArray<FString> Lines;
	WrapTextToWidth(Objective, Font, MaxWidth, Lines);
	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	float Y = 28.0f;
	for (const FString& Line : Lines)
	{
		DrawCenteredText(Line, Y, Font, FLinearColor(0.85f, 0.75f, 0.45f, 1.0f));
		Y += LineHeight;
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
	const float InnerWidth = FMath::Max(80.0f, PanelWidth - Padding * 2.0f);

	TArray<FString> LineRows;
	WrapTextToWidth(Node->Line.ToString(), Font, InnerWidth, LineRows);
	if (LineRows.IsEmpty())
	{
		LineRows.Add(FString());
	}

	const TArray<int32> Visible = Dialogue->GetVisibleChoiceIndices();
	TArray<TArray<FString>> ChoiceRows;
	ChoiceRows.Reserve(Visible.Num());
	int32 ChoiceLineCount = 0;
	for (int32 VisibleIndex = 0; VisibleIndex < Visible.Num(); ++VisibleIndex)
	{
		const int32 ChoiceIndex = Visible[VisibleIndex];
		FString ChoiceText = Node->Choices[ChoiceIndex].Text.ToString();
		const FString CheckLabel = UDCGameplayRules::FormatCheckLabels(Node->Choices[ChoiceIndex].Conditions);
		if (!CheckLabel.IsEmpty())
		{
			ChoiceText = CheckLabel + TEXT(" ") + ChoiceText;
		}
		const FString Choice = FString::Printf(TEXT("[%d]  %s"), VisibleIndex + 1, *ChoiceText);
		TArray<FString> Wrapped;
		WrapTextToWidth(Choice, Font, InnerWidth, Wrapped);
		if (Wrapped.IsEmpty())
		{
			Wrapped.Add(Choice);
		}
		ChoiceLineCount += Wrapped.Num();
		ChoiceRows.Add(MoveTemp(Wrapped));
	}
	ChoiceLineCount = FMath::Max(1, ChoiceLineCount);

	const float PanelHeight = Padding * 2.0f
		+ LineHeight * 1.25f
		+ LineHeight * LineRows.Num()
		+ LineHeight * 0.5f
		+ LineHeight * ChoiceLineCount;

	const float X = (Canvas->ClipX - PanelWidth) * 0.5f;
	float Y = Canvas->ClipY * 0.58f;
	if (Y + PanelHeight > Canvas->ClipY - 16.0f)
	{
		Y = FMath::Max(16.0f, Canvas->ClipY - 16.0f - PanelHeight);
	}

	DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f), X, Y, PanelWidth, PanelHeight);

	const float Left = X + Padding;
	Y += Padding;

	const FString Speaker = Node->Speaker.IsEmpty() ? TEXT("???") : Node->Speaker.ToString();
	DrawText(Speaker, FLinearColor(0.85f, 0.75f, 0.45f, 1.0f), Left, Y, Font);
	Y += LineHeight * 1.25f;

	Y = DrawWrappedLines(LineRows, Left, Y, Font, TextColor);
	Y += LineHeight * 0.5f;

	for (const TArray<FString>& Rows : ChoiceRows)
	{
		Y = DrawWrappedLines(Rows, Left, Y, Font, TextColor);
	}
}

void ADCHUD::WrapTextToWidth(const FString& Text, UFont* Font, float MaxWidth, TArray<FString>& OutLines)
{
	OutLines.Reset();
	if (Text.IsEmpty() || !Font || MaxWidth <= 0.0f)
	{
		if (!Text.IsEmpty())
		{
			OutLines.Add(Text);
		}
		return;
	}

	auto Measure = [this, Font](const FString& S) -> float
	{
		float Width = 0.0f;
		float Height = 0.0f;
		GetTextSize(S, Width, Height, Font);
		return Width;
	};

	auto FlushWord = [&](FString& Current, const FString& Word)
	{
		FString Remaining = Word;
		while (Remaining.Len() > 0 && Measure(Remaining) > MaxWidth)
		{
			int32 Fit = Remaining.Len();
			while (Fit > 1 && Measure(Remaining.Left(Fit)) > MaxWidth)
			{
				--Fit;
			}

			if (!Current.IsEmpty())
			{
				OutLines.Add(Current);
				Current.Reset();
			}

			OutLines.Add(Remaining.Left(Fit));
			Remaining.RightChopInline(Fit);
		}

		if (Remaining.IsEmpty())
		{
			return;
		}

		const FString Test = Current.IsEmpty() ? Remaining : Current + TEXT(" ") + Remaining;
		if (!Current.IsEmpty() && Measure(Test) > MaxWidth)
		{
			OutLines.Add(Current);
			Current = Remaining;
		}
		else
		{
			Current = Test;
		}
	};

	TArray<FString> Paragraphs;
	Text.ParseIntoArray(Paragraphs, TEXT("\n"), false);
	if (Paragraphs.Num() == 0)
	{
		Paragraphs.Add(Text);
	}

	for (int32 ParagraphIndex = 0; ParagraphIndex < Paragraphs.Num(); ++ParagraphIndex)
	{
		const FString& Paragraph = Paragraphs[ParagraphIndex];
		if (Paragraph.IsEmpty())
		{
			OutLines.Add(FString());
			continue;
		}

		TArray<FString> Words;
		Paragraph.ParseIntoArrayWS(Words);
		if (Words.Num() == 0)
		{
			OutLines.Add(Paragraph);
			continue;
		}

		FString Current;
		for (const FString& Word : Words)
		{
			FlushWord(Current, Word);
		}
		if (!Current.IsEmpty())
		{
			OutLines.Add(Current);
		}
	}
}

float ADCHUD::DrawWrappedLines(const TArray<FString>& Lines, float X, float Y, UFont* Font, const FLinearColor& Color)
{
	if (!Font)
	{
		return Y;
	}

	const float LineHeight = Font->GetMaxCharHeight() + 4.0f;
	for (const FString& Line : Lines)
	{
		DrawText(Line, Color, X, Y, Font);
		Y += LineHeight;
	}
	return Y;
}

void ADCHUD::DrawCenteredText(const FString& Text, float Y, UFont* Font, const FLinearColor& Color)
{
	float Width = 0.0f;
	float Height = 0.0f;
	GetTextSize(Text, Width, Height, Font);

	const float X = (Canvas->ClipX - Width) * 0.5f;
	DrawText(Text, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f * Color.A), X + 1.0f, Y + 1.0f, Font);
	DrawText(Text, Color, X, Y, Font);
}
