// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWAbilitySystemComponent)

UMHWAbilitySystemComponent::UMHWAbilitySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMHWAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

void UMHWAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		if (!HasMatchingGameplayTag(InputTag))
		{
			AddLooseGameplayTag(InputTag);
			// 可选：打个日志看看是不是只加了一次
			// UE_LOG(LogTemp, Log, TEXT("✅ 成功添加 InputTag: %s"), *InputTag.ToString());
		}
	}
}

void UMHWAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (InputTag.IsValid())
	{
		if (HasMatchingGameplayTag(InputTag))
		{
			RemoveLooseGameplayTag(InputTag);
			// UE_LOG(LogTemp, Log, TEXT("❌ 成功移除 InputTag: %s"), *InputTag.ToString());
		}
	}
}




