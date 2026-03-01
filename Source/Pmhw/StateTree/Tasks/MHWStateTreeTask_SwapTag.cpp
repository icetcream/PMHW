#include "MHWStateTreeTask_SwapTag.h"

void FMHWStateTreeTask_SwapTag::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

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
	}
}
