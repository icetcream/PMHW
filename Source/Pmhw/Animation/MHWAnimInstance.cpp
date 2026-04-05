// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWAnimInstance.h"
#include "Engine/Engine.h" 
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "MHWGameplayTags.h"
#include "Character/MHWCharacter.h"
#include "Character/MHWMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

namespace MHWAnimInstanceInternal
{
	static void DebugChargingState(const FColor& Color, const TCHAR* Message, const float Duration = 0.0f)
	{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
		}
#endif
	}

	static void RemoveGameplayTagEventDelegate(
		UAbilitySystemComponent* ASC,
		const FGameplayTag& Tag,
		FDelegateHandle& DelegateHandle)
	{
		if (!ASC || !DelegateHandle.IsValid())
		{
			return;
		}

		ASC->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::AnyCountChange).Remove(DelegateHandle);
	}
}

UMHWAnimInstance::UMHWAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FMHWControlRigInput UMHWAnimInstance::GetControlRigInput() const
{
	FMHWControlRigInput Input;
	Input.bFootTargetsValid = bFootTargetsValid;
	Input.PelvisOffsetAmount = MaxFootLockAlpha;

	const FVector LeftFinalLocation = FMath::Lerp(LeftFootLock.CurrentLocationCS, LeftFootLock.LockedLocationCS, LeftFootLock.LockAlpha);
	const FVector RightFinalLocation = FMath::Lerp(RightFootLock.CurrentLocationCS, RightFootLock.LockedLocationCS, RightFootLock.LockAlpha);

	const FQuat LeftFinalRotation = FQuat::Slerp(
		LeftFootLock.CurrentRotationCS.Quaternion(),
		LeftFootLock.LockedRotationCS.Quaternion(),
		LeftFootLock.LockAlpha);
	const FQuat RightFinalRotation = FQuat::Slerp(
		RightFootLock.CurrentRotationCS.Quaternion(),
		RightFootLock.LockedRotationCS.Quaternion(),
		RightFootLock.LockAlpha);

	Input.FootLeftLocation = LeftFinalLocation;
	Input.FootLeftRotation = LeftFinalRotation.Rotator();
	Input.FootRightLocation = RightFinalLocation;
	Input.FootRightRotation = RightFinalRotation.Rotator();

	return Input;
}

void UMHWAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC) return;

	if (CachedASC.Get() == ASC && ChargeParentTagDelegateHandle.IsValid())
	{
		UpdateChargingTypeFromTags();
		return;
	}

	UnbindFromAbilitySystem();
	CachedASC = ASC;

	// 监听父级与具体蓄力阶段 Tag。
	// 仅监听父级会在阶段切换时漏掉更新，导致动画出现 3 段瞬间跳回 1 段。
	ChargeParentTagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &UMHWAnimInstance::OnChargeTagChanged);
	ChargeLevel1TagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging_XLZ, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &UMHWAnimInstance::OnChargeTagChanged);
	ChargeLevel2TagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging_XLZ2, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &UMHWAnimInstance::OnChargeTagChanged);
	ChargeLevel3TagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging_XLZ3, EGameplayTagEventType::AnyCountChange)
		.AddUObject(this, &UMHWAnimInstance::OnChargeTagChanged);

	// 2. 初始化时，先手动拉取一次状态 (防止动画实例创建晚于 Tag 的添加)
	UpdateChargingTypeFromTags();
}

void UMHWAnimInstance::SetChargeTurnYaw_Implementation(float TurnYaw)
{
	CurrentChargeYaw = TurnYaw;
}

void UMHWAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	ResetAnimationRuntimeState();
	TryInitializeAbilitySystemFromOwner();
}

void UMHWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ResetAnimationRuntimeState();
	TryInitializeAbilitySystemFromOwner();
}

void UMHWAnimInstance::NativeUninitializeAnimation()
{
	UnbindFromAbilitySystem();
	ResetAnimationRuntimeState();
	Super::NativeUninitializeAnimation();
}

void UMHWAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AMHWCharacter* Character = Cast<AMHWCharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}

	UMHWMovementComponent* CharMoveComp = CastChecked<UMHWMovementComponent>(Character->GetCharacterMovement());
	const FMHWCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
	UpdateFootLockRuntimeData();
}

void UMHWAnimInstance::OnChargeTagChanged(const FGameplayTag /*CallbackTag*/, int32 /*NewCount*/)
{
	UpdateChargingTypeFromTags();
}

void UMHWAnimInstance::ResetAnimationRuntimeState()
{
	CurrentChargeYaw = 0.0f;
	CurrentCombatChargingType = ECombatChargingType::CombatNotCharging;
	bIsCharging = false;
	ResetFootLockRuntimeData();
}

