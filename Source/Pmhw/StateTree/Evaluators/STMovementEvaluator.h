#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "STMovementEvaluator.generated.h"

// ==========================================
// 1. 数据篮子：定义我们要采集什么数据
// ==========================================
USTRUCT()
struct FSTMovementEvaluator_InstanceData
{
	GENERATED_BODY()

	// 【输入/上下文】：我们要监控哪个角色？(通常在编辑器里绑定为 Context Actor)
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APlayerController> Controller = nullptr;

	// 【新增点】我们可以在这里缓存当前控制的 Character，方便其他 Task 读取
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<ACharacter> ControlledCharacter = nullptr;

	// 【输出】：StateTree 可以直接拿来做 IF 判断的变量！
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsFalling = false;

	// 【输出】：顺便把速度也采集了，以后肯定用得上
	UPROPERTY(EditAnywhere, Category = "Output")
	float CurrentSpeed = 0.0f;
	
	// 【输出】玩家当前是否有推摇杆？
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasMovementInput = false;

	// 【输出】摇杆推了多大？(0.0 到 1.0 之间，可用于区分走和跑)
	UPROPERTY(EditAnywhere, Category = "Output")
	float InputMagnitude = 0.0f;
};

// ==========================================
// 2. 采集逻辑：评估器核心
// ==========================================
USTRUCT(meta = (DisplayName = "Movement Evaluator", Category = "Locomotion"))
struct FSTMovementEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()
	

	FSTMovementEvaluator() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTMovementEvaluator_InstanceData::StaticStruct(); }

	// 每帧调用，把角色的实际状态更新到数据篮子里
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};