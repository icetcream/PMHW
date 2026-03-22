#include "MHWTurnMovementTask.h"

#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

EStateTreeRunStatus FMHWTurnMovementTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		// 1. 设置状态枚举为 Cycle
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Turn);
		
		// 2. 无缝衔接：让平滑向量等于当前真实输入，防止刚切状态时抽搐
		InputModel->MovementComponent->ResetSmoothInputDirection(InputModel->UserInputDirection);
		
		InstanceData.TurnCurrentTime = 0.f;
		InputModel->MovementComponent->TurnPercent = 0.f;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMHWTurnMovementTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;
	if (InputModel && InputModel->MovementComponent)
	{
		// 1. 设置状态枚举为 Cycle
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Turn);
		
		// 2. 无缝衔接：让平滑向量等于当前真实输入，防止刚切状态时抽搐
		InputModel->MovementComponent->ResetSmoothInputDirection(InputModel->UserInputDirection);
		
		InstanceData.TurnCurrentTime += DeltaTime;
		InputModel->MovementComponent->TurnPercent = FMath::Clamp(InstanceData.TurnCurrentTime/InstanceData.TurnTotalTime,0,1) ;
	}
	return EStateTreeRunStatus::Running;
}

void FMHWTurnMovementTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		// 1. 设置状态枚举为 Cycle
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Turn);
		
		// 2. 无缝衔接：让平滑向量等于当前真实输入，防止刚切状态时抽搐
		InputModel->MovementComponent->ResetSmoothInputDirection(InputModel->UserInputDirection);
		
		InstanceData.TurnCurrentTime = 0.f;
		InputModel->MovementComponent->TurnPercent = 0.f;
	}
	
}
