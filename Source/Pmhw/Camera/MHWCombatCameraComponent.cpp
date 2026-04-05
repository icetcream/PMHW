#include "Camera/MHWCombatCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "Character/MHWCharacter.h"
#include "Data/MHWCombatCameraDataTable.h"
#include "Engine/DataTable.h"
#include "GameFramework/SpringArmComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatCameraComponent)

namespace CombatCameraComponent
{
	static void SetBlendTargets(FMHWCombatCameraBlendState& BlendState, const FMHWCombatCameraArmMotionSettings& InMotionSettings)
	{
		BlendState.StartArmLengthOffset = BlendState.CurrentArmLengthOffset;
		BlendState.TargetArmLengthOffset = InMotionSettings.ArmLengthOffset;

		BlendState.StartSocketOffset = BlendState.CurrentSocketOffset;
		BlendState.TargetSocketOffset = InMotionSettings.SocketOffset;

		BlendState.StartTargetOffset = BlendState.CurrentTargetOffset;
		BlendState.TargetTargetOffset = InMotionSettings.TargetOffset;

		BlendState.StartBoomRotationOffset = BlendState.CurrentBoomRotationOffset;
		BlendState.TargetBoomRotationOffset = InMotionSettings.BoomRotationOffset;

		BlendState.StartFOVOffset = BlendState.CurrentFOVOffset;
		BlendState.TargetFOVOffset = InMotionSettings.FOVOffset;

		BlendState.BlendElapsed = 0.0f;
		BlendState.BlendDuration = FMath::Max(0.0f, InMotionSettings.BlendDuration);
		BlendState.bHasActiveBlend = true;
	}

	static void SnapBlendToTarget(FMHWCombatCameraBlendState& BlendState)
	{
		BlendState.CurrentArmLengthOffset = BlendState.TargetArmLengthOffset;
		BlendState.CurrentSocketOffset = BlendState.TargetSocketOffset;
		BlendState.CurrentTargetOffset = BlendState.TargetTargetOffset;
		BlendState.CurrentBoomRotationOffset = BlendState.TargetBoomRotationOffset;
		BlendState.CurrentFOVOffset = BlendState.TargetFOVOffset;
		BlendState.bHasActiveBlend = false;
	}
}

UMHWCombatCameraComponent::UMHWCombatCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMHWCombatCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	ResolveCameraReferences();
	CacheDefaultCameraState();
	ApplyCurrentStateToComponents();
}

void UMHWCombatCameraComponent::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CachedSpringArm.IsValid() || !CachedCamera.IsValid() || !bDefaultsCached)
	{
		ResolveCameraReferences();
		if (!bDefaultsCached)
		{
			CacheDefaultCameraState();
		}
	}

	UpdateBlendState(DeltaTime);
	ApplyCurrentStateToComponents();
}

void UMHWCombatCameraComponent::ApplyCameraMotion(const FMHWCombatCameraArmMotionSettings& InMotionSettings)
{
	ResolveCameraReferences();
	if (!bDefaultsCached)
	{
		CacheDefaultCameraState();
	}

	CombatCameraComponent::SetBlendTargets(BlendState, InMotionSettings);

	if (BlendState.BlendDuration <= 0.0f)
	{
		CombatCameraComponent::SnapBlendToTarget(BlendState);
	}
}

bool UMHWCombatCameraComponent::ApplyCameraMotionByTag(const FGameplayTag& CameraMotionTag)
{
	if (const FMHWCombatCameraMotionRow* CameraMotionRow = FindCameraMotionRowByTag(CameraMotionTag))
	{
		ApplyCameraMotion(CameraMotionRow->CameraMotion);
		return true;
	}

	return false;
}

void UMHWCombatCameraComponent::ClearCameraMotion(const float BlendOutDuration)
{
	FMHWCombatCameraArmMotionSettings ResetSettings;
	ResetSettings.BlendDuration = BlendOutDuration;
	ApplyCameraMotion(ResetSettings);
}

void UMHWCombatCameraComponent::SnapToDefaultCamera()
{
	ResolveCameraReferences();
	if (!bDefaultsCached)
	{
		CacheDefaultCameraState();
	}

	BlendState = FMHWCombatCameraBlendState();
	ApplyCurrentStateToComponents();
}

