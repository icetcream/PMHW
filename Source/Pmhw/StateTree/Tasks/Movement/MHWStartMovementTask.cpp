#include "MHWStartMovementTask.h"
#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"


EStateTreeRunStatus FMHWStartMovementTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Start);
		ApplyRotationInterpolationSettings(
			InputModel,
			InstanceData.RotationTickRLerpSpeed,
			InstanceData.RotationTargetConstantLerpSpeed);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMHWStartMovementTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
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

void FMHWStartMovementTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ClearRotationCurveCompensation(InstanceData.InputEvaluatorModel);
}
