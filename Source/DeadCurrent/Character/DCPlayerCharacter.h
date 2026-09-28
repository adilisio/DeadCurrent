#pragma once

#include "CoreMinimal.h"
#include "Combat/DCDamageable.h"
#include "GameFramework/Character.h"
#include "DCPlayerCharacter.generated.h"

class UAIPerceptionStimuliSourceComponent;
class UCameraComponent;
class ADCFirearm;
class UDCItemDefinition;
class UDCInteractorComponent;
class UDCInventoryComponent;
class UDCHealthComponent;
class UDCDialogueComponent;
class UDCQuestComponent;
class UInputAction;
class UInputComponent;
class USkeletalMeshComponent;
struct FInputActionValue;

/**
 *  First person player character.
 */
UCLASS(abstract)
class DEADCURRENT_API ADCPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UDCInteractorComponent* InteractorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UDCInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UDCHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UAIPerceptionStimuliSourceComponent* StimuliSource;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UDCDialogueComponent* DialogueComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UDCQuestComponent* QuestComponent;

protected:

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Gamepad/keyboard look */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Held to sprint */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

	/** Pressed to toggle crouch */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* CrouchAction;

	/** Pressed to use the focused interactable */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;

	/** Pressed to show or hide the inventory */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InventoryAction;

	/** Pressed to fire the equipped weapon */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* FireAction;

	/** Pressed to reload the equipped weapon */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ReloadAction;

	/** Pressed to holster or draw the carried firearm */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* EquipWeaponAction;

	/** Ground speed while sprinting. Normal speed is the movement component's MaxWalkSpeed. */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="0", Units="cm/s"))
	float SprintSpeed = 700.0f;

	/** Minimum forward input needed to sprint, so strafing and backpedaling stay at normal speed */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="0", ClampMax="1"))
	float SprintMinForwardInput = 0.5f;

	/** How far the view drops when fully crouched */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="0", Units="cm"))
	float CrouchEyeDrop = 70.0f;

	/** How quickly the view moves between standing and crouched height */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="0"))
	float CrouchEyeInterpSpeed = 12.0f;

public:
	ADCPlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category="Movement")
	bool IsSprinting() const { return bIsSprinting; }

protected:

	virtual void BeginPlay() override;

	void MoveInput(const FInputActionValue& Value);

	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchToggle();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoInteract();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoToggleInventory();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoFire();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoReload();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoToggleWeapon();

	UFUNCTION()
	void HandleInventoryChanged(UDCInventoryComponent* Inventory);

	UFUNCTION()
	void HandleDied(UDCHealthComponent* Health, const FDCDamageInfo& Damage);

	UFUNCTION()
	void HandleQuestUpdated(FName QuestId, FName StageId);

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:

	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UDCInteractorComponent* GetInteractorComponent() const { return InteractorComponent; }

	UDCInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	ADCFirearm* GetEquippedFirearm() const { return EquippedFirearm; }

	UDCHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UDCDialogueComponent* GetDialogueComponent() const { return DialogueComponent; }

	UDCQuestComponent* GetQuestComponent() const { return QuestComponent; }

	void BeginSaveRestore() { bRestoringSave = true; }

	void EndSaveRestore() { bRestoringSave = false; }

	void ClearEquippedFirearm();

	const UInputAction* GetInteractAction() const { return InteractAction; }

	const UInputAction* GetReloadAction() const { return ReloadAction; }

private:

	void UpdateSprint();

	void UpdateCrouchEyeHeight(float DeltaSeconds);

	void UpdateRecoilRecovery(float DeltaSeconds);

	const UDCItemDefinition* FindFirearmInInventory() const;

	void SpawnAndEquip(const UDCItemDefinition* Definition, bool bAutoReload = true);

	void Respawn();

	/** MaxWalkSpeed as authored on the movement component */
	float BaseWalkSpeed = 0.0f;

	/** First person mesh offset as authored; the crouch eye offset is applied on top of it */
	FVector BaseFirstPersonMeshLocation = FVector::ZeroVector;

	float CurrentCrouchEyeOffset = 0.0f;

	/** Forward axis of the most recent move input */
	float LastForwardInput = 0.0f;

	bool bSprintHeld = false;

	bool bIsSprinting = false;

	UPROPERTY()
	TObjectPtr<ADCFirearm> EquippedFirearm;

	float RecoilToRecover = 0.0f;

	float RecoilRecoverySpeed = 12.0f;

	FTimerHandle RespawnTimer;

	bool bRestoringSave = false;
};
