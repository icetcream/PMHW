#include "STT_ManageGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayTagsManager.h"
#include "StateTreeExecutionContext.h"

namespace ManageGameplayTagsTask
{
	static UAbilitySystemComponent* GetASC(AActor* InActor)
	{
		return InActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor) : nullptr;
	}

	static void ResetTrackedChanges(FSTT_ManageGameplayTagsInstanceData& InstanceData)
	{
		InstanceData.TagsWeAdded.Reset();
		InstanceData.TagsWeRemoved.Reset();
		InstanceData.bHasAppliedChanges = false;
	}

	static void RestoreTrackedChanges(UAbilitySystemComponent* ASC, const FSTT_ManageGameplayTagsInstanceData& InstanceData)
	{
		if (!ASC)
		{
			return;
		}

		if (!InstanceData.TagsWeAdded.IsEmpty())
		{
			ASC->RemoveLooseGameplayTags(InstanceData.TagsWeAdded);
		}

		if (!InstanceData.TagsWeRemoved.IsEmpty())
		{
			ASC->AddLooseGameplayTags(InstanceData.TagsWeRemoved);
		}
	}

	static FGameplayTagContainer BuildTagsToRemove(
		UAbilitySystemComponent* ASC,
		const FGameplayTagContainer& TagsToRemoveOnEnter,
		const bool bRemoveExactAndChildren)
	{
		FGameplayTagContainer ActualTagsToRemove;
		if (!ASC || TagsToRemoveOnEnter.IsEmpty())
		{
			return ActualTagsToRemove;
		}

		FGameplayTagContainer OwnedTags;
		ASC->GetOwnedGameplayTags(OwnedTags);

		if (!bRemoveExactAndChildren)
		{
			for (const FGameplayTag& Tag : TagsToRemoveOnEnter)
			{
				if (OwnedTags.HasTagExact(Tag))
				{
					ActualTagsToRemove.AddTagFast(Tag);
				}
			}

			return ActualTagsToRemove;
		}

		UGameplayTagsManager& TagManager = UGameplayTagsManager::Get();
		for (const FGameplayTag& TagToRemove : TagsToRemoveOnEnter)
		{
			FGameplayTagContainer TagAndChildren;
			TagAndChildren.AddTagFast(TagToRemove);
			TagAndChildren.AppendTags(TagManager.RequestGameplayTagChildren(TagToRemove));

			for (const FGameplayTag& CandidateTag : TagAndChildren)
			{
				if (OwnedTags.HasTagExact(CandidateTag))
				{
					ActualTagsToRemove.AddTagFast(CandidateTag);
				}
			}
		}

		return ActualTagsToRemove;
	}
}

EStateTreeRunStatus FSTT_ManageGameplayTags::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FSTT_ManageGameplayTagsInstanceData& InstanceData = Context.GetInstanceData<FSTT_ManageGameplayTagsInstanceData>(*this);
	UAbilitySystemComponent* ASC = ManageGameplayTagsTask::GetASC(InstanceData.TargetActor);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}

	// Recover stale changes first in case the previous ExitState never ran.
	if (bRestoreTagsOnExit && InstanceData.bHasAppliedChanges)
	{
		ManageGameplayTagsTask::RestoreTrackedChanges(ASC, InstanceData);
	}

	ManageGameplayTagsTask::ResetTrackedChanges(InstanceData);

	if (!TagsToAddOnEnter.IsEmpty())
	{
		InstanceData.TagsWeAdded.AppendTags(TagsToAddOnEnter);
		ASC->AddLooseGameplayTags(TagsToAddOnEnter);
	}

	if (!TagsToRemoveOnEnter.IsEmpty())
	{
		const FGameplayTagContainer ActualTagsToRemove = ManageGameplayTagsTask::BuildTagsToRemove(
			ASC,
			TagsToRemoveOnEnter,
			bRemoveExactAndChildren);
		if (!ActualTagsToRemove.IsEmpty())
		{
			InstanceData.TagsWeRemoved.AppendTags(ActualTagsToRemove);
			ASC->RemoveLooseGameplayTags(ActualTagsToRemove);
		}
	}

	InstanceData.bHasAppliedChanges = !InstanceData.TagsWeAdded.IsEmpty() || !InstanceData.TagsWeRemoved.IsEmpty();
	return EStateTreeRunStatus::Running;
}

void FSTT_ManageGameplayTags::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FSTT_ManageGameplayTagsInstanceData& InstanceData = Context.GetInstanceData<FSTT_ManageGameplayTagsInstanceData>(*this);

	if (!bRestoreTagsOnExit)
	{
		ManageGameplayTagsTask::ResetTrackedChanges(InstanceData);
		return;
	}

	if (!InstanceData.bHasAppliedChanges)
	{
		ManageGameplayTagsTask::ResetTrackedChanges(InstanceData);
		return;
	}

	UAbilitySystemComponent* ASC = ManageGameplayTagsTask::GetASC(InstanceData.TargetActor);

	if (ASC)
	{
		ManageGameplayTagsTask::RestoreTrackedChanges(ASC, InstanceData);
		ManageGameplayTagsTask::ResetTrackedChanges(InstanceData);
	}
}
