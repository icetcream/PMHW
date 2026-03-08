#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameFramework/Character.h"
#include "STT_StartMovement.generated.h"

class UControllerInputEvaluatorModel;
// ==========================================
// 数据篮子 (Instance Data)
// ==========================================
USTRUCT()
struct FSTT_StartMovement_InstanceData
{
	GENERATED_BODY()

	// 【上下文】我们要操作的角色
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UControllerInputEvaluatorModel> InputEvaluatorModel = nullptr;

	// 【输出】计算出的极其精确的运动夹角（-180 到 180），给动画蓝图用！
	UPROPERTY(EditAnywhere, Category = "Output")
	float LocomotionAngle = 0.0f;

	// 【输出】判断是否是 180 度向后猛转身起步（供物理层特殊处理用）
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsPivotStart = false; 
};

// ==========================================
// 任务逻辑 (Task)
// ==========================================
USTRUCT(meta = (DisplayName = "Start Movement (Warping)", Category = "Locomotion"))
struct FSTT_StartMovement : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_StartMovement() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_StartMovement_InstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};