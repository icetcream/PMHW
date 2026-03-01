#include "STT_LoopMovement.h"

#include "StateTreeExecutionContext.h"
#include "Character/MHWMovementComponent.h"
#include "Input/MHWInputComponent.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"
// #include "AdvancedLocoMovementComponent.h" 



EStateTreeRunStatus FSTT_LoopMovement::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	// 1. 获取当前 Task 的数据篮子
	FSTT_LoopMovement_InstanceData& InstanceData = Context.GetInstanceData<FSTT_LoopMovement_InstanceData>(*this);

	// 2. ✨ 优雅地从仓库里拿出 Model 指针 (完美对应你的截图)
	const UControllerInputEvaluatorModel* InputEvaluatorModel = InstanceData.InputEvaluatorModel;

	// 3. 安全检查：仓库存在，并且里面的移动组件指针不为空
	if (InputEvaluatorModel && InputEvaluatorModel->MovementComponent)
	{
		// 4. ✨ 优雅地提取组件 (完美对应你的截图)
		UMHWMovementComponent* MovementComponent = InputEvaluatorModel->MovementComponent;
		UMHWInputComponent* InputComponent = InputEvaluatorModel->InputComponent;

		// 5. 尽情地操作你的底层物理组件吧！没有任何 Cast 的性能损耗！
		MovementComponent->MaxAcceleration = InstanceData.MaxAcceleration;
		// MovementComponent->MaxWalkSpeed = FMath::FInterpTo(...);
	}

	return EStateTreeRunStatus::Running;
}
