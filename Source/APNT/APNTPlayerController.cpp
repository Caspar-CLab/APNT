// Copyright Epic Games, Inc. All Rights Reserved.

#include "APNTPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "APNTCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AAPNTPlayerController::AAPNTPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void AAPNTPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	DefaultCameraLocation = GetPawn()->GetActorLocation();
	DefaultCameraRotation = GetPawn()->GetActorRotation();
}

void AAPNTPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		UE_LOG(LogTemp, Warning, TEXT("### SetupInputcomponent()"));
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
		Subsystem->AddMappingContext(PlayerInputMapping, 0);
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AAPNTPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AAPNTPlayerController::OnSetDestinationTriggered);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AAPNTPlayerController::OnSetDestinationReleased);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AAPNTPlayerController::OnSetDestinationReleased);

		// Setup touch input events
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &AAPNTPlayerController::OnInputStarted);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &AAPNTPlayerController::OnTouchTriggered);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &AAPNTPlayerController::OnTouchReleased);
		EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &AAPNTPlayerController::OnTouchReleased);

		// Camera Controller
		EnhancedInputComponent->BindAction(IA_CameraControl, ETriggerEvent::Started, this, &AAPNTPlayerController::StartCameraControl);
		EnhancedInputComponent->BindAction(IA_CameraControl, ETriggerEvent::Completed, this, &AAPNTPlayerController::StopCameraControl);
		EnhancedInputComponent->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &AAPNTPlayerController::RotateCamera);
		
		// Moving Controller
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AAPNTPlayerController::Move);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AAPNTPlayerController::OnInputStarted()
{
	StopMovement();
}

// Triggered every frame when the input is held down
void AAPNTPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void AAPNTPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void AAPNTPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void AAPNTPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void AAPNTPlayerController::StartCameraControl()
{
	AActor* ControlledPawn = GetPawn();
	UCameraComponent* CameraComponent = ControlledPawn->FindComponentByClass<UCameraComponent>();
	FRotator CameraRotation = CameraComponent->GetComponentRotation();
	UE_LOG(LogTemp, Warning, TEXT("### Start Camera Rotation: %s"), *CameraRotation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("### StartCameraControl"));
	bIsCameraControlled = true;
}

void AAPNTPlayerController::StopCameraControl()
{
	UE_LOG(LogTemp, Warning, TEXT("### StopCameraControl"));
	bIsCameraControlled = false;
}

void AAPNTPlayerController::RotateCamera(const FInputActionValue& InputValue)
{
	AActor* ControlledPawn = GetPawn();
	if (!bIsCameraControlled || !ControlledPawn)
		return;

	FVector2D InputAxis = InputValue.Get<FVector2D>();
	USpringArmComponent* SpringArm = ControlledPawn->FindComponentByClass<USpringArmComponent>();
	UCameraComponent* CameraComponent = ControlledPawn->FindComponentByClass<UCameraComponent>();

	if (SpringArm)
	{
		FRotator NewRotation = SpringArm->GetComponentRotation();
		NewRotation.Yaw += InputAxis.X * CameraRotationSpeed;
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - InputAxis.Y * CameraRotationSpeed, -80.0f, 80.0f);

		SpringArm->SetWorldRotation(NewRotation);
		UE_LOG(LogTemp, Warning, TEXT("SpringArm Rotation: %s"), *NewRotation.ToString());
	}
}

void AAPNTPlayerController::Move(const FInputActionValue& InputValue)
{
	UE_LOG(LogTemp, Warning, TEXT("### Move"));
	FVector2D MovementVector = InputValue.Get<FVector2D>();
	APawn* ControlledPawn = GetPawn();

	if (ControlledPawn)
	{
		FVector Forward = ControlledPawn->GetActorForwardVector() * MovementVector.Y;
		FVector Right = ControlledPawn->GetActorRightVector() * MovementVector.X;

        FVector Movement = (Forward + Right).GetClampedToMaxSize(1.0f);
        ControlledPawn->AddMovementInput(Movement);
	}
}
