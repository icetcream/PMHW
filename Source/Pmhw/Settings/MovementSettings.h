#pragma once

#include "CoreMinimal.h"
#include "MHWGameplayTags.h"        // 你的 GameplayTag 定义
#include "Engine/DataTable.h"       
#include "Curves/CurveVector.h"     
#include "Curves/CurveFloat.h"      

#include "MovementSettings.generated.h" 

// ==========================================
// 1. 底层：具体的移动速度与曲线 (步态 Gait 数据都在这里)
// ==========================================
USTRUCT(BlueprintType)
struct PMHW_API FMHWMovementGaitSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float WalkSpeed{175.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float RunSpeed{375.0f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", Meta = (ClampMin = 0, ForceUnits = "cm/s"))
	float SprintSpeed{650.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	TObjectPtr<UCurveVector> AccelerationAndDecelerationAndGroundFrictionCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
	TObjectPtr<UCurveFloat> RotationInterpolationSpeedCurve;
};

// ==========================================
// 2. 第二层：姿态映射 (站立 / 蹲下)
// ==========================================
USTRUCT(BlueprintType)
struct PMHW_API FMHWMovementStanceSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FMHWMovementGaitSettings> Stances
	{
		{MHWStanceTags::Standing, {}},
		{MHWStanceTags::Crouching, {}}
	};
};

// ==========================================
// 3. 第三层：旋转模式映射 (自由方向 / 锁定敌人)
// ==========================================
USTRUCT(BlueprintType)
struct PMHW_API FMHWMovementRotationModeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FMHWMovementStanceSettings> RotationModes
	{
		{MHWRotationModeTags::VelocityDirection, {}},
		{MHWRotationModeTags::EnemyDirection, {}}
	};
};

// ==========================================
// 4. 最顶层 数据资产：武器状态映射 (收刀 / 拔刀大剑 / 拔刀太刀 等)
// ==========================================
UCLASS(Blueprintable, BlueprintType)
class PMHW_API UMHWMovementSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	// Key 是当前的武器/拔刀状态 Tag (例如: MHWWeaponTags::Unsheathed, MHWWeaponTags::GreatSword)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings", Meta = (ForceInlineRow))
	TMap<FGameplayTag, FMHWMovementRotationModeSettings> WeaponStates;
};