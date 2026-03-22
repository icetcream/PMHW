#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_CalcRollDirection.generated.h"

USTRUCT()
struct FSTT_CalcRollDirectionInstanceData
{
	GENERATED_BODY()

	// [Context] 必须有的上下文
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACharacter> Character = nullptr;

	// ================= [输出 Output] 给下个节点用 =================
	
	// 算出来的该播哪个片段
	UPROPERTY(EditAnywhere, Category = "Output")
	FName OutSectionName = NAME_None;

	// 算出来的精确闪避方向目标 (只转角度不转位移的 Transform)
	UPROPERTY(EditAnywhere, Category = "Output")
	FTransform OutWarpTarget = FTransform::Identity;
};

USTRUCT(meta = (DisplayName = "Calc Roll Direction (Math)"))
struct PMHW_API FSTT_CalcRollDirection : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_CalcRollDirection() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_CalcRollDirectionInstanceData::StaticStruct(); }

	// ================= [配置项 Input] 策划配的四个名字 =================
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Section_Forward = FName("Forward");

	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Section_Backward = FName("Backward");

	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Section_Left = FName("Left");

	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Section_Right = FName("Right");

	// 核心逻辑只在进入时算一次
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};