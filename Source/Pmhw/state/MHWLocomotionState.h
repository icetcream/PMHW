#pragma once

#include "MHWLocomotionState.generated.h"

USTRUCT(BlueprintType)
struct PMHW_API FMHWLocomotionState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	uint8 bHasInput : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float InputYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	uint8 bHasVelocity : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float Speed{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	FVector Velocity{ForceInit};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float VelocityYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	uint8 bMoving : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	uint8 bRotationTowardsLastInputDirectionBlocked : 1 {true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float TargetYawAngle{0.0f};

	// Used for extra smooth actor rotation, in other cases equal to the regular target yaw angle.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float SmoothTargetYawAngle{0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float ViewRelativeTargetYawAngle{0.0f};

	// Specifies the maximum angle by which the actor's rotation can differ from the view rotation when aiming.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW", Meta = (ClampMin = 0, ClampMax = 180, ForceUnits = "deg"))
	float AimingYawAngleLimit{180.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	uint8 bAimingLimitAppliedThisFrame : 1 {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW")
	uint8 bResetAimingLimit : 1 {true};
};
