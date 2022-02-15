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

	UPROPERTY(EditDefaultsOnly)
	UAnimationAsset* StartSwingAnim;
	
	UPROPERTY(EditDefaultsOnly)
	UAnimationAsset* SwingAnim;

	UPROPERTY(EditDefaultsOnly)
	UClass* defaultAnim;

protected:
	/* Set while we're swinging, the current attach point. */
	UPROPERTY(BlueprintReadWrite)
	ARopeHabschAttachPoint* CurrentAttach = nullptr;
	
	/* Degrees per second to swing at. */
	UPROPERTY(EditDefaultsOnly)
	float SwingSpeed = 180.f;

	/* Maximum angle to achieve before turning around */
	UPROPERTY(EditDefaultsOnly)
	float MaxAngle = 180.f;

	UCharacterMovementComponent* MovementComponent;
	USkeletalMeshComponent* playerMesh;
	ARopeHabschCharacter* player;
	UScanComponent* scanComponent;
	
	
	float RopeLength = 0.f;
	float CurrentAngle = 0.f;
	FVector DirToAttachPoint;
	FVector PlayerVelocity;
	FVector SwingVelocity;
	FRotator StartRot;

	void CorrectPlayerRotation();
	void ApplyForce();
};