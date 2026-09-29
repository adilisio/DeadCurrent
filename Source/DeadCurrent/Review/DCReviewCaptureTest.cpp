#include "Camera/CameraComponent.h"
#include "Character/DCCharacterProgressionComponent.h"
#include "Character/DCPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CollisionShape.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Misc/OutputDevice.h"
#include "Misc/ScopeLock.h"
#include "DeadCurrent.h"
#include "Dom/JsonObject.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTagContainer.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interaction/DCInteractable.h"
#include "Inventory/DCInventoryComponent.h"
#include "Items/DCItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Quest/DCQuestComponent.h"
#include "Save/DCPersistentRegistry.h"
#include "Save/DCSaveSubsystem.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Tests/AutomationCommon.h"
#include "UI/DCHUD.h"
#include "UnrealClient.h"
#include "World/DCInspectableActor.h"
#include "World/DCWorldStateSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 *  DeadCurrent.Review.Capture
 *
 *  Game-context tool, not part of Tools\RunTests.bat. Run it with Tools\ReviewCapture.bat
 *  (UnrealEditor -game, PlayTest cvars). It fails only when a frame could not be captured.
 *  Viewpoints live in Tools/Review/Lvl_Boathouse.json.
 */
namespace DCReviewCapture
{
	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_Boathouse");
	const TCHAR* TestSlot = TEXT("DeadCurrent_Review");
	const TCHAR* ArtTag = TEXT("ArtLayerSentinel");
	const int32 SettleFrames = 8;
	const double MapTimeoutSeconds = 60.0;
	const double ShotTimeoutSeconds = 15.0;

	struct FLandmark
	{
		FString Id;
		FString Subject;
		FVector Location = FVector::ZeroVector;
	};

	struct FShot
	{
		FString Id;
		FVector Location = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		FString Focus;
		FVector FocusOffset = FVector::ZeroVector;
		FString Subject;
		TArray<FString> Subjects;
		FVector ViewOffset = FVector::ZeroVector;
		bool bOffsetInWorld = false;
		FString Expectation;
		FString Source;
		TSharedPtr<FJsonObject> Setup;
		TArray<FLandmark> Landmarks;
	};

	class FReviewLogCapture : public FOutputDevice
	{
	public:
		TArray<FString> Warnings;
		TArray<FString> Errors;
		TArray<FString> Pool;
		bool bActive = false;
		FCriticalSection Mutex;

		void ResetLists()
		{
			FScopeLock Lock(&Mutex);
			Warnings.Reset();
			Errors.Reset();
			Pool.Reset();
		}

		void SetActive(bool bInActive)
		{
			FScopeLock Lock(&Mutex);
			bActive = bInActive;
		}

		virtual bool CanBeUsedOnAnyThread() const override { return true; }

		virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override
		{
			FScopeLock Lock(&Mutex);
			if (!bActive || !V)
			{
				return;
			}
			if (Verbosity != ELogVerbosity::Warning && Verbosity != ELogVerbosity::Error)
			{
				return;
			}
			const FString Line = FString::Printf(TEXT("%s: %s"), *Category.ToString(), V);
			const bool bPool = Line.Contains(TEXT("over budget"), ESearchCase::IgnoreCase)
				|| (Line.Contains(TEXT("pool"), ESearchCase::IgnoreCase) && Line.Contains(TEXT("budget"), ESearchCase::IgnoreCase));
			if (bPool && Pool.Num() < 20)
			{
				Pool.Add(Line);
			}
			if (Verbosity == ELogVerbosity::Error)
			{
				if (Errors.Num() < 50)
				{
					Errors.Add(Line);
				}
			}
			else if (Warnings.Num() < 50)
			{
				Warnings.Add(Line);
			}
		}
	};

	struct FRun
	{
		FAutomationTestBase* Test = nullptr;
		TArray<FShot> Shots;
		TArray<FVector> RoutePoints;
		FString RouteExpectation;
		FString RouteSource;
		float RoutePitch = -6.0f;
		FString OutDir;
		bool bFailed = false;
		int32 Index = 0;
		int32 Frames = 0;
		double PhaseStart = 0.0;
		float WalkSpeed = 450.0f;
		float RouteDistance = 0.0f;
		float RouteLength = 0.0f;
		TWeakObjectPtr<UWorld> PreviousWorld;
		TArray<TSharedPtr<FJsonValue>> ViewpointResults;
		TArray<TSharedPtr<FJsonValue>> RouteResults;
		TSharedPtr<FReviewLogCapture> Log;
		bool bLogAttached = false;
		TArray<double> FrameSamples;
		float PendingFrameMs = 0.0f;
		TArray<FString> DefaultMaterials;
		TArray<FString> MissingTextures;
		TArray<TSharedPtr<FJsonValue>> LandmarkResults;

		enum class EPhase : uint8
		{
			Boot,
			Open,
			Wait,
			Setup,
			Settle,
			Shot,
			WaitFile,
			RouteOpen,
			RouteWait,
			RouteSettle,
			RouteShot,
			RouteWaitFile,
			Finish
		};

		EPhase Phase = EPhase::Boot;
	};

	UWorld* GameWorld()
	{
		return AutomationCommon::GetAnyGameWorld();
	}

