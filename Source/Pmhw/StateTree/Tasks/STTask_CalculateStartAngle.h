#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameFramework/Character.h"
#include "STTask_CalculateStartAngle.generated.h"

class UControllerInputEvaluatorModel;
// ==========================================
// 1. 数据篮子 (Instance Data)
// ==========================================
USTRUCT()
struct FSTTask_CalculateStartAngle_InstanceData
{
	GENERATED_BODY()

	// ✨【输入】直接接入数据大巴车 (Model)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UControllerInputEvaluatorModel> InputEvaluatorModel = nullptr;

	// 【输出】计算出的角度
	UPROPERTY(EditAnywhere, Category = "Output")
	float ResultAngle = 0.0f;
};

// ==========================================
// 2. 任务逻辑 (Task)
// ==========================================
USTRUCT(meta = (DisplayName = "Calculate Start Angle", Category = "Locomotion"))
struct FSTTask_CalculateStartAngle : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_CalculateStartAngle() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTTask_CalculateStartAngle_InstanceData::StaticStruct(); }

	// ✨ 注意：我们重写的是 EnterState 而不是 Tick！因为它只需要算一次 ✨
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};