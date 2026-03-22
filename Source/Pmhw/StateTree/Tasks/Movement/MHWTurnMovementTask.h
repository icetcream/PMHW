#pragma once

#include "CoreMinimal.h"
#include "MHWBaseMovementTask.h"
#include "MHWTurnMovementTask.generated.h"



USTRUCT()
struct FMHWTurnMovementTaskInstanceData : public FMHWBaseMovementTaskInstanceData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Parameter|Turn")
	float TurnTotalTime = 1.f;
	UPROPERTY(EditAnywhere, Category = "Parameter|Turn")
	float TurnCurrentTime;
};

USTRUCT(meta = (DisplayName = "Turn Movement", Category = "MHW|Locomotion"))
struct FMHWTurnMovementTask : public FMHWBaseMovementTask
{
	GENERATED_BODY()

	using FInstanceDataType = FMHWTurnMovementTaskInstanceData;

	FMHWTurnMovementTask() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};