#include "UI/Widget/MHWOverlayUserWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Controller/MHWOverlayWidgetController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWOverlayUserWidget)

UMHWOverlayUserWidget::FOverlayBarAnimationSettings UMHWOverlayUserWidget::MakeHealthBarAnimationSettings() const
{
	FOverlayBarAnimationSettings Settings;
	Settings.bAnimateDecrease = bAnimateHealthBarOnDamage;
	Settings.DecreaseDelay = HealthBarDamageDelay;
	Settings.DecreaseInterpSpeed = HealthBarDamageInterpSpeed;
	Settings.bSnapOnIncrease = bSnapHealthBarOnHeal;
	Settings.IncreaseInterpSpeed = HealthBarHealInterpSpeed;
	return Settings;
}

UMHWOverlayUserWidget::FOverlayBarAnimationSettings UMHWOverlayUserWidget::MakeStaminaBarAnimationSettings() const
{
	FOverlayBarAnimationSettings Settings;
	Settings.bAnimateDecrease = bAnimateStaminaBar;
	Settings.DecreaseDelay = StaminaBarConsumeDelay;
	Settings.DecreaseInterpSpeed = StaminaBarConsumeInterpSpeed;
	Settings.bSnapOnIncrease = bSnapStaminaBarOnRestore;
	Settings.IncreaseInterpSpeed = StaminaBarRestoreInterpSpeed;
	return Settings;
}

void UMHWOverlayUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateAnimatedBar(InDeltaTime, MakeHealthBarAnimationSettings(), HealthBarState, HealthBarImage.Get(), HealthBarMaterialInstance);
	UpdateAnimatedBar(InDeltaTime, MakeStaminaBarAnimationSettings(), StaminaBarState, StaminaBarImage.Get(), StaminaBarMaterialInstance);
}

void UMHWOverlayUserWidget::NativeDestruct()
{
	UnbindFromController();
	Super::NativeDestruct();
}

void UMHWOverlayUserWidget::OnWidgetControllerSet()
{
	InitializeDynamicMaterials();

	UMHWOverlayWidgetController* OverlayWidgetController = Cast<UMHWOverlayWidgetController>(WidgetController);
	if (BoundOverlayWidgetController == OverlayWidgetController)
	{
		return;
	}

	UnbindFromController();
	BoundOverlayWidgetController = OverlayWidgetController;
	if (!BoundOverlayWidgetController)
	{
		return;
	}

	BoundOverlayWidgetController->OnHealthPercentChanged.AddDynamic(this, &ThisClass::HandleHealthPercentChanged);
	BoundOverlayWidgetController->OnStaminaPercentChanged.AddDynamic(this, &ThisClass::HandleStaminaPercentChanged);
}

void UMHWOverlayUserWidget::InitializeDynamicMaterials()
{
	if (!HealthBarMaterialInstance && HealthBarImage)
	{
		HealthBarMaterialInstance = HealthBarImage->GetDynamicMaterial();
	}

	if (!StaminaBarMaterialInstance && StaminaBarImage)
	{
		StaminaBarMaterialInstance = StaminaBarImage->GetDynamicMaterial();
	}
}

void UMHWOverlayUserWidget::UnbindFromController()
{
	if (!BoundOverlayWidgetController)
	{
		return;
	}

	BoundOverlayWidgetController->OnHealthPercentChanged.RemoveDynamic(this, &ThisClass::HandleHealthPercentChanged);
	BoundOverlayWidgetController->OnStaminaPercentChanged.RemoveDynamic(this, &ThisClass::HandleStaminaPercentChanged);
	BoundOverlayWidgetController = nullptr;
}

void UMHWOverlayUserWidget::ApplyBarPercent(
	UImage* BarImage,
	TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance,
	const float NewPercent)
{
	if (!MaterialInstance && BarImage)
	{
		MaterialInstance = BarImage->GetDynamicMaterial();
	}

	if (MaterialInstance)
	{
		MaterialInstance->SetScalarParameterValue(ProgressParameterName, FMath::Clamp(NewPercent, 0.0f, 1.0f));
	}
}

