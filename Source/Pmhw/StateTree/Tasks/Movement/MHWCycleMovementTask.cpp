#include "MHWCycleMovementTask.h"

#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

EStateTreeRunStatus FMHWCycleMovementTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Cycle);
		ApplyRotationInterpolationSettings(
			InputModel,
			InstanceData.RotationTickRLerpSpeed,
			InstanceData.RotationTargetConstantLerpSpeed);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMHWCycleMovementTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (!InputModel)
	{
		return EStateTreeRunStatus::Failed;
	}

	ApplyRotationCurveCompensation(
		InputModel,
		InstanceData.bEnableRotationCurveCompensation,
		InstanceData.RotationCompensationCurveName,
		InstanceData.RotationCompensationCurveScale);

	return EStateTreeRunStatus::Running;
}

void FMHWCycleMovementTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ClearRotationCurveCompensation(InstanceData.InputEvaluatorModel);
}
