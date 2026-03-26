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
	(void)Transition;
	FSTT_ManageGameplayTagsInstanceData& InstanceData = Context.GetInstanceData<FSTT_ManageGameplayTagsInstanceData>(*this);
	
	UAbilitySystemComponent* ASC = GetASCFromActor(InstanceData.TargetActor);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 兜底：如果上一次异常中断导致 Exit 没走到，这里先补做一次恢复，避免 BlockTag 残留。
	if (bRestoreTagsOnExit && InstanceData.bHasAppliedChanges)
	{
		if (!InstanceData.TagsWeAdded.IsEmpty())
		{
			ASC->RemoveLooseGameplayTags(InstanceData.TagsWeAdded);
		}
		if (!InstanceData.TagsWeRemoved.IsEmpty())
		{
			ASC->AddLooseGameplayTags(InstanceData.TagsWeRemoved);
		}
	}

	// 每次进入前，清空我们上次的记录
	InstanceData.TagsWeAdded.Reset();
	InstanceData.TagsWeRemoved.Reset();
	InstanceData.bHasAppliedChanges = false;

	// ==================== 1. 处理添加 Tags ====================
	if (!TagsToAddOnEnter.IsEmpty())
	{
		// 对称恢复策略：我们在 Enter 批量加了哪些，Exit 就按同一批量减掉。
		InstanceData.TagsWeAdded.AppendTags(TagsToAddOnEnter);
		ASC->AddLooseGameplayTags(TagsToAddOnEnter);
	}

	// ==================== 2. 处理移除 Tags ====================
	if (!TagsToRemoveOnEnter.IsEmpty())
	{
		FGameplayTagContainer OwnedTags;
		ASC->GetOwnedGameplayTags(OwnedTags);

		if (bRemoveExactAndChildren)
		{
			UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
			FGameplayTagContainer ActualTagsToRemove;

			for (const FGameplayTag& TagToRemove : TagsToRemoveOnEnter)
			{
				FGameplayTagContainer TagAndChildren;
				TagAndChildren.AddTagFast(TagToRemove); // 同时处理根标签本身
				TagAndChildren.AppendTags(TagManager.RequestGameplayTagChildren(TagToRemove));
				
				for (const FGameplayTag& CandidateTag : TagAndChildren)
				{
					if (OwnedTags.HasTagExact(CandidateTag))
					{
						ActualTagsToRemove.AddTagFast(CandidateTag);
					}
				}
			}

			if (!ActualTagsToRemove.IsEmpty())
			{
				InstanceData.TagsWeRemoved.AppendTags(ActualTagsToRemove);
				ASC->RemoveLooseGameplayTags(ActualTagsToRemove);
			}
		}
		else
		{
			FGameplayTagContainer ActualTagsToRemove;
			for (const FGameplayTag& Tag : TagsToRemoveOnEnter)
			{
				if (OwnedTags.HasTagExact(Tag))
				{
					ActualTagsToRemove.AddTagFast(Tag);
				}
			}

			if (!ActualTagsToRemove.IsEmpty())
			{
				InstanceData.TagsWeRemoved.AppendTags(ActualTagsToRemove);
				ASC->RemoveLooseGameplayTags(ActualTagsToRemove);
			}
		}
	}

	InstanceData.bHasAppliedChanges = !InstanceData.TagsWeAdded.IsEmpty() || !InstanceData.TagsWeRemoved.IsEmpty();

	// 任务瞬间执行完毕，不阻塞状态机，返回 Running 表示它在后台默默维持着这些状态
	return EStateTreeRunStatus::Running;
}

void FSTT_ManageGameplayTags::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	(void)Transition;
	FSTT_ManageGameplayTagsInstanceData& InstanceData = Context.GetInstanceData<FSTT_ManageGameplayTagsInstanceData>(*this);

	// 如果不要求恢复，直接清空运行时记录并返回。
	if (!bRestoreTagsOnExit)
	{
		InstanceData.TagsWeAdded.Reset();
		InstanceData.TagsWeRemoved.Reset();
		InstanceData.bHasAppliedChanges = false;
		return;
	}

	// 没应用过变更则无需恢复。
	if (!InstanceData.bHasAppliedChanges)
	{
		InstanceData.TagsWeAdded.Reset();
		InstanceData.TagsWeRemoved.Reset();
		return;
	}

	UAbilitySystemComponent* ASC = GetASCFromActor(InstanceData.TargetActor);

	if (ASC)
	{
		if (!InstanceData.TagsWeAdded.IsEmpty())
		{
			ASC->RemoveLooseGameplayTags(InstanceData.TagsWeAdded);
		}

		if (!InstanceData.TagsWeRemoved.IsEmpty())
		{
			ASC->AddLooseGameplayTags(InstanceData.TagsWeRemoved);
		}

		// 恢复成功后再清理；若 ASC 暂时失效，保留记录到下次 Enter 再兜底恢复。
		InstanceData.TagsWeAdded.Reset();
		InstanceData.TagsWeRemoved.Reset();
		InstanceData.bHasAppliedChanges = false;
	}
}
