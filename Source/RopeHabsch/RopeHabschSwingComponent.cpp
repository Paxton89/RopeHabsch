#include "RopeHabschSwingComponent.h"
#include "Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CableComponent.h"
#include "ScanComponent.h"
#include "RopeHabschAttachPoint.h"
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

	MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	playerMesh = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	scanComponent = Cast<UScanComponent>(GetOwner()->GetComponentByClass(UScanComponent::StaticClass()));
	player = Cast<ARopeHabschCharacter>(GetOwner());
}

void URopeHabschSwingComponent::StartSwinging()
{	
	if (scanComponent->CurrentAttachPoint != nullptr)
	{
		//Only allow StartSwing to run if we are close enough
		float distance = FVector::Distance(player->GetActorLocation(), scanComponent->CurrentAttachPoint->GetActorLocation());
		if(distance > MaxRopeLength)
			return;
		
		CurrentAttach = scanComponent->CurrentAttachPoint;

		FVector SwingLocation = CurrentAttach->GetActorLocation();
		FVector CharacterLocation = GetOwner()->GetActorLocation();
		RopeLength = FMath::Min(FVector::Dist(SwingLocation, CharacterLocation) - 100.f, MaxRopeLength);

		//Camera + Animation
		MovementComponent->MaxWalkSpeed = 1000;
		playerMesh->PlayAnimation(StartSwingAnim, false);
		player->GetCameraBoom()->bEnableCameraLag = true;
		player->GetCameraBoom()->CameraLagSpeed = 5.f;

		//Cable
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
	
	FVector LaunchVelocity = MovementComponent->Velocity + FVector(0,0, 400);
	player->LaunchCharacter(LaunchVelocity, false, false);
	CurrentAttach = nullptr;
	MovementComponent->AirControl = 0.6f;
	player->Cable->SetVisibility(false);
}

void URopeHabschSwingComponent::ApplyForce()
{
	FVector CableDirection = FVector(CurrentAttach->GetActorLocation() - player->GetActorLocation());
	float dot = FVector::DotProduct(PlayerVelocity, CableDirection);
	FVector Force = (CableDirection.GetSafeNormal() * dot) * -1.f; // Calculates the force
	
	MovementComponent->AirControl = 5.f;

	//Set rotation of player
	FRotator newRot = UKismetMathLibrary::FindLookAtRotation(player->GetActorLocation(), player->GetActorLocation() + PlayerVelocity);
	player->SetActorRotation(newRot);
	MovementComponent->AddForce(Force);
}

void URopeHabschSwingComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Draw Velocity
	PlayerVelocity = player->GetMovementComponent()->Velocity;
	DrawDebugDirectionalArrow(GetWorld(), player->GetActorLocation(), player->GetActorLocation() + PlayerVelocity, 100, FColor::Turquoise, false, DeltaTime, 0 , 5);
	
	if (CurrentAttach != nullptr)
	{
		ApplyForce();
	}
}
