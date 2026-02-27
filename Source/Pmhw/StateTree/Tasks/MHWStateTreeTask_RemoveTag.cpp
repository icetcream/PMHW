#include "MHWStateTreeTask_RemoveTag.h"

EStateTreeRunStatus FMHWStateTreeTask_RemoveTag::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	{
		// 1. 获取当前节点的数据实例
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 2. 获取绑定的 Actor
		
		if (!InstanceData.ContextActor)
		{
			return EStateTreeRunStatus::Failed;
		}

		// 3. 拿到 ASC 并移除 Tag
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.ContextActor))
		{
			if (InstanceData.TagToRemove.IsValid())
			{
				ASC->RemoveLooseGameplayTag(InstanceData.TagToRemove);
			}
			return EStateTreeRunStatus::Succeeded; // 执行成功！
		}

		return EStateTreeRunStatus::Failed;
	}
}
