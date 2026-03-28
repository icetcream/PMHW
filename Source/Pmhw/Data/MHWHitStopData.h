// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MHWHitStopData.generated.h"

/**
 * 
 */
UCLASS()
class PMHW_API UMHWHitStopData : public UDataAsset
{
	GENERATED_BODY()
public:
	// 是否开启卡肉
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop", meta = (DisplayName = "启用卡肉", ToolTip = "关闭后，这个数据资产不会产生任何冻结或慢放效果。"))
	bool bEnableHitstop = true;

	// 命中瞬间冻结攻击者当前 Mesh 姿势多久。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop", meta = (DisplayName = "冻结姿势时长", ToolTip = "命中瞬间把攻击者当前 Mesh 姿势定住多久。", ClampMin = "0.0", UIMin = "0.0"))
	float MeshFreezeDuration = 0.02f;

	// 解冻后，攻击 Montage 慢放恢复多久。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop", meta = (DisplayName = "慢放恢复时长", ToolTip = "姿势解冻后，当前攻击 Montage 需要多久恢复到正常播放速度。", ClampMin = "0.0", UIMin = "0.0"))
	float MontageSlowDuration = 0.08f;

	// 解冻后，Montage 恢复阶段的起始播放速率。不要设为 0，推荐 0.03~0.12。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop", meta = (DisplayName = "慢放起始倍率", ToolTip = "姿势解冻后，Montage 恢复阶段一开始的播放倍率。越小越黏。", ClampMin = "0.0001", UIMin = "0.0001", ClampMax = "1.0", UIMax = "1.0"))
	float MontageSlowStartPlayRate = 0.06f;

	// 攻击 Montage 的慢放恢复曲线 (X轴进度0~1, Y轴播放速率)。为空时用线性恢复到 1.0。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop", meta = (DisplayName = "慢放恢复曲线", ToolTip = "控制 Montage 从慢放恢复到正常速度的曲线。X 轴是 0 到 1 的恢复进度，Y 轴是播放倍率。"))
	TObjectPtr<UCurveFloat> MontageSlowCurve;

	// 是否对攻击者应用 Actor 级别的时间膨胀。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop|Time Dilation", meta = (DisplayName = "影响攻击者时间", ToolTip = "启用后，攻击者自身也会额外叠加 Actor 级别的时间缩放。默认建议关闭。"))
	bool bAffectOwnerTimeDilation = false;

	// 是否对受击者应用 Actor 级别的时间膨胀。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop|Time Dilation", meta = (DisplayName = "影响受击者时间", ToolTip = "启用后，受击者也会额外叠加 Actor 级别的时间缩放。默认建议关闭。"))
	bool bAffectHitTargetTimeDilation = false;

	// 如果启用 Time Dilation，冻结和慢放阶段的起始时间倍率。为空时恢复到 1.0。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop|Time Dilation", meta = (DisplayName = "时间缩放起始倍率", ToolTip = "仅在开启攻击者或受击者时间缩放时使用。表示冻结/慢放阶段一开始的时间倍率。", EditCondition = "bAffectOwnerTimeDilation || bAffectHitTargetTimeDilation", EditConditionHides, ClampMin = "0.0001", UIMin = "0.0001", ClampMax = "1.0", UIMax = "1.0"))
	float TimeDilationStartScale = 0.2f;

	// Time Dilation 的恢复曲线 (X轴进度0~1, Y轴时间倍率)。为空时用线性恢复到 1.0。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop|Time Dilation", meta = (DisplayName = "时间缩放恢复曲线", ToolTip = "仅在开启攻击者或受击者时间缩放时使用。X 轴是 0 到 1 的恢复进度，Y 轴是时间倍率。", EditCondition = "bAffectOwnerTimeDilation || bAffectHitTargetTimeDilation", EditConditionHides))
	TObjectPtr<UCurveFloat> TimeDilationCurve;
};
