// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProjectBreachCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class PROJECTBREACH_API AProjectBreachCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* SprintAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AimAction;

	/** Maximum movement speed while sprinting */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Speed", meta=(ClampMin="0.0", Units="cm/s"))
	float SprintSpeed = 750.0f;

	/** Maximum movement speed while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Speed", meta=(ClampMin="0.0", Units="cm/s"))
	float AimSpeed = 300.0f;

	/** Minimum input magnitude required to activate sprinting */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Movement|Sprint", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MinimumSprintInput = 0.1f;

	/** Camera boom length while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim", meta=(ClampMin="0.0", Units="cm"))
	float AimedCameraArmLength = 250.0f;

	/** Camera field of view while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim", meta=(ClampMin="5.0", ClampMax="170.0", Units="degrees"))
	float AimedFieldOfView = 70.0f;

	/** Camera socket offset while aiming */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim")
	FVector AimedCameraSocketOffset = FVector(0.0f, 60.0f, 0.0f);

	/** Speed used to interpolate between normal and aimed camera settings */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Camera|Aim", meta=(ClampMin="0.0"))
	float CameraInterpolationSpeed = 10.0f;

	/** True while the character is actively sprinting */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Movement|State", meta=(AllowPrivateAccess="true"))
	bool bIsSprinting = false;

	/** True while the character is actively aiming */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category="Movement|State", meta=(AllowPrivateAccess="true"))
	bool bIsAiming = false;

	/** True while the player is holding the sprint input */
	bool bSprintRequested = false;

	/** True while the player is providing meaningful movement input */
	bool bHasMovementInput = false;

	/** Normal movement speed captured from the Character Movement component */
	float NormalWalkSpeed = 0.0f;

	/** Normal camera settings captured after Blueprint defaults are applied */
	float NormalCameraArmLength = 0.0f;
	float NormalCameraFieldOfView = 0.0f;
	FVector NormalCameraSocketOffset = FVector::ZeroVector;

public:

	/** Constructor */
	AProjectBreachCharacter();

	/** Update camera interpolation */
	virtual void Tick(float DeltaSeconds) override;

protected:

	/** Capture Blueprint-configured movement and camera defaults */
	virtual void BeginPlay() override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Reset held input state when control is removed from this character */
	virtual void UnPossessed() override;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called when movement input stops */
	void MoveCompleted(const FInputActionValue& Value);

	/** Raw sprint input callbacks */
	void SprintStarted(const FInputActionValue& Value);
	void SprintCompleted(const FInputActionValue& Value);

	/** Raw aim input callbacks */
	void AimStarted(const FInputActionValue& Value);
	void AimCompleted(const FInputActionValue& Value);

	/** Reevaluate whether the current sprint request can become active */
	void RefreshSprintState();

	/** Apply validated locomotion state changes */
	void SetSprinting(bool bNewIsSprinting);
	void SetAiming(bool bNewIsAiming);

	/** Apply the movement speed selected by the current state */
	void ApplyMovementSpeed();

	/** Smoothly update camera settings toward the current state target */
	void UpdateCamera(float DeltaSeconds);

	/** Clear transient locomotion input and state */
	void ResetLocomotionState();

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Handles sprint pressed inputs from controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	/** Handles sprint released inputs from controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintEnd();

	/** Handles aim pressed inputs from controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAimStart();

	/** Handles aim released inputs from controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAimEnd();

	/** Returns true while the character is actively sprinting */
	UFUNCTION(BlueprintPure, Category="Movement|State")
	bool IsSprinting() const { return bIsSprinting; }

	/** Returns true while the character is actively aiming */
	UFUNCTION(BlueprintPure, Category="Movement|State")
	bool IsAiming() const { return bIsAiming; }

	/** Returns true when the current locomotion state permits firing */
	UFUNCTION(BlueprintPure, Category="Combat")
	bool CanFire() const { return !bIsSprinting; }

	/** Cosmetic Blueprint notification for sprint state changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Movement|State", meta=(DisplayName="On Sprint State Changed"))
	void BP_OnSprintStateChanged(bool bNewIsSprinting);

	/** Cosmetic Blueprint notification for aim state changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Movement|State", meta=(DisplayName="On Aim State Changed"))
	void BP_OnAimStateChanged(bool bNewIsAiming);

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

