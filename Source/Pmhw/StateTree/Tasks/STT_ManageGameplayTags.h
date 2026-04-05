#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "STT_ManageGameplayTags.generated.h"

class AActor;
class UAbilitySystemComponent;

USTRUCT()
struct FSTT_ManageGameplayTagsInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> TargetActor = nullptr;

	// Track only the tags this task changed so Exit can restore without touching external state.
	FGameplayTagContainer TagsWeAdded;
	FGameplayTagContainer TagsWeRemoved;
	bool bHasAppliedChanges = false;
};

USTRUCT(meta = (DisplayName = "Manage Gameplay Tags"))
struct PMHW_API FSTT_ManageGameplayTags : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_ManageGameplayTags() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_ManageGameplayTagsInstanceData::StaticStruct(); }

	UPROPERTY(EditAnywhere, Category = "Tags To Add", meta = (ToolTip = "进入状态时，将这些 Tag 添加到目标身上。"))
	FGameplayTagContainer TagsToAddOnEnter;

	UPROPERTY(EditAnywhere, Category = "Tags To Remove", meta = (ToolTip = "进入状态时，将这些 Tag 从目标身上移除。"))
	FGameplayTagContainer TagsToRemoveOnEnter;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ToolTip = "如果勾选，移除 Tag 时会连同其所有的子 Tag 一起移除（精确匹配变更为层级匹配）。"))
	bool bRemoveExactAndChildren = true;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ToolTip = "如果勾选，退出状态时，会自动撤销在这个任务中对 Tag 的修改（加的减掉，减的加回）。"))
	bool bRestoreTagsOnExit = true;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
