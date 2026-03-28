#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHWGameplayTags.h"
#include "StateTreeTaskBase.h"
#include "STT_ComboMontagePlay.generated.h"

class AMHWCharacter;
class UAnimInstance;
class UAnimMontage;

USTRUCT()
struct FSTT_ComboMontagePlayInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AMHWCharacter> MHWCharacter = nullptr;

	UPROPERTY(EditAnywhere, Category = "Input|Motion Warping")
	bool bUseMotionWarping = false;

	UPROPERTY(EditAnywhere, Category = "Input|Motion Warping", meta = (EditCondition = "bUseMotionWarping", EditConditionHides))
	FName MotionWarpingName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Input|Motion Warping", meta = (EditCondition = "bUseMotionWarping", EditConditionHides))
	FTransform TargetTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, Category = "Input|Motion Warping", meta = (EditCondition = "bUseMotionWarping", EditConditionHides))
	bool bClearMotionWarpingOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Input|Montage")
	FName StartSection = NAME_None;

	TWeakObjectPtr<UAnimInstance> CachedAnimInstance = nullptr;

	float PlayedMontageLength = 0.0f;
	float LastMontagePosition = 0.0f;
	FName ActiveMotionWarpingName = NAME_None;
	bool bMontageStarted = false;
	bool bMontageEnded = false;
	bool bMontageInterrupted = false;
	bool bInterruptedCleanupDone = false;
	bool bAppliedMotionWarping = false;
};

USTRUCT(meta = (DisplayName = "Combo Montage Play"))
struct PMHW_API FSTT_ComboMontagePlay : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_ComboMontagePlay();

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_ComboMontagePlayInstanceData::StaticStruct(); }

	UPROPERTY(EditAnywhere, Category = "Montage")
	TObjectPtr<UAnimMontage> ComboMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = "Montage")
	float PlayRate = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Montage")
	bool bShouldStopAllMontages = true;

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (ToolTip = "如果填写，则在播放 Montage 前按该标签从当前武器的攻击数据表中读取耐力消耗，并调用角色的 ConsumeStamina。", Categories = "Data.AttackSpec"))
	FGameplayTag AttackSpecTag;

	UPROPERTY(EditAnywhere, Category = "Montage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SuccessCompletionThreshold = 0.95f;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags")
	bool bRemoveTagOnEnter = false;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags", meta = (EditCondition = "bRemoveTagOnEnter", EditConditionHides))
	bool bRemoveTagAndChildrenOnEnter = true;

	UPROPERTY(EditAnywhere, Category = "Gameplay Tags", meta = (EditCondition = "bRemoveTagOnEnter", EditConditionHides))
	FGameplayTagContainer RemoveTagsOnEnter;

	UPROPERTY(EditAnywhere, Category = "Exit")
	bool bStopMontageOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Exit")
	bool bClearPreInputOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Exit")
	bool bDisablePreInputOnExit = true;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
