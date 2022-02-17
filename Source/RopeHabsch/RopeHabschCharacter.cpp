// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "RopeHabschCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "RopeHabschSwingComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "CableComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "HookPoint.h"
#include "ScanComponent.h"
#include "SwingPoint.h"
#include "RopeHabsxhHookComponent.h"

//////////////////////////////////////////////////////////////////////////
// ARopeHabschCharacter

ARopeHabschCharacter::ARopeHabschCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	root = Cast<USceneComponent>(GetComponentByClass(USceneComponent::StaticClass()));
	
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ARopeHabschCharacter::BeginPlay()
{
	Super::BeginPlay();
	SwingComponent = Cast<URopeHabschSwingComponent>(GetComponentByClass(URopeHabschSwingComponent::StaticClass()));
	HookComponent = Cast<URopeHabsxhHookComponent>(GetComponentByClass(URopeHabsxhHookComponent::StaticClass()));
	Mesh = Cast<USkeletalMeshComponent>(GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	ScanComponent = Cast<UScanComponent>(GetComponentByClass(UScanComponent::StaticClass()));
	Cable = Cast<UCableComponent>(GetComponentByClass(UCableComponent::StaticClass()));
	InitialRotation = GetActorRotation();
}

void ARopeHabschCharacter::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	swingCoolDown -= DeltaTime;
	CheckIfShouldRotCorrect();
	Cable->SetWorldLocation(Mesh->GetSocketTransform("SwingSocket").GetLocation());
	
	if(!bIsSwinging && bShouldRotCorrect) //Player let go of rope - correct to initial rotation
	{
		UE_LOG(LogTemp, Warning, TEXT("Correcting"));
		FRotator currentRotation = GetActorRotation();
		float lerpedX = UKismetMathLibrary::Lerp(currentRotation.Roll, InitialRotation.Roll, 0.9f);
		float lerpedY = UKismetMathLibrary::Lerp(currentRotation.Pitch, 0, 0.9f);
		float lerpedZ = UKismetMathLibrary::Lerp(currentRotation.Yaw, 0, 0.9f);
		
		FRotator newRot = FRotator(lerpedY, lerpedZ, lerpedX);
		SetActorRotation(newRot);
		
		if(FMath::IsNearlyEqual(lerpedX,InitialRotation.Roll)) //We're back to initial rot.X - stop correcting
		{
			UE_LOG(LogTemp, Warning, TEXT("DONE"));
			Mesh->SetAnimInstanceClass(SwingComponent->defaultAnim);
			InitialRotation = GetActorRotation();
			bShouldRotCorrect = false;
		}
	}
}

void ARopeHabschCharacter::CheckIfShouldRotCorrect()
{
	FHitResult hit;
	TArray<AActor*> IgnoreList;
	IgnoreList.Add(Cast<AActor>(this));
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_WorldDynamic);
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), GetActorLocation(), GetActorLocation() + FVector::DownVector * 100, TraceType, false, IgnoreList, EDrawDebugTrace::ForOneFrame, hit, true);
	if(hit.bBlockingHit) //We hit ground, Reset Camera & WalkSpeed - Stop RotCorrecting
	{
    	GetCameraBoom()->CameraLagSpeed = 50.f;
		GetCameraBoom()->bEnableCameraLag = false;
		GetCharacterMovement()->MaxWalkSpeed = 600;
		
		bShouldRotCorrect = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARopeHabschCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ARopeHabschCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ARopeHabschCharacter::StopInteract);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARopeHabschCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARopeHabschCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARopeHabschCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARopeHabschCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ARopeHabschCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ARopeHabschCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ARopeHabschCharacter::OnResetVR);
}

void ARopeHabschCharacter::Interact()
{
	if(swingCoolDown > 0)
		return;
	if(ScanComponent->CurrentAttachPoint == nullptr)
		return;
		
	if(ScanComponent->CurrentAttachPoint->IsA(ASwingPoint::StaticClass())) //Are we attaching to a SwingPoint?
	{
		bIsSwinging = true;
		SwingComponent->StartSwinging();	
	}
	else if(ScanComponent->CurrentAttachPoint->IsA(AHookPoint::StaticClass())) //Are we attaching to a HookPoint?
	{
		HookComponent->StartHook();
	}
}

void ARopeHabschCharacter::StopInteract()
{
	if(ScanComponent->CurrentAttachPoint == nullptr)
		return;
	
	swingCoolDown = 0.1f;
	
	if(ScanComponent->CurrentAttachPoint->IsA(ASwingPoint::StaticClass())) // Stop Swinging
	{
		bIsSwinging = false;
		bShouldRotCorrect = true;
		SwingComponent->StopSwinging();	
	}
	else if(ScanComponent->CurrentAttachPoint->IsA(AHookPoint::StaticClass())) // Stop Hooking
	{
		HookComponent->StopHook();
	}
}

void ARopeHabschCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ARopeHabschCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ARopeHabschCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ARopeHabschCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARopeHabschCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ARopeHabschCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ARopeHabschCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