const FMHWCombatCameraMotionRow* UMHWCombatCameraComponent::FindCameraMotionRowByTag(const FGameplayTag& CameraMotionTag) const
{
	if (!CameraMotionDataTable || !CameraMotionTag.IsValid())
	{
		return nullptr;
	}

	static const FString ContextString(TEXT("FindCameraMotionRowByTag"));
	TArray<FMHWCombatCameraMotionRow*> Rows;
	CameraMotionDataTable->GetAllRows<FMHWCombatCameraMotionRow>(ContextString, Rows);
	for (const FMHWCombatCameraMotionRow* Row : Rows)
	{
		if (Row && Row->CameraMotionTag == CameraMotionTag)
		{
			return Row;
		}
	}

	return nullptr;
}

void UMHWCombatCameraComponent::ResolveCameraReferences()
{
	if (!CachedSpringArm.IsValid())
	{
		if (const AMHWCharacter* Character = Cast<AMHWCharacter>(GetOwner()))
		{
			CachedSpringArm = Character->GetCameraBoom();
		}
		else
		{
			CachedSpringArm = GetOwner() ? GetOwner()->FindComponentByClass<USpringArmComponent>() : nullptr;
		}
	}

	if (!CachedCamera.IsValid())
	{
		if (const AMHWCharacter* Character = Cast<AMHWCharacter>(GetOwner()))
		{
			CachedCamera = Character->GetFollowCamera();
		}
		else
		{
			CachedCamera = GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
		}
	}
}

void UMHWCombatCameraComponent::CacheDefaultCameraState()
{
	bool bHasResolvedAnyDefaults = false;

	if (USpringArmComponent* SpringArm = CachedSpringArm.Get())
	{
		DefaultArmLength = SpringArm->TargetArmLength;
		DefaultSocketOffset = SpringArm->SocketOffset;
		DefaultTargetOffset = SpringArm->TargetOffset;
		DefaultBoomRotation = SpringArm->GetRelativeRotation();
		bHasResolvedAnyDefaults = true;
	}

	if (UCameraComponent* Camera = CachedCamera.Get())
	{
		DefaultCameraFOV = Camera->FieldOfView;
		bHasResolvedAnyDefaults = true;
	}

	bDefaultsCached = bHasResolvedAnyDefaults;
}

void UMHWCombatCameraComponent::UpdateBlendState(const float DeltaTime)
{
	if (!BlendState.bHasActiveBlend)
	{
		return;
	}

	if (BlendState.BlendDuration <= 0.0f)
	{
		CombatCameraComponent::SnapBlendToTarget(BlendState);
		return;
	}

	BlendState.BlendElapsed = FMath::Min(BlendState.BlendElapsed + DeltaTime, BlendState.BlendDuration);
	const float Alpha = FMath::Clamp(BlendState.BlendElapsed / BlendState.BlendDuration, 0.0f, 1.0f);
	const float EaseAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);

	BlendState.CurrentArmLengthOffset = FMath::Lerp(BlendState.StartArmLengthOffset, BlendState.TargetArmLengthOffset, EaseAlpha);
	BlendState.CurrentSocketOffset = FMath::Lerp(BlendState.StartSocketOffset, BlendState.TargetSocketOffset, EaseAlpha);
	BlendState.CurrentTargetOffset = FMath::Lerp(BlendState.StartTargetOffset, BlendState.TargetTargetOffset, EaseAlpha);
	BlendState.CurrentBoomRotationOffset = FMath::Lerp(BlendState.StartBoomRotationOffset, BlendState.TargetBoomRotationOffset, EaseAlpha);
	BlendState.CurrentFOVOffset = FMath::Lerp(BlendState.StartFOVOffset, BlendState.TargetFOVOffset, EaseAlpha);

	if (Alpha >= 1.0f)
	{
		BlendState.bHasActiveBlend = false;
	}
}

void UMHWCombatCameraComponent::ApplyCurrentStateToComponents()
{
	if (USpringArmComponent* SpringArm = CachedSpringArm.Get())
	{
		SpringArm->TargetArmLength = DefaultArmLength + BlendState.CurrentArmLengthOffset;
		SpringArm->SocketOffset = DefaultSocketOffset + BlendState.CurrentSocketOffset;
		SpringArm->TargetOffset = DefaultTargetOffset + BlendState.CurrentTargetOffset;
		SpringArm->SetRelativeRotation(DefaultBoomRotation + BlendState.CurrentBoomRotationOffset);
	}

	if (UCameraComponent* Camera = CachedCamera.Get())
	{
		Camera->SetFieldOfView(DefaultCameraFOV + BlendState.CurrentFOVOffset);
	}
}
