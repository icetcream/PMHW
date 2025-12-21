// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MHWPlayerState.h"

#include "AbilitySystem/MHWAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPlayerState)

AMHWPlayerState::AMHWPlayerState(const FObjectInitializer& ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UMHWAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* AMHWPlayerState::GetAbilitySystemComponent() const
{
	return GetMHWAbilitySystemComponent();
}
