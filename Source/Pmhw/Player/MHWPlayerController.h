// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerController.h"
#include "MHWPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class PMHW_API AMHWPlayerController : public APlayerController, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	mutable TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
