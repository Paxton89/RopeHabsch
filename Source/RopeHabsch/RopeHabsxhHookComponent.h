#pragma once

#include "CoreMinimal.h"
#include "RopeHabschAttachPoint.h"
#include "RopeHabschCharacter.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RopeHabsxhHookComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROPEHABSCH_API URopeHabsxhHookComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URopeHabsxhHookComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	ARopeHabschAttachPoint* CurrentAttach = nullptr;
	
	UCharacterMovementComponent* MovementComponent;
	USkeletalMeshComponent* playerMesh;
	ARopeHabschCharacter* player;
	UScanComponent* scanComponent;
	URopeHabschSwingComponent* SwingComponent;
	
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void StartHook();
	void StopHook();
};
