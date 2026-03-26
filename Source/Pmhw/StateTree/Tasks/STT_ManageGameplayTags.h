#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "STT_ManageGameplayTags.generated.h"

class AActor;
class UAbilitySystemComponent;

// =========================================================================
// 实例数据：存放运行时需要的上下文和内部状态
// =========================================================================
USTRUCT()
struct FSTT_ManageGameplayTagsInstanceData
{
	GENERATED_BODY()

	// 必须有的上下文：通常是当前被 StateTree 控制的 Actor (比如 Character)
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;

	// [内部状态]：记录我们在 EnterState 时，真正成功添加了哪些 Tag。
	// 这样在 ExitState 恢复时，我们只移除我们自己加的，不碰别人加的。
	FGameplayTagContainer TagsWeAdded;

	// [内部状态]：记录我们在 EnterState 时，真正成功移除了哪些 Tag。
	FGameplayTagContainer TagsWeRemoved;

	// [内部状态]：记录本任务是否应用过 Tag 变更。
	// 用于下一次 Enter 时兜底恢复上次异常中断残留。
	bool bHasAppliedChanges = false;
};

// =========================================================================
// 任务定义：暴漏给 StateTree 编辑器的配置项
// =========================================================================
USTRUCT(meta = (DisplayName = "Manage Gameplay Tags"))
struct PMHW_API FSTT_ManageGameplayTags : public FStateTreeTaskCommonBase{
	GENERATED_BODY()

	FSTT_ManageGameplayTags() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_ManageGameplayTagsInstanceData::StaticStruct(); }

	// ================= 配置项 =================

	UPROPERTY(EditAnywhere, Category = "Tags To Add", meta = (ToolTip = "进入状态时，将这些 Tag 添加到目标身上。"))
	FGameplayTagContainer TagsToAddOnEnter;

	UPROPERTY(EditAnywhere, Category = "Tags To Remove", meta = (ToolTip = "进入状态时，将这些 Tag 从目标身上移除。"))
	FGameplayTagContainer TagsToRemoveOnEnter;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ToolTip = "如果勾选，移除 Tag 时会连同其所有的子 Tag 一起移除（精确匹配变更为层级匹配）。"))
	bool bRemoveExactAndChildren = true;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ToolTip = "如果勾选，退出状态时，会自动撤销在这个任务中对 Tag 的修改（加的减掉，减的加回）。"))
	bool bRestoreTagsOnExit = true;

	// ================= 生命周期 =================
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
	// 注意：不需要重写 Tick，因为这只是一个瞬间执行的状态修饰器
};
