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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop")
	bool bEnableHitstop = true;

	// 卡肉的总持续真实时间（秒）。大剑建议 0.15~0.25，太刀 0.05~0.1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float HitstopDuration = 0.2f;

	// 控制卡肉速度变化的曲线 (X轴进度0~1, Y轴时间缩放倍率)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitstop")
	TObjectPtr<UCurveFloat> HitstopCurve;
};
