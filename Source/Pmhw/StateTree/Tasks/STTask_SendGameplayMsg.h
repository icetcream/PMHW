#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h" // GAS 的事件数据载荷
#include "STTask_SendGameplayMsg.generated.h"

class UControllerInputEvaluatorModel;
// ==========================================
// 1. 数据结构体 (Instance Data)
// 用于在编辑器中暴露变量，让策划配置 Tag 和目标
// ==========================================
USTRUCT()
struct FSTTask_SendGameplayMsg_InstanceData
{
	GENERATED_BODY()

	// 【上下文】我们要把事件发给谁？（通常绑定为 StateTree 的所有者，比如 Character）
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UControllerInputEvaluatorModel> InputEvaluatorModel = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayEventData Payload;
};

// ==========================================
// 2. 逻辑结构体 (Task Component)
// 包含具体的执行逻辑
// ==========================================
USTRUCT(meta = (DisplayName = "Send Gameplay Msg", Category = "Locomotion"))
struct FSTTask_SendGameplayMsg : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTTask_SendGameplayMsg() = default;

	// 告诉状态树，这个任务对应的数据结构体是哪一个
	virtual const UStruct* GetInstanceDataType() const override { return FSTTask_SendGameplayMsg_InstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

};