// Copyright Epic Games, Inc. All Rights Reserved.

#include "Characters/ProjectBreachCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ProjectBreach.h"

AProjectBreachCharacter::AProjectBreachCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AProjectBreachCharacter::BeginPlay()
{
	Super::BeginPlay();

	NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	NormalCameraArmLength = CameraBoom->TargetArmLength;
	NormalCameraSocketOffset = CameraBoom->SocketOffset;
	NormalCameraFieldOfView = FollowCamera->FieldOfView;

	ApplyMovementSpeed();
}

void AProjectBreachCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCamera(DeltaSeconds);
}

void AProjectBreachCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AProjectBreachCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AProjectBreachCharacter::MoveCompleted);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AProjectBreachCharacter::MoveCompleted);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AProjectBreachCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AProjectBreachCharacter::Look);

		// Sprinting
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AProjectBreachCharacter::SprintStarted);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AProjectBreachCharacter::SprintCompleted);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AProjectBreachCharacter::SprintCompleted);
		}

		// Aiming
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AProjectBreachCharacter::AimStarted);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AProjectBreachCharacter::AimCompleted);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &AProjectBreachCharacter::AimCompleted);
		}
	}
	else
	{
		UE_LOG(LogProjectBreach, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AProjectBreachCharacter::UnPossessed()
{
	Super::UnPossessed();

	ResetLocomotionState();
}

void AProjectBreachCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AProjectBreachCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AProjectBreachCharacter::MoveCompleted(const FInputActionValue& Value)
{
	bHasMovementInput = false;
	RefreshSprintState();
}

void AProjectBreachCharacter::SprintStarted(const FInputActionValue& Value)
{
	DoSprintStart();
}

void AProjectBreachCharacter::SprintCompleted(const FInputActionValue& Value)
{
	DoSprintEnd();
}

void AProjectBreachCharacter::AimStarted(const FInputActionValue& Value)
{
	DoAimStart();
}

void AProjectBreachCharacter::AimCompleted(const FInputActionValue& Value)
{
	DoAimEnd();
}

void AProjectBreachCharacter::DoMove(float Right, float Forward)
{
	const FVector2D MovementInput(Right, Forward);
	bHasMovementInput = MovementInput.SizeSquared() > FMath::Square(MinimumSprintInput);
	RefreshSprintState();

	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AProjectBreachCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AProjectBreachCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AProjectBreachCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void AProjectBreachCharacter::DoSprintStart()
{
	bSprintRequested = true;
	RefreshSprintState();

	if (bIsAiming)
	{
		UE_LOG(LogProjectBreach, Log, TEXT("'%s' Sprint requested while aiming; request will remain pending."), *GetNameSafe(this));
	}
}

void AProjectBreachCharacter::DoSprintEnd()
{
	bSprintRequested = false;
	RefreshSprintState();
}

void AProjectBreachCharacter::DoAimStart()
{
	SetAiming(true);
}

void AProjectBreachCharacter::DoAimEnd()
{
	SetAiming(false);
}

void AProjectBreachCharacter::RefreshSprintState()
{
	SetSprinting(bSprintRequested && bHasMovementInput && !bIsAiming);
}

void AProjectBreachCharacter::SetSprinting(bool bNewIsSprinting)
{
	if (bIsSprinting == bNewIsSprinting)
	{
		return;
	}

	bIsSprinting = bNewIsSprinting;
	ApplyMovementSpeed();

	UE_LOG(LogProjectBreach, Log, TEXT("'%s' Sprint state changed: %s"), *GetNameSafe(this), bIsSprinting ? TEXT("Active") : TEXT("Inactive"));
	BP_OnSprintStateChanged(bIsSprinting);
}

void AProjectBreachCharacter::SetAiming(bool bNewIsAiming)
{
	if (bIsAiming == bNewIsAiming)
	{
		return;
	}

	bIsAiming = bNewIsAiming;
	RefreshSprintState();
	ApplyMovementSpeed();

	UE_LOG(LogProjectBreach, Log, TEXT("'%s' Aim state changed: %s"), *GetNameSafe(this), bIsAiming ? TEXT("Active") : TEXT("Inactive"));
	BP_OnAimStateChanged(bIsAiming);
}

void AProjectBreachCharacter::ApplyMovementSpeed()
{
	float TargetSpeed = NormalWalkSpeed;

	if (bIsAiming)
	{
		TargetSpeed = AimSpeed;
	}
	else if (bIsSprinting)
	{
		TargetSpeed = SprintSpeed;
	}

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
}

void AProjectBreachCharacter::UpdateCamera(float DeltaSeconds)
{
	const float TargetArmLength = bIsAiming ? AimedCameraArmLength : NormalCameraArmLength;
	const float TargetFieldOfView = bIsAiming ? AimedFieldOfView : NormalCameraFieldOfView;
	const FVector TargetSocketOffset = bIsAiming ? AimedCameraSocketOffset : NormalCameraSocketOffset;

	if (CameraInterpolationSpeed <= 0.0f)
	{
		CameraBoom->TargetArmLength = TargetArmLength;
		CameraBoom->SocketOffset = TargetSocketOffset;
		FollowCamera->SetFieldOfView(TargetFieldOfView);
		return;
	}

	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, TargetArmLength, DeltaSeconds, CameraInterpolationSpeed);
	CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, TargetSocketOffset, DeltaSeconds, CameraInterpolationSpeed);
	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFieldOfView, DeltaSeconds, CameraInterpolationSpeed));
}

void AProjectBreachCharacter::ResetLocomotionState()
{
	bSprintRequested = false;
	bHasMovementInput = false;
	SetAiming(false);
	SetSprinting(false);

	UE_LOG(LogProjectBreach, Log, TEXT("'%s' Locomotion state reset after unpossession."), *GetNameSafe(this));
}
