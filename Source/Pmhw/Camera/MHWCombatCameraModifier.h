#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "Camera/MHWCombatCameraTypes.h"
#include "MHWCombatCameraModifier.generated.h"

USTRUCT()
struct FMHWActiveSpringCameraShake
{
	GENERATED_BODY()

	FMHWCameraSpringShakeSettings Settings;
	float ElapsedTime = 0.0f;
};

USTRUCT()
struct FMHWCameraMotionRuntimeState
{
	GENERATED_BODY()

	FVector StartLocalOffset = FVector::ZeroVector;
	FVector TargetLocalOffset = FVector::ZeroVector;
	FVector CurrentLocalOffset = FVector::ZeroVector;

	FRotator StartRotationOffset = FRotator::ZeroRotator;
	FRotator TargetRotationOffset = FRotator::ZeroRotator;
	FRotator CurrentRotationOffset = FRotator::ZeroRotator;

	float StartFOVOffset = 0.0f;
	float TargetFOVOffset = 0.0f;
	float CurrentFOVOffset = 0.0f;

	float BlendElapsed = 0.0f;
	float BlendDuration = 0.0f;

	bool bHasActiveBlend = false;
};

UCLASS()
class PMHW_API UMHWCombatCameraModifier : public UCameraModifier
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void PlaySpringShake(const FMHWCameraSpringShakeSettings& InShakeSettings);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void ApplyCameraMotion(const FMHWCameraMotionSettings& InMotionSettings);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void ClearCameraMotion(float BlendOutDuration = 0.12f);

	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

private:
	void UpdateCameraMotion(float DeltaTime);
	void ApplySpringShakes(float DeltaTime, FVector& InOutLocalLocationOffset, FRotator& InOutRotationOffset, float& InOutFOVOffset);

	UPROPERTY(Transient)
	TArray<FMHWActiveSpringCameraShake> ActiveSpringShakes;

	UPROPERTY(Transient)
	FMHWCameraMotionRuntimeState CameraMotionState;
};
