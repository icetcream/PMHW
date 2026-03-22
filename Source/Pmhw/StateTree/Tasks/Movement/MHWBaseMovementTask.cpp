#include "MHWBaseMovementTask.h"

#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"
#include "Character/MHWMovementComponent.h"
#include "Character/MHWCharacter.h"

EStateTreeRunStatus FMHWBaseMovementTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FMHWBaseMovementTaskInstanceData>(*this);
	SetLocomotionState(InstanceData.InputEvaluatorModel, InstanceData.TargetState);
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

void FMHWBaseMovementTask::ApplyRotationCurveCompensation(UControllerInputEvaluatorModel* EvaluatorModel, bool bEnable, FName CurveName, float CurveScale) const
{
	if (!EvaluatorModel || !EvaluatorModel->ControlledActor)
	{
		return;
	}

	if (AMHWCharacter* Character = Cast<AMHWCharacter>(EvaluatorModel->ControlledActor))
	{
		Character->SetRotationCurveCompensation(bEnable, CurveName, CurveScale);
	}
}

void FMHWBaseMovementTask::ClearRotationCurveCompensation(UControllerInputEvaluatorModel* EvaluatorModel) const
{
	ApplyRotationCurveCompensation(EvaluatorModel, false, NAME_None, 1.0f);
}

void FMHWBaseMovementTask::ApplyRotationInterpolationSettings(UControllerInputEvaluatorModel* EvaluatorModel, float InRotationTickRLerpSpeed, float InRotationTargetConstantLerpSpeed) const
{
	if (!EvaluatorModel || !EvaluatorModel->ControlledActor)
	{
		return;
	}

	if (AMHWCharacter* Character = Cast<AMHWCharacter>(EvaluatorModel->ControlledActor))
	{
		Character->SetRotationInterpolationSettings(InRotationTickRLerpSpeed, InRotationTargetConstantLerpSpeed);
	}
}
