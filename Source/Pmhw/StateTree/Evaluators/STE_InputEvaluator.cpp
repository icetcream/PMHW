#include "STE_InputEvaluator.h"

#include "MHWLogChannels.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "Input/MHWInputComponent.h"
#include "Character/MHWMovementComponent.h"

void FSTE_InputEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FSTE_InputEvaluator_InstanceData& Data = Context.GetInstanceData<FSTE_InputEvaluator_InstanceData>(*this);

	// 1. 在状态树刚开始运行时，New 一个 Model 对象出来，存放在 Evaluator 的输出变量里
	if (!Data.EvaluatorModel)
	{
		Data.EvaluatorModel = NewObject<UControllerInputEvaluatorModel>();
	}
}

void FSTE_InputEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTE_InputEvaluator_InstanceData& Data = Context.GetInstanceData<FSTE_InputEvaluator_InstanceData>(*this);

	if (Data.Controller && Data.EvaluatorModel)
	{
		if (ACharacter* CurrentChar = Cast<ACharacter>(Data.Controller->GetPawn()))
		{
			UMHWMovementComponent* MoveComp = CurrentChar->GetComponentByClass<UMHWMovementComponent>();
			UMHWInputComponent* InputComp = CurrentChar->GetComponentByClass<UMHWInputComponent>();

			if (MoveComp && InputComp)
			{
				// 1. 存进 Model (给 C++ Task 用)
				Data.EvaluatorModel->MovementComponent = MoveComp;
				Data.EvaluatorModel->InputComponent = InputComp;
				Data.EvaluatorModel->ActorForwardDirection = CurrentChar->GetActorForwardVector().GetSafeNormal2D();
				Data.EvaluatorModel->UserInputDirection = MoveComp->GetLastInputVector();
				
				// 2. ✨ 把 UI 需要的判断条件暴露
				Data.bHasMovementInput = !InputComp->RawMoveInput.IsNearlyZero();
				
				// 注意：这里是 SizeSquared2D，适合判断是否有速度，但如果你需要真实速度（cm/s），应该用 Size2D()
				Data.GroundVelocity = MoveComp->Velocity.SizeSquared2D();
				Data.MaxGroundVelocity = MoveComp->MaxWalkSpeed;
				Data.bHasVelocity = Data.GroundVelocity > 0.01f;
				Data.bIsFalling = MoveComp->IsFalling();
				Data.bHasRootMotion = CurrentChar->HasAnyRootMotion();

				// ==========================================
				// ✨ 3. 新增计算：速度与朝向夹角
				// ==========================================
				if (Data.bHasVelocity)
				{
					// 获取速度的朝向
					FRotator VelocityRot = MoveComp->Velocity.Rotation();
					// 获取角色的朝向
					FRotator ActorRot = CurrentChar->GetActorRotation();
					// 计算差值
					FRotator DeltaRot = VelocityRot - ActorRot;
					// 归一化（非常重要！这会把角度约束到 -180 到 180 之间）
					DeltaRot.Normalize(); 
					Data.VelocityLocalAngle = DeltaRot.Yaw;
				}
				else
				{
					// 没有速度时角度归零（或者你可以选择保留上一帧的数值，取决于你的 Locomotion 需求）
					Data.VelocityLocalAngle = 0.f; 
				}

				// ==========================================
				// ✨ 4. 新增计算：加速度与朝向夹角
				// 加速度代表的是"玩家真正按摇杆的意图方向"，通常比速度响应更快
				// ==========================================
				FVector Acceleration = MoveComp->GetCurrentAcceleration();
				if (Acceleration.SizeSquared2D() > UE_KINDA_SMALL_NUMBER)
				{
					FRotator AccelRot = Acceleration.Rotation();
					FRotator ActorRot = CurrentChar->GetActorRotation();
					FRotator DeltaRot = AccelRot - ActorRot;
					DeltaRot.Normalize();
					Data.AccelerationLocalAngle = DeltaRot.Yaw;
				}
				else
				{
					Data.AccelerationLocalAngle = 0.f;
				}
			}
		}
		else
		{
			// 清空逻辑...
			Data.bHasMovementInput = false;
			Data.bIsFalling = false;
			Data.bHasVelocity = false;
			Data.VelocityLocalAngle = 0.f;       // ✨ 清空角度
			Data.AccelerationLocalAngle = 0.f;   // ✨ 清空角度
		}
	}
}
