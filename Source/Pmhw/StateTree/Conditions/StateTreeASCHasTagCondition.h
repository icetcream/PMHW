#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "StateTreeASCHasTagCondition.generated.h"

// =========================================================================
// 实例数据：定义我们要暴露给状态树面板的参数
// 规范：命名为 FStateTree + 功能名 + ConditionInstanceData
// =========================================================================
USTRUCT(BlueprintType)
struct PMHW_API FStateTreeASCHasTagConditionInstanceData
{
	GENERATED_BODY()

	// 目标 Actor（我们需要检查谁的 ASC？）
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 我们要查询的 Tag（例如：State.Combat.ComboWindow, State.Buff.Invincible）
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag TagToCheck;

	// 是否需要精确匹配？
	// 如果 false (默认)：查询 State.Combat，拥有 State.Combat.ComboWindow 也会返回 true
	// 如果 true：必须一模一样才返回 true
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bExactMatch = false; 
};

// =========================================================================
// 条件节点：定义在下拉菜单中显示的名字和分类
// 规范：分类统一放到 "Ability System" 下
// =========================================================================
USTRUCT(BlueprintType,meta = (DisplayName = "ASC Has Gameplay Tag", Category = "Ability System"))
struct PMHW_API FStateTreeASCHasTagCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeASCHasTagConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// 执行真正的判断逻辑
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};