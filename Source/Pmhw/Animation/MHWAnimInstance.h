// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Interface/CombatAnimInterface.h"
#include "MHWAnimInstance.generated.h"

struct FGameplayTag;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FMHWFootLockRuntimeData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FVector CurrentLocationWS = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FRotator CurrentRotationWS = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FVector CurrentLocationCS = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FRotator CurrentRotationCS = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	float LockAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	bool bLocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FVector LockedLocationWS = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FRotator LockedRotationWS = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FVector LockedLocationCS = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FRotator LockedRotationCS = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FMHWControlRigInput
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Rig")
	bool bFootTargetsValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Rig", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PelvisOffsetAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Rig")
	FVector FootLeftLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Rig")
	FRotator FootLeftRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Rig")
	FVector FootRightLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Control Rig")
	FRotator FootRightRotation = FRotator::ZeroRotator;
};
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

	UFUNCTION(BlueprintPure, Category = "Control Rig", meta = (BlueprintThreadSafe))
	FMHWControlRigInput GetControlRigInput() const;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Lock")
	FName LeftFootCurveName = TEXT("FootLock_L");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Lock")
	FName RightFootCurveName = TEXT("FootLock_R");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Lock")
	FName LeftFootBoneName = TEXT("foot_l");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Lock")
	FName RightFootBoneName = TEXT("foot_r");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Foot Lock", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FootLockThreshold = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FMHWFootLockRuntimeData LeftFootLock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	FMHWFootLockRuntimeData RightFootLock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	float MaxFootLockAlpha = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot Lock")
	bool bFootTargetsValid = false;
	
private:

	// ====== 内部处理 Tag 监听 ======

	// 缓存的 ASC 指针
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// Tag 变动时的回调函数
	void OnChargeTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	// 保存委托绑定的句柄，以便清理
	FDelegateHandle ChargeParentTagDelegateHandle;
	FDelegateHandle ChargeLevel1TagDelegateHandle;
	FDelegateHandle ChargeLevel2TagDelegateHandle;
	FDelegateHandle ChargeLevel3TagDelegateHandle;

	// 一个辅助函数，用于在 Tag 变化时重新计算当前处于哪种枚举状态
	void UpdateChargingTypeFromTags();
	void ResetAnimationRuntimeState();
	void TryInitializeAbilitySystemFromOwner();

	void ResetFootLockRuntimeData();
	void UpdateFootLockRuntimeData();
	void UpdateSingleFootLockRuntimeData(
		USkeletalMeshComponent* MeshComponent,
		const FName CurveName,
		const FName BoneName,
		FMHWFootLockRuntimeData& FootLockData) const;

	// 清理 ASC 标签监听，避免重复绑定和悬挂委托
	void UnbindFromAbilitySystem();
};
