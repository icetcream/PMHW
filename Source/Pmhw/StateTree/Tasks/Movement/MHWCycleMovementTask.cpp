#include "MHWCycleMovementTask.h"

#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h" // 如果你需要获取受控 Actor
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

// -------------------------------------------------------------------------
// EnterState: 初始化循环状态
// -------------------------------------------------------------------------
EStateTreeRunStatus FMHWCycleMovementTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		// 1. 设置状态枚举为 Cycle
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Cycle);
		
		// 2. 无缝衔接：让平滑向量等于当前真实输入，防止刚切状态时抽搐
		InputModel->MovementComponent->ResetSmoothInputDirection(InputModel->UserInputDirection);
	}

	return EStateTreeRunStatus::Running;
}

// ---------------------------------------------------------
// Tick (Cycle 状态每一帧)
// ---------------------------------------------------------
EStateTreeRunStatus FMHWCycleMovementTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	/*FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (!InputModel || !InputModel->MovementComponent) return EStateTreeRunStatus::Failed;

	// 1. 拿原始输入
	FVector RawInput = InputModel->UserInputDirection;

	// 2. ✨ 把脏活累活全扔给 MovementComponent！
	// 注意这里传入了属于 Cycle Task 独有的平滑速度：InstanceData.CycleInputSmoothingSpeed
	InputModel->MovementComponent->UpdateSmoothInputDirection(RawInput, InstanceData.CycleInputSmoothingSpeed, DeltaTime);

	// 3. (可选) 如果你在这个 Task 里需要直接拿算好的数据去转 Actor：
	// FRotator TargetRot = InputModel->MovementComponent->SmoothedInputDirection.Rotation();
	// InputModel->ControlledActor->SetActorRotation(TargetRot);*/

	return EStateTreeRunStatus::Running;
}