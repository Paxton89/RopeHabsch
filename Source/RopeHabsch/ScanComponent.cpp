
#include "ScanComponent.h"
#include "DrawDebugHelpers.h"
#include "RopeHabschSwingComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

UScanComponent::UScanComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UScanComponent::BeginPlay()
{
	Super::BeginPlay();
	Player = Cast<ARopeHabschCharacter>(GetOwner());
	SwingComponent = Cast<URopeHabschSwingComponent>(Player->GetComponentByClass(URopeHabschSwingComponent::StaticClass()));
	CollectAllAttachPoints();
}

void UScanComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// FindAttachPoint in center of screen - then check to see if it's close enough
	currentlyClosest = TNumericLimits<float>::Max();
	for (auto AttachPoint : AttachPoints)
	{
		float distance = FVector::Distance(Player->GetActorLocation(), AttachPoint->GetActorLocation());
		float dot = FVector::DotProduct(Player->GetFollowCamera()->GetForwardVector(),(AttachPoint->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal());
		if(distance < currentlyClosest && dot > 0.9)
		{
			currentlyClosest = distance;
			CurrentAttachPoint = AttachPoint;
		}
	}
	
	if(CurrentAttachPoint == nullptr)
		return;
	MarkClosestAttach(DeltaTime, CurrentAttachPoint);
}

void UScanComponent::CollectAllAttachPoints()
{
	TArray<AActor*> tempArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARopeHabschAttachPoint::StaticClass(), tempArray);
	for (auto item : tempArray)
	{
		AttachPoints.Add(Cast<ARopeHabschAttachPoint>(item));
	}
}

void UScanComponent::MarkClosestAttach(float lifetime, ARopeHabschAttachPoint* AttachPoint)
{
	float distance = FVector::Distance(AttachPoint->GetActorLocation(), Player->GetActorLocation());
	
	if (distance < SwingComponent->MaxRopeLength) //Within MaxDistance - LEGAL
	{
	DrawDebugBox(
    GetWorld(),
    AttachPoint->GetActorLocation(),
    FVector(100,100,100),
    AttachPoint->GetActorRotation().Quaternion(),
    FColor::Green,
    false,
    lifetime,
    0,
    11);	
	}
	else // NOT Within MaxDistance - ILLEGAL
	{
	DrawDebugBox(
	GetWorld(),
	AttachPoint->GetActorLocation(),
	FVector(100,100,100),
	AttachPoint->GetActorRotation().Quaternion(),
	FColor::Red,
	false,
	lifetime,
	0,
	11);
	}
}

