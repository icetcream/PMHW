// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWAnimInstance.h"
#include "Engine/Engine.h" 
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "MHWGameplayTags.h"
#include "Character/MHWCharacter.h"
#include "Character/MHWMovementComponent.h"

UMHWAnimInstance::UMHWAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
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
	CurrentChargeYaw = 0.0f;
	CurrentCombatChargingType = ECombatChargingType::CombatNotCharging;
	bIsCharging = false;
	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UMHWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	CurrentChargeYaw = 0.0f;
	CurrentCombatChargingType = ECombatChargingType::CombatNotCharging;
	bIsCharging = false;
	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UMHWAnimInstance::NativeUninitializeAnimation()
{
	UnbindFromAbilitySystem();
	CurrentChargeYaw = 0.0f;
	CurrentCombatChargingType = ECombatChargingType::CombatNotCharging;
	bIsCharging = false;
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
}

void UMHWAnimInstance::OnChargeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	UpdateChargingTypeFromTags();
}



void UMHWAnimInstance::UpdateChargingTypeFromTags()
{
	const ECombatChargingType PreviousChargingType = CurrentCombatChargingType;

	// 1. 尝试获取缓存的 ASC
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
		
		// 打印灰色：未蓄力
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Silver, TEXT("当前状态：未蓄力 (没有父级 Tag)")); }
		return;
	}

	// 2. 如果在蓄力，具体是哪一种？(优先级从高到低判断)
	// (使用 HasMatchingGameplayTag 精确查询具体的子 Tag)
	
	if (ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging_XLZ3)) // 真蓄
	{
		CurrentCombatChargingType = ECombatChargingType::CombatChargingZLZ3;
		// 打印红色：真蓄力
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("当前状态：真蓄力斩 (XLZ3)")); }
	}
	else if (ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging_XLZ2)) // 强蓄
	{
		CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ2;
		// 打印橙色：强蓄力
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange, TEXT("当前状态：强蓄力斩 (XLZ2)")); }
	}
	else if (ASC->HasMatchingGameplayTag(MHWStateTags::Combat_Charging_XLZ)) // 普蓄
	{
		CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ1;
		// 打印黄色：普蓄力
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT("当前状态：普通蓄力斩 (XLZ1)")); }
	}
	else
	{
		// 阶段切换时可能会出现一帧父 Tag 还在、具体阶段 Tag 还没补上的情况。
		// 这时保持上一次有效阶段，避免 3 段瞬间闪回 1 段。
		if (PreviousChargingType != ECombatChargingType::CombatNotCharging)
		{
			CurrentCombatChargingType = PreviousChargingType;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, TEXT("Charging 阶段 Tag 短暂缺失，保持上一阶段"));
			}
		}
		else
		{
			CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ1;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Magenta, TEXT("Charging 父级 Tag 已存在但没有具体阶段，默认使用 1段"));
			}
		}
	}
}

void UMHWAnimInstance::UnbindFromAbilitySystem()
{
	if (UAbilitySystemComponent* ASC = CachedASC.Get())
	{
		if (ChargeParentTagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging, EGameplayTagEventType::AnyCountChange)
				.Remove(ChargeParentTagDelegateHandle);
		}

		if (ChargeLevel1TagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging_XLZ, EGameplayTagEventType::AnyCountChange)
				.Remove(ChargeLevel1TagDelegateHandle);
		}

		if (ChargeLevel2TagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging_XLZ2, EGameplayTagEventType::AnyCountChange)
				.Remove(ChargeLevel2TagDelegateHandle);
		}

		if (ChargeLevel3TagDelegateHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging_XLZ3, EGameplayTagEventType::AnyCountChange)
				.Remove(ChargeLevel3TagDelegateHandle);
		}
	}

	ChargeParentTagDelegateHandle.Reset();
	ChargeLevel1TagDelegateHandle.Reset();
	ChargeLevel2TagDelegateHandle.Reset();
	ChargeLevel3TagDelegateHandle.Reset();
	CachedASC.Reset();
}
