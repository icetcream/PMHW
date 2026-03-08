#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Character/MHWMovementComponent.h"
#include "STT_StopMovement.generated.h"

class UControllerInputEvaluatorModel;
// ==========================================
// 数据篮子 (Instance Data)
// ==========================================
USTRUCT()
struct FSTT_StopMovement_InstanceData
{
	GENERATED_BODY()

	// ✨【输入】接入数据大巴车
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UControllerInputEvaluatorModel> InputEvaluatorModel = nullptr;

	// ================= 物理刹车参数 =================
	// 决定刹车滑行距离的两个核心参数
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float BrakingFrictionFactor = 2.0f; // 刹车时的额外摩擦力乘数 (值越大，刹得越快)

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float BrakingDeceleration = 2048.0f; // 刹车减速度 (虚幻默认通常是 2048)

	// ================= 输出状态 =================
	// ✨【输出】告诉状态树：角色彻底停稳了！可以切回 Idle 了！
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasStopped = false; 

	// ✨【输出】(进阶) 预测还要往前滑多远，供动画蓝图选那一帧脚刚好落地的刹车动画
	UPROPERTY(EditAnywhere, Category = "Output")
	float PredictedStopDistance = 0.0f;
};

// ==========================================
// 任务逻辑 (Task)
// ==========================================
USTRUCT(meta = (DisplayName = "Stop Movement", Category = "Locomotion"))
struct FSTT_StopMovement : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_StopMovement() = default;
	virtual const UStruct* GetInstanceDataType() const override { return FSTT_StopMovement_InstanceData::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};