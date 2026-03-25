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

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasVelocity = false;

	// 基于当前加速度方向生成的朝向 Transform（位置为角色当前位置）
	UPROPERTY(EditAnywhere, Category = "Output")
	FTransform AccelerationFacingTransform = FTransform::Identity;
};

USTRUCT(meta = (DisplayName = "MHW Character Evaluator"))
struct FSTE_MHWCharacterEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	FSTE_MHWCharacterEvaluator() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FSTE_MHWCharacterEvaluator_InstanceData::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
