// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Interface/CombatAnimInterface.h"
#include "MHWAnimInstance.generated.h"

struct FGameplayTag;
class UAbilitySystemComponent;
/**
 * 
 */

UENUM(BlueprintType)
enum class ECombatChargingType: uint8
{
	CombatChargingXLZ1 UMETA(DisplayName = "蓄力斩 1段 (XLZ1)"),
	CombatChargingXLZ2 UMETA(DisplayName = "强蓄力斩 2段 (XLZ2)"),
	CombatChargingZLZ3 UMETA(DisplayName = "真蓄力斩 3段 (XLZ3)"), // 注意你原来代码里写的是 ZLZ3，如果打错字了可以改一下
	CombatNotCharging  UMETA(DisplayName = "未蓄力")
};
UCLASS()
class PMHW_API UMHWAnimInstance : public UAnimInstance, public ICombatAnimInterface
{
	GENERATED_BODY()

public:

	UMHWAnimInstance(const FObjectInitializer& ObjectInitializer);
	
	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);
	virtual void SetChargeTurnYaw_Implementation(float TurnYaw) override;

protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUninitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	
	UPROPERTY(BlueprintReadOnly, Category = "Character State Data")
	float GroundDistance = -1.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
	float CurrentChargeYaw = 0.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
	ECombatChargingType CurrentCombatChargingType;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
	bool bIsCharging = false;
	
private:

	// ====== 内部处理 Tag 监听 ======

	// 缓存的 ASC 指针
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// Tag 变动时的回调函数
	void OnChargeTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// 保存委托绑定的句柄，以便清理
	FDelegateHandle ChargeTagDelegateHandle;

	// 一个辅助函数，用于在 Tag 变化时重新计算当前处于哪种枚举状态
	void UpdateChargingTypeFromTags();
};
