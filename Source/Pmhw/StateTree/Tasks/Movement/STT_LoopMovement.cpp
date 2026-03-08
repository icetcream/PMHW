#include "STT_LoopMovement.h"

#include "MHWLogChannels.h"
#include "StateTreeExecutionContext.h"
#include "Character/MHWMovementComponent.h"
#include "Input/MHWInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"




EStateTreeRunStatus FSTT_LoopMovement::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FSTT_LoopMovement_InstanceData& Data = Context.GetInstanceData<FSTT_LoopMovement_InstanceData>(*this);
	const UControllerInputEvaluatorModel* Model = Data.InputEvaluatorModel;

	if (Model && Model->MovementComponent && Model->InputComponent)
	{
		UMHWMovementComponent* MoveComp = Model->MovementComponent;
		UMHWInputComponent* InputComp = Model->InputComponent;

		// 1. 设置目标加速度
		MoveComp->MaxAcceleration = Data.MaxAcceleration;

		// 2. 进入时做一次速度插值 (防止从 Walk 切到 Run 时速度突变)
		// 注意：StateTree 的 EnterState 没有直接的 DeltaTime 参数，我们通过 World 获取
		float DeltaSeconds = 0.0f;
		if (UWorld* World = MoveComp->GetWorld())
		{
			DeltaSeconds = World->GetDeltaSeconds();
		}
		MoveComp->MaxWalkSpeed = UKismetMathLibrary::FInterpTo(MoveComp->MaxWalkSpeed, Data.MaxWalkSpeed, DeltaSeconds, SpeedInterpSpeed);

		// 3. 清理掉之前状态（比如起步状态）残留的输入向量
		MoveComp->ConsumeInputVector();

		// 4. 注入当前的平滑输入方向 (让角色开始移动)
		/*MoveComp->AddInputVector(InputComp->GetSmoothInputDirection(), false);*/
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_LoopMovement::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTT_LoopMovement_InstanceData& Data = Context.GetInstanceData<FSTT_LoopMovement_InstanceData>(*this);
	const UControllerInputEvaluatorModel* Model = Data.InputEvaluatorModel;

	if (Model && Model->MovementComponent && Model->InputComponent)
	{
		UMHWMovementComponent* MoveComp = Model->MovementComponent;
		UMHWInputComponent* InputComp = Model->InputComponent;

		// 1. 持续平滑速度
		MoveComp->MaxWalkSpeed = UKismetMathLibrary::FInterpTo(MoveComp->MaxWalkSpeed, Data.MaxWalkSpeed, DeltaTime, SpeedInterpSpeed);

		UE_LOG(LogPMHW,Log, TEXT("UserInputDirection = % s"), *Model->UserInputDirection.ToString());
		// 2. 更新平滑转向 (Steering)
		// 拿玩家纯粹的输入方向 (UserInputDirection)，通过插值算法，慢慢逼近目标方向，消除手抖
		InputComp->UpdateSmoothInput(Model->UserInputDirection, SteeringInterpSpeed, DeltaTime);

		// 3. 注入最新的平滑方向驱动角色移动
		MoveComp->AddInputVector(InputComp->GetSmoothInputDirection(), false);

		// 4. ✨ 核心机制：检测急停 (Pivot) ✨
		if (bCanCheckPivot)
		{
			// 拿着"身体实际飞行的方向" 和 "玩家想去的方向" 做对比
			Data.bIsPivot = MoveComp->CalculateIsInPivot(
				MoveComp->Velocity.GetSafeNormal(), 
				Model->UserInputDirection
			);
			
			// 把结果同步给移动组件 (可能底层的动画蓝图或者其他地方也要读)
			MoveComp->SetIsPivot(Data.bIsPivot);
		}
	}

	return EStateTreeRunStatus::Running;
}
