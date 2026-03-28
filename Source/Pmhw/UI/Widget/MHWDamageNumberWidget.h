#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "UI/Widget/MHWUserWidgetBase.h"
#include "MHWDamageNumberWidget.generated.h"

class UTextBlock;

UCLASS()
class PMHW_API UMHWDamageNumberWidget : public UMHWUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MHW|UI|Damage Number")
	void SetDamageNumber(float InDamageAmount, EMHWCriticalHitType InCriticalHitType);

	UFUNCTION(BlueprintPure, Category = "MHW|UI|Damage Number")
	float GetDamageAmount() const { return DamageAmount; }

	UFUNCTION(BlueprintPure, Category = "MHW|UI|Damage Number")
	EMHWCriticalHitType GetCriticalHitType() const { return CriticalHitType; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI|Damage Number", meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DamageText = nullptr;

	UFUNCTION(BlueprintImplementableEvent, Category = "MHW|UI|Damage Number")
	void OnDamageNumberSet(float InDamageAmount, EMHWCriticalHitType InCriticalHitType);

private:
	UPROPERTY(Transient)
	float DamageAmount = 0.0f;

	UPROPERTY(Transient)
	EMHWCriticalHitType CriticalHitType = EMHWCriticalHitType::None;
};
