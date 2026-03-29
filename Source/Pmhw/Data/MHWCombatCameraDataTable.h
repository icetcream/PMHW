#pragma once

#include "CoreMinimal.h"
#include "Camera/MHWCombatCameraTypes.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MHWCombatCameraDataTable.generated.h"

USTRUCT(BlueprintType)
struct PMHW_API FMHWCombatCameraMotionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Camera", meta = (DisplayName = "镜头动作标签", Categories = "Data.CameraMotion"))
	FGameplayTag CameraMotionTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Camera", meta = (DisplayName = "镜头动作名称"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Camera", meta = (DisplayName = "镜头参数"))
	FMHWCombatCameraArmMotionSettings CameraMotion;
};
