#include "RopeHabschSwingComponent.h"
#include "Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CableComponent.h"
#include "RopeHabschCharacter.h"
#include "Kismet/KismetMathLibrary.h"

URopeHabschSwingComponent::URopeHabschSwingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void URopeHabschSwingComponent::BeginPlay()
{
	Super::BeginPlay();

	playerMesh = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	scanComponent = Cast<UScanComponent>(GetOwner()->GetComponentByClass(UScanComponent::StaticClass()));
	player = Cast<ARopeHabschCharacter>(GetOwner());
}

void URopeHabschSwingComponent::StartSwinging()
{	
	if (scanComponent->CurrentAttachPoint != nullptr)
	{	
		float distance = FVector::Distance(player->GetActorLocation(), scanComponent->CurrentAttachPoint->GetActorLocation());
		if(distance > MaxRopeLength)
			return;
		
		CurrentAttach = scanComponent->CurrentAttachPoint;
		CurrentAttach->CreateRope(GetOwner());
		
		UCharacterMovementComponent* MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
		MovementComponent->GravityScale = 0.f;

		FVector SwingLocation = CurrentAttach->GetActorLocation();
		FVector CharacterLocation = GetOwner()->GetActorLocation();

		RopeLength = FMath::Min(FVector::Dist(SwingLocation, CharacterLocation) - 100.f, MaxRopeLength);
		SwingDirection = (SwingLocation - CharacterLocation).GetSafeNormal();

		FQuat CurrentRot = FQuat::FindBetweenVectors(-1.f * SwingDirection, -1.f * FVector::UpVector);
		CurrentAngle = CurrentRot.GetAngle();
		
		StartRot = UKismetMathLibrary::FindLookAtRotation(player->GetActorLocation(), CurrentAttach->GetActorLocation());
		GetOwner()->SetActorRotation(StartRot);

		player->Cable->SetVisibility(true);
		player->Cable->SetAttachEndToComponent(CurrentAttach->root);
		player->Cable->CableLength = RopeLength;
		player->Cable->bAttachEnd = true;
	}
}

void URopeHabschSwingComponent::StopSwinging()
{
	if (CurrentAttach == nullptr)
		return;

	CurrentAttach->DestroyRope();
	CurrentAttach = nullptr;

	UCharacterMovementComponent* MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	MovementComponent->GravityScale = 1.f;

	MovementComponent->StopMovementImmediately();
	MovementComponent->AddImpulse(SwingVelocity, true);

	player->Cable->SetVisibility(false);
}

void URopeHabschSwingComponent::CorrectPlayerRotation()
{
	FRotator currentRot = player->GetActorRotation();
	FRotator targetRot = FRotator(player->InitialRotation);

	FQuat newRot = FQuat::Slerp(currentRot.Quaternion(), targetRot.Quaternion(), 0.04);
	player->SetActorRotation(newRot);
	//FVector aa = FVector(CurrentAttach->GetActorLocation() - player->GetActorLocation()).GetSafeNormal();
	//player->SetActorRotation(UKismetMathLibrary::MakeRotFromZ(aa));
}

void URopeHabschSwingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	PlayerVelocity = player->GetActorLocation() + player->GetMovementComponent()->Velocity;
	DrawDebugLine(GetWorld(), player->GetActorLocation(),PlayerVelocity, FColor::Magenta, false, 0.f, SDPG_Foreground, 5.f);
	
	if (CurrentAttach != nullptr)
	{
		// Update the angle over time
		CurrentAngle -= FMath::DegreesToRadians(SwingSpeed) * DeltaTime;
		
		// Turn around if we've reached out maximum angle
		if (CurrentAngle <= -1.f * FMath::DegreesToRadians(MaxAngle))
		{	
			CurrentAngle *= -1.f;
			SwingDirection *= -1.f;
			//GetOwner()->SetActorRotation(UKismetMathLibrary::MakeRotFromX(SwingDirection));	
		}
		
		// Update the position based on the angle
		FVector SwingLocation = CurrentAttach->GetActorLocation();

		FVector SwingRightVector = FVector::CrossProduct(SwingDirection, -1.f * FVector::UpVector);
		FQuat SwingRotation = FQuat(SwingRightVector.GetSafeNormal(), CurrentAngle);

		FVector SwingOffset = FVector(0.f, 0.f, -1.f * RopeLength);
		SwingOffset = SwingRotation.RotateVector(SwingOffset);

		FVector NewLocation = SwingLocation + SwingOffset;
		if (DeltaTime > 0.f)
			SwingVelocity = (NewLocation - GetOwner()->GetActorLocation()) / DeltaTime;
		GetOwner()->SetActorLocation(NewLocation);
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Lerp player rotation back to initial rot
		CorrectPlayerRotation();
	}
}
