#include "STT_CalcDirectionByAcceleration.h"

#include "DrawDebugHelpers.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

EStateTreeRunStatus FSTT_CalcDirectionByAcceleration::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTT_CalcDirectionByAccelerationInstanceData& InstanceData = Context.GetInstanceData<FSTT_CalcDirectionByAccelerationInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;

	if (!Character) return EStateTreeRunStatus::Failed;

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	FVector InputVector = MoveComp ? MoveComp->GetCurrentAcceleration() : FVector::ZeroVector;

	// 默认向前
	InstanceData.OutDirectionSection = Direction_Forward;
	FRotator TargetWarpRot = Character->GetActorRotation();

	if (InputVector.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		FRotator InputWorldRot = InputVector.Rotation();
		float InputYawDelta = (InputWorldRot - Character->GetActorRotation()).GetNormalized().Yaw;

		// 定义动画本身自带的角度偏移
		float AnimOffsetYaw = 0.0f;
		const float AbsYaw = FMath::Abs(InputYawDelta);
		const float ClampedForwardHalfAngle = FMath::Clamp(ForwardHalfAngle, 0.0f, 89.9f);

		if (OutputMode == EMHWDirectionOutputMode::FourWay)
		{
			if (AbsYaw <= ClampedForwardHalfAngle)
			{
				InstanceData.OutDirectionSection = Direction_Forward;
				AnimOffsetYaw = 0.0f;
			}
			else if (InputYawDelta > ClampedForwardHalfAngle && InputYawDelta <= 180.0f - ClampedForwardHalfAngle)
			{
				InstanceData.OutDirectionSection = Direction_Right;
				AnimOffsetYaw = bRightTurnUseNegativeWarpCompensation ? -90.0f : 90.0f;
			}
			else if (InputYawDelta < -ClampedForwardHalfAngle && InputYawDelta >= -180.0f + ClampedForwardHalfAngle)
			{
				InstanceData.OutDirectionSection = Direction_Left;
				AnimOffsetYaw = -90.0f;
			}
			else
			{
				InstanceData.OutDirectionSection = Direction_Backward;
				AnimOffsetYaw = 180.0f;
			}
		}
		else
		{
			// ThreeWay：前、左、右（后左并左，后右并右）
			if (AbsYaw <= ClampedForwardHalfAngle)
			{
				InstanceData.OutDirectionSection = Direction_Forward;
				AnimOffsetYaw = 0.0f;
			}
			else if (InputYawDelta > 0.0f)
			{
				InstanceData.OutDirectionSection = Direction_Right;
				AnimOffsetYaw = bRightTurnUseNegativeWarpCompensation ? -90.0f : 90.0f;
			}
			else
			{
				InstanceData.OutDirectionSection = Direction_Left;
				AnimOffsetYaw = -90.0f;
			}
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

	if (bDebugDrawArrows)
	{
		if (UWorld* World = Character->GetWorld())
		{
			const FVector BaseLocation = Character->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
			const FVector InputDir = InputVector.GetSafeNormal();
			const FVector InputEnd = BaseLocation + (InputDir.IsNearlyZero() ? Character->GetActorForwardVector() : InputDir) * 100.0f;
			const FVector WarpEnd = BaseLocation + TargetWarpRot.Vector() * 120.0f;

			DrawDebugDirectionalArrow(World, BaseLocation, InputEnd, 18.0f, FColor::Cyan, false, 1.5f, 0, 2.0f);
			DrawDebugDirectionalArrow(World, BaseLocation + FVector(0.0f, 0.0f, 8.0f), WarpEnd + FVector(0.0f, 0.0f, 8.0f), 18.0f, FColor::Yellow, false, 1.5f, 0, 2.0f);
		}
	}

	return EStateTreeRunStatus::Running;
}
