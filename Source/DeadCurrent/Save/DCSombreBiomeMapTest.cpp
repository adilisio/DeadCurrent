#include "Core/DCMapTestHelpers.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Character/DCPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/FileManager.h"
#include "Interaction/DCInteractable.h"
#include "Materials/Material.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "World/DCCellPortal.h"
#include "World/DCLocationVolume.h"

/**
 *  Phase 6 VS-06: the rocky-shoreline recipe's output on the real map (game context, Tools\RunTests.bat Map.Sombre).
 *  Owned by the recipe (WP-BIOME). It does not re-plan anything: it checks the saved biome sublevel against the
 *  committed manifest (Tools/Biomes/out/Lvl_PointeSombre.json), and the manifest against its own hash.
 *
 *    BiomeExclusions   the manifest's hash re-computed from its payload; its island hash is the terrain's; the
 *                      sublevel's instances are exactly the manifest's placements; nothing inside an authored or pad
 *                      exclusion of its zone, or within the recipe's automatic radius of a live interactable, portal,
 *                      player start, character, or location volume; every Tier C component NoCollision and out of
 *                      navigation; nothing floats above the terrain.
 */
namespace DCSombreBiomeTest
{
	using namespace DCMapTest;

	const TCHAR* MapPath = TEXT("/Game/Maps/Lvl_PointeSombre");
	const TCHAR* TestSlot = TEXT("DeadCurrent_SombreBiomeTest");
	const TCHAR* BiomeLevel = TEXT("Lvl_PointeSombre_Biome");

