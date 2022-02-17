#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScanComponent.generated.h"

class ARopeHabschAttachPoint;
class ARopeHabschCharacter;
class URopeHabschSwingComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROPEHABSCH_API UScanComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UScanComponent();
	void MarkClosestAttach(float lifetime, ARopeHabschAttachPoint* AttachPoint);
	ARopeHabschAttachPoint* CurrentAttachPoint;
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void CollectAllAttachPoints();

	ARopeHabschCharacter* Player;
	URopeHabschSwingComponent* SwingComponent;
	TArray<ARopeHabschAttachPoint*> AttachPoints;

	float currentlyClosest;	
};
