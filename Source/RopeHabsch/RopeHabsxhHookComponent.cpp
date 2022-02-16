#include "RopeHabsxhHookComponent.h"
#include "RopeHabschSwingComponent.h"
#include "ScanComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

URopeHabsxhHookComponent::URopeHabsxhHookComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URopeHabsxhHookComponent::BeginPlay()
{
	Super::BeginPlay();
	MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	playerMesh = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	scanComponent = Cast<UScanComponent>(GetOwner()->GetComponentByClass(UScanComponent::StaticClass()));
	SwingComponent = Cast<URopeHabschSwingComponent>(GetOwner()->GetComponentByClass(URopeHabschSwingComponent::StaticClass()));
	player = Cast<ARopeHabschCharacter>(GetOwner());
}

void URopeHabsxhHookComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if(scanComponent->CurrentAttachPoint == nullptr)
		return;
	
	float distance = FVector::Distance(player->GetActorLocation(), scanComponent->CurrentAttachPoint->GetActorLocation());
	if(distance < 400)
	{
		StopHook();
	}

	
}

void URopeHabsxhHookComponent::StartHook()
{
	if(scanComponent->CurrentAttachPoint == nullptr)
		return;
	
	//Only allow StartHook to run if we are close enough
	float distance = FVector::Distance(player->GetActorLocation(), scanComponent->CurrentAttachPoint->GetActorLocation());
	if(distance > SwingComponent->MaxRopeLength)
		return;

	CurrentAttach = scanComponent->CurrentAttachPoint;


	MovementComponent->MaxWalkSpeed = 1000;
	playerMesh->PlayAnimation(SwingComponent->StartSwingAnim, false);
	player->GetCameraBoom()->bEnableCameraLag = true;
	player->GetCameraBoom()->CameraLagSpeed = 5.f;

	
	FVector LaunchDir = FVector(CurrentAttach->GetActorLocation() - player->GetActorLocation()).GetSafeNormal();
	FVector LaunchVelocity = FVector(LaunchDir * 3000 + FVector(0,0,700));
	player->LaunchCharacter(LaunchVelocity, false, false);
	

	player->Cable->SetVisibility(true);
	player->Cable->SetAttachEndToComponent(CurrentAttach->root);
	player->Cable->CableLength = SwingComponent->RopeLength;
	player->Cable->bAttachEnd = true;
}

void URopeHabsxhHookComponent::StopHook()
{
	if(scanComponent->CurrentAttachPoint == nullptr)
		return;

	player->bShouldRotCorrect = true;
	CurrentAttach = nullptr;
	player->Cable->SetVisibility(false);
}

