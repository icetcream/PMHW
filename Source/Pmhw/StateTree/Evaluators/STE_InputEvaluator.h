#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"
#include "STE_InputEvaluator.generated.h"

// ==========================================
// Evaluator 数据篮子
// ==========================================
USTRUCT()
struct FSTE_InputEvaluator_InstanceData
{
	GENERATED_BODY()

	// 【输入】状态树挂载的 Controller
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APlayerController> Controller = nullptr;

	// ✨【输出】这就是我们建好的数据仓库，暴露给所有 Task 使用！
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<UControllerInputEvaluatorModel> EvaluatorModel = nullptr;
};

// ==========================================
// Evaluator 逻辑
// ==========================================
USTRUCT(meta = (DisplayName = "Controller Input Evaluator"))
struct FSTE_InputEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	FSTE_InputEvaluator() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FSTE_InputEvaluator_InstanceData::StaticStruct(); }

	// 状态树刚启动时，实例化那个 Model 仓库
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	
	// 每一帧，把最新的指针塞进仓库
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};