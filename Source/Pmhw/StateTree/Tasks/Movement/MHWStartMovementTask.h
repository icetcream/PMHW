#pragma once

#include "CoreMinimal.h"
#include "MHWBaseMovementTask.h"
#include "MHWStartMovementTask.generated.h"

// ==============================================================================
// 1. 子类的实例数据
// ==============================================================================
USTRUCT()
struct FMHWStartMovementTaskInstanceData : public FMHWBaseMovementTaskInstanceData
{
	GENERATED_BODY()

	// [起步专属参数] 策划可以在 State Tree 面板调节这些数值来改变起步的手感
	
	// 1. 向量平滑速度：Start 阶段通常需要极快的响应，所以这个值可以偏大 (比如 15.0 ~ 20.0)
	UPROPERTY(EditAnywhere, Category = "Parameter|Start")
	float StartInputSmoothingSpeed;
};

USTRUCT(meta = (DisplayName = "Start Movement", Category = "MHW|Locomotion"))
struct FMHWStartMovementTask : public FMHWBaseMovementTask
{
	GENERATED_BODY()

	using FInstanceDataType = FMHWStartMovementTaskInstanceData;

	FMHWStartMovementTask() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};