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

	CachedASC = ASC;

	// 1. 监听父级 Tag "State.Combat.Charging"
	// 因为子 Tag (如 XLZ) 添加时，父级 Tag 的计数也会变化，所以我们监听这一个就够了！
	ChargeTagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Combat_Charging, EGameplayTagEventType::NewOrRemoved)
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

	// 1. 尝试获取缓存的 ASC
	UAbilitySystemComponent* ASC = CachedASC.Get();
	
	if (!ASC) return;
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
		// 兜底防错 (万一打了父 Tag 却没打任何子 Tag)
		
		// ！！！注意这里我把你之前的代码改了 ！！！
		// 既然它有父级 Tag，说明它肯定是在蓄力，只是没找到对应的招式。
		// 把它重置为“未蓄力”会导致动画直接退出！
		// 正确的做法是：给它一个默认的 1 段蓄力动作，这样你至少能看到动画在播。
		CurrentCombatChargingType = ECombatChargingType::CombatChargingXLZ1;
		
		// 打印紫色警告：发现父 Tag 但找不到子 Tag！
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Magenta, TEXT("异常：有 Charging 父级 Tag，但没找到 XLZ1/2/3 子 Tag！强制使用 1段")); }
	}
}
