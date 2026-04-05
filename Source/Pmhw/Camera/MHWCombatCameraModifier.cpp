#include "Camera/MHWCombatCameraModifier.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatCameraModifier)

namespace CombatCameraModifier
{
	static void SetMotionTargets(FMHWCameraMotionRuntimeState& CameraMotionState, const FMHWCameraMotionSettings& InMotionSettings)
	{
		CameraMotionState.StartLocalOffset = CameraMotionState.CurrentLocalOffset;
		CameraMotionState.StartRotationOffset = CameraMotionState.CurrentRotationOffset;
		CameraMotionState.StartFOVOffset = CameraMotionState.CurrentFOVOffset;

		CameraMotionState.TargetLocalOffset = InMotionSettings.LocalOffset;
		CameraMotionState.TargetRotationOffset = InMotionSettings.RotationOffset;
		CameraMotionState.TargetFOVOffset = InMotionSettings.FOVOffset;
		CameraMotionState.BlendElapsed = 0.0f;
		CameraMotionState.BlendDuration = FMath::Max(0.0f, InMotionSettings.BlendDuration);
		CameraMotionState.bHasActiveBlend = true;
	}

	static void SnapMotionToTarget(FMHWCameraMotionRuntimeState& CameraMotionState)
	{
		CameraMotionState.CurrentLocalOffset = CameraMotionState.TargetLocalOffset;
		CameraMotionState.CurrentRotationOffset = CameraMotionState.TargetRotationOffset;
		CameraMotionState.CurrentFOVOffset = CameraMotionState.TargetFOVOffset;
		CameraMotionState.bHasActiveBlend = false;
	}
}

void UMHWCombatCameraModifier::PlaySpringShake(const FMHWCameraSpringShakeSettings& InShakeSettings)
{
	if (!InShakeSettings.IsEnabled())
	{
		return;
	}

	FMHWActiveSpringCameraShake& NewShake = ActiveSpringShakes.AddDefaulted_GetRef();
	NewShake.Settings = InShakeSettings;
	NewShake.ElapsedTime = 0.0f;
}

void UMHWCombatCameraModifier::ApplyCameraMotion(const FMHWCameraMotionSettings& InMotionSettings)
{
	CombatCameraModifier::SetMotionTargets(CameraMotionState, InMotionSettings);

	if (CameraMotionState.BlendDuration <= 0.0f)
	{
		CombatCameraModifier::SnapMotionToTarget(CameraMotionState);
	}
}

void UMHWCombatCameraModifier::ClearCameraMotion(const float BlendOutDuration)
{
	FMHWCameraMotionSettings ResetMotion;
	ResetMotion.BlendDuration = BlendOutDuration;
	ApplyCameraMotion(ResetMotion);
}

bool UMHWCombatCameraModifier::ModifyCamera(const float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	Super::ModifyCamera(DeltaTime, InOutPOV);

	UpdateCameraMotion(DeltaTime);

	FVector TotalLocalLocationOffset = CameraMotionState.CurrentLocalOffset;
	FRotator TotalRotationOffset = CameraMotionState.CurrentRotationOffset;
	float TotalFOVOffset = CameraMotionState.CurrentFOVOffset;

	ApplySpringShakes(DeltaTime, TotalLocalLocationOffset, TotalRotationOffset, TotalFOVOffset);

	InOutPOV.Location += InOutPOV.Rotation.RotateVector(TotalLocalLocationOffset);
	InOutPOV.Rotation = (InOutPOV.Rotation + TotalRotationOffset).GetNormalized();
	InOutPOV.FOV = FMath::Max(5.0f, InOutPOV.FOV + TotalFOVOffset);
	return false;
}

void UMHWCombatCameraModifier::UpdateCameraMotion(const float DeltaTime)
{
	if (!CameraMotionState.bHasActiveBlend)
	{
		return;
	}

	if (CameraMotionState.BlendDuration <= 0.0f)
	{
		CombatCameraModifier::SnapMotionToTarget(CameraMotionState);
		return;
	}

	CameraMotionState.BlendElapsed = FMath::Min(CameraMotionState.BlendElapsed + DeltaTime, CameraMotionState.BlendDuration);
	const float MotionAlpha = FMath::Clamp(CameraMotionState.BlendElapsed / CameraMotionState.BlendDuration, 0.0f, 1.0f);
	const float BlendAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, MotionAlpha, 2.0f);

	CameraMotionState.CurrentLocalOffset = FMath::Lerp(CameraMotionState.StartLocalOffset, CameraMotionState.TargetLocalOffset, BlendAlpha);
	CameraMotionState.CurrentRotationOffset = FMath::Lerp(CameraMotionState.StartRotationOffset, CameraMotionState.TargetRotationOffset, BlendAlpha);
	CameraMotionState.CurrentFOVOffset = FMath::Lerp(CameraMotionState.StartFOVOffset, CameraMotionState.TargetFOVOffset, BlendAlpha);

	if (MotionAlpha >= 1.0f)
	{
		CameraMotionState.bHasActiveBlend = false;
	}
}

void UMHWCombatCameraModifier::ApplySpringShakes(const float DeltaTime, FVector& InOutLocalLocationOffset, FRotator& InOutRotationOffset, float& InOutFOVOffset)
{
	for (int32 ShakeIndex = ActiveSpringShakes.Num() - 1; ShakeIndex >= 0; --ShakeIndex)
	{
		FMHWActiveSpringCameraShake& ActiveShake = ActiveSpringShakes[ShakeIndex];
		ActiveShake.ElapsedTime += DeltaTime;

		if (ActiveShake.ElapsedTime >= ActiveShake.Settings.Duration)
		{
			ActiveSpringShakes.RemoveAtSwap(ShakeIndex);
			continue;
		}

		const float Envelope = FMath::Exp(-ActiveShake.Settings.Damping * ActiveShake.ElapsedTime);
		const float Phase = 2.0f * PI * ActiveShake.Settings.Frequency * ActiveShake.ElapsedTime;
		const float Oscillation = Envelope * FMath::Cos(Phase);

		InOutLocalLocationOffset += ActiveShake.Settings.LocalLocationAmplitude * Oscillation;
		InOutRotationOffset.Pitch += ActiveShake.Settings.RotationAmplitude.Pitch * Oscillation;
		InOutRotationOffset.Yaw += ActiveShake.Settings.RotationAmplitude.Yaw * Oscillation;
		InOutRotationOffset.Roll += ActiveShake.Settings.RotationAmplitude.Roll * Oscillation;
		InOutFOVOffset += ActiveShake.Settings.FOVAmplitude * Oscillation;
	}
}
