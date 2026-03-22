#pragma once

#include "CoreMinimal.h"
#include "MHWBaseMovementTask.h"
#include "MHWStartMovementTask.generated.h"

// ============================================================================
// 1. Child instance data
// ============================================================================
USTRUCT()
struct FMHWStartMovementTaskInstanceData : public FMHWBaseMovementTaskInstanceData
{
	GENERATED_BODY()
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
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
