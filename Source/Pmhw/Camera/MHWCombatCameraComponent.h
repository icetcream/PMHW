#pragma once

#include "CoreMinimal.h"
#include "Camera/MHWCombatCameraTypes.h"
#include "Components/ActorComponent.h"
#include "MHWCombatCameraComponent.generated.h"

class UCameraComponent;
class UDataTable;
class USpringArmComponent;
struct FMHWCombatCameraMotionRow;

USTRUCT()
struct FMHWCombatCameraBlendState
{
	GENERATED_BODY()

	float StartArmLengthOffset = 0.0f;
	float TargetArmLengthOffset = 0.0f;
	float CurrentArmLengthOffset = 0.0f;

	FVector StartSocketOffset = FVector::ZeroVector;
	FVector TargetSocketOffset = FVector::ZeroVector;
	FVector CurrentSocketOffset = FVector::ZeroVector;

	FVector StartTargetOffset = FVector::ZeroVector;
	FVector TargetTargetOffset = FVector::ZeroVector;
	FVector CurrentTargetOffset = FVector::ZeroVector;

	FRotator StartBoomRotationOffset = FRotator::ZeroRotator;
	FRotator TargetBoomRotationOffset = FRotator::ZeroRotator;
	FRotator CurrentBoomRotationOffset = FRotator::ZeroRotator;

	float StartFOVOffset = 0.0f;
	float TargetFOVOffset = 0.0f;
	float CurrentFOVOffset = 0.0f;

	float BlendElapsed = 0.0f;
	float BlendDuration = 0.0f;
	bool bHasActiveBlend = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWCombatCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMHWCombatCameraComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void ApplyCameraMotion(const FMHWCombatCameraArmMotionSettings& InMotionSettings);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	bool ApplyCameraMotionByTag(const FGameplayTag& CameraMotionTag);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void ClearCameraMotion(float BlendOutDuration = 0.2f);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void SnapToDefaultCamera();

	const FMHWCombatCameraMotionRow* FindCameraMotionRowByTag(const FGameplayTag& CameraMotionTag) const;

private:
	void ResolveCameraReferences();
	void CacheDefaultCameraState();
	void UpdateBlendState(float DeltaTime);
	void ApplyCurrentStateToComponents();

	UPROPERTY(Transient)
	TWeakObjectPtr<USpringArmComponent> CachedSpringArm;

	UPROPERTY(Transient)
	TWeakObjectPtr<UCameraComponent> CachedCamera;

	UPROPERTY(Transient)
	float DefaultArmLength = 0.0f;

	UPROPERTY(Transient)
	FVector DefaultSocketOffset = FVector::ZeroVector;

	UPROPERTY(Transient)
	FVector DefaultTargetOffset = FVector::ZeroVector;

	UPROPERTY(Transient)
	FRotator DefaultBoomRotation = FRotator::ZeroRotator;

	UPROPERTY(Transient)
	float DefaultCameraFOV = 90.0f;

	UPROPERTY(Transient)
	bool bDefaultsCached = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Camera|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> CameraMotionDataTable;

	UPROPERTY(Transient)
	FMHWCombatCameraBlendState BlendState;
};
