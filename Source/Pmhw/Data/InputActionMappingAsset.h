// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputActionMappingAsset.generated.h"

USTRUCT(BlueprintType)
struct FInputActionTagAndPriority
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag = FGameplayTag();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FInputActionTagSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInputActionTagAndPriority InputTag; // 原始输入标签 (如 Input.X)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInputActionTagAndPriority HoldTag; // 蓄力标签 (如 Input.X.Hold)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FInputActionTagAndPriority CompleteTag; // 完成标签 (如 Input.X.Completed)
};

// Data Asset 类
UCLASS()
class UInputActionMappingAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FInputActionTagSet> ActionMappings;
	
	UFUNCTION(BlueprintCallable, Category = "TagSystem")
	bool GetTagSetByInput(FGameplayTag InputTag, FInputActionTagSet& OutTagSet) const;
};
