// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWPlayerController.h"
#include "AbilitySystemComponent.h" 
#include "MHWPlayerState.h"
#include "Input/MHWInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPlayerController)

UAbilitySystemComponent* AMHWPlayerController::GetAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		AMHWPlayerState* PS = GetPlayerState<AMHWPlayerState>();
		if (PS)
		{
			AbilitySystemComponent = PS->GetAbilitySystemComponent();
		}
	}
	return ASC;
}


