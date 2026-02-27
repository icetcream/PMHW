#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
// 注意：下面这个 include 必须是你的头文件名加 .generated.h
#include "MHWStateTreeTask_SwapTag.generated.h" 

// ==========================================
// 第一部分：定义数据结构 (存放输入和参数)
// ==========================================
USTRUCT()
struct FMHWStateTreeTask_SwapTag_InstanceData
{
	GENERATED_BODY()

	// [输入] 绑定上下文中的 Actor（主角）
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> ContextActor;

	// [参数] 你想要移除的旧标签 (比如：State.Weapon.Sheathed)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag TagToRemove;

	// [参数] 你想要添加的新标签 (比如：State.Weapon.Drawn)
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag TagToAdd;
};


// ==========================================
// 第二部分：定义逻辑结构 (真正的 Task)
// ==========================================
USTRUCT(meta = (DisplayName = "STT Swap Gameplay Tag (C++)")) 
struct PMHW_API FMHWStateTreeTask_SwapTag : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	// 告诉底层引擎：我的实例数据结构是哪一个
	using FInstanceDataType = FMHWStateTreeTask_SwapTag_InstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 核心逻辑：当 StateTree 进入这个节点时执行
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
	{
		// 1. 获取当前节点独有的数据实例 (拿到面板上填的参数)
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 2. 解析绑定的 Context Actor
		if (!InstanceData.ContextActor)
		{
			// 如果没拿到主角，任务失败
			return EStateTreeRunStatus::Failed;
		}

		// 3. 从主角身上获取 Ability System Component (ASC)
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.ContextActor))
		{
			// 4. 如果配置了要移除的 Tag，就撕掉它
			if (InstanceData.TagToRemove.IsValid())
			{
				ASC->RemoveLooseGameplayTag(InstanceData.TagToRemove);
			}

			// 5. 如果配置了要添加的 Tag，就贴上它
			if (InstanceData.TagToAdd.IsValid())
			{
				ASC->AddLooseGameplayTag(InstanceData.TagToAdd);
			}

			// 6. 全部执行完毕，向上级汇报成功！
			return EStateTreeRunStatus::Succeeded; 
		}

		return EStateTreeRunStatus::Failed;
	}
};