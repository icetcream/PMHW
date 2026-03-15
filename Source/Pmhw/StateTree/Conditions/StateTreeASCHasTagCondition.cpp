#include "StateTreeASCHasTagCondition.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"

bool FStateTreeASCHasTagCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	// 获取面板上填写的参数
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 如果没有绑定目标 Actor，或者要查的 Tag 是空的，直接返回 false
	if (!InstanceData.TargetActor || !InstanceData.TagToCheck.IsValid())
	{
		return false;
	}

	// 核心：使用 GAS 官方的全局函数安全地获取 ASC
	// 这个函数支持实现了 IAbilitySystemInterface 的 Character，也支持直接挂载了 ASC 的普通 Actor
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.TargetActor);
	
	if (ASC)
	{
		// 根据是否需要精确匹配来调用对应的 ASC 接口
		if (InstanceData.bExactMatch)
		{
			return ASC->HasMatchingGameplayTag(InstanceData.TagToCheck);
		}
		else
		{
			// HasAnyMatchingGameplayTags 支持层级（父子关系）匹配
			FGameplayTagContainer TagContainer;
			TagContainer.AddTag(InstanceData.TagToCheck);
			return ASC->HasAnyMatchingGameplayTags(TagContainer);
		}
	}

	// 如果这个 Actor 根本没有 ASC 组件，返回 false
	return false;
}