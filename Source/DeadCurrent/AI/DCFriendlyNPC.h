#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/DCInteractable.h"
#include "DCFriendlyNPC.generated.h"

class UDCDialogueAsset;

/**
 *  Idle friendly character. Talk starts a dialogue asset (FP-11).
 */
UCLASS()
class DEADCURRENT_API ADCFriendlyNPC : public ACharacter, public IDCInteractable
{
	GENERATED_BODY()

public:
	ADCFriendlyNPC();

	virtual void Tick(float DeltaSeconds) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual FDCInteractionPrompt GetInteractionPrompt_Implementation(AActor* Interactor) const override;
	virtual FGameplayTag GetInteractionType_Implementation() const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC")
	FText DisplayName;

	/** Conversation started by Talk. Required for FP-11. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC")
	TSoftObjectPtr<UDCDialogueAsset> Dialogue;

	/** Fallback line if Dialogue is unset (editor / early setup). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC", meta=(MultiLine="true"))
	FText Greeting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC", meta=(ClampMin="0", Units="s"))
	float GreetingDuration = 5.0f;

	/** Yaw toward the player when they are this close */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC", meta=(ClampMin="0", Units="cm"))
	float NoticeRange = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC", meta=(ClampMin="0"))
	float TurnSpeed = 6.0f;

private:

	void FaceActor(const AActor* Target, float DeltaSeconds);
};
