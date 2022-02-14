#include "RopeHabschAttachPoint.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

ARopeHabschAttachPoint::ARopeHabschAttachPoint()
{
	root = CreateDefaultSubobject<USceneComponent>("RootComp");
	RootComponent = root;
}

void ARopeHabschAttachPoint::BeginPlay()
{
}

void ARopeHabschAttachPoint::CreateRope(AActor* AttachTo)
{
}

void ARopeHabschAttachPoint::DestroyRope()
{
}