void UMHWAnimInstance::TryInitializeAbilitySystemFromOwner()
{
	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UMHWAnimInstance::UpdateChargingTypeFromTags()
{
	const ECombatChargingType PreviousChargingType = CurrentCombatChargingType;

	UAbilitySystemComponent* ASC = CachedASC.Get();
	if (!ASC)
	{
		bIsCharging = false;
		CurrentCombatChargingType = ECombatChargingType::CombatNotCharging;
		CurrentChargeYaw = 0.0f;
		return;
	}
	// 1. 先判断大类：角色现在身上有没有任何 "State.Combat.Charging" 相关的 Tag？
	bIsCharging = ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging);

	if (!bIsCharging)
	{
		// 如果没在蓄力，直接重置并返回 (效率最高)
		CurrentCombatChargingType = ECombatChargingType::CombatNotCharging;
		CurrentChargeYaw = 0.0f; // 可选：结束时把偏航角归零，防止下次带入旧数据
		MHWAnimInstanceInternal::DebugChargingState(FColor::Silver, TEXT("当前状态：未蓄力 (没有父级 Tag)"));
		return;
	}

	if (ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging_XLZ3)) // 真蓄
	{
		CurrentCombatChargingType = ECombatChargingType::CombatChargingZLZ3;
		MHWAnimInstanceInternal::DebugChargingState(FColor::Red, TEXT("当前状态：真蓄力斩 (XLZ3)"));
	}
	else if (ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging_XLZ2)) // 强蓄
	{
		CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ2;
		MHWAnimInstanceInternal::DebugChargingState(FColor::Orange, TEXT("当前状态：强蓄力斩 (XLZ2)"));
	}
	else if (ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging_XLZ)) // 普蓄
	{
		CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ1;
		MHWAnimInstanceInternal::DebugChargingState(FColor::Yellow, TEXT("当前状态：普通蓄力斩 (XLZ1)"));
	}
	else
	{
		// Stage tags can momentarily disappear during transitions; keep the last valid stage
		// instead of flashing back to level 1 for a frame.
		if (PreviousChargingType != ECombatChargingType::CombatNotCharging)
		{
			CurrentCombatChargingType = PreviousChargingType;
			MHWAnimInstanceInternal::DebugChargingState(FColor::Magenta, TEXT("Charging 阶段 Tag 短暂缺失，保持上一阶段"), 0.5f);
		}
		else
		{
			CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ1;
			MHWAnimInstanceInternal::DebugChargingState(FColor::Magenta, TEXT("Charging 父级 Tag 已存在但没有具体阶段，默认使用 1段"), 0.5f);
		}
	}
}

void UMHWAnimInstance::UnbindFromAbilitySystem()
{
	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		MHWAnimInstanceInternal::RemoveGameplayTagEventDelegate(ASC, MHWStateTags::Combat_Charging, ChargeParentTagDelegateHandle);
		MHWAnimInstanceInternal::RemoveGameplayTagEventDelegate(ASC, MHWStateTags::Combat_Charging_XLZ, ChargeLevel1TagDelegateHandle);
		MHWAnimInstanceInternal::RemoveGameplayTagEventDelegate(ASC, MHWStateTags::Combat_Charging_XLZ2, ChargeLevel2TagDelegateHandle);
		MHWAnimInstanceInternal::RemoveGameplayTagEventDelegate(ASC, MHWStateTags::Combat_Charging_XLZ3, ChargeLevel3TagDelegateHandle);
	}

	ChargeParentTagDelegateHandle.Reset();
	ChargeLevel1TagDelegateHandle.Reset();
	ChargeLevel2TagDelegateHandle.Reset();
	ChargeLevel3TagDelegateHandle.Reset();
	CachedASC.Reset();
}

void UMHWAnimInstance::ResetFootLockRuntimeData()
{
	LeftFootLock = FMHWFootLockRuntimeData();
	RightFootLock = FMHWFootLockRuntimeData();
	MaxFootLockAlpha = 0.0f;
	bFootTargetsValid = false;
}

void UMHWAnimInstance::UpdateFootLockRuntimeData()
{
	USkeletalMeshComponent* MeshComponent = GetSkelMeshComponent();
	if (!MeshComponent)
	{
		ResetFootLockRuntimeData();
		return;
	}

	UpdateSingleFootLockRuntimeData(MeshComponent, LeftFootCurveName, LeftFootBoneName, LeftFootLock);
	UpdateSingleFootLockRuntimeData(MeshComponent, RightFootCurveName, RightFootBoneName, RightFootLock);

	MaxFootLockAlpha = FMath::Max(LeftFootLock.LockAlpha, RightFootLock.LockAlpha);
	bFootTargetsValid = true;
}

void UMHWAnimInstance::UpdateSingleFootLockRuntimeData(
	USkeletalMeshComponent* MeshComponent,
	const FName CurveName,
	const FName BoneName,
	FMHWFootLockRuntimeData& FootLockData) const
{
	if (!MeshComponent || BoneName.IsNone())
	{
		FootLockData = FMHWFootLockRuntimeData();
		return;
	}

	FootLockData.LockAlpha = FMath::Clamp(GetCurveValue(CurveName), 0.0f, 1.0f);
	const bool bShouldLock = FootLockData.LockAlpha > FootLockThreshold;

	const FTransform CurrentFootWorldTransform = MeshComponent->GetSocketTransform(BoneName, RTS_World);
	FootLockData.CurrentLocationWS = CurrentFootWorldTransform.GetLocation();
	FootLockData.CurrentRotationWS = CurrentFootWorldTransform.Rotator();

	const FTransform MeshWorldTransform = MeshComponent->GetComponentTransform();
	FootLockData.CurrentLocationCS = MeshWorldTransform.InverseTransformPosition(FootLockData.CurrentLocationWS);
	FootLockData.CurrentRotationCS = MeshWorldTransform.InverseTransformRotation(FootLockData.CurrentRotationWS.Quaternion()).Rotator();

	if (bShouldLock)
	{
		if (!FootLockData.bLocked)
		{
			FootLockData.bLocked = true;
			FootLockData.LockedLocationWS = CurrentFootWorldTransform.GetLocation();
			FootLockData.LockedRotationWS = CurrentFootWorldTransform.Rotator();
		}
	}
	else
	{
		FootLockData.bLocked = false;
		FootLockData.LockedLocationWS = CurrentFootWorldTransform.GetLocation();
		FootLockData.LockedRotationWS = CurrentFootWorldTransform.Rotator();
	}

	FootLockData.LockedLocationCS = MeshWorldTransform.InverseTransformPosition(FootLockData.LockedLocationWS);
	FootLockData.LockedRotationCS = MeshWorldTransform.InverseTransformRotation(FootLockData.LockedRotationWS.Quaternion()).Rotator();
}
