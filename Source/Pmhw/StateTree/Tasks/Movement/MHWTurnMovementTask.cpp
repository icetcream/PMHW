#include "MHWTurnMovementTask.h"

#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

EStateTreeRunStatus FMHWTurnMovementTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (InputModel && InputModel->MovementComponent)
	{
		InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Turn);
		ApplyRotationInterpolationSettings(
			InputModel,
			InstanceData.RotationTickRLerpSpeed,
			InstanceData.RotationTargetConstantLerpSpeed);

		InstanceData.TurnCurrentTime = 0.0f;
		InputModel->MovementComponent->TurnPercent = 0.0f;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMHWTurnMovementTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;

	if (!InputModel || !InputModel->MovementComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	InputModel->MovementComponent->SetLocomotionState(ELocomotionState::Turn);

	InstanceData.TurnCurrentTime += DeltaTime;
	const float SafeTotalTime = FMath::Max(InstanceData.TurnTotalTime, UE_SMALL_NUMBER);
	InputModel->MovementComponent->TurnPercent = FMath::Clamp(InstanceData.TurnCurrentTime / SafeTotalTime, 0.0f, 1.0f);

	ApplyRotationCurveCompensation(
		InputModel,
		InstanceData.bEnableRotationCurveCompensation,
		InstanceData.RotationCompensationCurveName,
		InstanceData.RotationCompensationCurveScale);

	return EStateTreeRunStatus::Running;
}

void FMHWTurnMovementTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	UControllerInputEvaluatorModel* InputModel = InstanceData.InputEvaluatorModel;
	InstanceData.TurnCurrentTime = 0.0f;

	if (InputModel && InputModel->MovementComponent)
	{
		InputModel->MovementComponent->TurnPercent = 0.0f;
	}

	ClearRotationCurveCompensation(InputModel);
}
