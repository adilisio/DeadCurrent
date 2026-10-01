#include "Camera/CameraComponent.h"
#include "Character/DCCharacterProgressionComponent.h"
#include "Character/DCPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "CollisionShape.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "Misc/OutputDevice.h"
#include "Misc/PackageName.h"
#include "Misc/ScopeLock.h"
#include "RHIStats.h"
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
#if WITH_EDITOR
#include "ShaderCompiler.h"
#endif
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
 *  The map is `-ReviewMap=/Game/Maps/<name>` (Tools\ReviewCapture.bat passes it; the default is Lvl_Boathouse).
 *  Viewpoints live in Tools/Review/<name>.json plus every Tools/Review/<name>/*.json (one file per content cell,
 *  each owned by one builder). A file may set "art_sentinels" (how many ArtLayerSentinel actors mark the art
 *  sublevels as loaded; default 1, 0 to not wait) and, in exactly one file, the "route". A route may name the
 *  terrain it walks on ("ground_tag"): each frame is then placed by a trace from "probe_from_cm" (default 20000)
 *  straight down, and a frame whose first hit is not an actor with that tag fails the capture (VS-06: the old
 *  4 m probe started inside the tower rock and silently shot from inside the island).
 *  Same-session A/B (Phase 6 gates): -ReviewMaxFPS=<n> replaces the PlayTest 60 FPS cap (0 = uncapped), and
 *  -ReviewHideTag=<tag> hides every actor with that tag after each load (the biome ON/OFF pair), and
 *  -ReviewSampleSeconds=<s> samples each view's frame time for longer than the default half second, and
 *  -ReviewNoRoute skips the walking route.
 */
namespace DCReviewCapture
{
	/** The map under review, from the command line; Lvl_Boathouse when none is given. */
	const FString& MapPath()
	{
		static const FString Path = []()
		{
			FString Value;
			if (!FParse::Value(FCommandLine::Get(), TEXT("ReviewMap="), Value) || Value.IsEmpty())
			{
				Value = TEXT("/Game/Maps/Lvl_Boathouse");
			}
			return Value;
		}();
		return Path;
	}

	/** -ReviewMaxFPS=<n>: the frame cap during capture (PlayTest's 60 by default; 0 removes it). */
	int32 MaxFps()
	{
		int32 Value = 60;
		FParse::Value(FCommandLine::Get(), TEXT("ReviewMaxFPS="), Value);
		return FMath::Max(0, Value);
	}

	/** -ReviewSampleSeconds=<s>: how long each view's frame time is sampled once it has settled (0.5 by default).
	 *  The same-session A/B gates use several seconds: half a second varied by about 1 ms per view between runs. */
	double SampleSeconds()
	{
		double Value = 0.5;
		FParse::Value(FCommandLine::Get(), TEXT("ReviewSampleSeconds="), Value);
		return FMath::Max(0.5, Value);
	}

	/** -ReviewToggleTag=<tag>[+<tag>...]: an A/B inside one run. At each view (or only those whose id starts with
	 *  -ReviewToggleViews=<prefix>), after the normal sample, the tagged actors or components are hidden and shown in
	 *  windows of -ReviewSampleSeconds, in ABBA order (off, on, on, off, ...), -ReviewToggleCycles times (default 4),
	 *  and the view records the mean frame time of each state. Clock and thermal drift between runs falls on both
	 *  states alike, so a sub-millisecond cost is measurable on this laptop (VS-06: separate runs swung by 1-9 ms). */
	const FString& ToggleTag()
	{
		static const FString Tag = []()
		{
			FString Value;
			FParse::Value(FCommandLine::Get(), TEXT("ReviewToggleTag="), Value);
			return Value;
		}();
		return Tag;
	}

	const FString& ToggleViews()
	{
		static const FString Prefix = []()
		{
			FString Value;
			FParse::Value(FCommandLine::Get(), TEXT("ReviewToggleViews="), Value);
			return Value;
		}();
		return Prefix;
	}

