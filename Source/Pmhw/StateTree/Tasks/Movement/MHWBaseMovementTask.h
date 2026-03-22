#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Character/MHWMovementComponent.h"
#include "MHWBaseMovementTask.generated.h"

class UControllerInputEvaluatorModel;
// ==============================================================================
// 1. 基类的实例数据
// 现在只需要这一个终极仓库！再也不用绑一堆 Component 了。
// ==============================================================================
USTRUCT()
struct FMHWBaseMovementTaskInstanceData
{
	GENERATED_BODY()

	// ✨【核心输入】从 Evaluator 传过来的中央仓库
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UControllerInputEvaluatorModel> InputEvaluatorModel = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Parameter")
	ELocomotionState TargetState = ELocomotionState::Idle;
};

// ==============================================================================
// 2. 基类 Task
// ==============================================================================
USTRUCT(meta = (DisplayName = "Base Movement"), Category = "Locomotion")
struct FMHWBaseMovementTask : public FStateTreeTaskCommonBase{
	GENERATED_BODY()

	using FInstanceDataType = FMHWBaseMovementTaskInstanceData;

	FMHWBaseMovementTask() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	// ✨【核心功能】基类提供的受保护方法，供所有子类调用！
	// 只要传入你的仓库模型和目标状态，基类帮你把脏活累活干完。
	bool SetLocomotionState(UControllerInputEvaluatorModel* EvaluatorModel, ELocomotionState NewState) const;
};