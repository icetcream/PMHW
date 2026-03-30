#include "Animation/Modifiers/MHWAnimModifier_AddFootLockCurves.h"

#include "AnimationBlueprintLibrary.h"
#include "Animation/AnimSequence.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWAnimModifier_AddFootLockCurves)

void UMHWAnimModifier_AddFootLockCurves::OnApply_Implementation(UAnimSequence* AnimationSequence)
{
	if (!AnimationSequence)
	{
		return;
	}

	EnsureCurve(AnimationSequence, LeftFootCurveName);
	EnsureCurve(AnimationSequence, RightFootCurveName);

	if (!bAddDefaultKeys)
	{
		return;
	}

	const float SequenceLength = AnimationSequence->GetPlayLength();
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, LeftFootCurveName, 0.0f, DefaultCurveValue);
	UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, RightFootCurveName, 0.0f, DefaultCurveValue);

	if (SequenceLength > 0.0f)
	{
		UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, LeftFootCurveName, SequenceLength, DefaultCurveValue);
		UAnimationBlueprintLibrary::AddFloatCurveKey(AnimationSequence, RightFootCurveName, SequenceLength, DefaultCurveValue);
	}
}

void UMHWAnimModifier_AddFootLockCurves::OnRevert_Implementation(UAnimSequence* AnimationSequence)
{
	if (!AnimationSequence)
	{
		return;
	}

	if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, LeftFootCurveName, ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, LeftFootCurveName, false);
	}

	if (UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, RightFootCurveName, ERawCurveTrackTypes::RCT_Float))
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, RightFootCurveName, false);
	}
}

void UMHWAnimModifier_AddFootLockCurves::EnsureCurve(UAnimSequence* AnimationSequence, const FName CurveName) const
{
	if (!AnimationSequence || CurveName.IsNone())
	{
		return;
	}

	const bool bCurveExists = UAnimationBlueprintLibrary::DoesCurveExist(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float);
	if (bCurveExists && bRecreateExistingCurves)
	{
		UAnimationBlueprintLibrary::RemoveCurve(AnimationSequence, CurveName, false);
	}

	if (!bCurveExists || bRecreateExistingCurves)
	{
		UAnimationBlueprintLibrary::AddCurve(AnimationSequence, CurveName, ERawCurveTrackTypes::RCT_Float, false);
	}
}