	ADCPlayerCharacter* Player()
	{
		UWorld* World = GameWorld();
		return World ? Cast<ADCPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)) : nullptr;
	}

	bool ReadVector(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, FVector& Out)
	{
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (!Object.IsValid() || !Object->TryGetArrayField(Field, Values) || !Values || Values->Num() < 3)
		{
			return false;
		}
		Out.X = (*Values)[0]->AsNumber();
		Out.Y = (*Values)[1]->AsNumber();
		Out.Z = (*Values)[2]->AsNumber();
		return true;
	}

	bool ReadRotator(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, FRotator& Out)
	{
		FVector Value;
		if (!ReadVector(Object, Field, Value))
		{
			return false;
		}
		Out = FRotator(Value.X, Value.Y, Value.Z);
		return true;
	}

	bool LoadList(const FString& Path, FRun& Run, FString& Error)
	{
		FString Text;
		if (!FFileHelper::LoadFileToString(Text, *Path))
		{
			Error = FString::Printf(TEXT("Could not read %s"), *Path);
			return false;
		}

		TSharedPtr<FJsonObject> Root;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Text);
		if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
		{
			Error = FString::Printf(TEXT("Could not parse %s"), *Path);
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* Viewpoints = nullptr;
		if (!Root->TryGetArrayField(TEXT("viewpoints"), Viewpoints) || !Viewpoints || Viewpoints->Num() == 0)
		{
			Error = TEXT("Review list has no viewpoints");
			return false;
		}

		for (const TSharedPtr<FJsonValue>& Value : *Viewpoints)
		{
			const TSharedPtr<FJsonObject> Object = Value->AsObject();
			FShot Shot;
			if (!Object.IsValid() || !Object->TryGetStringField(TEXT("id"), Shot.Id) || Shot.Id.IsEmpty()
				|| !ReadVector(Object, TEXT("location"), Shot.Location)
				|| !ReadRotator(Object, TEXT("rotation"), Shot.Rotation)
				|| !Object->TryGetStringField(TEXT("expectation"), Shot.Expectation)
				|| !Object->TryGetStringField(TEXT("source"), Shot.Source))
			{
				Error = TEXT("A viewpoint is missing id, location, rotation, expectation, or source");
				return false;
			}
			Object->TryGetStringField(TEXT("focus"), Shot.Focus);
			Object->TryGetStringField(TEXT("subject"), Shot.Subject);
			ReadVector(Object, TEXT("focus_offset"), Shot.FocusOffset);
			if (!ReadVector(Object, TEXT("view_offset"), Shot.ViewOffset))
			{
				Shot.ViewOffset = Shot.FocusOffset;
			}
			const TArray<TSharedPtr<FJsonValue>>* SubjectNames = nullptr;
			if (Object->TryGetArrayField(TEXT("subjects"), SubjectNames) && SubjectNames)
			{
				for (const TSharedPtr<FJsonValue>& SubjectValue : *SubjectNames)
				{
					Shot.Subjects.Add(SubjectValue->AsString());
				}
			}
			FString OffsetSpace;
			if (Object->TryGetStringField(TEXT("view_offset_space"), OffsetSpace))
			{
				Shot.bOffsetInWorld = OffsetSpace.Equals(TEXT("world"), ESearchCase::IgnoreCase);
			}
			else
			{
				Shot.bOffsetInWorld = Shot.Subjects.Num() > 0;
			}
			if (Object->HasField(TEXT("setup")))
			{
				Shot.Setup = Object->GetObjectField(TEXT("setup"));
			}
			const TArray<TSharedPtr<FJsonValue>>* Landmarks = nullptr;
			if (Object->TryGetArrayField(TEXT("landmarks"), Landmarks) && Landmarks)
			{
				for (const TSharedPtr<FJsonValue>& LandmarkValue : *Landmarks)
				{
					const TSharedPtr<FJsonObject> LandmarkObject = LandmarkValue->AsObject();
					FLandmark Landmark;
					if (!LandmarkObject.IsValid() || !LandmarkObject->TryGetStringField(TEXT("id"), Landmark.Id))
					{
						Error = FString::Printf(TEXT("Viewpoint %s has a landmark without an id"), *Shot.Id);
						return false;
					}
					LandmarkObject->TryGetStringField(TEXT("subject"), Landmark.Subject);
					if (!ReadVector(LandmarkObject, TEXT("location"), Landmark.Location) && Landmark.Subject.IsEmpty())
					{
						Error = FString::Printf(TEXT("Viewpoint %s landmark %s needs a location or a subject"), *Shot.Id, *Landmark.Id);
						return false;
					}
					Shot.Landmarks.Add(Landmark);
				}
			}
			Run.Shots.Add(Shot);
		}

		const TSharedPtr<FJsonObject>* Route = nullptr;
		if (!Root->TryGetObjectField(TEXT("route"), Route) || !Route || !Route->IsValid())
		{
			Error = TEXT("Review list has no route");
			return false;
		}
		(*Route)->TryGetStringField(TEXT("expectation"), Run.RouteExpectation);
		(*Route)->TryGetStringField(TEXT("source"), Run.RouteSource);
		double Pitch = Run.RoutePitch;
		if ((*Route)->TryGetNumberField(TEXT("look_pitch"), Pitch))
		{
			Run.RoutePitch = static_cast<float>(Pitch);
		}
		const TArray<TSharedPtr<FJsonValue>>* Points = nullptr;
		if (!(*Route)->TryGetArrayField(TEXT("points"), Points) || !Points || Points->Num() < 2)
		{
			Error = TEXT("Review route needs at least two points");
			return false;
		}
		for (const TSharedPtr<FJsonValue>& PointValue : *Points)
		{
			const TArray<TSharedPtr<FJsonValue>> Point = PointValue->AsArray();
			if (Point.Num() < 2)
			{
				Error = TEXT("A route point needs x and y");
				return false;
			}
			Run.RoutePoints.Add(FVector(Point[0]->AsNumber(), Point[1]->AsNumber(), 0.0));
		}
		for (int32 PointIndex = 1; PointIndex < Run.RoutePoints.Num(); ++PointIndex)
		{
			Run.RouteLength += FVector::Dist2D(Run.RoutePoints[PointIndex - 1], Run.RoutePoints[PointIndex]);
		}
		return true;
	}

	FString OutputDirectory()
	{
		FString Dir;
		if (FParse::Value(FCommandLine::Get(), TEXT("ReviewDir="), Dir) && !Dir.IsEmpty())
		{
			return FPaths::ConvertRelativePathToFull(Dir);
		}
		const FString Stamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H%M"));
		return FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Review") / Stamp);
	}

	void ApplyPlayTestCvars()
	{
		UWorld* World = GameWorld();
		if (!World || !GEngine)
		{
			return;
		}
		static const TCHAR* Commands[] = {
			TEXT("r.DynamicGlobalIlluminationMethod 0"),
			TEXT("r.ReflectionMethod 0"),
			TEXT("r.Shadow.Virtual.Enable 0"),
			TEXT("r.VolumetricCloud 0"),
			TEXT("r.VolumetricFog 0"),
			TEXT("r.RayTracing 0"),
			TEXT("r.Lumen.DiffuseIndirect.Allow 0"),
			TEXT("r.AntiAliasingMethod 0"),
			TEXT("r.BloomQuality 0"),
			TEXT("r.MotionBlurQuality 0"),
			TEXT("r.DepthOfFieldQuality 0"),
			TEXT("r.LensFlareQuality 0"),
			TEXT("r.AmbientOcclusionLevels 0"),
			TEXT("r.DefaultFeature.Bloom 0"),
			TEXT("r.DefaultFeature.MotionBlur 0"),
			TEXT("r.ShadowQuality 0"),
			TEXT("r.Streaming.PoolSize 400"),
			TEXT("r.ScreenPercentage 70"),
			TEXT("scalability 0"),
			TEXT("sg.ResolutionQuality 70"),
			TEXT("t.MaxFPS 60"),
		};
		for (const TCHAR* Command : Commands)
		{
			GEngine->Exec(World, Command);
		}
	}

	void UseScratchSlot()
	{
		UWorld* World = GameWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		if (UDCSaveSubsystem* Saves = GameInstance ? GameInstance->GetSubsystem<UDCSaveSubsystem>() : nullptr)
		{
			Saves->SetSlotName(TestSlot);
		}
		UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
	}

	void RestoreSlot()
	{
		UGameplayStatics::DeleteGameInSlot(TestSlot, 0);
		UWorld* World = GameWorld();
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		if (UDCSaveSubsystem* Saves = GameInstance ? GameInstance->GetSubsystem<UDCSaveSubsystem>() : nullptr)
		{
			Saves->SetSlotName(UDCSaveSubsystem::DefaultSlotName);
		}
	}

	int32 CountArtSentinels(UWorld* World)
	{
		int32 Count = 0;
		if (!World)
		{
			return 0;
		}
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->Tags.Contains(ArtTag))
			{
				++Count;
			}
		}
		return Count;
	}

	ADCInspectableActor* FindInspectable(const FString& DisplayName)
	{
		UWorld* World = GameWorld();
		if (!World)
		{
			return nullptr;
		}
		for (TActorIterator<ADCInspectableActor> It(World); It; ++It)
		{
			if (It->GetDisplayName().ToString() == DisplayName)
			{
				return *It;
			}
		}
		return nullptr;
	}

	void Fail(FRun& Run, const FString& Message)
	{
		Run.bFailed = true;
		Run.Test->AddError(Message);
		UE_LOG(LogDeadCurrent, Error, TEXT("[DCREVIEW] %s"), *Message);
	}

	bool ApplySetup(const FShot& Shot, FString& Error)
	{
		ADCPlayerCharacter* Pawn = Player();
		if (!Pawn)
		{
			Error = TEXT("No player");
			return false;
		}

		const TSharedPtr<FJsonObject> Setup = Shot.Setup;
		if (!Setup.IsValid())
		{
			return true;
		}

		if (UDCWorldStateSubsystem* WorldState = UDCWorldStateSubsystem::Get(Pawn))
		{
			const TArray<TSharedPtr<FJsonValue>>* Flags = nullptr;
			if (Setup->TryGetArrayField(TEXT("flags"), Flags) && Flags)
			{
				for (const TSharedPtr<FJsonValue>& FlagValue : *Flags)
				{
					const FName Flag(*FlagValue->AsString());
					WorldState->SetFlag(Flag);
					if (!WorldState->HasFlag(Flag))
					{
						Error = FString::Printf(TEXT("Could not set flag %s"), *Flag.ToString());
						return false;
					}
				}
			}
		}

		FString QuestId;
		FString StageId;
		Setup->TryGetStringField(TEXT("quest"), QuestId);
		Setup->TryGetStringField(TEXT("stage"), StageId);
		if (!QuestId.IsEmpty())
		{
			UDCQuestComponent* Quest = Pawn->GetQuestComponent();
			const FName QuestName(*QuestId);
			const FName StageName = StageId.IsEmpty() ? NAME_None : FName(*StageId);
			if (!Quest || !Quest->StartQuest(QuestName, StageName))
			{
				Error = FString::Printf(TEXT("Could not start quest %s at %s"), *QuestId, *StageId);
				return false;
			}
			if (!StageId.IsEmpty() && Quest->GetStage(QuestName) != StageName)
			{
				Error = FString::Printf(TEXT("Quest %s did not enter %s"), *QuestId, *StageId);
				return false;
			}
		}

		if (UDCCharacterProgressionComponent* Build = Pawn->GetProgressionComponent())
		{
			auto ApplyNumbers = [&](const TCHAR* Field, bool bPerk) -> bool
			{
				const TSharedPtr<FJsonObject>* Object = nullptr;
				if (!Setup->TryGetObjectField(Field, Object) || !Object || !Object->IsValid())
				{
					return true;
				}
				for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*Object)->Values)
				{
					const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*Pair.Key), false);
					if (!Tag.IsValid())
					{
						Error = FString::Printf(TEXT("Unknown tag %s"), *Pair.Key);
						return false;
					}
					if (bPerk)
					{
						Build->SetPerkOwned(Tag, Pair.Value->AsBool());
					}
					else if (FCString::Strcmp(Field, TEXT("attributes")) == 0)
					{
						Build->SetAttributeValue(Tag, static_cast<int32>(Pair.Value->AsNumber()));
					}
					else
					{
						Build->SetSkillValue(Tag, static_cast<int32>(Pair.Value->AsNumber()));
					}
				}
				return true;
			};
			if (!ApplyNumbers(TEXT("attributes"), false) || !ApplyNumbers(TEXT("skills"), false))
			{
				return false;
			}
			const TArray<TSharedPtr<FJsonValue>>* Perks = nullptr;
			if (Setup->TryGetArrayField(TEXT("perks"), Perks) && Perks)
			{
				for (const TSharedPtr<FJsonValue>& PerkValue : *Perks)
				{
					const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*PerkValue->AsString()), false);
					if (!Tag.IsValid())
					{
						Error = FString::Printf(TEXT("Unknown perk %s"), *PerkValue->AsString());
						return false;
					}
					Build->SetPerkOwned(Tag, true);
				}
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
		if (Setup->TryGetArrayField(TEXT("items"), Items) && Items)
		{
			for (const TSharedPtr<FJsonValue>& ItemValue : *Items)
			{
				const TSharedPtr<FJsonObject> ItemObject = ItemValue->AsObject();
				FString ItemId;
				if (!ItemObject.IsValid() || !ItemObject->TryGetStringField(TEXT("id"), ItemId))
				{
					Error = TEXT("An item entry is missing id");
					return false;
				}
				double Quantity = 1.0;
				ItemObject->TryGetNumberField(TEXT("quantity"), Quantity);
				const UDCItemDefinition* Definition = UDCItemDefinition::FindByItemId(FName(*ItemId));
				if (!Definition || !Pawn->GetInventoryComponent()
					|| Pawn->GetInventoryComponent()->AddItem(Definition, static_cast<int32>(Quantity)) <= 0)
				{
					Error = FString::Printf(TEXT("Could not carry %s"), *ItemId);
					return false;
				}
			}
		}
		return true;
	}

	void LockCamera(ADCPlayerCharacter* Pawn, const FVector& Eye, const FRotator& Rotation)
	{
		Pawn->SetActorLocation(Eye - FVector(0.0, 0.0, 64.0), false, nullptr, ETeleportType::TeleportPhysics);
		Pawn->SetActorRotation(FRotator(0.0, Rotation.Yaw, 0.0));
		if (UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Flying);
		}
		if (APlayerController* Controller = Cast<APlayerController>(Pawn->GetController()))
		{
			Controller->SetControlRotation(Rotation);
			Controller->SetIgnoreLookInput(true);
			Controller->SetIgnoreMoveInput(true);
		}
		if (UCameraComponent* Camera = Pawn->GetFirstPersonCameraComponent())
		{
			Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			Camera->bUsePawnControlRotation = false;
			Camera->SetAbsolute(true, true, false);
			Camera->SetWorldLocationAndRotation(Eye, Rotation);
		}
	}

	FString ActorLabel(const AActor* Actor)
	{
		if (!Actor)
		{
			return FString();
		}
#if WITH_EDITOR
		const FString Label = Actor->GetActorLabel();
		if (!Label.IsEmpty())
		{
			return Label;
		}
#endif
		return Actor->GetName();
	}

	AActor* FindByLabel(const FString& Label)
	{
		UWorld* World = GameWorld();
		if (!World || Label.IsEmpty())
		{
			return nullptr;
		}
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (ActorLabel(*It).Equals(Label, ESearchCase::IgnoreCase))
			{
				return *It;
			}
		}
		return nullptr;
	}

	AActor* FindSubjectActor(const FString& Name)
	{
		if (Name.IsEmpty())
		{
			return nullptr;
		}
		if (ADCInspectableActor* Inspectable = FindInspectable(Name))
		{
			return Inspectable;
		}
		if (UWorld* World = GameWorld())
		{
			if (const UDCPersistentRegistry* Registry = World->GetSubsystem<UDCPersistentRegistry>())
			{
				if (AActor* Actor = Registry->FindActor(FName(*Name)))
				{
					return Actor;
				}
			}
		}
		return FindByLabel(Name);
	}

	bool SubjectBounds(const FShot& Shot, FVector& OutCenter, FVector& OutExtent, TArray<AActor*>& OutActors, FQuat& OutRotation, FString& Error)
	{
		OutActors.Reset();
		if (Shot.Subjects.Num() > 0)
		{
			FBox Box(ForceInit);
			for (const FString& Label : Shot.Subjects)
			{
				AActor* Actor = FindByLabel(Label);
				if (!Actor)
				{
					Error = FString::Printf(TEXT("Could not find %s"), *Label);
					return false;
				}
				FVector Center;
				FVector Extent;
				Actor->GetActorBounds(false, Center, Extent);
				Box += FBox::BuildAABB(Center, Extent);
				OutActors.Add(Actor);
			}
			OutCenter = Box.GetCenter();
			OutExtent = Box.GetExtent();
			OutRotation = FQuat::Identity;
			return true;
		}

		const FString Name = !Shot.Subject.IsEmpty() ? Shot.Subject : Shot.Focus;
		AActor* Actor = FindSubjectActor(Name);
		if (!Actor)
		{
			Error = FString::Printf(TEXT("Could not find %s"), *Name);
			return false;
		}
		Actor->GetActorBounds(false, OutCenter, OutExtent);
		OutRotation = Actor->GetActorQuat();
		OutActors.Add(Actor);
		return true;
	}

	void AimAtSubject(UWorld* World, const FVector& Center, const TArray<AActor*>& Actors, const FQuat& Rotation, const FVector& Offset, bool bWorldOffset, FVector& OutEye, FRotator& OutRotation)
	{
		FVector WorldOffset = bWorldOffset ? Offset : Rotation.RotateVector(Offset);
		if (WorldOffset.IsNearlyZero())
		{
			WorldOffset = FVector(200.0, 0.0, 40.0);
		}
		const FVector Direction = WorldOffset.GetSafeNormal();
		FVector Eye = Center + WorldOffset;

		FCollisionQueryParams Params(SCENE_QUERY_STAT(ReviewAim), false);
		for (AActor* Actor : Actors)
		{
			Params.AddIgnoredActor(Actor);
		}
		if (ADCPlayerCharacter* Pawn = Player())
		{
			Params.AddIgnoredActor(Pawn);
		}
		const FCollisionShape Sphere = FCollisionShape::MakeSphere(16.0f);
		for (int32 Step = 0; Step < 16 && World->OverlapBlockingTestByChannel(Eye, FQuat::Identity, ECC_WorldStatic, Sphere, Params); ++Step)
		{
			Eye += Direction * 40.0f;
		}
		OutEye = Eye;
		OutRotation = (Center - OutEye).Rotation();
	}

	void ClearTransientHud(bool bKeepMessage)
	{
		ADCPlayerCharacter* Pawn = Player();
		APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
		ADCHUD* HUD = Controller ? Controller->GetHUD<ADCHUD>() : nullptr;
		if (!HUD)
		{
			return;
		}
		if (bKeepMessage)
		{
			HUD->ClearBanner();
			return;
		}
		HUD->ClearTransient();
	}

	bool PlaceShot(const FShot& Shot, FVector& OutEye, FRotator& OutRotation, FString& Error)
	{
		ADCPlayerCharacter* Pawn = Player();
		if (!Pawn)
		{
			Error = TEXT("No player");
			return false;
		}

		OutEye = Shot.Location;
		OutRotation = Shot.Rotation;
		const bool bHasSubject = !Shot.Subject.IsEmpty() || !Shot.Focus.IsEmpty() || Shot.Subjects.Num() > 0;
		if (bHasSubject)
		{
			FVector Center;
			FVector Extent;
			TArray<AActor*> Actors;
			FQuat Rotation;
			if (!SubjectBounds(Shot, Center, Extent, Actors, Rotation, Error))
			{
				return false;
			}
			const FVector Offset = Shot.ViewOffset.IsNearlyZero() ? Shot.FocusOffset : Shot.ViewOffset;
			AimAtSubject(Pawn->GetWorld(), Center, Actors, Rotation, Offset, Shot.bOffsetInWorld, OutEye, OutRotation);
		}
		LockCamera(Pawn, OutEye, OutRotation);

		FString InspectName;
		if (Shot.Setup.IsValid() && Shot.Setup->TryGetStringField(TEXT("inspect"), InspectName) && !InspectName.IsEmpty())
		{
			ADCInspectableActor* Target = FindInspectable(InspectName);
			if (!Target)
			{
				Error = FString::Printf(TEXT("Could not inspect %s"), *InspectName);
				return false;
			}
			IDCInteractable::Execute_Interact(Target, Pawn);
		}
		return true;
	}

	FVector PointOnRoute(const TArray<FVector>& Points, float Distance, FVector& OutForward)
	{
		float Remaining = Distance;
		for (int32 PointIndex = 1; PointIndex < Points.Num(); ++PointIndex)
		{
			const FVector From = Points[PointIndex - 1];
			const FVector To = Points[PointIndex];
			const float Segment = FVector::Dist2D(From, To);
			if (Remaining <= Segment || PointIndex == Points.Num() - 1)
			{
				const float Alpha = Segment > 1.0f ? FMath::Clamp(Remaining / Segment, 0.0f, 1.0f) : 1.0f;
				OutForward = (To - From).GetSafeNormal2D();
				if (OutForward.IsNearlyZero())
				{
					OutForward = FVector::ForwardVector;
				}
				return FMath::Lerp(From, To, Alpha);
			}
			Remaining -= Segment;
		}
		OutForward = FVector::ForwardVector;
		return Points.Last();
	}

	bool PlaceOnRoute(FRun& Run, FVector& OutEye, FRotator& OutRotation)
	{
		ADCPlayerCharacter* Pawn = Player();
		UWorld* World = GameWorld();
		if (!Pawn || !World || !Pawn->GetCapsuleComponent())
		{
			return false;
		}
		FVector Forward;
		FVector Feet = PointOnRoute(Run.RoutePoints, Run.RouteDistance, Forward);
		const FVector Probe(Feet.X, Feet.Y, Feet.Z + 400.0);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Probe, Probe - FVector(0.0, 0.0, 1200.0), ECC_WorldStatic))
		{
			Feet.Z = Hit.ImpactPoint.Z;
		}
		const float HalfHeight = Pawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector ActorLocation(Feet.X, Feet.Y, Feet.Z + HalfHeight + 2.0);
		OutRotation = FRotator(Run.RoutePitch, Forward.Rotation().Yaw, 0.0);
		OutEye = ActorLocation + FVector(0.0, 0.0, 64.0);
		LockCamera(Pawn, OutEye, OutRotation);
		Pawn->SetActorLocation(ActorLocation, false, nullptr, ETeleportType::TeleportPhysics);
		if (UCameraComponent* Camera = Pawn->GetFirstPersonCameraComponent())
		{
			Camera->SetWorldLocationAndRotation(OutEye, OutRotation);
		}
		return true;
	}

	void ReadCamera(FVector& OutLocation, FRotator& OutRotation)
	{
		if (ADCPlayerCharacter* Pawn = Player())
		{
			if (const UCameraComponent* Camera = Pawn->GetFirstPersonCameraComponent())
			{
				OutLocation = Camera->GetComponentLocation();
				OutRotation = Camera->GetComponentRotation();
				return;
			}
		}
		OutLocation = FVector::ZeroVector;
		OutRotation = FRotator::ZeroRotator;
	}

	void SetVectorArray(TSharedRef<FJsonObject> Object, const TCHAR* Field, const FVector& Value)
	{
		TArray<TSharedPtr<FJsonValue>> Numbers;
		Numbers.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Value.X * 10.0) / 10.0));
		Numbers.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Value.Y * 10.0) / 10.0));
		Numbers.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Value.Z * 10.0) / 10.0));
		Object->SetArrayField(Field, Numbers);
	}

	void SetRotatorArray(TSharedRef<FJsonObject> Object, const TCHAR* Field, const FRotator& Value)
	{
		TArray<TSharedPtr<FJsonValue>> Numbers;
		Numbers.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Value.Pitch * 10.0) / 10.0));
		Numbers.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Value.Yaw * 10.0) / 10.0));
		Numbers.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(Value.Roll * 10.0) / 10.0));
		Object->SetArrayField(Field, Numbers);
	}

	void BeginViewpointCapture(FRun& Run)
	{
		Run.FrameSamples.Reset();
		Run.DefaultMaterials.Reset();
		Run.MissingTextures.Reset();
		Run.LandmarkResults.Reset();
		Run.PendingFrameMs = 0.0f;
		if (!Run.Log.IsValid())
		{
			return;
		}
		Run.Log->ResetLists();
		if (!Run.bLogAttached)
		{
			GLog->AddOutputDevice(Run.Log.Get());
			Run.bLogAttached = true;
		}
		Run.Log->SetActive(true);
	}

	void StopViewpointCapture(FRun& Run)
	{
		if (Run.Log.IsValid())
		{
			Run.Log->SetActive(false);
		}
	}

	void DetachViewpointCapture(FRun& Run)
	{
		StopViewpointCapture(Run);
		if (Run.bLogAttached && Run.Log.IsValid())
		{
			GLog->RemoveOutputDevice(Run.Log.Get());
			Run.bLogAttached = false;
		}
	}

	float AverageFrameMs(const TArray<double>& Samples)
	{
		if (Samples.Num() == 0)
		{
			return 0.0f;
		}
		double Sum = 0.0;
		for (const double Sample : Samples)
		{
			Sum += Sample;
		}
		return static_cast<float>(Sum / Samples.Num() * 1000.0);
	}

	bool PointInFrustum(const FVector& WorldPoint)
	{
		ADCPlayerCharacter* Pawn = Player();
		APlayerController* Controller = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
		UCameraComponent* Camera = Pawn ? Pawn->GetFirstPersonCameraComponent() : nullptr;
		if (!Controller || !Camera)
		{
			return false;
		}
		const FVector ToPoint = WorldPoint - Camera->GetComponentLocation();
		if (FVector::DotProduct(ToPoint, Camera->GetForwardVector()) <= 0.0)
		{
			return false;
		}
		FVector2D Screen;
		if (!Controller->ProjectWorldLocationToScreen(WorldPoint, Screen, true))
		{
			return false;
		}
		int32 SizeX = 0;
		int32 SizeY = 0;
		Controller->GetViewportSize(SizeX, SizeY);
		return SizeX > 0 && SizeY > 0 && Screen.X >= 0.0 && Screen.Y >= 0.0 && Screen.X <= SizeX && Screen.Y <= SizeY;
	}

	bool IsDefaultOrGridMaterial(const UMaterialInterface* Material)
	{
		if (!Material)
		{
			return true;
		}
		const FString Path = Material->GetPathName();
		return Path.Contains(TEXT("WorldGridMaterial"))
			|| Path.Contains(TEXT("PrototypeGrid"))
			|| Path.Contains(TEXT("DefaultMaterial"))
			|| Path.Contains(TEXT("DefaultColorway"))
			|| Path.Contains(TEXT("BasicShapeMaterial"));
	}

	bool OwnedByPlayer(const AActor* Actor)
	{
		for (const AActor* Cursor = Actor; Cursor; Cursor = Cursor->GetOwner())
		{
			if (Cursor->IsA<ADCPlayerCharacter>())
			{
				return true;
			}
		}
		return false;
	}

	void NoteUnique(TArray<FString>& Lines, const FString& Line)
	{
		if (Lines.Num() >= 40 || Lines.Contains(Line))
		{
			return;
		}
		Lines.Add(Line);
	}

	void GatherSceneChecks(FRun& Run)
	{
		Run.DefaultMaterials.Reset();
		Run.MissingTextures.Reset();
		Run.LandmarkResults.Reset();
		UWorld* World = GameWorld();
		ADCPlayerCharacter* Pawn = Player();
		if (!World || !Pawn || Run.Index >= Run.Shots.Num())
		{
			return;
		}

		for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
		{
			if (OwnedByPlayer(*ActorIt))
			{
				continue;
			}
			TInlineComponentArray<UPrimitiveComponent*> Primitives;
			ActorIt->GetComponents(Primitives);
			for (UPrimitiveComponent* Primitive : Primitives)
			{
				if (!Primitive || !Primitive->IsVisible() || Primitive->bHiddenInGame)
				{
					continue;
				}
				if (!Primitive->WasRecentlyRendered(1.0f) || !PointInFrustum(Primitive->Bounds.Origin))
				{
					continue;
				}
				if (Primitive->bOnlyOwnerSee || Primitive->bOwnerNoSee)
				{
					continue;
				}
				const int32 MaterialCount = FMath::Max(Primitive->GetNumMaterials(), 1);
				for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
				{
					UMaterialInterface* Material = Primitive->GetMaterial(MaterialIndex);
					if (IsDefaultOrGridMaterial(Material))
					{
						const FString Who = FString::Printf(TEXT("%s (%s)"), *Material->GetPathName(), *ActorIt->GetActorLabel());
						NoteUnique(Run.DefaultMaterials, Who);
						continue;
					}
					if (!Material)
					{
						continue;
					}
					TArray<UTexture*> Textures;
					Material->GetUsedTextures(Textures, EMaterialQualityLevel::Low);
					for (UTexture* Texture : Textures)
					{
						if (!Texture)
						{
							NoteUnique(Run.MissingTextures, Material->GetPathName() + TEXT(" -> (null texture)"));
							continue;
						}
						const FString TexturePath = Texture->GetPathName();
						if (TexturePath.Contains(TEXT("DefaultTexture")) || TexturePath.Contains(TEXT("DefaultDiffuse"))
							|| TexturePath.Contains(TEXT("DefaultNormal")))
						{
							NoteUnique(Run.MissingTextures, Material->GetPathName() + TEXT(" -> ") + TexturePath);
						}
					}
					if (const UMaterialInstance* Instance = Cast<UMaterialInstance>(Material))
					{
						for (const FTextureParameterValue& Parameter : Instance->TextureParameterValues)
						{
							if (!Parameter.ParameterValue)
							{
								NoteUnique(Run.MissingTextures, Material->GetPathName() + TEXT(" -> ") + Parameter.ParameterInfo.Name.ToString());
							}
						}
					}
				}
			}
		}

		const UCameraComponent* Camera = Pawn->GetFirstPersonCameraComponent();
		const FVector CameraLocation = Camera ? Camera->GetComponentLocation() : Pawn->GetActorLocation();
		for (const FLandmark& Landmark : Run.Shots[Run.Index].Landmarks)
		{
			AActor* LandmarkActor = FindSubjectActor(Landmark.Subject);
			FVector Target = Landmark.Location;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(ReviewLandmark), false);
			Params.AddIgnoredActor(Pawn);
			if (LandmarkActor)
			{
				FVector Center;
				FVector Extent;
				LandmarkActor->GetActorBounds(false, Center, Extent);
				const FVector TowardCamera = (CameraLocation - Center).GetSafeNormal();
				Target = Center + FVector(TowardCamera.X * Extent.X, TowardCamera.Y * Extent.Y, TowardCamera.Z * Extent.Z);
				Params.AddIgnoredActor(LandmarkActor);
			}
			for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
			{
				FVector Center;
				FVector Extent;
				ActorIt->GetActorBounds(false, Center, Extent);
				const FVector Delta = Target - Center;
				if (FMath::Abs(Delta.X) <= Extent.X + 20.0f && FMath::Abs(Delta.Y) <= Extent.Y + 20.0f && FMath::Abs(Delta.Z) <= Extent.Z + 40.0f)
				{
					Params.AddIgnoredActor(*ActorIt);
				}
			}
			FHitResult Hit;
			const bool bHit = World->LineTraceSingleByChannel(Hit, CameraLocation, Target, ECC_Visibility, Params);
			const float TargetDistance = FVector::Dist(CameraLocation, Target);
			const bool bUnblocked = !bHit || (TargetDistance - Hit.Distance) <= 80.0f;
			TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
			Result->SetStringField(TEXT("id"), Landmark.Id);
			Result->SetBoolField(TEXT("in_frustum"), PointInFrustum(Target));
			Result->SetBoolField(TEXT("trace_unblocked"), bUnblocked);
			Run.LandmarkResults.Add(MakeShared<FJsonValueObject>(Result));
		}
	}

	TArray<TSharedPtr<FJsonValue>> StringArray(const TArray<FString>& Lines)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		for (const FString& Line : Lines)
		{
			Values.Add(MakeShared<FJsonValueString>(Line));
		}
		return Values;
	}

	TSharedRef<FJsonObject> BuildChecks(FRun& Run)
	{
		TSharedRef<FJsonObject> Checks = MakeShared<FJsonObject>();
		Checks->SetArrayField(TEXT("default_or_grid_materials"), StringArray(Run.DefaultMaterials));
		Checks->SetArrayField(TEXT("missing_textures"), StringArray(Run.MissingTextures));
		TArray<FString> Pool;
		TArray<FString> Warnings;
		TArray<FString> Errors;
		if (Run.Log.IsValid())
		{
			FScopeLock Lock(&Run.Log->Mutex);
			Pool = Run.Log->Pool;
			Warnings = Run.Log->Warnings;
			Errors = Run.Log->Errors;
		}
		Checks->SetArrayField(TEXT("texture_pool_warnings"), StringArray(Pool));
		Checks->SetArrayField(TEXT("warnings"), StringArray(Warnings));
		Checks->SetArrayField(TEXT("errors"), StringArray(Errors));
		Checks->SetArrayField(TEXT("landmarks"), Run.LandmarkResults);
		return Checks;
	}

	void PlotGlyph(TArray<FColor>& Pixels, int32 Width, int32 Height, int32 X, int32 Y, TCHAR Char, const FColor& Ink)
	{
		static const TCHAR* Alphabet = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
		static const uint8 Rows[43][7] = {
			{0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}, {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}, {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
			{0x1E,0x11,0x11,0x11,0x11,0x11,0x1E}, {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}, {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},
			{0x0E,0x11,0x10,0x17,0x11,0x11,0x0E}, {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}, {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},
			{0x01,0x01,0x01,0x01,0x11,0x11,0x0E}, {0x11,0x12,0x14,0x18,0x14,0x12,0x11}, {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
			{0x11,0x1B,0x15,0x15,0x11,0x11,0x11}, {0x11,0x19,0x15,0x13,0x11,0x11,0x11}, {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
			{0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}, {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}, {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},
			{0x0E,0x11,0x10,0x0E,0x01,0x11,0x0E}, {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}, {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},
			{0x11,0x11,0x11,0x11,0x11,0x0A,0x04}, {0x11,0x11,0x11,0x15,0x15,0x1B,0x11}, {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},
			{0x11,0x11,0x0A,0x04,0x04,0x04,0x04}, {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},
			{0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}, {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}, {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},
			{0x1F,0x01,0x02,0x06,0x01,0x11,0x0E}, {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}, {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},
			{0x06,0x08,0x10,0x1E,0x11,0x11,0x0E}, {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}, {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},
			{0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C}, {0x00,0x00,0x00,0x00,0x00,0x00,0x1F}
		};
		const int32 Index = FCString::Strchr(Alphabet, FChar::ToUpper(Char)) ? static_cast<int32>(FCString::Strchr(Alphabet, FChar::ToUpper(Char)) - Alphabet) : INDEX_NONE;
		if (Index < 0 || Index >= 43)
		{
			return;
		}
		for (int32 Row = 0; Row < 7; ++Row)
		{
			for (int32 Col = 0; Col < 5; ++Col)
			{
				if ((Rows[Index][Row] & (1 << (4 - Col))) == 0)
				{
					continue;
				}
				const int32 PixelX = X + Col;
				const int32 PixelY = Y + Row;
				if (PixelX >= 0 && PixelY >= 0 && PixelX < Width && PixelY < Height)
				{
					Pixels[PixelY * Width + PixelX] = Ink;
				}
			}
		}
	}

	void DrawLabel(TArray<FColor>& Pixels, int32 Width, int32 Height, int32 X, int32 Y, const FString& Text)
	{
		int32 Cursor = X;
		for (const TCHAR Char : Text)
		{
			PlotGlyph(Pixels, Width, Height, Cursor, Y, Char, FColor::White);
			Cursor += 6;
		}
	}

	bool LoadPng(const FString& Path, TArray<FColor>& OutPixels, int32& OutWidth, int32& OutHeight)
	{
		TArray<uint8> Compressed;
		if (!FFileHelper::LoadFileToArray(Compressed, *Path))
		{
			return false;
		}
		IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(EImageFormat::PNG);
		if (!Wrapper.IsValid() || !Wrapper->SetCompressed(Compressed.GetData(), Compressed.Num()))
		{
			return false;
		}
		TArray<uint8> Raw;
		if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, Raw))
		{
			return false;
		}
		OutWidth = Wrapper->GetWidth();
		OutHeight = Wrapper->GetHeight();
		OutPixels.SetNum(OutWidth * OutHeight);
		FMemory::Memcpy(OutPixels.GetData(), Raw.GetData(), Raw.Num());
		return OutWidth > 0 && OutHeight > 0;
	}

	void BlitThumb(TArray<FColor>& Sheet, int32 SheetWidth, const TArray<FColor>& Source, int32 SourceWidth, int32 SourceHeight, int32 DestX, int32 DestY, int32 ThumbWidth, int32 ThumbHeight)
	{
		for (int32 Y = 0; Y < ThumbHeight; ++Y)
		{
			const int32 SourceY0 = Y * SourceHeight / ThumbHeight;
			const int32 SourceY1 = FMath::Max(SourceY0 + 1, (Y + 1) * SourceHeight / ThumbHeight);
			for (int32 X = 0; X < ThumbWidth; ++X)
			{
				const int32 SourceX0 = X * SourceWidth / ThumbWidth;
				const int32 SourceX1 = FMath::Max(SourceX0 + 1, (X + 1) * SourceWidth / ThumbWidth);
				int32 Red = 0;
				int32 Green = 0;
				int32 Blue = 0;
				int32 Count = 0;
				for (int32 SourceY = SourceY0; SourceY < SourceY1; ++SourceY)
				{
					for (int32 SourceX = SourceX0; SourceX < SourceX1; ++SourceX)
					{
						const FColor& Pixel = Source[SourceY * SourceWidth + SourceX];
						Red += Pixel.R;
						Green += Pixel.G;
						Blue += Pixel.B;
						++Count;
					}
				}
				if (Count > 0)
				{
					Sheet[(DestY + Y) * SheetWidth + DestX + X] = FColor(Red / Count, Green / Count, Blue / Count);
				}
			}
		}
	}

	void WriteContactSheet(const FRun& Run)
	{
		const int32 Columns = 4;
		const int32 ThumbWidth = 320;
		const int32 ThumbHeight = 180;
		const int32 LabelHeight = 14;
		const int32 Pad = 8;
		const int32 CellHeight = ThumbHeight + LabelHeight;
		const int32 Rows = FMath::DivideAndRoundUp(Run.Shots.Num(), Columns);
		const int32 SheetWidth = Columns * ThumbWidth + (Columns + 1) * Pad;
		const int32 SheetHeight = Rows * CellHeight + (Rows + 1) * Pad;
		TArray<FColor> Sheet;
		Sheet.Init(FColor(18, 18, 18), SheetWidth * SheetHeight);

		for (int32 Index = 0; Index < Run.Shots.Num(); ++Index)
		{
			const int32 Column = Index % Columns;
			const int32 Row = Index / Columns;
			const int32 DestX = Pad + Column * (ThumbWidth + Pad);
			const int32 DestY = Pad + Row * (CellHeight + Pad);
			TArray<FColor> Pixels;
			int32 SourceWidth = 0;
			int32 SourceHeight = 0;
			if (LoadPng(Run.OutDir / (Run.Shots[Index].Id + TEXT(".png")), Pixels, SourceWidth, SourceHeight))
			{
				BlitThumb(Sheet, SheetWidth, Pixels, SourceWidth, SourceHeight, DestX, DestY, ThumbWidth, ThumbHeight);
			}
			DrawLabel(Sheet, SheetWidth, SheetHeight, DestX, DestY + ThumbHeight + 3, Run.Shots[Index].Id);
		}

		IImageWrapperModule& Module = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
		const TSharedPtr<IImageWrapper> Wrapper = Module.CreateImageWrapper(EImageFormat::PNG);
		if (!Wrapper.IsValid())
		{
			return;
		}
		Wrapper->SetRaw(Sheet.GetData(), Sheet.Num() * sizeof(FColor), SheetWidth, SheetHeight, ERGBFormat::BGRA, 8);
		const TArray64<uint8> Compressed = Wrapper->GetCompressed(100);
		FFileHelper::SaveArrayToFile(Compressed, *(Run.OutDir / TEXT("contact_sheet.png")));
	}

	void WriteManifest(const FRun& Run)
	{
		TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
		Root->SetStringField(TEXT("map"), MapPath);
		Root->SetStringField(TEXT("render"), TEXT("Tools/PlayTest.bat cvars, scalability 0, 1280x720"));
		Root->SetStringField(TEXT("contact_sheet"), TEXT("contact_sheet.png"));
		Root->SetArrayField(TEXT("viewpoints"), Run.ViewpointResults);
		TSharedRef<FJsonObject> Route = MakeShared<FJsonObject>();
		Route->SetStringField(TEXT("expectation"), Run.RouteExpectation);
		Route->SetStringField(TEXT("source"), Run.RouteSource);
		Route->SetArrayField(TEXT("frames"), Run.RouteResults);
		Root->SetObjectField(TEXT("route"), Route);

		FString Text;
		const TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
			TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Text);
		FJsonSerializer::Serialize(Root, Writer);
		IFileManager::Get().MakeDirectory(*Run.OutDir, true);
		FFileHelper::SaveStringToFile(Text, *(Run.OutDir / TEXT("manifest.json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	TSharedRef<FJsonObject> ShotRecord(const FShot& Shot, const FString& ImageFile, const FVector& CameraLocation, const FRotator& CameraRotation, bool bCaptured, const FString& Error)
	{
		TSharedRef<FJsonObject> Record = MakeShared<FJsonObject>();
		Record->SetStringField(TEXT("id"), Shot.Id);
		Record->SetObjectField(TEXT("setup"), Shot.Setup.IsValid() ? Shot.Setup : TSharedPtr<FJsonObject>(MakeShared<FJsonObject>()));
		Record->SetStringField(TEXT("expectation"), Shot.Expectation);
		Record->SetStringField(TEXT("source"), Shot.Source);
		Record->SetStringField(TEXT("image"), ImageFile);
		TSharedRef<FJsonObject> Camera = MakeShared<FJsonObject>();
		SetVectorArray(Camera, TEXT("location"), CameraLocation);
		SetRotatorArray(Camera, TEXT("rotation"), CameraRotation);
		Record->SetObjectField(TEXT("camera"), Camera);
		Record->SetBoolField(TEXT("captured"), bCaptured);
		if (!Error.IsEmpty())
		{
			Record->SetStringField(TEXT("error"), Error);
		}
		return Record;
	}

	void AddCaptureChecks(const TSharedRef<FJsonObject>& Record, float FrameTimeMs, const TSharedPtr<FJsonObject>& Checks)
	{
		if (!Checks.IsValid())
		{
			return;
		}
		Record->SetNumberField(TEXT("frame_time_ms"), FMath::RoundToDouble(FrameTimeMs * 10.0) / 10.0);
		Record->SetObjectField(TEXT("checks"), Checks);
	}

	bool FileReady(const FString& Path)
	{
		return IFileManager::Get().FileExists(*Path) && IFileManager::Get().FileSize(*Path) > 8000;
	}

	void BeginOpen(FRun& Run)
	{
		UseScratchSlot();
		Run.PreviousWorld = GameWorld();
		if (UWorld* World = GameWorld(); World && GEngine)
		{
			GEngine->Exec(World, *FString::Printf(TEXT("Open %s"), MapPath));
		}
		Run.PhaseStart = FPlatformTime::Seconds();
	}

	bool WorldReady(const FRun& Run)
	{
		UWorld* World = GameWorld();
		return World && World != Run.PreviousWorld.Get() && World->HasBegunPlay() && Player() != nullptr
			&& CountArtSentinels(World) == 1;
	}

	bool Tick(TSharedRef<FRun> RunRef)
	{
		FRun& Run = RunRef.Get();
		const double Now = FPlatformTime::Seconds();

		switch (Run.Phase)
		{
		case FRun::EPhase::Boot:
			if (!GameWorld() || !Player())
			{
				if (Run.PhaseStart == 0.0)
				{
					Run.PhaseStart = Now;
				}
				if (Now - Run.PhaseStart > MapTimeoutSeconds)
				{
					Fail(Run, TEXT("Timed out waiting for a game world"));
					Run.Phase = FRun::EPhase::Finish;
				}
				return false;
			}
			UseScratchSlot();
			ApplyPlayTestCvars();
			if (ADCPlayerCharacter* Pawn = Player())
			{
				if (const UCharacterMovementComponent* Movement = Pawn->GetCharacterMovement())
				{
					Run.WalkSpeed = Movement->MaxWalkSpeed;
				}
			}
			IFileManager::Get().MakeDirectory(*Run.OutDir, true);
			UE_LOG(LogDeadCurrent, Display, TEXT("[DCREVIEW] writing %s"), *Run.OutDir);
			Run.Index = 0;
			Run.Phase = FRun::EPhase::Open;
			BeginOpen(Run);
			Run.Phase = FRun::EPhase::Wait;
			return false;

		case FRun::EPhase::Wait:
		case FRun::EPhase::RouteWait:
			if (!WorldReady(Run))
			{
				if (Now - Run.PhaseStart > MapTimeoutSeconds)
				{
					const int32 Sentinels = CountArtSentinels(GameWorld());
					Fail(Run, FString::Printf(TEXT("Timed out loading %s (art sentinels=%d, player=%s)"),
						MapPath, Sentinels, Player() ? TEXT("yes") : TEXT("no")));
					Run.Phase = FRun::EPhase::Finish;
				}
				return false;
			}
			ApplyPlayTestCvars();
			Run.Frames = 0;
			Run.PhaseStart = Now;
			Run.Phase = Run.Phase == FRun::EPhase::RouteWait ? FRun::EPhase::RouteSettle : FRun::EPhase::Setup;
			return false;

		case FRun::EPhase::Setup:
		{
			const FShot& Shot = Run.Shots[Run.Index];
			FString Error;
			FVector Eye;
			FRotator Rotation;
			if (!ApplySetup(Shot, Error) || !PlaceShot(Shot, Eye, Rotation, Error))
			{
				Fail(Run, FString::Printf(TEXT("%s: %s"), *Shot.Id, *Error));
				Run.ViewpointResults.Add(MakeShared<FJsonValueObject>(ShotRecord(Shot, Shot.Id + TEXT(".png"), FVector::ZeroVector, FRotator::ZeroRotator, false, Error)));
				WriteManifest(Run);
				++Run.Index;
				Run.Phase = Run.Index < Run.Shots.Num() ? FRun::EPhase::Open : FRun::EPhase::RouteOpen;
				if (Run.Phase == FRun::EPhase::Open)
				{
					BeginOpen(Run);
					Run.Phase = FRun::EPhase::Wait;
				}
				return false;
			}
			Run.Frames = 0;
			Run.PhaseStart = Now;
			BeginViewpointCapture(Run);
			Run.Phase = FRun::EPhase::Settle;
			return false;
		}

		case FRun::EPhase::Settle:
			Run.FrameSamples.Add(FApp::GetDeltaTime());
			++Run.Frames;
			if (Run.Frames < SettleFrames || Now - Run.PhaseStart < 0.5)
			{
				return false;
			}
			Run.PendingFrameMs = AverageFrameMs(Run.FrameSamples);
			GatherSceneChecks(Run);
			Run.Phase = FRun::EPhase::Shot;
			return false;

		case FRun::EPhase::RouteSettle:
			if (Run.Frames == 0)
			{
				Run.FrameSamples.Reset();
				FVector Eye;
				FRotator Rotation;
				if (!PlaceOnRoute(Run, Eye, Rotation))
				{
					Fail(Run, TEXT("Could not place the route camera"));
					Run.Phase = FRun::EPhase::Finish;
					return false;
				}
			}
			Run.FrameSamples.Add(FApp::GetDeltaTime());
			++Run.Frames;
			if (Run.Frames < SettleFrames || Now - Run.PhaseStart < 0.5)
			{
				return false;
			}
			Run.PendingFrameMs = AverageFrameMs(Run.FrameSamples);
			Run.Phase = FRun::EPhase::RouteShot;
			return false;

		case FRun::EPhase::Shot:
		case FRun::EPhase::RouteShot:
		{
			const bool bRoute = Run.Phase == FRun::EPhase::RouteShot;
			FString ImageName;
			FVector Eye = FVector::ZeroVector;
			FRotator Rotation = FRotator::ZeroRotator;
			if (bRoute)
			{
				ImageName = FString::Printf(TEXT("route_%02d.png"), Run.RouteResults.Num());
				if (!PlaceOnRoute(Run, Eye, Rotation))
				{
					Fail(Run, TEXT("Could not place the route camera"));
					Run.Phase = FRun::EPhase::Finish;
					return false;
				}
			}
			else
			{
				const FShot& Shot = Run.Shots[Run.Index];
				ImageName = Shot.Id + TEXT(".png");
				FString Error;
				if (!PlaceShot(Shot, Eye, Rotation, Error))
				{
					Fail(Run, FString::Printf(TEXT("%s: %s"), *Shot.Id, *Error));
					Run.Phase = FRun::EPhase::Finish;
					return false;
				}
			}
			if (UCameraComponent* Camera = Player() ? Player()->GetFirstPersonCameraComponent() : nullptr)
			{
				Camera->SetWorldLocationAndRotation(Eye, Rotation);
			}
			ClearTransientHud(!bRoute && Run.Shots[Run.Index].Id == TEXT("hud_inspect"));
			const FString ImagePath = Run.OutDir / ImageName;
			FScreenshotRequest::RequestScreenshot(ImagePath, true, false, false);
			Run.PhaseStart = Now;
			Run.Phase = bRoute ? FRun::EPhase::RouteWaitFile : FRun::EPhase::WaitFile;
			return false;
		}

		case FRun::EPhase::WaitFile:
		case FRun::EPhase::RouteWaitFile:
		{
			const bool bRoute = Run.Phase == FRun::EPhase::RouteWaitFile;
			const FString ImageName = bRoute
				? FString::Printf(TEXT("route_%02d.png"), Run.RouteResults.Num())
				: Run.Shots[Run.Index].Id + TEXT(".png");
			const FString ImagePath = Run.OutDir / ImageName;
			const FString Requested = FScreenshotRequest::GetFilename();
			const bool bRequestDone = !FScreenshotRequest::IsScreenshotRequested();
			const bool bReady = FileReady(ImagePath) || (!Requested.IsEmpty() && FileReady(Requested));
			if (!(bRequestDone && bReady))
			{
				if (Now - Run.PhaseStart > ShotTimeoutSeconds)
				{
					StopViewpointCapture(Run);
					const FString Message = FString::Printf(TEXT("Screenshot was not written: %s"), *ImagePath);
					Fail(Run, Message);
					if (bRoute)
					{
						Run.Phase = FRun::EPhase::Finish;
					}
					else
					{
						Run.ViewpointResults.Add(MakeShared<FJsonValueObject>(ShotRecord(
							Run.Shots[Run.Index], ImageName, FVector::ZeroVector, FRotator::ZeroRotator, false, Message)));
						++Run.Index;
						Run.Phase = Run.Index < Run.Shots.Num() ? FRun::EPhase::Open : FRun::EPhase::RouteOpen;
					}
					WriteManifest(Run);
				}
				return false;
			}
			if (!FileReady(ImagePath) && FileReady(Requested))
			{
				IFileManager::Get().Copy(*ImagePath, *Requested);
			}
			FVector CameraLocation;
			FRotator CameraRotation;
			ReadCamera(CameraLocation, CameraRotation);
			if (bRoute)
			{
				TSharedRef<FJsonObject> Frame = MakeShared<FJsonObject>();
				Frame->SetNumberField(TEXT("index"), Run.RouteResults.Num());
				Frame->SetStringField(TEXT("image"), ImageName);
				SetVectorArray(Frame, TEXT("location"), CameraLocation);
				SetRotatorArray(Frame, TEXT("rotation"), CameraRotation);
				Frame->SetNumberField(TEXT("frame_time_ms"), FMath::RoundToDouble(Run.PendingFrameMs * 10.0) / 10.0);
				Run.RouteResults.Add(MakeShared<FJsonValueObject>(Frame));
				UE_LOG(LogDeadCurrent, Display, TEXT("[DCREVIEW] route %d"), Run.RouteResults.Num() - 1);
				const bool bAtEnd = Run.RouteDistance >= Run.RouteLength - 1.0f;
				if (bAtEnd)
				{
					Run.Phase = FRun::EPhase::Finish;
				}
				else
				{
					Run.RouteDistance = FMath::Min(Run.RouteDistance + Run.WalkSpeed, Run.RouteLength);
					Run.Frames = 0;
					Run.PhaseStart = Now;
					Run.Phase = FRun::EPhase::RouteSettle;
				}
			}
			else
			{
				const FShot& Shot = Run.Shots[Run.Index];
				StopViewpointCapture(Run);
				TSharedRef<FJsonObject> Record = ShotRecord(Shot, ImageName, CameraLocation, CameraRotation, true, FString());
				AddCaptureChecks(Record, Run.PendingFrameMs, BuildChecks(Run));
				Run.ViewpointResults.Add(MakeShared<FJsonValueObject>(Record));
				UE_LOG(LogDeadCurrent, Display, TEXT("[DCREVIEW] %s"), *Shot.Id);
				++Run.Index;
				if (Run.Index < Run.Shots.Num())
				{
					BeginOpen(Run);
					Run.Phase = FRun::EPhase::Wait;
				}
				else
				{
					Run.Phase = FRun::EPhase::RouteOpen;
				}
			}
			WriteManifest(Run);
			return false;
		}

		case FRun::EPhase::RouteOpen:
			Run.RouteDistance = 0.0f;
			BeginOpen(Run);
			Run.Phase = FRun::EPhase::RouteWait;
			return false;

		case FRun::EPhase::Open:
			BeginOpen(Run);
			Run.Phase = FRun::EPhase::Wait;
			return false;

		case FRun::EPhase::Finish:
			DetachViewpointCapture(Run);
			WriteContactSheet(Run);
			WriteManifest(Run);
			RestoreSlot();
			if (!Run.bFailed)
			{
				UE_LOG(LogDeadCurrent, Display, TEXT("[DCREVIEW] captured %d viewpoints and %d route frames"),
					Run.ViewpointResults.Num(), Run.RouteResults.Num());
			}
			return true;

		default:
			return true;
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCReviewCaptureTest, "DeadCurrent.Review.Capture",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCReviewCaptureTest::RunTest(const FString& Parameters)
{
	using namespace DCReviewCapture;
	if (!FApp::CanEverRender())
	{
		AddError(TEXT("Review capture needs a rendered game window. Run Tools\\ReviewCapture.bat."));
		return false;
	}

	TSharedRef<FRun> Run = MakeShared<FRun>();
	Run->Test = this;
	Run->Log = MakeShared<FReviewLogCapture>();
	Run->OutDir = OutputDirectory();
	const FString ListPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Tools/Review/Lvl_Boathouse.json"));
	FString Error;
	if (!LoadList(ListPath, Run.Get(), Error))
	{
		AddError(Error);
		return false;
	}

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Run]()
	{
		return Tick(Run);
	}));
	return true;
}

#endif
