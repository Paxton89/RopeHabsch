#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RopeHabschAttachPoint.h"
#include "ScanComponent.h"
#include "RopeHabschSwingComponent.generated.h"

class ARopeHabschCharacter;
class USkeletalMeshComponent;

UCLASS(BlueprintType, Meta=(BlueprintSpawnableComponent))

class URopeHabschSwingComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	URopeHabschSwingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void StartSwinging();
	void StopSwinging();


	/* Maximum length of the rope. */
	UPROPERTY(EditDefaultsOnly)
	float MaxRopeLength = 1800.f;

	float RopeLength = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	UAnimationAsset* StartSwingAnim;
	
	UPROPERTY(EditDefaultsOnly)
	UAnimationAsset* SwingAnim;

	UPROPERTY(EditDefaultsOnly)
	UClass* defaultAnim;

protected:

	UPROPERTY(BlueprintReadWrite)
	ARopeHabschAttachPoint* CurrentAttach = nullptr;

	UCharacterMovementComponent* MovementComponent;
	USkeletalMeshComponent* playerMesh;
	ARopeHabschCharacter* player;
	UScanComponent* scanComponent;
	
	
	float CurrentAngle = 0.f;
	FVector DirToAttachPoint;
	FVector PlayerVelocity;
	FVector SwingVelocity;
	FRotator StartRot;

	void ApplyForce();
};