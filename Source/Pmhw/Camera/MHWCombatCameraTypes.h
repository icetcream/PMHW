#pragma once

#include "CoreMinimal.h"
#include "MHWCombatCameraTypes.generated.h"

USTRUCT(BlueprintType)
struct PMHW_API FMHWCameraSpringShakeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "启用命中抖动"))
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "位置振幅"))
	FVector LocalLocationAmplitude = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "旋转振幅"))
	FRotator RotationAmplitude = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "视野振幅"))
	float FOVAmplitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "振动频率", ClampMin = "0.0", UIMin = "0.0"))
	float Frequency = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "衰减系数", ClampMin = "0.0", UIMin = "0.0"))
	float Damping = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "持续时长", ClampMin = "0.0", UIMin = "0.0"))
	float Duration = 0.16f;

	bool IsEnabled() const
	{
		return bEnabled
			&& Duration > 0.0f
			&& (!LocalLocationAmplitude.IsNearlyZero()
				|| !RotationAmplitude.IsNearlyZero()
				|| !FMath::IsNearlyZero(FOVAmplitude));
	}
};

USTRUCT(BlueprintType)
struct PMHW_API FMHWCameraMotionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "本地位置偏移"))
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "旋转偏移"))
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "视野偏移"))
	float FOVOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "过渡时长", ClampMin = "0.0", UIMin = "0.0"))
	float BlendDuration = 0.12f;

	bool IsNearlyZero() const
	{
		return LocalOffset.IsNearlyZero()
			&& RotationOffset.IsNearlyZero()
			&& FMath::IsNearlyZero(FOVOffset);
	}
};

USTRUCT(BlueprintType)
struct PMHW_API FMHWCombatCameraArmMotionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "臂长偏移"))
	float ArmLengthOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "Socket 偏移"))
	FVector SocketOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "Target 偏移"))
	FVector TargetOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "摇臂旋转偏移"))
	FRotator BoomRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "视野偏移"))
	float FOVOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Camera", meta = (DisplayName = "过渡时长", ClampMin = "0.0", UIMin = "0.0"))
	float BlendDuration = 0.12f;

	bool IsNearlyZero() const
	{
		return FMath::IsNearlyZero(ArmLengthOffset)
			&& SocketOffset.IsNearlyZero()
			&& TargetOffset.IsNearlyZero()
			&& BoomRotationOffset.IsNearlyZero()
			&& FMath::IsNearlyZero(FOVOffset);
	}
};
