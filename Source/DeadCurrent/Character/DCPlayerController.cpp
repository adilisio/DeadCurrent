#include "Character/DCPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Character/DCPlayerCameraManager.h"
#include "DeadCurrent.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Save/DCSaveSubsystem.h"
#include "Widgets/Input/SVirtualJoystick.h"

ADCPlayerController::ADCPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerCameraManagerClass = ADCPlayerCameraManager::StaticClass();
}

void ADCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogDeadCurrent, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void ADCPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (WasInputKeyJustPressed(EKeys::F5))
	{
		HandleSave();
	}
	else if (WasInputKeyJustPressed(EKeys::F9))
	{
		HandleLoad();
	}
}

void ADCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}

		if (!ShouldUseTouchControls())
		{
			for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void ADCPlayerController::HandleSave()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDCSaveSubsystem* Save = GI->GetSubsystem<UDCSaveSubsystem>())
		{
			Save->SaveCurrentGame();
		}
	}
}

void ADCPlayerController::HandleLoad()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UDCSaveSubsystem* Save = GI->GetSubsystem<UDCSaveSubsystem>())
		{
			Save->LoadCurrentGame();
		}
	}
}

bool ADCPlayerController::ShouldUseTouchControls() const
{
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}