	int32 ToggleCycles()
	{
		int32 Value = 4;
		FParse::Value(FCommandLine::Get(), TEXT("ReviewToggleCycles="), Value);
		return FMath::Clamp(Value, 1, 16);
	}

	/** -ReviewNoRoute: skip the walking route (a timing pair over the fixed views only). */
	bool NoRoute()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("ReviewNoRoute"));
	}

	/** -ReviewHideTag=<tag>[+<tag>...]: actors, or single components, with one of these tags are hidden after every load
	 *  (empty: none). A component tag hides one part of an actor: Biome:scrub hides only the recipe's weeds. */
	const FString& HideTag()
	{
		static const FString Tag = []()
		{
			FString Value;
			FParse::Value(FCommandLine::Get(), TEXT("ReviewHideTag="), Value);
			return Value;
		}();
		return Tag;
	}

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
		TArray<FString> ViewFiles;
		int32 ExpectedSentinels = 1;
		bool bSentinelsSet = false;
		TArray<FVector> RoutePoints;
		FString RouteExpectation;
		FString RouteSource;
		float RoutePitch = -6.0f;
		FName RouteGroundTag;            // empty: the original short probe (Lvl_Boathouse)
		float RouteProbeFrom = 20000.0f;
		FString RouteError;
		int32 HiddenActors = 0;
		int32 ToggleWindow = 0;
		double WindowStart = 0.0;
		TArray<double> WindowSamples;
		TArray<double> ToggleOffMs;
		TArray<double> ToggleOnMs;
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
		// Cost of the current view (Phase 6, VS-02): what the scene scan saw on screen, and the highest per-frame
		// RHI draw-call and primitive counts sampled while the frame settled (0 when the RHI keeps no counts).
		int32 VisiblePrimitiveComponents = 0;
		int32 VisibleMaterialSlots = 0;
		int32 VisibleInstances = 0;
		int32 PeakRhiDrawCalls = 0;
		int32 PeakRhiPrimitives = 0;

		enum class EPhase : uint8
		{
			Boot,
			Open,
			Wait,
			Setup,
			Settle,
			Toggle,
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

	/** Reads one review file into the run: its viewpoints are appended, its route (if any) is taken. */
	bool LoadViewFile(const FString& Path, FRun& Run, FString& Error)
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

		double Sentinels = 0.0;
		if (Root->TryGetNumberField(TEXT("art_sentinels"), Sentinels))
		{
			if (Run.bSentinelsSet && Run.ExpectedSentinels != static_cast<int32>(Sentinels))
			{
				Error = FString::Printf(TEXT("%s sets art_sentinels to %d but another review file set %d"),
					*Path, static_cast<int32>(Sentinels), Run.ExpectedSentinels);
				return false;
			}
			Run.ExpectedSentinels = static_cast<int32>(Sentinels);
			Run.bSentinelsSet = true;
		}

		const TArray<TSharedPtr<FJsonValue>>* Viewpoints = nullptr;
		if (!Root->TryGetArrayField(TEXT("viewpoints"), Viewpoints) || !Viewpoints || Viewpoints->Num() == 0)
		{
			Error = FString::Printf(TEXT("Review file %s has no viewpoints"), *Path);
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
				Error = FString::Printf(TEXT("A viewpoint in %s is missing id, location, rotation, expectation, or source"), *Path);
				return false;
			}
			for (const FShot& Existing : Run.Shots)
			{
				if (Existing.Id == Shot.Id)
				{
					Error = FString::Printf(TEXT("Viewpoint id %s is defined twice (second time in %s)"), *Shot.Id, *Path);
					return false;
				}
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
			return true;   // a cell's file may have views only; LoadReviewSet requires one route in the whole set
		}
		if (Run.RoutePoints.Num() > 0)
		{
			Error = FString::Printf(TEXT("%s defines a route, but another review file already did"), *Path);
			return false;
		}
		(*Route)->TryGetStringField(TEXT("expectation"), Run.RouteExpectation);
		(*Route)->TryGetStringField(TEXT("source"), Run.RouteSource);
		double Pitch = Run.RoutePitch;
		if ((*Route)->TryGetNumberField(TEXT("look_pitch"), Pitch))
		{
			Run.RoutePitch = static_cast<float>(Pitch);
		}
		FString GroundTag;
		if ((*Route)->TryGetStringField(TEXT("ground_tag"), GroundTag) && !GroundTag.IsEmpty())
		{
			Run.RouteGroundTag = FName(*GroundTag);
		}
		double ProbeFrom = Run.RouteProbeFrom;
		if ((*Route)->TryGetNumberField(TEXT("probe_from_cm"), ProbeFrom))
		{
			Run.RouteProbeFrom = static_cast<float>(ProbeFrom);
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

	/**
	 *  Loads a map's whole review set: Tools/Review/<name>.json (if present), then every Tools/Review/<name>/*.json in
	 *  file-name order. Viewpoint ids are unique across the set and exactly one file defines the route.
	 */
	bool LoadReviewSet(const FString& ShortMapName, FRun& Run, FString& Error)
	{
		const FString ReviewDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Tools/Review"));
		TArray<FString> Files;
		const FString Base = ReviewDir / (ShortMapName + TEXT(".json"));
		if (IFileManager::Get().FileExists(*Base))
		{
			Files.Add(Base);
		}
		TArray<FString> CellFiles;
		IFileManager::Get().FindFiles(CellFiles, *(ReviewDir / ShortMapName / TEXT("*.json")), true, false);
		CellFiles.Sort();
		for (const FString& CellFile : CellFiles)
		{
			Files.Add(ReviewDir / ShortMapName / CellFile);
		}
		if (Files.IsEmpty())
		{
			Error = FString::Printf(TEXT("No review views for %s: add Tools/Review/%s.json or Tools/Review/%s/<cell>.json"),
				*ShortMapName, *ShortMapName, *ShortMapName);
			return false;
		}
		for (const FString& File : Files)
		{
			if (!LoadViewFile(File, Run, Error))
			{
				return false;
			}
			Run.ViewFiles.Add(File.RightChop(ReviewDir.Len() + 1));
		}
		if (Run.RoutePoints.IsEmpty())
		{
			Error = FString::Printf(TEXT("The review set for %s has no route (one file must define it)"), *ShortMapName);
			return false;
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
		};
		for (const TCHAR* Command : Commands)
		{
			GEngine->Exec(World, Command);
		}
		GEngine->Exec(World, *FString::Printf(TEXT("t.MaxFPS %d"), MaxFps()));
	}

	/** Hide every actor carrying the -ReviewHideTag tag (rendering only; nothing else about the level changes). */
	int32 SetTaggedHidden(const FString& TagList, bool bHidden);

	int32 HideTaggedActors()
	{
		return SetTaggedHidden(HideTag(), true);
	}

	/** Hide or show every actor, or single component, carrying one of the '+'-separated tags. Returns how many. */
	int32 SetTaggedHidden(const FString& TagList, bool bHidden)
	{
		UWorld* World = GameWorld();
		if (!World || TagList.IsEmpty())
		{
			return 0;
		}
		TArray<FString> Tags;
		TagList.ParseIntoArray(Tags, TEXT("+"));
		int32 Hidden = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			for (const FString& TagText : Tags)
			{
				const FName Tag(*TagText);
				if (It->ActorHasTag(Tag))
				{
					It->SetActorHiddenInGame(bHidden);
					++Hidden;
					continue;
				}
				TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
				for (UPrimitiveComponent* Primitive : Primitives)
				{
					if (Primitive->ComponentHasTag(Tag))
					{
						Primitive->SetHiddenInGame(bHidden);
						++Hidden;
					}
				}
			}
		}
		return Hidden;
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
		FHitResult Hit;
		if (!Run.RouteGroundTag.IsNone())
		{
			// From well above the highest ground, straight down past the sea floor; the first thing hit must be the
			// named terrain, or the frame is not a walk on the surface and the capture fails.
			const FVector Probe(Feet.X, Feet.Y, Run.RouteProbeFrom);
			const FCollisionQueryParams Params(SCENE_QUERY_STAT(ReviewRouteProbe), true, Pawn);   // not the player's own capsule
			if (!World->LineTraceSingleByChannel(Hit, Probe, FVector(Feet.X, Feet.Y, -50000.0), ECC_WorldStatic, Params))
			{
				Run.RouteError = FString::Printf(TEXT("no ground under route point (%.0f, %.0f)"), Feet.X, Feet.Y);
				return false;
			}
			if (!Hit.GetActor() || !Hit.GetActor()->ActorHasTag(Run.RouteGroundTag))
			{
				Run.RouteError = FString::Printf(TEXT("route point (%.0f, %.0f) first hits %s, not %s"), Feet.X, Feet.Y,
					Hit.GetActor() ? *Hit.GetActor()->GetActorNameOrLabel() : TEXT("nothing"), *Run.RouteGroundTag.ToString());
				return false;
			}
			Feet.Z = Hit.ImpactPoint.Z;
		}
		else
		{
			const FVector Probe(Feet.X, Feet.Y, Feet.Z + 400.0);
			if (World->LineTraceSingleByChannel(Hit, Probe, Probe - FVector(0.0, 0.0, 1200.0), ECC_WorldStatic))
			{
				Feet.Z = Hit.ImpactPoint.Z;
			}
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
		Run.PeakRhiDrawCalls = 0;
		Run.PeakRhiPrimitives = 0;
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

	/** Median of frame times (seconds) in ms. This laptop flips between two clock states (frames of about 3 and about
	 *  11 ms for the same view, VS-06), so the toggle uses medians: a window and a state each report their majority. */
	double MedianMs(TArray<double> Samples)
	{
		if (Samples.Num() == 0)
		{
			return 0.0;
		}
		Samples.Sort();
		const int32 Mid = Samples.Num() / 2;
		return (Samples.Num() % 2 ? Samples[Mid] : 0.5 * (Samples[Mid - 1] + Samples[Mid])) * 1000.0;
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
		Run.VisiblePrimitiveComponents = 0;
		Run.VisibleMaterialSlots = 0;
		Run.VisibleInstances = 0;
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
				++Run.VisiblePrimitiveComponents;
				Run.VisibleMaterialSlots += MaterialCount;
				if (const UInstancedStaticMeshComponent* Instanced = Cast<UInstancedStaticMeshComponent>(Primitive))
				{
					Run.VisibleInstances += Instanced->GetInstanceCount();
				}
				for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
				{
					UMaterialInterface* Material = Primitive->GetMaterial(MaterialIndex);
					if (IsDefaultOrGridMaterial(Material))
					{
						const FString Who = FString::Printf(TEXT("%s (%s)"), *Material->GetPathName(), *ActorLabel(*ActorIt));
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
		// Cost measurements (Phase 6, VS-02), for same-session A/B gates: the scene scan counts what is visible and in the
		// frustum; the RHI peaks are the highest per-frame counts sampled while the frame settled.
		Checks->SetNumberField(TEXT("visible_primitive_components"), Run.VisiblePrimitiveComponents);
		Checks->SetNumberField(TEXT("visible_material_slots"), Run.VisibleMaterialSlots);
		Checks->SetNumberField(TEXT("visible_instances"), Run.VisibleInstances);
		Checks->SetNumberField(TEXT("rhi_draw_calls_peak"), Run.PeakRhiDrawCalls);
		Checks->SetNumberField(TEXT("rhi_primitives_peak"), Run.PeakRhiPrimitives);
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
		Root->SetStringField(TEXT("map"), MapPath());
		TArray<TSharedPtr<FJsonValue>> ViewFileValues;
		for (const FString& ViewFile : Run.ViewFiles)
		{
			ViewFileValues.Add(MakeShared<FJsonValueString>(ViewFile));
		}
		Root->SetArrayField(TEXT("view_files"), ViewFileValues);
		Root->SetStringField(TEXT("render"), TEXT("Tools/PlayTest.bat cvars, scalability 0, 1280x720"));
		Root->SetNumberField(TEXT("max_fps"), MaxFps());
		Root->SetNumberField(TEXT("sample_seconds"), SampleSeconds());
		Root->SetBoolField(TEXT("route_skipped"), NoRoute());
		Root->SetStringField(TEXT("toggle_tag"), ToggleTag());
		Root->SetStringField(TEXT("hidden_tag"), HideTag());
		Root->SetNumberField(TEXT("hidden_actors"), Run.HiddenActors);
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
			GEngine->Exec(World, *FString::Printf(TEXT("Open %s"), *MapPath()));
		}
		Run.PhaseStart = FPlatformTime::Seconds();
	}

	bool WorldReady(const FRun& Run)
	{
		UWorld* World = GameWorld();
		return World && World != Run.PreviousWorld.Get() && World->HasBegunPlay() && Player() != nullptr
			&& (Run.ExpectedSentinels == 0 || CountArtSentinels(World) == Run.ExpectedSentinels);
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
						*MapPath(), Sentinels, Player() ? TEXT("yes") : TEXT("no")));
					Run.Phase = FRun::EPhase::Finish;
				}
				return false;
			}
			ApplyPlayTestCvars();
			Run.HiddenActors = HideTaggedActors();
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
			const bool bSetUp = ApplySetup(Shot, Error);
			if (bSetUp)
			{
				// The setup is a restore, not play: presence rules snap to it before the camera is aimed.
				if (UDCWorldStateSubsystem* WorldState = Player() ? UDCWorldStateSubsystem::Get(Player()) : nullptr)
				{
					WorldState->NotifyRestored();
				}
			}
			if (!bSetUp || !PlaceShot(Shot, Eye, Rotation, Error))
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
#if WITH_EDITOR
			// A material whose shaders are still compiling renders as the engine's grey fallback, and the compile
			// competes for the CPU. Content rebuilds recreate the prop masters, so finish compiling before judging a
			// frame or timing one (Phase 5: Mara's head rendered grey in several runs for this reason).
			if (GShaderCompilingManager)
			{
				GShaderCompilingManager->FinishAllCompilation();
			}
#endif
			Run.Frames = 0;
			Run.PhaseStart = Now;
			BeginViewpointCapture(Run);
			Run.Phase = FRun::EPhase::Settle;
			return false;
		}

		case FRun::EPhase::Settle:
			Run.FrameSamples.Add(FApp::GetDeltaTime());
			Run.PeakRhiDrawCalls = FMath::Max(Run.PeakRhiDrawCalls, GNumDrawCallsRHI[0]);
			Run.PeakRhiPrimitives = FMath::Max(Run.PeakRhiPrimitives, GNumPrimitivesDrawnRHI[0]);
			++Run.Frames;
			if (Run.Frames < SettleFrames || Now - Run.PhaseStart < SampleSeconds())
			{
				return false;
			}
			Run.PendingFrameMs = AverageFrameMs(Run.FrameSamples);
			GatherSceneChecks(Run);
			Run.ToggleOffMs.Reset();
			Run.ToggleOnMs.Reset();
			// No -ReviewToggleViews means every view (FString::StartsWith is false for an empty prefix; VS-08's first
			// gate-1 run toggled nothing because of it).
			if (!ToggleTag().IsEmpty() && (ToggleViews().IsEmpty() || Run.Shots[Run.Index].Id.StartsWith(ToggleViews())))
			{
				Run.ToggleWindow = 0;
				Run.WindowStart = Now;
				Run.WindowSamples.Reset();
				SetTaggedHidden(ToggleTag(), true);
				Run.Phase = FRun::EPhase::Toggle;
				return false;
			}
			Run.Phase = FRun::EPhase::Shot;
			return false;

		case FRun::EPhase::Toggle:
		{
			// ABBA: off, on, on, off, repeated. A window discards its first 0.3 s (the change settling), then samples.
			static const bool Pattern[4] = { true, false, false, true };   // true = hidden (off)
			const double Elapsed = Now - Run.WindowStart;
			if (Elapsed > 0.3)
			{
				Run.WindowSamples.Add(FApp::GetDeltaTime());
			}
			if (Elapsed < 0.3 + SampleSeconds())
			{
				return false;
			}
			(Pattern[Run.ToggleWindow % 4] ? Run.ToggleOffMs : Run.ToggleOnMs).Add(MedianMs(Run.WindowSamples));
			++Run.ToggleWindow;
			Run.WindowSamples.Reset();
			Run.WindowStart = Now;
			if (Run.ToggleWindow >= 2 * ToggleCycles())
			{
				SetTaggedHidden(ToggleTag(), false);
				HideTaggedActors();
				Run.Phase = FRun::EPhase::Shot;
				return false;
			}
			SetTaggedHidden(ToggleTag(), Pattern[Run.ToggleWindow % 4]);
			return false;
		}

		case FRun::EPhase::RouteSettle:
			if (Run.Frames == 0)
			{
				Run.FrameSamples.Reset();
				FVector Eye;
				FRotator Rotation;
				if (!PlaceOnRoute(Run, Eye, Rotation))
				{
					Fail(Run, FString::Printf(TEXT("Could not place the route camera: %s"), *Run.RouteError));
					Run.Phase = FRun::EPhase::Finish;
					return false;
				}
			}
			Run.FrameSamples.Add(FApp::GetDeltaTime());
			++Run.Frames;
			if (Run.Frames < SettleFrames || Now - Run.PhaseStart < SampleSeconds())
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
					Fail(Run, FString::Printf(TEXT("Could not place the route camera: %s"), *Run.RouteError));
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
				if (Run.ToggleOffMs.Num() > 0 && Run.ToggleOnMs.Num() > 0)
				{
					auto Mean = [](const TArray<double>& Values)
					{
						// Median of the windows (ms); MedianMs takes seconds.
						TArray<double> Seconds;
						for (const double V : Values) { Seconds.Add(V / 1000.0); }
						return MedianMs(Seconds);
					};
					auto Rounded = [](const TArray<double>& Values)
					{
						TArray<TSharedPtr<FJsonValue>> Out;
						for (const double V : Values) { Out.Add(MakeShared<FJsonValueNumber>(FMath::RoundToDouble(V * 100.0) / 100.0)); }
						return Out;
					};
					TSharedRef<FJsonObject> Toggle = MakeShared<FJsonObject>();
					Toggle->SetStringField(TEXT("tag"), ToggleTag());
					Toggle->SetStringField(TEXT("order"), TEXT("ABBA, off first; each window its median frame, each state the median window"));
					Toggle->SetArrayField(TEXT("off_ms"), Rounded(Run.ToggleOffMs));
					Toggle->SetArrayField(TEXT("on_ms"), Rounded(Run.ToggleOnMs));
					Toggle->SetNumberField(TEXT("off_median_ms"), FMath::RoundToDouble(Mean(Run.ToggleOffMs) * 100.0) / 100.0);
					Toggle->SetNumberField(TEXT("on_median_ms"), FMath::RoundToDouble(Mean(Run.ToggleOnMs) * 100.0) / 100.0);
					Toggle->SetNumberField(TEXT("delta_ms"), FMath::RoundToDouble((Mean(Run.ToggleOnMs) - Mean(Run.ToggleOffMs)) * 100.0) / 100.0);
					Record->SetObjectField(TEXT("toggle"), Toggle);
				}
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
			if (NoRoute())
			{
				Run.Phase = FRun::EPhase::Finish;
				return false;
			}
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
	FString Error;
	if (!LoadReviewSet(FPackageName::GetShortName(MapPath()), Run.Get(), Error))
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
