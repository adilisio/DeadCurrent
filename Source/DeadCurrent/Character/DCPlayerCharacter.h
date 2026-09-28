#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DCPlayerCharacter.generated.h"

class UCameraComponent;
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

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:

	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

private:

	void UpdateSprint();

	void UpdateCrouchEyeHeight(float DeltaSeconds);

	/** MaxWalkSpeed as authored on the movement component */
	float BaseWalkSpeed = 0.0f;

	/** First person mesh offset as authored; the crouch eye offset is applied on top of it */
	FVector BaseFirstPersonMeshLocation = FVector::ZeroVector;

	float CurrentCrouchEyeOffset = 0.0f;

	/** Forward axis of the most recent move input */
	float LastForwardInput = 0.0f;

	bool bSprintHeld = false;

	bool bIsSprinting = false;
};
