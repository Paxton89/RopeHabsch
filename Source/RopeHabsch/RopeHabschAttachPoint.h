#pragma once

#include "CoreMinimal.h"
#include "CableComponent.h"
#include "GameFramework/Actor.h"
#include "RopeHabschAttachPoint.generated.h"

UCLASS()
class ARopeHabschAttachPoint : public AActor
{
	GENERATED_BODY()
public:
	ARopeHabschAttachPoint();

	//void CreateRope(AActor* AttachedTo);
	//void DestroyRope();
	virtual void BeginPlay() override;
	USceneComponent* root;
};
