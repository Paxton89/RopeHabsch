
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
	
	currentlyClosest = TNumericLimits<float>::Max();
	for (auto AttachPoint : AttachPoints)
	{
		auto distance = FVector::Distance(Player->GetActorLocation(), AttachPoint->GetActorLocation());
		auto dot = FVector::DotProduct(Player->GetFollowCamera()->GetForwardVector(),(AttachPoint->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal());
		if(distance < currentlyClosest && dot > 0.9)
		{
			currentlyClosest = distance;
			CurrentAttachPoint = AttachPoint;
			//UE_LOG(LogTemp, Log, TEXT("%f"), currentlyClosest);
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
	UE_LOG(LogTemp, Warning, TEXT("%d ATTACHPOINTS FOUND"), AttachPoints.Num());
}

void UScanComponent::MarkClosestAttach(float lifetime, ARopeHabschAttachPoint* AttachPoint)
{
	auto distance = FVector::Distance(AttachPoint->GetActorLocation(), Player->GetActorLocation());
	if (distance < SwingComponent->MaxRopeLength)
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
	else
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

