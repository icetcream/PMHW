#include "UI/Widget/MHWOverlayUserWidget.h"

#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/Controller/MHWOverlayWidgetController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWOverlayUserWidget)

void UMHWOverlayUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateAnimatedHealthBar(InDeltaTime);
	UpdateAnimatedStaminaBar(InDeltaTime);
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

void UMHWOverlayUserWidget::ApplyHealthBarPercent(float NewPercent)
{
	InitializeDynamicMaterials();
	if (HealthBarMaterialInstance)
	{
		HealthBarMaterialInstance->SetScalarParameterValue(ProgressParameterName, FMath::Clamp(NewPercent, 0.0f, 1.0f));
	}
}

void UMHWOverlayUserWidget::ApplyStaminaBarPercent(float NewPercent)
{
	InitializeDynamicMaterials();
	if (StaminaBarMaterialInstance)
	{
		StaminaBarMaterialInstance->SetScalarParameterValue(ProgressParameterName, FMath::Clamp(NewPercent, 0.0f, 1.0f));
	}
}

void UMHWOverlayUserWidget::UpdateAnimatedHealthBar(float DeltaTime)
{
	if (!bHasInitializedHealthBar)
	{
		return;
	}

	if (FMath::IsNearlyEqual(CurrentDisplayedHealthPercent, TargetHealthPercent, KINDA_SMALL_NUMBER))
	{
		CurrentDisplayedHealthPercent = TargetHealthPercent;
		return;
	}

	if (CurrentDisplayedHealthPercent > TargetHealthPercent)
	{
		if (!bAnimateHealthBarOnDamage)
		{
			CurrentDisplayedHealthPercent = TargetHealthPercent;
			ApplyHealthBarPercent(CurrentDisplayedHealthPercent);
			return;
		}

		if (RemainingHealthDamageDelay > 0.0f)
		{
			RemainingHealthDamageDelay = FMath::Max(0.0f, RemainingHealthDamageDelay - DeltaTime);
			return;
		}

		CurrentDisplayedHealthPercent = FMath::FInterpConstantTo(
			CurrentDisplayedHealthPercent,
			TargetHealthPercent,
			DeltaTime,
			HealthBarDamageInterpSpeed);
	}
	else
	{
		RemainingHealthDamageDelay = 0.0f;

		if (bSnapHealthBarOnHeal)
		{
			CurrentDisplayedHealthPercent = TargetHealthPercent;
		}
		else
		{
			CurrentDisplayedHealthPercent = FMath::FInterpConstantTo(
			CurrentDisplayedHealthPercent,
			TargetHealthPercent,
			DeltaTime,
			HealthBarHealInterpSpeed);
		}
	}

	ApplyHealthBarPercent(CurrentDisplayedHealthPercent);
}

void UMHWOverlayUserWidget::UpdateAnimatedStaminaBar(float DeltaTime)
{
	if (!bHasInitializedStaminaBar)
	{
		return;
	}

	if (FMath::IsNearlyEqual(CurrentDisplayedStaminaPercent, TargetStaminaPercent, KINDA_SMALL_NUMBER))
	{
		CurrentDisplayedStaminaPercent = TargetStaminaPercent;
		return;
	}

	if (CurrentDisplayedStaminaPercent > TargetStaminaPercent)
	{
		if (!bAnimateStaminaBar)
		{
			CurrentDisplayedStaminaPercent = TargetStaminaPercent;
			ApplyStaminaBarPercent(CurrentDisplayedStaminaPercent);
			return;
		}

		if (RemainingStaminaConsumeDelay > 0.0f)
		{
			RemainingStaminaConsumeDelay = FMath::Max(0.0f, RemainingStaminaConsumeDelay - DeltaTime);
			return;
		}

		CurrentDisplayedStaminaPercent = FMath::FInterpConstantTo(
			CurrentDisplayedStaminaPercent,
			TargetStaminaPercent,
			DeltaTime,
			StaminaBarConsumeInterpSpeed);
	}
	else
	{
		RemainingStaminaConsumeDelay = 0.0f;

		if (bSnapStaminaBarOnRestore)
		{
			CurrentDisplayedStaminaPercent = TargetStaminaPercent;
		}
		else
		{
			CurrentDisplayedStaminaPercent = FMath::FInterpConstantTo(
				CurrentDisplayedStaminaPercent,
				TargetStaminaPercent,
				DeltaTime,
				StaminaBarRestoreInterpSpeed);
		}
	}

	ApplyStaminaBarPercent(CurrentDisplayedStaminaPercent);
}

void UMHWOverlayUserWidget::HandleHealthPercentChanged(float NewPercent)
{
	const float ClampedPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);
	InitializeDynamicMaterials();

	if (!bHasInitializedHealthBar)
	{
		bHasInitializedHealthBar = true;
		CurrentDisplayedHealthPercent = ClampedPercent;
		TargetHealthPercent = ClampedPercent;
		RemainingHealthDamageDelay = 0.0f;
		ApplyHealthBarPercent(ClampedPercent);
		return;
	}

	TargetHealthPercent = ClampedPercent;

	if (TargetHealthPercent < CurrentDisplayedHealthPercent)
	{
		RemainingHealthDamageDelay = HealthBarDamageDelay;
		if (!bAnimateHealthBarOnDamage)
		{
			CurrentDisplayedHealthPercent = TargetHealthPercent;
			ApplyHealthBarPercent(CurrentDisplayedHealthPercent);
		}
		return;
	}

	if (bSnapHealthBarOnHeal)
	{
		CurrentDisplayedHealthPercent = TargetHealthPercent;
	}

	ApplyHealthBarPercent(CurrentDisplayedHealthPercent);
}

void UMHWOverlayUserWidget::HandleStaminaPercentChanged(float NewPercent)
{
	const float ClampedPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);

	if (!bHasInitializedStaminaBar)
	{
		bHasInitializedStaminaBar = true;
		CurrentDisplayedStaminaPercent = ClampedPercent;
		TargetStaminaPercent = ClampedPercent;
		RemainingStaminaConsumeDelay = 0.0f;
		ApplyStaminaBarPercent(ClampedPercent);
		return;
	}

	TargetStaminaPercent = ClampedPercent;

	if (TargetStaminaPercent < CurrentDisplayedStaminaPercent)
	{
		RemainingStaminaConsumeDelay = StaminaBarConsumeDelay;
		if (!bAnimateStaminaBar)
		{
			CurrentDisplayedStaminaPercent = TargetStaminaPercent;
			ApplyStaminaBarPercent(CurrentDisplayedStaminaPercent);
		}
		return;
	}

	if (bSnapStaminaBarOnRestore)
	{
		CurrentDisplayedStaminaPercent = TargetStaminaPercent;
	}

	ApplyStaminaBarPercent(CurrentDisplayedStaminaPercent);
}
