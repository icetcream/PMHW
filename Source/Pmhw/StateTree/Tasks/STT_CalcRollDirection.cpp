#include "STT_CalcRollDirection.h"

#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

EStateTreeRunStatus FSTT_CalcRollDirection::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTT_CalcRollDirectionInstanceData& InstanceData = Context.GetInstanceData<FSTT_CalcRollDirectionInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;

	if (!Character) return EStateTreeRunStatus::Failed;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	FVector InputVector = MoveComp ? MoveComp->GetCurrentAcceleration() : FVector::ZeroVector;

	// 默认向前
	InstanceData.OutSectionName = Section_Forward;
	FRotator TargetWarpRot = Character->GetActorRotation();

	if (InputVector.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		FRotator InputWorldRot = InputVector.Rotation();
		float InputYawDelta = (InputWorldRot - Character->GetActorRotation()).GetNormalized().Yaw;

		// 定义这个动画本身自带的角度偏移
		float AnimOffsetYaw = 0.0f;

		if (InputYawDelta >= -45.0f && InputYawDelta <= 45.0f)
		{
			InstanceData.OutSectionName = Section_Forward;
			AnimOffsetYaw = 0.0f;    // 向前翻滚，自带 0 度
		}
		else if (InputYawDelta > 45.0f && InputYawDelta <= 135.0f)
		{
			InstanceData.OutSectionName = Section_Right;
			AnimOffsetYaw = 90.0f;   // 向右翻滚，自带向右 90 度
		}
		else if (InputYawDelta < -45.0f && InputYawDelta >= -135.0f)
		{
			InstanceData.OutSectionName = Section_Left;
			AnimOffsetYaw = -90.0f;  // 向左翻滚，自带向左 -90 度
		}
		else
		{
			InstanceData.OutSectionName = Section_Backward;
			AnimOffsetYaw = 180.0f;  // 向后翻滚，自带 180 度
		}

		// 核心魔法在这里：
		// 目标旋转 = 输入的绝对旋转 - 动画本身自带的旋转
		TargetWarpRot = InputWorldRot;
		TargetWarpRot.Yaw -= AnimOffsetYaw;
		TargetWarpRot.Normalize(); // 规范化角度到 -180 ~ 180，防止度数溢出
	}

	// 包装成 Transform 输出
	InstanceData.OutWarpTarget.SetLocation(Character->GetActorLocation());
	InstanceData.OutWarpTarget.SetRotation(TargetWarpRot.Quaternion());

	return EStateTreeRunStatus::Succeeded;
}