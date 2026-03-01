#include "STTask_CalculateStartAngle.h"

#include "StateTreeExecutionContext.h"
#include "Math/UnrealMathUtility.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"


EStateTreeRunStatus FSTTask_CalculateStartAngle::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FSTTask_CalculateStartAngle_InstanceData& Data = Context.GetInstanceData<FSTTask_CalculateStartAngle_InstanceData>(*this);
	const UControllerInputEvaluatorModel* Model = Data.InputEvaluatorModel;

	// 1. 安全检查：仓库存在，且有输入方向
	if (Model && !Model->UserInputDirection.IsNearlyZero())
	{
		// 2. 直接从仓库拿准备好的数据
		FVector ForwardDir = Model->ActorForwardDirection;
		FVector InputDir = Model->UserInputDirection;

		// 3. 纯粹的数学运算 (极其高效，没有任何 Cast)
		float DotResult = FVector::DotProduct(ForwardDir, InputDir);
		DotResult = FMath::Clamp(DotResult, -1.0f, 1.0f); 
		Data.ResultAngle = FMath::RadiansToDegrees(FMath::Acos(DotResult));
	}
	else
	{
		Data.ResultAngle = 0.0f;
	}

	return EStateTreeRunStatus::Running;
}
