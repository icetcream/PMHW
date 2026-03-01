#include "STE_InputEvaluator.h"

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
		// 2. 只有在这里，我们才执行极其昂贵的 GetPawn() 和 Cast 操作！
		if (ACharacter* CurrentChar = Cast<ACharacter>(Data.Controller->GetPawn()))
		{
			// 3. 把指针安全地存进 Model 仓库里
			Data.EvaluatorModel->MovementComponent = CurrentChar->GetComponentByClass<UMHWMovementComponent>();
			Data.EvaluatorModel->InputComponent = CurrentChar->GetComponentByClass<UMHWInputComponent>();
			
			// 顺便把输入方向也算好存进去
			Data.EvaluatorModel->ControlledActor = CurrentChar;
			Data.EvaluatorModel->ActorForwardDirection = CurrentChar->GetActorForwardVector().GetSafeNormal2D();
			Data.EvaluatorModel->UserInputDirection = CurrentChar->GetLastMovementInputVector().GetSafeNormal2D();
		}
		else
		{
			// 如果玩家死了（没有身体），立刻清空仓库，防止野指针崩溃
			Data.EvaluatorModel->MovementComponent = nullptr;
			Data.EvaluatorModel->InputComponent = nullptr;
			Data.EvaluatorModel->UserInputDirection = FVector::ZeroVector;
		}
	}
}
