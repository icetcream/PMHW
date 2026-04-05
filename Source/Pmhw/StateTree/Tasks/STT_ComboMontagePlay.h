#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHWGameplayTags.h"
#include "StateTreeTaskBase.h"
#include "STT_ComboMontagePlay.generated.h"

class AMHWCharacter;
class UAnimInstance;
class UAnimMontage;

struct FComboMontagePlayRuntimeState
{
	bool bMontageEnded = false;
	bool bMontageInterrupted = false;
	bool bInterruptedCleanupDone = false;
};

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

	// Montage delegates can outlive StateTree instance storage, so task callback state
	// stays on the heap instead of capturing addresses inside InstanceData.
	TSharedPtr<FComboMontagePlayRuntimeState> RuntimeState;
	TWeakObjectPtr<UAnimInstance> CachedAnimInstance = nullptr;

	float PlayedMontageLength = 0.0f;
	float LastMontagePosition = 0.0f;
	FName ActiveMotionWarpingName = NAME_None;
	bool bMontageStarted = false;
	bool bAppliedMotionWarping = false;
	bool bAppliedAttackSpecTagOverride = false;
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

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (ToolTip = "If set, stamina cost is resolved from the current weapon attack data row before the montage starts.", Categories = "Data.AttackSpec"))
	FGameplayTag AttackSpecTag;

	UPROPERTY(EditAnywhere, Category = "Cost")
	bool bUsePendingChargeLevel = false;

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (EditCondition = "!bUsePendingChargeLevel", EditConditionHides))
	bool bClearPendingChargeLevelOnEnter = false;

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (EditCondition = "bUsePendingChargeLevel", EditConditionHides))
	bool bClearPendingChargeLevelOnExit = true;

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (EditCondition = "bUsePendingChargeLevel", EditConditionHides, Categories = "Data.AttackSpec"))
	FGameplayTag ChargeLevel2AttackSpecTag;

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (EditCondition = "bUsePendingChargeLevel", EditConditionHides, Categories = "Data.AttackSpec"))
	FGameplayTag ChargeLevel3AttackSpecTag;

	UPROPERTY(EditAnywhere, Category = "Cost", meta = (EditCondition = "bUsePendingChargeLevel", EditConditionHides, Categories = "Data.AttackSpec"))
	FGameplayTag OverchargedAttackSpecTag;

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
