// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "MHWPlayerState.generated.h"

class UMHWEquipmentManagerComponent;
class UMHWAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class PMHW_API AMHWPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AMHWPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "MHW|PlayerState")
	UMHWAbilitySystemComponent* GetMHWAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "MHW|PlayerState")
	TObjectPtr<UMHWAbilitySystemComponent> AbilitySystemComponent;
};
