// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWPlayerController.h"
#include "AbilitySystemComponent.h" 
#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Character/MHWCombatComponent.h"
#include "MHWPlayerState.h"
#include "Input/MHWInputComponent.h"
#include "UI/MHWHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPlayerController)

void AMHWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TryInitOverlay(GetPawn());
}

void AMHWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	TryInitOverlay(InPawn);
}

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

void AMHWPlayerController::TryInitOverlay(APawn* InPawn)
{
	if (!IsLocalController())
	{
		return;
	}

	AMHWHUD* MHWHUD = GetHUD<AMHWHUD>();
	AMHWPlayerState* PS = GetPlayerState<AMHWPlayerState>();
	UMHWCombatComponent* CombatComponent = InPawn ? InPawn->FindComponentByClass<UMHWCombatComponent>() : nullptr;
	if (!MHWHUD || !PS || !CombatComponent)
	{
		return;
	}

	MHWHUD->InitOverlay(
		this,
		PS,
		PS->GetMHWAbilitySystemComponent(),
		const_cast<UMHWCombatAttributeSet*>(PS->GetCombatAttributeSet()),
		CombatComponent);
}


