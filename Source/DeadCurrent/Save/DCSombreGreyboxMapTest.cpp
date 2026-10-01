#include "Core/DCMapTestHelpers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Character/DCPlayerCharacter.h"
#include "Dom/JsonObject.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "World/DCCellPortal.h"
#include "World/DCLocationVolume.h"

/**
 *  Phase 6 VS-08: the exterior greybox of Lvl_PointeSombre, proved on the real map (Tools\RunTests.bat Map.Sombre).
 *  Integrator-owned.
 *
 *    Greybox   the six location volumes; every ledger anchor stands at capsule height on collision; the fence stops the
 *              player above the high ground; nothing inside the fence is void; the real player walks every
 *              compressed-geography leg (Tools/PointeSombre/routes.json over island.json's paths) and each walk is
 *              within ±20% of its target window; every interior and the lamp room are reached by their (greybox)
 *              portals and land on their anchors.
 */
namespace DCSombreGreyboxTest
{
	using namespace DCMapTest;

	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_PointeSombre");
	const TCHAR* TestSlot = TEXT("DeadCurrent_SombreGreyboxTest");

	TSharedPtr<FJsonObject> LoadJson(FAutomationTestBase& Test, const FString& RelativePath)
	{
		FString Text;
		TSharedPtr<FJsonObject> Object;
		if (!Test.TestTrue(*FString::Printf(TEXT("%s loads"), *RelativePath), FFileHelper::LoadFileToString(Text, *(FPaths::ProjectDir() / RelativePath)))
			|| !Test.TestTrue(*FString::Printf(TEXT("%s parses"), *RelativePath),
				FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Object) && Object.IsValid()))
		{
			return nullptr;
		}
		return Object;
	}

	AActor* Tagged(const FString& Tag)
	{
		const FName Name(*Tag);
		for (TActorIterator<AActor> It(GameWorld()); It; ++It)
		{
			if (It->ActorHasTag(Name))
			{
				return *It;
			}
		}
		return nullptr;
	}

	/** First thing that stops a pawn straight below Where (the player ignored), or false. */
	bool Below(const FVector& Where, double Depth, FHitResult& Hit)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(SombreGreyboxBelow), true, Player());
		return GameWorld()->LineTraceSingleByChannel(Hit, Where, Where - FVector(0.0, 0.0, Depth), ECC_Pawn, Params);
	}

	struct FLeg
	{
		FString Id;
		double TargetLo = 0.0;
		double TargetHi = 0.0;
		TArray<FVector2D> Points;   // cm
		double Seconds = -1.0;
		bool bArrived = false;
		FString Failure;
	};

	/** Walks the possessed player along a leg's points with movement input (as a person would hold W toward each
	 *  point), and records the game time from the first step to the last point. Fails when stuck or far too slow. */
	class FWalkLeg : public IAutomationLatentCommand
	{
	public:
		FWalkLeg(TSharedRef<FLeg> InLeg) : Leg(InLeg) {}

		virtual bool Update() override
		{
			ADCPlayerCharacter* Pawn = Player();
			UWorld* World = GameWorld();
			if (!Pawn || !World)
			{
				Leg->Failure = TEXT("no player");
				return true;
			}
			const double Now = World->GetTimeSeconds();
			if (Index < 0)
			{
				Index = 1;
				Start = LastProgress = Now;
				Best = TNumericLimits<double>::Max();
			}
			if (Index >= Leg->Points.Num())
			{
				Leg->Seconds = Now - Start;
				Leg->bArrived = true;
				return true;
			}
			const FVector2D Here(Pawn->GetActorLocation());
			const FVector2D Target = Leg->Points[Index];
			const double Distance = FVector2D::Distance(Here, Target);
			// The last point must be reached; the corners are passed within a stride, as a walker cuts them.
			const double Reach = Index == Leg->Points.Num() - 1 ? 120.0 : 200.0;
			if (Distance < Reach)
			{
				++Index;
				Best = TNumericLimits<double>::Max();
				LastProgress = Now;
				return false;
			}
			if (Distance < Best - 50.0)
			{
				Best = Distance;
				LastProgress = Now;
			}
			if (Now - LastProgress > 5.0)
			{
				Leg->Failure = FString::Printf(TEXT("stuck at (%.0f, %.0f, %.0f) heading for point %d"), Here.X, Here.Y,
					Pawn->GetActorLocation().Z, Index);
				return true;
			}
			if (Now - Start > Leg->TargetHi * 3.0)
			{
				Leg->Failure = TEXT("far too slow");
				return true;
			}
			const FVector2D Direction = (Target - Here).GetSafeNormal();
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				PC->SetControlRotation(FRotator(0.0, FMath::RadiansToDegrees(FMath::Atan2(Direction.Y, Direction.X)), 0.0));
			}
			Pawn->AddMovementInput(FVector(Direction, 0.0), 1.0f);
			return false;
		}

	private:
		TSharedRef<FLeg> Leg;
		int32 Index = -1;
		double Start = 0.0;
		double LastProgress = 0.0;
		double Best = 0.0;
	};

	void PlaceOnGround(const FVector2D& Where)
	{
		FHitResult Hit;
		const FVector Top(Where.X, Where.Y, 20000.0);
		const double Z = Below(Top, 30000.0, Hit) ? Hit.ImpactPoint.Z : 0.0;
		Teleport(FVector(Where.X, Where.Y, Z + 100.0));
		if (ADCPlayerCharacter* Pawn = Player())
		{
			Pawn->GetCharacterMovement()->StopMovementImmediately();
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreGreyboxTest, "DeadCurrent.Map.Sombre.Greybox",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreGreyboxTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreGreyboxTest;
	QueueFreshMap(MapPath, TestSlot);

	TSharedRef<TArray<TSharedRef<FLeg>>> Legs = MakeShared<TArray<TSharedRef<FLeg>>>();
	const TSharedPtr<FJsonObject> Island = LoadJson(*this, TEXT("Tools/PointeSombre/island.json"));
	const TSharedPtr<FJsonObject> Routes = LoadJson(*this, TEXT("Tools/PointeSombre/routes.json"));
	const TSharedPtr<FJsonObject> Layout = LoadJson(*this, TEXT("Tools/PointeSombre/greybox.json"));
	if (!Island || !Routes || !Layout)
	{
		return true;
	}
	TMap<FString, TArray<FVector2D>> Paths;
	for (const TSharedPtr<FJsonValue>& Value : Island->GetArrayField(TEXT("paths")))
	{
		TArray<FVector2D> Points;
		for (const TSharedPtr<FJsonValue>& Point : Value->AsObject()->GetArrayField(TEXT("points")))
		{
			const TArray<TSharedPtr<FJsonValue>> P = Point->AsArray();
			Points.Add(FVector2D(P[0]->AsNumber() * 100.0, P[1]->AsNumber() * 100.0));
		}
		Paths.Add(Value->AsObject()->GetStringField(TEXT("id")), Points);
	}
	for (const TSharedPtr<FJsonValue>& Value : Routes->GetArrayField(TEXT("legs")))
	{
		const TSharedPtr<FJsonObject> L = Value->AsObject();
		TSharedRef<FLeg> Leg = MakeShared<FLeg>();
		Leg->Id = L->GetStringField(TEXT("id"));
		const TArray<TSharedPtr<FJsonValue>> Target = L->GetArrayField(TEXT("target_s"));
		Leg->TargetLo = Target[0]->AsNumber();
		Leg->TargetHi = Target[1]->AsNumber();
		Leg->Points = Paths.FindRef(L->GetStringField(TEXT("path")));
		TestTrue(*FString::Printf(TEXT("Leg %s has its path"), *Leg->Id), Leg->Points.Num() >= 2);
		Legs->Add(Leg);
	}
	const double Tolerance = Routes->GetNumberField(TEXT("tolerance"));

	// The places, the anchors, containment, and no void.
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Layout, Island]()
	{
		if (!TestNotNull(TEXT("Player"), Player()))
		{
			return true;
		}
		TSet<FName> Locations;
		for (TActorIterator<ADCLocationVolume> It(GameWorld()); It; ++It)
		{
			Locations.Add(It->GetLocationId());
		}
		for (const TCHAR* Id : { TEXT("sombre.harbor"), TEXT("sombre.light"), TEXT("sombre.vault"), TEXT("sombre.headland"),
			TEXT("sombre.cable_hut"), TEXT("sombre.ashland_grey") })
		{
			TestTrue(*FString::Printf(TEXT("Location volume %s"), Id), Locations.Contains(FName(Id)));
		}
		TestFalse(TEXT("No settlement or loft location was minted"), Locations.Contains(FName(TEXT("sombre.settlement")))
			|| Locations.Contains(FName(TEXT("sombre.net_loft"))));

		// Every anchor in the ledger exists and a capsule stands on collision at it (except the berth, a vessel pivot).
		int32 Anchors = 0;
		for (const auto& Pair : Layout->GetObjectField(TEXT("anchors"))->Values)
		{
			const FString Name(*Pair.Key);
			AActor* Anchor = Tagged(TEXT("Anchor:") + Name);
			if (!TestNotNull(*FString::Printf(TEXT("%s exists"), *Name), Anchor))
			{
				continue;
			}
			++Anchors;
			if (Name == TEXT("Anchor_IdaBerth"))
			{
				continue;
			}
			FHitResult Hit;
			const bool bGround = Below(Anchor->GetActorLocation(), 400.0, Hit);
			const double Height = bGround ? Anchor->GetActorLocation().Z - Hit.ImpactPoint.Z : -1.0;
			TestTrue(*FString::Printf(TEXT("%s stands at capsule height on collision (%.0f cm)"), *Name, Height),
				bGround && Height > 60.0 && Height < 140.0);
		}
		for (const TCHAR* Shipped : { TEXT("Anchor_NewGame_Deck"), TEXT("Anchor_CrossingExit_Quay"), TEXT("Anchor_Respawn_TowerBase") })
		{
			TestNotNull(*FString::Printf(TEXT("%s exists"), Shipped), Tagged(FString(TEXT("Anchor:")) + Shipped));
		}
		AddInfo(FString::Printf(TEXT("Greybox: %d location volumes, %d ledger anchors"), Locations.Num(), Anchors + 3));

		// Containment: across every fence segment, at the height of the tower's gallery, a pawn is stopped.
		const TArray<TSharedPtr<FJsonValue>> Bounds = Island->GetObjectField(TEXT("bounds"))->GetArrayField(TEXT("points"));
		FVector2D Centroid(0.0, 0.0);
		TArray<FVector2D> Fence;
		for (const TSharedPtr<FJsonValue>& Value : Bounds)
		{
			const TArray<TSharedPtr<FJsonValue>> P = Value->AsArray();
			Fence.Add(FVector2D(P[0]->AsNumber() * 100.0, P[1]->AsNumber() * 100.0));
			Centroid += Fence.Last();
		}
		Centroid /= Fence.Num();
		int32 Crossings = 0, Stopped = 0;
		for (int32 I = 0; I < Fence.Num(); ++I)
		{
			const FVector2D A = Fence[I];
			const FVector2D B = Fence[(I + 1) % Fence.Num()];
			const FVector2D Mid = (A + B) * 0.5;
			const FVector2D Normal = FVector2D(B.Y - A.Y, A.X - B.X).GetSafeNormal();
			const FVector2D Out = FVector2D::DotProduct(Normal, Mid - Centroid) > 0.0 ? Normal : -Normal;
			for (const double Z : { 3000.0, 5000.0 })
			{
				++Crossings;
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(SombreFence), true, Player());
				const FVector From(Mid - Out * 300.0, Z);
				const FVector To(Mid + Out * 300.0, Z);
				if (GameWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Pawn, Params) && Hit.GetActor()
					&& Hit.GetActor()->ActorHasTag(TEXT("SombreEdge")))
				{
					++Stopped;
				}
			}
		}
		TestEqual(TEXT("The fence stops a pawn at 30 m and 50 m on every segment"), Stopped, Crossings);

		// No void: every 5 m inside the fence, something stops a pawn falling, above the sea floor.
		auto InsideFence = [&Fence](const FVector2D& P)
		{
			bool bInside = false;
			for (int32 I = 0, J = Fence.Num() - 1; I < Fence.Num(); J = I++)
			{
				if ((Fence[I].Y > P.Y) != (Fence[J].Y > P.Y)
					&& P.X < (Fence[J].X - Fence[I].X) * (P.Y - Fence[I].Y) / (Fence[J].Y - Fence[I].Y) + Fence[I].X)
				{
					bInside = !bInside;
				}
			}
			return bInside;
		};
		int32 Samples = 0, Void = 0;
		for (double X = -14000.0; X <= 13000.0; X += 500.0)
		{
			for (double Y = -26200.0; Y <= 21800.0; Y += 500.0)
			{
				if (!InsideFence(FVector2D(X, Y)))
				{
					continue;
				}
				++Samples;
				FHitResult Hit;
				if (!Below(FVector(X, Y, 20000.0), 20500.0, Hit) || Hit.ImpactPoint.Z < -200.0)
				{
					++Void;
					if (Void <= 5)
					{
						AddError(FString::Printf(TEXT("Void at (%.0f, %.0f)"), X, Y));
					}
				}
			}
		}
		AddInfo(FString::Printf(TEXT("Void check: %d points inside the fence, %d void"), Samples, Void));
		TestTrue(TEXT("Points to check"), Samples > 1000);
		TestEqual(TEXT("Nothing inside the fence is void"), Void, 0);
		return true;
	}));

	// Walk every compressed-geography leg with the real player.
	for (const TSharedRef<FLeg>& Leg : *Legs)
	{
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Leg]()
		{
			PlaceOnGround(Leg->Points[0]);
			return true;
		}));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FWalkLeg(Leg));
	}
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Legs, Tolerance]()
	{
		for (const TSharedRef<FLeg>& Leg : *Legs)
		{
			AddInfo(FString::Printf(TEXT("Route %s: %.1f s walked (target %.0f-%.0f s)%s"), *Leg->Id, Leg->Seconds,
				Leg->TargetLo, Leg->TargetHi, Leg->Failure.IsEmpty() ? TEXT("") : *(TEXT(" — ") + Leg->Failure)));
			TestTrue(*FString::Printf(TEXT("%s walked to its end"), *Leg->Id), Leg->bArrived);
			TestTrue(*FString::Printf(TEXT("%s within ±%.0f%% of its window (%.1f s)"), *Leg->Id, Tolerance * 100.0, Leg->Seconds),
				Leg->bArrived && Leg->Seconds >= Leg->TargetLo * (1.0 - Tolerance) && Leg->Seconds <= Leg->TargetHi * (1.0 + Tolerance));
		}
		return true;
	}));

	// Every interior and the lamp room, through their (greybox) portals.
	struct FStub { const TCHAR* Portal; const TCHAR* Anchor; };
	static const FStub Stubs[] = {
		{ TEXT("Greybox_TowerStair_Up"), TEXT("Anchor_TowerStair_Lamp") },
		{ TEXT("Greybox_TowerStair_Down"), TEXT("Anchor_TowerStair_Base") },
		{ TEXT("Greybox_VaultHatch_Out"), TEXT("Anchor_VaultHatch_Bottom") },
		{ TEXT("Greybox_VaultHatch_In"), TEXT("Anchor_VaultHatch_Top") },
		{ TEXT("Greybox_VaultLower_Out"), TEXT("Anchor_VaultLower_In") },
		{ TEXT("Greybox_VaultLower_In"), TEXT("Anchor_VaultLower_Out") },
		{ TEXT("Greybox_VaultConduit_Out"), TEXT("Anchor_VaultConduit_In") },
		{ TEXT("Greybox_VaultConduit_In"), TEXT("Anchor_VaultConduit_Out") },
		{ TEXT("Greybox_LoftStair_Up"), TEXT("Anchor_LoftStair_Loft") },
		{ TEXT("Greybox_LoftStair_Down"), TEXT("Anchor_LoftStair_Store") },
	};
	for (const FStub& Stub : Stubs)
	{
		const FString PortalLabel = Stub.Portal;
		const FString AnchorName = Stub.Anchor;
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, PortalLabel]()
		{
			ADCCellPortal* Portal = nullptr;
			for (TActorIterator<ADCCellPortal> It(GameWorld()); It; ++It)
			{
				if (It->GetActorNameOrLabel() == PortalLabel)
				{
					Portal = *It;
				}
			}
			if (!TestNotNull(*FString::Printf(TEXT("%s exists"), *PortalLabel), Portal))
			{
				return true;
			}
			TestEqual(*FString::Printf(TEXT("%s can be used"), *PortalLabel), Portal->TryUse(Player()), EDCPortalUse::Passed);
			return true;
		}));
		QueueWaitUntil(this, []()
		{
			for (TActorIterator<ADCCellPortal> It(GameWorld()); It; ++It)
			{
				if (It->IsTransitioning())
				{
					return false;
				}
			}
			return true;
		}, TEXT("the portal's transition ends"), 5.0);
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, PortalLabel, AnchorName]()
		{
			const AActor* Anchor = Tagged(TEXT("Anchor:") + AnchorName);
			if (TestNotNull(*AnchorName, Anchor) && Player())
			{
				TestTrue(*FString::Printf(TEXT("%s lands at %s"), *PortalLabel, *AnchorName),
					FVector::Dist(Player()->GetActorLocation(), Anchor->GetActorLocation()) < 150.0);
			}
			return true;
		}));
	}

	QueueCleanup(TestSlot);
	return true;
}

#endif
