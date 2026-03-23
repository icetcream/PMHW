#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeEvaluatorBase.h"
#include "STE_MHWCharacterEvaluator.generated.h"

class AMHWCharacter;

USTRUCT()
struct FSTE_MHWCharacterEvaluator_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AMHWCharacter> Character = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag CurrentWeaponState;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTagContainer CurrentWeaponStateContainer;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasMovementInput = false;
};

USTRUCT(meta = (DisplayName = "MHW Character Evaluator"))
struct FSTE_MHWCharacterEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	FSTE_MHWCharacterEvaluator() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FSTE_MHWCharacterEvaluator_InstanceData::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