void UMHWOverlayUserWidget::UpdateAnimatedBar(
	const float DeltaTime,
	const FOverlayBarAnimationSettings& Settings,
	FOverlayBarAnimationState& State,
	UImage* BarImage,
	TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance)
{
	if (!State.bInitialized)
	{
		return;
	}

	if (FMath::IsNearlyEqual(State.CurrentDisplayedPercent, State.TargetPercent, KINDA_SMALL_NUMBER))
	{
		State.CurrentDisplayedPercent = State.TargetPercent;
		ApplyBarPercent(BarImage, MaterialInstance, State.CurrentDisplayedPercent);
		return;
	}

	if (State.CurrentDisplayedPercent > State.TargetPercent)
	{
		if (!Settings.bAnimateDecrease)
		{
			State.CurrentDisplayedPercent = State.TargetPercent;
			ApplyBarPercent(BarImage, MaterialInstance, State.CurrentDisplayedPercent);
			return;
		}

		if (State.RemainingDelay > 0.0f)
		{
			State.RemainingDelay = FMath::Max(0.0f, State.RemainingDelay - DeltaTime);
			return;
		}

		// Resource loss is delayed slightly to preserve the MHW-style "lagging" bar feel.
		State.CurrentDisplayedPercent = FMath::FInterpConstantTo(
			State.CurrentDisplayedPercent,
			State.TargetPercent,
			DeltaTime,
			Settings.DecreaseInterpSpeed);
	}
	else
	{
		State.RemainingDelay = 0.0f;

		// Resource recovery can either snap or ease depending on the per-bar tuning.
		if (Settings.bSnapOnIncrease)
		{
			State.CurrentDisplayedPercent = State.TargetPercent;
		}
		else
		{
			State.CurrentDisplayedPercent = FMath::FInterpConstantTo(
				State.CurrentDisplayedPercent,
				State.TargetPercent,
				DeltaTime,
				Settings.IncreaseInterpSpeed);
		}
	}

	ApplyBarPercent(BarImage, MaterialInstance, State.CurrentDisplayedPercent);
}

void UMHWOverlayUserWidget::HandleBarPercentChanged(
	const float NewPercent,
	const FOverlayBarAnimationSettings& Settings,
	FOverlayBarAnimationState& State,
	UImage* BarImage,
	TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance)
{
	const float ClampedPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);

	if (!State.bInitialized)
	{
		// Seed the runtime state from the first controller broadcast so the bar does not animate from a fake default.
		State.bInitialized = true;
		State.CurrentDisplayedPercent = ClampedPercent;
		State.TargetPercent = ClampedPercent;
		State.RemainingDelay = 0.0f;
		ApplyBarPercent(BarImage, MaterialInstance, ClampedPercent);
		return;
	}

	State.TargetPercent = ClampedPercent;

	if (State.TargetPercent < State.CurrentDisplayedPercent)
	{
		State.RemainingDelay = Settings.DecreaseDelay;
		if (!Settings.bAnimateDecrease)
		{
			State.CurrentDisplayedPercent = State.TargetPercent;
			ApplyBarPercent(BarImage, MaterialInstance, State.CurrentDisplayedPercent);
		}
		return;
	}

	State.RemainingDelay = 0.0f;

	if (Settings.bSnapOnIncrease)
	{
		State.CurrentDisplayedPercent = State.TargetPercent;
	}

	ApplyBarPercent(BarImage, MaterialInstance, State.CurrentDisplayedPercent);
}

void UMHWOverlayUserWidget::HandleHealthPercentChanged(const float NewPercent)
{
	HandleBarPercentChanged(NewPercent, MakeHealthBarAnimationSettings(), HealthBarState, HealthBarImage.Get(), HealthBarMaterialInstance);
}

void UMHWOverlayUserWidget::HandleStaminaPercentChanged(const float NewPercent)
{
	HandleBarPercentChanged(NewPercent, MakeStaminaBarAnimationSettings(), StaminaBarState, StaminaBarImage.Get(), StaminaBarMaterialInstance);
}
