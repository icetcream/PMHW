#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "MHWStateTreeTask_RemoveTag.generated.h"

// ==========================================
// 第一部分：定义数据结构 (存放输入、输出和参数)
// ==========================================
USTRUCT()
struct FMHWStateTreeTask_RemoveTag_InstanceData
{
	GENERATED_BODY()

	// [输入] 绑定上下文中的 Actor（比如主角）
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> ContextActor;

	// [参数] 你要在面板上配置的 Tag
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag TagToRemove;
};

// ==========================================
// 第二部分：定义逻辑结构 (真正的 Task)
// ==========================================
USTRUCT(meta = (DisplayName = "STT Remove Gameplay Tag (C++)")) // 填你在编辑器里想看到的名字
struct PMHW_API FMHWStateTreeTask_RemoveTag : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	// 告诉底层：我的数据存放在哪里
	using FInstanceDataType = FMHWStateTreeTask_RemoveTag_InstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 核心逻辑：进入状态时执行
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};