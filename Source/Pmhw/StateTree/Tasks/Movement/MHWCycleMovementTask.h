#pragma once

#include "CoreMinimal.h"
#include "MHWBaseMovementTask.h"
#include "MHWCycleMovementTask.generated.h"



USTRUCT()
struct FMHWCycleMovementTaskInstanceData : public FMHWBaseMovementTaskInstanceData
{
	GENERATED_BODY()

	// [循环专属参数] 
	// 跑动中的变向通常比静止起步显得更“沉”，所以这些速度可以稍微调小一点，
	// 让角色在变向时划出一个真实的物理弧线 (U型弯)。
	
	// 1. 向量平滑速度：(比如 8.0 ~ 12.0)
	UPROPERTY(EditAnywhere, Category = "Parameter|Cycle")
	float CycleInputSmoothingSpeed;
	
};

USTRUCT(meta = (DisplayName = "Cycle Movement", Category = "MHW|Locomotion"))
struct FMHWCycleMovementTask : public FMHWBaseMovementTask
{
	GENERATED_BODY()

	using FInstanceDataType = FMHWCycleMovementTaskInstanceData;

	FMHWCycleMovementTask() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};