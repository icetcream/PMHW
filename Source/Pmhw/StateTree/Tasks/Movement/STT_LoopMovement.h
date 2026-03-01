#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_LoopMovement.generated.h"

class UControllerInputEvaluatorModel;

USTRUCT()
struct FSTT_LoopMovement_InstanceData
{
	GENERATED_BODY()

	// ✨【输入】这就是从 Evaluator 那里接过来的仓库！
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UControllerInputEvaluatorModel> InputEvaluatorModel = nullptr;

	// Task 自带的参数
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxWalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxAcceleration = 2000.0f;
};

USTRUCT(meta = (DisplayName = "Loop Movement Task"))
struct FSTT_LoopMovement : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_LoopMovement() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FSTT_LoopMovement_InstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};