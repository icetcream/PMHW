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
			// 1. 把指针存进 Model (给 C++ Task 用)
			Data.EvaluatorModel->MovementComponent = CurrentChar->GetComponentByClass<UMHWMovementComponent>();
			// ...
			Data.EvaluatorModel->InputComponent = CurrentChar->GetComponentByClass<UMHWInputComponent>();
			Data.EvaluatorModel->ActorForwardDirection = CurrentChar->GetActorForwardVector().GetSafeNormal2D();
			Data.EvaluatorModel->UserInputDirection = Data.EvaluatorModel->MovementComponent->GetLastInputVector();
			
			// 2. ✨ 把 UI 需要的判断条件，直接暴露在 Data 上 ✨

			Data.bHasMovementInput = !Data.EvaluatorModel->InputComponent->RawMoveInput.IsNearlyZero();
			FVector Raw = Data.EvaluatorModel->InputComponent->RawMoveInput;
			Data.GroundVelocity = Data.EvaluatorModel->MovementComponent->Velocity.SizeSquared2D();
            Data.bHasVelocity = Data.GroundVelocity > 0.01f;
			if (UMHWMovementComponent* MoveComp = Data.EvaluatorModel->MovementComponent)
			{
				Data.bIsFalling = MoveComp->IsFalling();
			}
		}
		else
		{
			// 清空逻辑...
			Data.bHasMovementInput = false;
			Data.bIsFalling = false;
			Data.bHasVelocity = false;
		}
	}
}
