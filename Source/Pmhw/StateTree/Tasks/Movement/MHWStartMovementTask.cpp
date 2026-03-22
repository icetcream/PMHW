#include "MHWStartMovementTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

// -------------------------------------------------------------------------
// EnterState: 当状态刚进入时的初始化
// -------------------------------------------------------------------------
EStateTreeRunStatus FMHWStartMovementTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		// 1. 设置状态枚举为 Cycle
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Start);
		
		// 2. 无缝衔接：让平滑向量等于当前真实输入，防止刚切状态时抽搐
		InputModel->MovementComponent->ResetSmoothInputDirection(InputModel->UserInputDirection);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMHWStartMovementTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	/*
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (!InputModel || !InputModel->MovementComponent) return EStateTreeRunStatus::Failed;

	// 1. 拿原始输入
	FVector RawInput = InputModel->UserInputDirection;

	// 2. ✨ 把脏活累活全扔给 MovementComponent！
	// 注意这里传入了属于 Cycle Task 独有的平滑速度：InstanceData.CycleInputSmoothingSpeed
	InputModel->MovementComponent->UpdateSmoothInputDirection(RawInput, InstanceData.StartInputSmoothingSpeed, DeltaTime);

	// 3. (可选) 如果你在这个 Task 里需要直接拿算好的数据去转 Actor：
	// FRotator TargetRot = InputModel->MovementComponent->SmoothedInputDirection.Rotation();
	// InputModel->ControlledActor->SetActorRotation(TargetRot);

	
	*/
	return EStateTreeRunStatus::Running;
}