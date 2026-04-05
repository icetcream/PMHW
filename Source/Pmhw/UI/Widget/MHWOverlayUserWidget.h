#pragma once

#include "CoreMinimal.h"
#include "UI/Widget/MHWUserWidgetBase.h"
#include "MHWOverlayUserWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;
class UMHWOverlayWidgetController;

UCLASS()
class PMHW_API UMHWOverlayUserWidget : public UMHWUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

protected:
	virtual void OnWidgetControllerSet() override;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UImage> HealthBarImage;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UImage> StaminaBarImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI")
	FName ProgressParameterName = TEXT("Progress");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Health")
	bool bAnimateHealthBarOnDamage = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Health", meta = (EditCondition = "bAnimateHealthBarOnDamage", ClampMin = "0.0"))
	float HealthBarDamageDelay = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Health", meta = (EditCondition = "bAnimateHealthBarOnDamage", ClampMin = "0.01"))
	float HealthBarDamageInterpSpeed = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Health")
	bool bSnapHealthBarOnHeal = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Health", meta = (EditCondition = "!bSnapHealthBarOnHeal", ClampMin = "0.01"))
	float HealthBarHealInterpSpeed = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Stamina")
	bool bAnimateStaminaBar = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Stamina", meta = (EditCondition = "bAnimateStaminaBar", ClampMin = "0.0"))
	float StaminaBarConsumeDelay = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Stamina", meta = (EditCondition = "bAnimateStaminaBar", ClampMin = "0.01"))
	float StaminaBarConsumeInterpSpeed = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Stamina")
	bool bSnapStaminaBarOnRestore = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI|Stamina", meta = (EditCondition = "!bSnapStaminaBarOnRestore", ClampMin = "0.01"))
	float StaminaBarRestoreInterpSpeed = 6.0f;

private:
	struct FOverlayBarAnimationSettings
	{
		bool bAnimateDecrease = true;
		float DecreaseDelay = 0.0f;
		float DecreaseInterpSpeed = 1.0f;
		bool bSnapOnIncrease = true;
		float IncreaseInterpSpeed = 1.0f;
	};

	struct FOverlayBarAnimationState
	{
		float CurrentDisplayedPercent = 1.0f;
		float TargetPercent = 1.0f;
		float RemainingDelay = 0.0f;
		bool bInitialized = false;
	};

	FOverlayBarAnimationSettings MakeHealthBarAnimationSettings() const;
	FOverlayBarAnimationSettings MakeStaminaBarAnimationSettings() const;
	void InitializeDynamicMaterials();
	void UnbindFromController();
	void ApplyBarPercent(UImage* BarImage, TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance, float NewPercent);
	void UpdateAnimatedBar(
		float DeltaTime,
		const FOverlayBarAnimationSettings& Settings,
		FOverlayBarAnimationState& State,
		UImage* BarImage,
		TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance);
	void HandleBarPercentChanged(
		float NewPercent,
		const FOverlayBarAnimationSettings& Settings,
		FOverlayBarAnimationState& State,
		UImage* BarImage,
		TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance);

	UFUNCTION()
	void HandleHealthPercentChanged(float NewPercent);

	UFUNCTION()
	void HandleStaminaPercentChanged(float NewPercent);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HealthBarMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StaminaBarMaterialInstance;

	UPROPERTY(Transient)
	TObjectPtr<UMHWOverlayWidgetController> BoundOverlayWidgetController;

	FOverlayBarAnimationState HealthBarState;
	FOverlayBarAnimationState StaminaBarState;
};
