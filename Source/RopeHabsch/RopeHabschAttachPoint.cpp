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