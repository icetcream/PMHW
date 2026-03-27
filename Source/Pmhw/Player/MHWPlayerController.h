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
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	void TryInitOverlay(APawn* InPawn);

	mutable TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
