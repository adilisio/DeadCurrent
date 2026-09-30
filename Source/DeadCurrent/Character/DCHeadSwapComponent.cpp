#include "Character/DCHeadSwapComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"

UDCHeadSwapComponent::UDCHeadSwapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDCHeadSwapComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* Body = Character ? Character->GetMesh() : nullptr;
	UStaticMesh* Head = HeadMesh.LoadSynchronous();
	if (!Body || !Head || !Body->GetSkeletalMeshAsset() || Body->GetBoneIndex(AttachBone) == INDEX_NONE)
	{
		return;
	}

	Body->HideBoneByName(AttachBone, PBO_Term);

	HeadComponent = NewObject<UStaticMeshComponent>(Character, TEXT("SwappedHead"));
	HeadComponent->SetStaticMesh(Head);
	HeadComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadComponent->SetCanEverAffectNavigation(false);
	HeadComponent->SetRelativeLocation(Offset);
	HeadComponent->SetRelativeRotation(Rotation);
	HeadComponent->SetRelativeScale3D(FVector(Scale));
	HeadComponent->SetupAttachment(Body, AttachBone);
	HeadComponent->RegisterComponent();
	HeadComponent->AttachToComponent(Body, FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachBone);
	HeadComponent->SetRelativeLocation(Offset);
	HeadComponent->SetRelativeRotation(Rotation);
}
