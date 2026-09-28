#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/WorldSettings.h"

/**
 *  A throwaway game world for automation tests: world subsystems exist (persistent registry,
 *  world state) and BeginPlay has been dispatched, so components added with AddComponent run
 *  their BeginPlay just as they would in a level. Destroyed when it goes out of scope.
 */
class FDCTestWorld
{
public:

	FDCTestWorld()
	{
		World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("DCTestWorld"));
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		World->GetWorldSettings()->NotifyBeginPlay();
	}

	~FDCTestWorld()
	{
		World->EndPlay(EEndPlayReason::Quit);
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}

	UWorld* Get() const { return World; }

	AActor* SpawnActor() const
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
	}

	/** Creates, configures, then registers a component (registration runs BeginPlay). */
	template <class T>
	static T* AddComponent(AActor* Actor, TFunctionRef<void(T*)> Setup)
	{
		T* Component = NewObject<T>(Actor);
		Setup(Component);
		Component->RegisterComponent();
		return Component;
	}

	template <class T>
	static T* AddComponent(AActor* Actor)
	{
		return AddComponent<T>(Actor, [](T*) {});
	}

private:

	UWorld* World = nullptr;
};

#endif
