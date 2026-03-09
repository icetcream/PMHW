// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ConfigManager.generated.h"

class UInputActionMappingAsset;
/**
 * 
 */
UCLASS()
class PMHW_API UConfigManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetInputActionMappingAsset(UInputActionMappingAsset* MappingAsset);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UInputActionMappingAsset* InputActionMappingAsset;
};
