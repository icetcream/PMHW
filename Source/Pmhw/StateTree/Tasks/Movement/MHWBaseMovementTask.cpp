#include "MHWBaseMovementTask.h"

#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"
#include "Character/MHWMovementComponent.h"

EStateTreeRunStatus FMHWBaseMovementTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FMHWBaseMovementTaskInstanceData>(*this);
	bool bSuccess = SetLocomotionState(InstanceData.InputEvaluatorModel, InstanceData.TargetState);
	return EStateTreeRunStatus::Running;
}

bool FMHWBaseMovementTask::SetLocomotionState(UControllerInputEvaluatorModel* EvaluatorModel, ELocomotionState NewState) const
{
	if (!EvaluatorModel)
	{
		UE_LOG(LogStateTree, Error, TEXT("SetLocomotionState Failed: InputEvaluatorModel is null!"));
		return false;
	}
	
	if (UMHWMovementComponent* MoveComp = EvaluatorModel->MovementComponent)
	{
		MoveComp->SetLocomotionState(NewState); 
		return true;
	}
	UE_LOG(LogStateTree, Error, TEXT("SetLocomotionState Failed: MovementComponent inside Evaluator is null!"));
	return false;
}
