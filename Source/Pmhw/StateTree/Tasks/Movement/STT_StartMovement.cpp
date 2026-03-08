#include "STT_StartMovement.h"

#include "StateTreeExecutionContext.h"
#include "Character/MHWMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

EStateTreeRunStatus FSTT_StartMovement::EnterState(FStateTreeExecutionContext& Context,
                                                   const FStateTreeTransitionResult& Transition) const
{
	FSTT_StartMovement_InstanceData& Data = Context.GetInstanceData<FSTT_StartMovement_InstanceData>(*this);

	const UControllerInputEvaluatorModel* Model = Data.InputEvaluatorModel;

	// 3. 安全检查：必须有 Model，且玩家确实推了摇杆
	if (Model && !Model->UserInputDirection.IsNearlyZero())
	{
		// 4. 直接拿到 Evaluator 早就帮我们缓存好的方向向量！
		FVector ForwardDir = Model->ActorForwardDirection;
		FVector InputDir = Model->UserInputDirection;

		// 5. ✨ 核心数学：计算带符号的夹角 (-180 到 180) ✨
		FRotator ForwardRot = ForwardDir.Rotation();
		FRotator InputRot = InputDir.Rotation();
		
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(InputRot, ForwardRot);
		Data.LocomotionAngle = DeltaRot.Yaw;

		// 6. 物理特判：是不是极其剧烈的向后转身起步？(比如夹角大于 135 度)
		Data.bIsPivotStart = FMath::Abs(Data.LocomotionAngle) > 135.0f;

		// 7. ✨ 物理限制：向后 180 度起步时，必须从 Model 里拿出移动组件，并关闭平滑转向！✨
		if (Data.bIsPivotStart && Model->MovementComponent)
		{
			// 这里假设你在自定义组件里写了 SetSteeringEnabled 方法。
			// 如果没写，可以用原生的 bOrientRotationToMovement = false; 代替。
			Model->MovementComponent->bOrientRotationToMovement = false;
			
			// 如果你需要在这里把角度存进组件给动画蓝图用，也可以直接写：
			Model->MovementComponent->CurrentLocomotionAngle = Data.LocomotionAngle;
		}
	}
	else
	{
		// 如果因为某些原因没有输入，角度归零
		Data.LocomotionAngle = 0.0f;
		Data.bIsPivotStart = false;
	}

	// 告诉状态树：起步动作正在播放，请保持在此状态！
	return EStateTreeRunStatus::Running;
}

void FSTT_StartMovement::ExitState(FStateTreeExecutionContext& Context,
                                   const FStateTreeTransitionResult& Transition) const
{
	FSTT_StartMovement_InstanceData& Data = Context.GetInstanceData<FSTT_StartMovement_InstanceData>(*this);

	const UControllerInputEvaluatorModel* Model = Data.InputEvaluatorModel;

	// 1. 如果刚才做的是向后起步，并且移动组件存在
	if (Data.bIsPivotStart && Model && Model->MovementComponent)
	{
		// 2. ✨ 离开时，务必恢复物理层的平滑转向功能！✨
		// 这样角色进入后续的 Cycle 奔跑时，又能平滑转弯了。
		Model->MovementComponent->bOrientRotationToMovement = true;
	}
}
