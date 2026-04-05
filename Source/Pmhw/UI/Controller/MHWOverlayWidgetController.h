#pragma once

#include "CoreMinimal.h"
#include "UI/Controller/MHWWidgetController.h"
#include "MHWOverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMHWOnFloatValueChangedSignature, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMHWOnDamageReceivedSignature, float, DamageAmount, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMHWOnOverlayDeathSignature);

UCLASS(BlueprintType, Blueprintable)
class PMHW_API UMHWOverlayWidgetController : public UMHWWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnFloatValueChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnFloatValueChangedSignature OnMaxHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnFloatValueChangedSignature OnHealthPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnFloatValueChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnFloatValueChangedSignature OnMaxStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnFloatValueChangedSignature OnStaminaPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnDamageReceivedSignature OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|UI|Overlay")
	FMHWOnOverlayDeathSignature OnDeath;

private:
	void UnbindFromCombatComponent();
	void BroadcastResourceChanged(
		float NewValue,
		float MaxValue,
		FMHWOnFloatValueChangedSignature& OnValueChanged,
		FMHWOnFloatValueChangedSignature& OnMaxValueChanged,
		FMHWOnFloatValueChangedSignature& OnPercentChanged) const;

	UFUNCTION()
	void HandleHealthChanged(float NewValue, float MaxValue, float DeltaValue);

	UFUNCTION()
	void HandleStaminaChanged(float NewValue, float MaxValue, float DeltaValue);

	UFUNCTION()
	void HandleDamaged(float DamageAmount, float NewHealth);

	UFUNCTION()
	void HandleDeath();

	UPROPERTY(Transient)
	TObjectPtr<UMHWCombatComponent> BoundCombatComponent;
};
