#include "STT_ManageGameplayTags.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h" // 需要用到全局 Tag 管理器来查子类
#include "StateTreeExecutionContext.h"

// 一个内部辅助函数：安全获取 ASC
static UAbilitySystemComponent* GetASCFromActor(AActor* InActor)
{
	if (InActor)
	{
		return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor);
	}
	return nullptr;
}

EStateTreeRunStatus FSTT_ManageGameplayTags::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTT_ManageGameplayTagsInstanceData& InstanceData = Context.GetInstanceData<FSTT_ManageGameplayTagsInstanceData>(*this);
	
	UAbilitySystemComponent* ASC = GetASCFromActor(InstanceData.TargetActor);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 每次进入前，清空我们上次的记录
	InstanceData.TagsWeAdded.Reset();
	InstanceData.TagsWeRemoved.Reset();

	// ==================== 1. 处理添加 Tags ====================
	if (!TagsToAddOnEnter.IsEmpty())
	{
		// 记录一下：如果目标身上本来没有这个 Tag，我们才把它记到“是我们加的”小本本里
		for (const FGameplayTag& Tag : TagsToAddOnEnter)
		{
			if (!ASC->HasMatchingGameplayTag(Tag)) // 注意：AddLooseGameplayTag 是精确添加，所以查的时候最好查精确
			{
				InstanceData.TagsWeAdded.AddTagFast(Tag);
			}
		}

		// 批量添加 Loose Tags (非常适合外部系统如 StateTree 来修改状态)
		ASC->AddLooseGameplayTags(TagsToAddOnEnter);
	}

	// ==================== 2. 处理移除 Tags ====================
	if (!TagsToRemoveOnEnter.IsEmpty())
	{
		if (bRemoveExactAndChildren)
		{
			// 【高级玩法】：移除该 Tag 及其所有子 Tag！
			// 例如配置了 State.Combat，连 State.Combat.Charging 也一起拔掉。
			
			UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
			FGameplayTagContainer ActualTagsToRemove;

			for (const FGameplayTag& TagToRemove : TagsToRemoveOnEnter)
			{
				// 获取这个 Tag 及其所有子代 Tag 的集合
				FGameplayTagContainer TagAndChildren = TagManager.RequestGameplayTagChildren(TagToRemove);
				
				for (const FGameplayTag& ChildTag : TagAndChildren)
				{
					// 如果角色身上确实有这个 Tag，我们才记下来打算移除它
					if (ASC->HasMatchingGameplayTag(ChildTag))
					{
						ActualTagsToRemove.AddTagFast(ChildTag);
					}
				}
			}

			// 记录并批量移除
			InstanceData.TagsWeRemoved.AppendTags(ActualTagsToRemove);
			ASC->RemoveLooseGameplayTags(ActualTagsToRemove);
		}
		else
		{
			// 普通的精确移除
			for (const FGameplayTag& Tag : TagsToRemoveOnEnter)
			{
				if (ASC->HasMatchingGameplayTag(Tag))
				{
					InstanceData.TagsWeRemoved.AddTagFast(Tag);
				}
			}
			ASC->RemoveLooseGameplayTags(TagsToRemoveOnEnter);
		}
	}

	// 任务瞬间执行完毕，不阻塞状态机，返回 Running 表示它在后台默默维持着这些状态
	return EStateTreeRunStatus::Running;
}

void FSTT_ManageGameplayTags::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 如果不要求恢复，直接拍屁股走人
	if (!bRestoreTagsOnExit)
	{
		return;
	}

	FSTT_ManageGameplayTagsInstanceData& InstanceData = Context.GetInstanceData<FSTT_ManageGameplayTagsInstanceData>(*this);
	UAbilitySystemComponent* ASC = GetASCFromActor(InstanceData.TargetActor);

	if (ASC)
	{
		// ==================== 3. 恢复退出时的状态 ====================
		
		// 把我们进门时加的 Tag，全拔掉
		if (!InstanceData.TagsWeAdded.IsEmpty())
		{
			ASC->RemoveLooseGameplayTags(InstanceData.TagsWeAdded);
		}

		// 把我们进门时拔掉的 Tag（包括那些被连坐的子 Tag），全加回来
		if (!InstanceData.TagsWeRemoved.IsEmpty())
		{
			ASC->AddLooseGameplayTags(InstanceData.TagsWeRemoved);
		}
	}

	// 清理小本本
	InstanceData.TagsWeAdded.Reset();
	InstanceData.TagsWeRemoved.Reset();
}