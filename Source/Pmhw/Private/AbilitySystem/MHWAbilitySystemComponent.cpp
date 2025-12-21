// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/MHWAbilitySystemComponent.h"

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




