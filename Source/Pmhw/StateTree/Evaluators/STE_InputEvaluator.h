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
	
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasMovementInput = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsFalling = false;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasVelocity = false;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	float GroundVelocity = 0.f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float Velocity2D = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	float MaxGroundVelocity = 0.f;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector CurrentAcceleration = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "Output")
	float VelocityLocalAngle = 0.f;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasRootMotion = false;
	
	// 【输出】加速度（实际输入意图）与角色正前方的局部夹角（范围：-180 到 180，正数为偏右，负数为偏左）
	UPROPERTY(EditAnywhere, Category = "Output")
	float AccelerationLocalAngle = 0.f;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag CurrentWeaponState;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTagContainer CurrentWeaponStateContainer;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag CurrentGait;
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
