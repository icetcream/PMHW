#include "STT_StartMovement.h"

#include "StateTreeExecutionContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

EStateTreeRunStatus FSTT_StartMovement::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FSTT_StartMovement_InstanceData& Data = Context.GetInstanceData<FSTT_StartMovement_InstanceData>(*this);

	if (!Data.Character) return EStateTreeRunStatus::Failed;

	// 1. 获取角色的面朝方向 (Actor Forward) 和 玩家推摇杆的期望移动方向 (Input)
	// 忽略 Z 轴，保证是纯粹的地面 2D 夹角
	FVector ForwardDir = Data.Character->GetActorForwardVector().GetSafeNormal2D();
	FVector InputDir = Data.Character->GetLastMovementInputVector().GetSafeNormal2D();

	// 如果没有输入，角度为 0
	if (InputDir.IsNearlyZero())
	{
		Data.LocomotionAngle = 0.0f;
		Data.bIsPivotStart = false;
	}
	else
	{
		// 2. ✨ 核心数学：计算带符号的夹角 (-180 到 180) ✨
		// 为什么不用 Dot + Acos？因为点乘算不出“偏左”还是“偏右”。
		// 这里推荐使用 UKismetMathLibrary::NormalizedDeltaRotator，它是虚幻里最稳妥的算角度方法。
		
		// 把方向向量转成旋转体 (Rotator)
		FRotator ForwardRot = ForwardDir.Rotation();
		FRotator InputRot = InputDir.Rotation();
		
		// 算出两者的差值角 (Delta)，取 Yaw(偏航角) 即为地面夹角
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(InputRot, ForwardRot);
		Data.LocomotionAngle = DeltaRot.Yaw;

		// 3. 物理特判：是不是极其剧烈的向后转身起步？(比如夹角大于 135 度)
		Data.bIsPivotStart = FMath::Abs(Data.LocomotionAngle) > 135.0f;

		// 4. ✨ 物理限制：如果你向后 180 度起步，强行关闭胶囊体的平滑转向 (Steering) ✨
		// 因为向后起步的 Warping 动画本身会带有极大的身体扭转和 Root Motion 位移，
		// 如果物理层也跟着转，脚底就会疯狂打滑。
		if (Data.bIsPivotStart)
		{
			// 注意：如果你使用了自定义的 UAdvancedLocoMovementComponent，这里应该强制转换并调用你写的 SetSteeringEnabled(false)。
			// 这里用系统默认组件做演示，关闭其“自动朝向运动方向”的功能。
			Data.Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		}
	}

	// 告诉 StateTree，起步任务正在完美运行，请保持在这个状态不要动！
	return EStateTreeRunStatus::Running;
}

void FSTT_StartMovement::ExitState(FStateTreeExecutionContext& Context,
                                   const FStateTreeTransitionResult& Transition) const
{
	FSTT_StartMovement_InstanceData& Data = Context.GetInstanceData<FSTT_StartMovement_InstanceData>(*this);

	if (Data.Character && Data.bIsPivotStart)
	{
		// 离开向后起步状态时，务必恢复物理层的平滑转向功能！
		// 这样角色进入 Cycle 奔跑状态后，又能像开车一样平滑控制方向了。
		Data.Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}