	TSharedPtr<FJsonObject> LoadJson(FAutomationTestBase& Test, const FString& RelativePath)
	{
		FString Text;
		TSharedPtr<FJsonObject> Object;
		const FString Path = FPaths::ProjectDir() / RelativePath;
		if (!Test.TestTrue(*FString::Printf(TEXT("%s exists"), *RelativePath), FFileHelper::LoadFileToString(Text, *Path))
			|| !Test.TestTrue(*FString::Printf(TEXT("%s parses"), *RelativePath),
				FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Object) && Object.IsValid()))
		{
			return nullptr;
		}
		return Object;
	}

	/** An object's keys as FStrings (the 5.8 JSON object keys its map by a shared string type). */
	TArray<FString> Keys(const TSharedPtr<FJsonObject>& Object)
	{
		TArray<FString> Out;
		for (const auto& Pair : Object->Values)
		{
			Out.Add(FString(*Pair.Key));
		}
		return Out;
	}

	/** Python's json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False), for the integer-and-
	 *  string payload the planner hashes. A non-integer number makes the payload non-canonical (bOk false). */
	void Canonical(const TSharedPtr<FJsonValue>& Value, FString& Out, bool& bOk)
	{
		switch (Value->Type)
		{
		case EJson::Object:
		{
			const TSharedPtr<FJsonObject> Object = Value->AsObject();
			TArray<FString> Sorted = Keys(Object);
			Sorted.Sort([](const FString& A, const FString& B) { return A.Compare(B, ESearchCase::CaseSensitive) < 0; });
			Out += TEXT("{");
			for (int32 Index = 0; Index < Sorted.Num(); ++Index)
			{
				Out += Index ? TEXT(",") : TEXT("");
				Canonical(MakeShared<FJsonValueString>(Sorted[Index]), Out, bOk);
				Out += TEXT(":");
				Canonical(Object->TryGetField(Sorted[Index]), Out, bOk);
			}
			Out += TEXT("}");
			return;
		}
		case EJson::Array:
		{
			Out += TEXT("[");
			const TArray<TSharedPtr<FJsonValue>>& Items = Value->AsArray();
			for (int32 Index = 0; Index < Items.Num(); ++Index)
			{
				Out += Index ? TEXT(",") : TEXT("");
				Canonical(Items[Index], Out, bOk);
			}
			Out += TEXT("]");
			return;
		}
		case EJson::String:
		{
			Out += TEXT("\"");
			for (const TCHAR C : Value->AsString())
			{
				switch (C)
				{
				case TEXT('"'): Out += TEXT("\\\""); break;
				case TEXT('\\'): Out += TEXT("\\\\"); break;
				case TEXT('\n'): Out += TEXT("\\n"); break;
				case TEXT('\r'): Out += TEXT("\\r"); break;
				case TEXT('\t'): Out += TEXT("\\t"); break;
				default:
					if (C < 0x20)
					{
						Out += FString::Printf(TEXT("\\u%04x"), static_cast<int32>(C));
					}
					else
					{
						Out.AppendChar(C);
					}
				}
			}
			Out += TEXT("\"");
			return;
		}
		case EJson::Number:
		{
			const double Number = Value->AsNumber();
			if (FMath::RoundToDouble(Number) != Number)
			{
				bOk = false;
			}
			Out += FString::Printf(TEXT("%lld"), static_cast<int64>(Number));
			return;
		}
		case EJson::Boolean:
			Out += Value->AsBool() ? TEXT("true") : TEXT("false");
			return;
		default:
			Out += TEXT("null");
			return;
		}
	}

	FString PayloadHash(const TSharedPtr<FJsonObject>& Manifest, bool& bOk)
	{
		TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
		for (const TCHAR* Key : { TEXT("planner"), TEXT("island_hash"), TEXT("inputs"), TEXT("counts"), TEXT("placements") })
		{
			if (!Manifest->HasField(Key))
			{
				bOk = false;
				return FString();
			}
			Payload->SetField(Key, Manifest->TryGetField(Key));
		}
		FString Text;
		Canonical(MakeShared<FJsonValueObject>(Payload), Text, bOk);
		const FTCHARToUTF8 Utf8(*Text);
		return FSHA1::HashBuffer(Utf8.Get(), Utf8.Length()).ToString().ToLower();
	}

	struct FInstance
	{
		FString Zone;
		FString Family;
		int32 Mesh = -1;
		FTransform Transform;
		float BottomZ = 0.0f;
		bool bMatched = false;
	};

	FString TagValue(const TArray<FName>& Tags, const FString& Prefix)
	{
		for (const FName& Tag : Tags)
		{
			const FString Text = Tag.ToString();
			if (Text.StartsWith(Prefix) && Text.Len() > Prefix.Len())
			{
				return Text.RightChop(Prefix.Len());
			}
		}
		return FString();
	}

	/** An exclusion shape from a zone file or a pad (metres), with a margin. */
	struct FShape
	{
		FString Id;
		FString Kind;        // circle, box, polygon, polyline (a trail: within Radius of its centre line)
		FVector2D Center = FVector2D::ZeroVector;
		double Radius = 0.0;
		FVector2D Half = FVector2D::ZeroVector;
		double YawDeg = 0.0;
		TArray<FVector2D> Points;
		double Margin = 0.0;

		bool Contains(const FVector2D& P) const
		{
			if (Kind == TEXT("circle"))
			{
				return FVector2D::Distance(P, Center) <= Radius + Margin;
			}
			if (Kind == TEXT("box"))
			{
				const double A = FMath::DegreesToRadians(YawDeg);
				const FVector2D D = P - Center;
				const double LX = D.X * FMath::Cos(A) + D.Y * FMath::Sin(A);
				const double LY = -D.X * FMath::Sin(A) + D.Y * FMath::Cos(A);
				return FMath::Abs(LX) <= Half.X + Margin && FMath::Abs(LY) <= Half.Y + Margin;
			}
			if (Kind == TEXT("polyline"))
			{
				for (int32 I = 1; I < Points.Num(); ++I)
				{
					if (FVector2D::Distance(P, FMath::ClosestPointOnSegment2D(P, Points[I - 1], Points[I])) <= Radius + Margin)
					{
						return true;
					}
				}
				return false;
			}
			// Polygon: inside, or within the margin of an edge.
			bool bInside = false;
			for (int32 I = 0, J = Points.Num() - 1; I < Points.Num(); J = I++)
			{
				const FVector2D& A = Points[I];
				const FVector2D& B = Points[J];
				if ((A.Y > P.Y) != (B.Y > P.Y) && P.X < (B.X - A.X) * (P.Y - A.Y) / (B.Y - A.Y) + A.X)
				{
					bInside = !bInside;
				}
			}
			if (bInside || Margin <= 0.0)
			{
				return bInside;
			}
			for (int32 I = 0, J = Points.Num() - 1; I < Points.Num(); J = I++)
			{
				const FVector2D Closest = FMath::ClosestPointOnSegment2D(P, Points[J], Points[I]);
				if (FVector2D::Distance(P, Closest) <= Margin)
				{
					return true;
				}
			}
			return false;
		}
	};

	FVector2D Pair(const TArray<TSharedPtr<FJsonValue>>& Values)
	{
		return Values.Num() >= 2 ? FVector2D(Values[0]->AsNumber(), Values[1]->AsNumber()) : FVector2D::ZeroVector;
	}

	FShape ShapeFrom(const TSharedPtr<FJsonObject>& Object, const FString& Id)
	{
		FShape Shape;
		Shape.Id = Id;
		Shape.Kind = Object->GetStringField(TEXT("shape"));
		Object->TryGetNumberField(TEXT("margin_m"), Shape.Margin);
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
		if (Object->TryGetArrayField(TEXT("center"), Values))
		{
			Shape.Center = Pair(*Values);
		}
		if (Object->TryGetArrayField(TEXT("half"), Values))
		{
			Shape.Half = Pair(*Values);
		}
		Object->TryGetNumberField(TEXT("radius"), Shape.Radius);
		Object->TryGetNumberField(TEXT("yaw"), Shape.YawDeg);
		if (Object->TryGetArrayField(TEXT("points"), Values))
		{
			for (const TSharedPtr<FJsonValue>& Point : *Values)
			{
				Shape.Points.Add(Pair(Point->AsArray()));
			}
		}
		return Shape;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDCSombreBiomeExclusionsTest, "DeadCurrent.Map.Sombre.BiomeExclusions",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FDCSombreBiomeExclusionsTest::RunTest(const FString& Parameters)
{
	using namespace DCSombreBiomeTest;
	QueueFreshMap(MapPath, TestSlot);

	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this]()
	{
		UWorld* World = GameWorld();
		const TSharedPtr<FJsonObject> Manifest = LoadJson(*this, TEXT("Tools/Biomes/out/Lvl_PointeSombre.json"));
		const TSharedPtr<FJsonObject> Recipe = LoadJson(*this, TEXT("Tools/Biomes/great_lakes_rocky_shore.json"));
		const TSharedPtr<FJsonObject> Island = LoadJson(*this, TEXT("Tools/PointeSombre/island.json"));
		const TSharedPtr<FJsonObject> Probe = LoadJson(*this, TEXT("Tools/PointeSombre/out/terrain_probe.json"));
		if (!TestNotNull(TEXT("Game world"), World) || !Manifest || !Recipe || !Island || !Probe)
		{
			return true;
		}

		// 1. The manifest is what its hash says, and it was planned on the terrain that is built.
		bool bCanonical = true;
		const FString Recomputed = PayloadHash(Manifest, bCanonical);
		TestTrue(TEXT("The hashed payload holds only integers and strings"), bCanonical);
		TestEqual(TEXT("The manifest hash matches its payload"), Recomputed, Manifest->GetStringField(TEXT("hash")));
		TestEqual(TEXT("The manifest was planned on the built terrain (island hash)"),
			Manifest->GetStringField(TEXT("island_hash")), Probe->GetStringField(TEXT("island_hash")));

		// 2. The biome sublevel's instances.
		TArray<FInstance> Instances;
		int32 BiomeActors = 0, Components = 0, Collides = 0, AffectsNav = 0, DefaultMaterials = 0;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (!It->ActorHasTag(TEXT("Biome")))
			{
				continue;
			}
			++BiomeActors;
			TestTrue(*FString::Printf(TEXT("%s is in the biome sublevel"), *It->GetActorNameOrLabel()),
				It->GetLevel() && It->GetLevel()->GetOutermost()->GetName().EndsWith(BiomeLevel));
			const FString Zone = TagValue(It->Tags, TEXT("Biome:"));
			TInlineComponentArray<UPrimitiveComponent*> Primitives(*It);
			for (UPrimitiveComponent* Primitive : Primitives)
			{
				Collides += Primitive->GetCollisionEnabled() != ECollisionEnabled::NoCollision ? 1 : 0;
				AffectsNav += Primitive->CanEverAffectNavigation() ? 1 : 0;
				const UHierarchicalInstancedStaticMeshComponent* Hism = Cast<UHierarchicalInstancedStaticMeshComponent>(Primitive);
				if (!Hism)
				{
					continue;
				}
				++Components;
				const UStaticMesh* Mesh = Hism->GetStaticMesh();
				TestNotNull(TEXT("Every biome component has a mesh"), Mesh);
				for (int32 Slot = 0; Slot < Hism->GetNumMaterials(); ++Slot)
				{
					const UMaterialInterface* Material = Hism->GetMaterial(Slot);
					DefaultMaterials += (!Material || Material == UMaterial::GetDefaultMaterial(MD_Surface)) ? 1 : 0;
				}
				const FString Family = TagValue(Hism->ComponentTags, TEXT("Biome:"));
				const int32 MeshIndex = FCString::Atoi(*TagValue(Hism->ComponentTags, TEXT("BiomeMesh:")));
				for (int32 Index = 0; Index < Hism->GetInstanceCount(); ++Index)
				{
					FInstance Instance;
					Instance.Zone = Zone;
					Instance.Family = Family;
					Instance.Mesh = MeshIndex;
					Hism->GetInstanceTransform(Index, Instance.Transform, true);
					Instance.BottomZ = Mesh ? Mesh->GetBoundingBox().TransformBy(Instance.Transform).Min.Z : 0.0f;
					Instances.Add(Instance);
				}
			}
		}
		TestTrue(TEXT("The biome sublevel is loaded with its actors"), BiomeActors > 0 && Components > 0);
		TestEqual(TEXT("Every Tier C component is NoCollision"), Collides, 0);
		TestEqual(TEXT("No Tier C component affects navigation"), AffectsNav, 0);
		TestEqual(TEXT("No Tier C component draws a default material"), DefaultMaterials, 0);

		// 3. The instances are exactly the manifest's placements (a hand-moved rock or a stale sublevel fails).
		const TArray<TSharedPtr<FJsonValue>>& Placements = Manifest->GetArrayField(TEXT("placements"));
		int32 Matched = 0;
		TMap<FString, int32> FamilyCounts;
		for (const TSharedPtr<FJsonValue>& Value : Placements)
		{
			const TSharedPtr<FJsonObject> P = Value->AsObject();
			const FVector Location(P->GetNumberField(TEXT("x_mm")) / 10.0, P->GetNumberField(TEXT("y_mm")) / 10.0,
				P->GetNumberField(TEXT("z_mm")) / 10.0);
			const FQuat Rotation = FRotator(P->GetNumberField(TEXT("pitch_cdeg")) / 100.0, P->GetNumberField(TEXT("yaw_cdeg")) / 100.0,
				P->GetNumberField(TEXT("roll_cdeg")) / 100.0).Quaternion();
			const double Scale = P->GetNumberField(TEXT("scale_milli")) / 1000.0;
			const FString Family = P->GetStringField(TEXT("family"));
			FamilyCounts.FindOrAdd(Family) += 1;
			for (FInstance& Instance : Instances)
			{
				if (Instance.bMatched || Instance.Zone != P->GetStringField(TEXT("zone")) || Instance.Family != Family
					|| Instance.Mesh != static_cast<int32>(P->GetNumberField(TEXT("mesh"))))
				{
					continue;
				}
				if (FVector::Dist(Instance.Transform.GetLocation(), Location) < 0.2
					&& Instance.Transform.GetRotation().AngularDistance(Rotation) < FMath::DegreesToRadians(0.05)
					&& FMath::Abs(Instance.Transform.GetScale3D().X - Scale) < 0.002)
				{
					Instance.bMatched = true;
					++Matched;
					break;
				}
			}
		}
		AddInfo(FString::Printf(TEXT("Biome: %d actors, %d components, %d instances, %d manifest placements, %d matched"),
			BiomeActors, Components, Instances.Num(), Placements.Num(), Matched));
		TestEqual(TEXT("Every manifest placement is a live instance"), Matched, Placements.Num());
		TestEqual(TEXT("Every live instance is a manifest placement"), Instances.Num(), Placements.Num());
		const TSharedPtr<FJsonObject> Counts = Manifest->GetObjectField(TEXT("counts"));
		for (const FString& Family : Keys(Counts))
		{
			TestEqual(*FString::Printf(TEXT("%s count"), *Family), FamilyCounts.FindRef(Family),
				static_cast<int32>(Counts->GetNumberField(Family)));
		}

		// 4. Authored and pad exclusions, read from the zone files and island.json (the test cannot drift from them).
		TMap<FString, TArray<FVector2D>> PathPoints;
		TMap<FString, double> PathHalfWidths;
		const TArray<TSharedPtr<FJsonValue>>* IslandPaths = nullptr;
		if (Island->TryGetArrayField(TEXT("paths"), IslandPaths))
		{
			for (const TSharedPtr<FJsonValue>& Value : *IslandPaths)
			{
				const TSharedPtr<FJsonObject> Path = Value->AsObject();
				TArray<FVector2D> Points;
				for (const TSharedPtr<FJsonValue>& Point : Path->GetArrayField(TEXT("points")))
				{
					Points.Add(Pair(Point->AsArray()));
				}
				double HalfWidth = 1.5;
				Path->TryGetNumberField(TEXT("half_width_m"), HalfWidth);
				PathPoints.Add(Path->GetStringField(TEXT("id")), Points);
				PathHalfWidths.Add(Path->GetStringField(TEXT("id")), HalfWidth);
			}
		}
		TMap<FString, FVector2D> PadCenters;
		TMap<FString, double> PadRadii;
		for (const TSharedPtr<FJsonValue>& Value : Island->GetArrayField(TEXT("pads")))
		{
			const TSharedPtr<FJsonObject> Pad = Value->AsObject();
			PadCenters.Add(Pad->GetStringField(TEXT("name")), Pair(Pad->GetArrayField(TEXT("center"))));
			PadRadii.Add(Pad->GetStringField(TEXT("name")), Pad->GetNumberField(TEXT("radius")));
		}
		TArray<FString> ZoneFiles;
		const FString ZoneDir = FPaths::ProjectDir() / TEXT("Tools/Biomes/zones");
		IFileManager::Get().FindFiles(ZoneFiles, *(ZoneDir / TEXT("*.json")), true, false);
		int32 Shapes = 0, Inside = 0;
		for (const FString& File : ZoneFiles)
		{
			const TSharedPtr<FJsonObject> Zone = LoadJson(*this, TEXT("Tools/Biomes/zones/") + File);
			if (!Zone)
			{
				continue;
			}
			const FString ZoneId = Zone->GetStringField(TEXT("id"));
			TArray<FShape> ZoneShapes;
			const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
			if (Zone->TryGetArrayField(TEXT("exclusions"), Values))
			{
				for (const TSharedPtr<FJsonValue>& Value : *Values)
				{
					ZoneShapes.Add(ShapeFrom(Value->AsObject(), Value->AsObject()->GetStringField(TEXT("id"))));
				}
			}
			if (Zone->TryGetArrayField(TEXT("exclude_pads"), Values))
			{
				for (const TSharedPtr<FJsonValue>& Value : *Values)
				{
					const FString Name = Value->AsObject()->GetStringField(TEXT("name"));
					FShape Pad;
					Pad.Id = TEXT("pad ") + Name;
					Pad.Kind = TEXT("circle");
					Pad.Center = PadCenters.FindRef(Name);
					Pad.Radius = PadRadii.FindRef(Name);
					Value->AsObject()->TryGetNumberField(TEXT("margin_m"), Pad.Margin);
					TestTrue(*FString::Printf(TEXT("Pad '%s' exists in island.json"), *Name), PadRadii.Contains(Name));
					ZoneShapes.Add(Pad);
				}
			}
			if (Zone->TryGetArrayField(TEXT("exclude_paths"), Values))
			{
				for (const TSharedPtr<FJsonValue>& Value : *Values)
				{
					const FString Id = Value->AsObject()->GetStringField(TEXT("id"));
					FShape Trail;
					Trail.Id = TEXT("path ") + Id;
					Trail.Kind = TEXT("polyline");
					Trail.Points = PathPoints.FindRef(Id);
					Trail.Radius = PathHalfWidths.FindRef(Id);
					Value->AsObject()->TryGetNumberField(TEXT("margin_m"), Trail.Margin);
					TestTrue(*FString::Printf(TEXT("Path '%s' exists in island.json"), *Id), PathPoints.Contains(Id));
					ZoneShapes.Add(Trail);
				}
			}
			Shapes += ZoneShapes.Num();
			for (const FInstance& Instance : Instances)
			{
				if (Instance.Zone != ZoneId)
				{
					continue;
				}
				const FVector2D P(Instance.Transform.GetLocation().X / 100.0, Instance.Transform.GetLocation().Y / 100.0);
				for (const FShape& Shape : ZoneShapes)
				{
					FShape Strict = Shape;
					Strict.Margin -= 0.002;   // the stored point is rounded to 1 mm
					if (Strict.Contains(P))
					{
						++Inside;
						AddError(FString::Printf(TEXT("%s instance at (%.2f, %.2f) m is inside exclusion %s"),
							*Instance.Family, P.X, P.Y, *Shape.Id));
					}
				}
			}
		}
		TestTrue(TEXT("The zone files declare exclusions to check"), Shapes > 0);
		TestEqual(TEXT("No instance inside an authored or pad exclusion"), Inside, 0);

		// 5. Automatic radii around the live map's actors (exterior only), from the recipe's own numbers.
		const TSharedPtr<FJsonObject> Radii = Recipe->GetObjectField(TEXT("automatic_radii_m"));
		struct FLive { FString Id; FString Kind; FVector2D Center; FVector2D Half = FVector2D::ZeroVector; double Yaw = 0.0; };
		TArray<FLive> Live;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			const FVector L = Actor->GetActorLocation();
			if (L.Y > 200000.0 || Actor->IsA<ADCPlayerCharacter>())
			{
				continue;
			}
			const FVector2D C(L.X / 100.0, L.Y / 100.0);
			if (Actor->IsA<ADCCellPortal>())
			{
				Live.Add({ Actor->GetActorNameOrLabel(), TEXT("portal"), C });
			}
			else if (Actor->GetClass()->ImplementsInterface(UDCInteractable::StaticClass()))
			{
				Live.Add({ Actor->GetActorNameOrLabel(), TEXT("interactable"), C });
			}
			else if (Actor->IsA<APlayerStart>())
			{
				Live.Add({ Actor->GetActorNameOrLabel(), TEXT("player_start"), C });
			}
			else if (Actor->IsA<ACharacter>())
			{
				Live.Add({ Actor->GetActorNameOrLabel(), TEXT("npc"), C });
			}
			else if (const ADCLocationVolume* Volume = Cast<ADCLocationVolume>(Actor))
			{
				const UBoxComponent* Box = Volume->GetBounds();
				const FVector Extent = Box ? Box->GetScaledBoxExtent() : FVector::ZeroVector;
				const FVector Centre = Box ? Box->GetComponentLocation() : L;
				Live.Add({ Actor->GetActorNameOrLabel(), TEXT("location_volume"), FVector2D(Centre.X / 100.0, Centre.Y / 100.0),
					FVector2D(Extent.X / 100.0, Extent.Y / 100.0), Actor->GetActorRotation().Yaw });
			}
		}
		int32 TooClose = 0, NearZone = 0;
		for (const FLive& Item : Live)
		{
			double Radius = 0.0;
			TestTrue(*FString::Printf(TEXT("The recipe has a radius for %s"), *Item.Kind), Radii->TryGetNumberField(Item.Kind, Radius));
			FShape Shape;
			Shape.Id = Item.Id;
			Shape.Kind = Item.Half.IsZero() ? TEXT("circle") : TEXT("box");
			Shape.Center = Item.Center;
			Shape.Half = Item.Half;
			Shape.YawDeg = Item.Yaw;
			Shape.Margin = Radius - 0.01;
			FShape Near = Shape;
			Near.Margin = Radius * 3.0;
			bool bNearAny = false;
			for (const FInstance& Instance : Instances)
			{
				const FVector2D P(Instance.Transform.GetLocation().X / 100.0, Instance.Transform.GetLocation().Y / 100.0);
				if (Shape.Contains(P))
				{
					++TooClose;
					AddError(FString::Printf(TEXT("%s instance at (%.2f, %.2f) m is within %.1f m of %s %s"),
						*Instance.Family, P.X, P.Y, Radius, *Item.Kind, *Item.Id));
				}
				bNearAny |= Near.Contains(P);
			}
			NearZone += bNearAny ? 1 : 0;
		}
		AddInfo(FString::Printf(TEXT("Automatic exclusions: %d live actors, %d with instances within three radii"), Live.Num(), NearZone));
		TestTrue(TEXT("Live actors to check exist"), Live.Num() > 0);
		TestEqual(TEXT("No instance within an automatic radius"), TooClose, 0);

		// 6. Seated, not floating: each instance's lowest point is at or below the terrain under its pivot.
		int32 Floating = 0, NoGround = 0;
		for (const FInstance& Instance : Instances)
		{
			const FVector L = Instance.Transform.GetLocation();
			// Down from 200 m to the terrain, passing through anything else (a hidden edge wall near the outline).
			FCollisionQueryParams Params(SCENE_QUERY_STAT(SombreBiomeProbe), true, Player());
			FHitResult Hit;
			bool bGround = false;
			for (int32 Attempt = 0; Attempt < 6 && !bGround; ++Attempt)
			{
				if (!World->LineTraceSingleByChannel(Hit, FVector(L.X, L.Y, 20000.0), FVector(L.X, L.Y, -3000.0), ECC_WorldStatic, Params))
				{
					break;
				}
				bGround = Hit.GetActor() && Hit.GetActor()->ActorHasTag(TEXT("SombreTerrain"));
				if (!bGround && Hit.GetActor())
				{
					Params.AddIgnoredActor(Hit.GetActor());
				}
			}
			if (!bGround)
			{
				++NoGround;
				continue;
			}
			if (Instance.BottomZ > Hit.ImpactPoint.Z + 1.0)
			{
				++Floating;
				AddError(FString::Printf(TEXT("%s instance at (%.0f, %.0f) floats %.1f cm above the terrain"),
					*Instance.Family, L.X, L.Y, Instance.BottomZ - Hit.ImpactPoint.Z));
			}
		}
		TestEqual(TEXT("Every instance stands on the terrain"), NoGround, 0);
		TestEqual(TEXT("No instance floats"), Floating, 0);
		return true;
	}));

	QueueCleanup(TestSlot);
	return true;
}

#endif
