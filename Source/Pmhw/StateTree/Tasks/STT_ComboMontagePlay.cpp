#include "STT_ComboMontagePlay.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/MHWAttackComponent.h"
#include "Character/MHWCharacter.h"
#include "Character/MHWComboPreInputComponent.h"
#include "Character/MeleeTraceComponent.h"
#include "Data/MHWAttackDataTable.h"
#include "Interface/MHWCharacterInterface.h"
#include "MotionWarpingComponent.h"
#include "StateTreeExecutionContext.h"

namespace ComboMontagePlayTask
{
	static UAbilitySystemComponent* GetASC(AActor* InActor)
	{
		return InActor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor) : nullptr;
	}

	static UMHWAttackComponent* GetAttackComponent(AActor* InActor)
	{
		if (!InActor || !InActor->Implements<UMHWCharacterInterface>())
		{
			return nullptr;
		}

		return IMHWCharacterInterface::Execute_GetAttackComponent(InActor);
	}

	static void ResetInstanceData(FSTT_ComboMontagePlayInstanceData& InstanceData)
	{
		InstanceData.RuntimeState.Reset();
		InstanceData.CachedAnimInstance = nullptr;
		InstanceData.PlayedMontageLength = 0.0f;
		InstanceData.LastMontagePosition = 0.0f;
		InstanceData.ActiveMotionWarpingName = NAME_None;
		InstanceData.bMontageStarted = false;
		InstanceData.bAppliedMotionWarping = false;
		InstanceData.bAppliedAttackSpecTagOverride = false;
	}

	static void UnbindMontageDelegates(UAnimInstance* AnimInstance, UAnimMontage* Montage)
	{
		if (!AnimInstance || !Montage)
		{
			return;
		}

		FOnMontageBlendingOutStarted EmptyBlendOutDelegate;
		AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendOutDelegate, Montage);

		FOnMontageEnded EmptyEndDelegate;
		AnimInstance->Montage_SetEndDelegate(EmptyEndDelegate, Montage);
	}

	static void RemoveConfiguredTags(
		AActor* Actor,
		const bool bRemoveTags,
		const bool bRemoveTagAndChildren,
		const FGameplayTagContainer& TagsToRemove)
	{
		if (!bRemoveTags || TagsToRemove.IsEmpty())
		{
			return;
		}

		UAbilitySystemComponent* ASC = GetASC(Actor);
		if (!ASC)
		{
			return;
		}

		if (bRemoveTagAndChildren)
		{
			FGameplayTagContainer OwnedTags;
			ASC->GetOwnedGameplayTags(OwnedTags);

			FGameplayTagContainer ActualTagsToRemove;
			for (const FGameplayTag& RootTag : TagsToRemove)
			{
				for (const FGameplayTag& OwnedTag : OwnedTags)
				{
					if (OwnedTag.MatchesTag(RootTag))
					{
						ActualTagsToRemove.AddTagFast(OwnedTag);
					}
				}
			}

			if (!ActualTagsToRemove.IsEmpty())
			{
				ASC->RemoveLooseGameplayTags(ActualTagsToRemove);
			}
			return;
		}

		ASC->RemoveLooseGameplayTags(TagsToRemove);
	}

	static bool ResolveMotionWarpingTarget(
		AMHWCharacter* Character,
		const bool bUseMotionWarping,
		const FName ConfiguredMotionWarpingName,
		const FTransform& ConfiguredTargetTransform,
		FName& OutMotionWarpingName,
		FTransform& OutTargetTransform)
	{
		if (!bUseMotionWarping || !Character)
		{
			return false;
		}

		if (!ConfiguredMotionWarpingName.IsNone())
		{
			OutMotionWarpingName = ConfiguredMotionWarpingName;
			OutTargetTransform = ConfiguredTargetTransform;
			return true;
		}

		return Character->ConsumePendingMotionWarpTarget(OutMotionWarpingName, OutTargetTransform);
	}

	static void CleanupCharacterState(
		AMHWCharacter* Character,
		const bool bClearMotionWarpingOnExit,
		const FName MotionWarpingName,
		const bool bClearPreInputOnExit,
		const bool bDisablePreInputOnExit)
	{
		if (!Character)
		{
			return;
		}

		if (bClearMotionWarpingOnExit && !MotionWarpingName.IsNone())
		{
			if (UMotionWarpingComponent* MotionWarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>())
			{
				MotionWarpingComp->RemoveWarpTarget(MotionWarpingName);
			}
		}

		if (Character->Implements<UMHWCharacterInterface>())
		{
			if (UMHWComboPreInputComponent* ComboPreInputComp = IMHWCharacterInterface::Execute_GetComboPreInputComponent(Character))
			{
				if (bClearPreInputOnExit)
				{
					ComboPreInputComp->ClearBuffer();
				}
				if (bDisablePreInputOnExit)
				{
					ComboPreInputComp->bCanPreInput = false;
				}
			}
		}
	}

	static void CleanupTaskState(
		AMHWCharacter* Character,
		UAnimMontage* ComboMontage,
		const bool bStopMontage,
		const bool bClearMotionWarpingOnExit,
		const FName MotionWarpingName,
		const bool bClearPreInputOnExit,
		const bool bDisablePreInputOnExit,
		FSTT_ComboMontagePlayInstanceData& InstanceData)
	{
		if (!Character)
		{
			ResetInstanceData(InstanceData);
			return;
		}

		UAnimInstance* AnimInstance = InstanceData.CachedAnimInstance.Get();
		if (!AnimInstance)
		{
			if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
			{
				AnimInstance = MeshComponent->GetAnimInstance();
			}
		}

		// Stop montage first, so AnimNotifyState can receive end callbacks while context is still intact.
		if (bStopMontage && ComboMontage)
		{
			Character->StopAnimMontage(ComboMontage);
		}

		UnbindMontageDelegates(AnimInstance, ComboMontage);

		CleanupCharacterState(
			Character,
			bClearMotionWarpingOnExit,
			MotionWarpingName,
			bClearPreInputOnExit,
			bDisablePreInputOnExit);

		ResetInstanceData(InstanceData);
	}

	static bool TryConsumeConfiguredStamina(AMHWCharacter* Character, const FGameplayTag& AttackSpecTag)
	{
		if (!Character || !AttackSpecTag.IsValid())
		{
			return true;
		}

		if (!Character->Implements<UMHWCharacterInterface>())
		{
			UE_LOG(LogStateTree, Warning, TEXT("ComboMontagePlay failed to consume stamina: %s does not implement MHWCharacterInterface."),
				*GetNameSafe(Character));
			return true;
		}

		UMeleeTraceComponent* MeleeTraceComponent = IMHWCharacterInterface::Execute_GetMeleeTraceComponent(Character);
		if (!MeleeTraceComponent)
		{
			UE_LOG(LogStateTree, Verbose, TEXT("ComboMontagePlay skipped stamina consume: %s has no MeleeTraceComponent for AttackSpecTag [%s]."),
				*GetNameSafe(Character),
				*AttackSpecTag.ToString());
			return true;
		}

		const FMHWAttackDataRow* AttackDataRow = MeleeTraceComponent->FindAttackDataRowBySpecTag(AttackSpecTag);
		if (!AttackDataRow)
		{
			UE_LOG(LogStateTree, Verbose, TEXT("ComboMontagePlay skipped stamina consume: attack data row not found for AttackSpecTag [%s] on %s."),
				*AttackSpecTag.ToString(),
				*GetNameSafe(Character));
			return true;
		}

		if (AttackDataRow->StaminaCost <= 0.0f)
		{
			return true;
		}

		if (!Character->ConsumeStamina(AttackDataRow->StaminaCost))
		{
			UE_LOG(LogStateTree, Verbose, TEXT("ComboMontagePlay blocked by stamina cost: %s requires %.2f stamina for [%s]."),
				*GetNameSafe(Character),
				AttackDataRow->StaminaCost,
				*AttackSpecTag.ToString());
			return false;
		}

		return true;
	}

	static FGameplayTag ResolveAttackSpecTagForChargeLevel(
		const EMHWChargeLevel ChargeLevel,
		const FGameplayTag& DefaultAttackSpecTag,
		const FGameplayTag& ChargeLevel2AttackSpecTag,
		const FGameplayTag& ChargeLevel3AttackSpecTag,
		const FGameplayTag& OverchargedAttackSpecTag)
	{
		switch (ChargeLevel)
		{
		case EMHWChargeLevel::Level1:
			return DefaultAttackSpecTag;
		case EMHWChargeLevel::Level2:
			return ChargeLevel2AttackSpecTag.IsValid() ? ChargeLevel2AttackSpecTag : DefaultAttackSpecTag;
		case EMHWChargeLevel::Level3:
			if (ChargeLevel3AttackSpecTag.IsValid())
			{
				return ChargeLevel3AttackSpecTag;
			}
			return ChargeLevel2AttackSpecTag.IsValid() ? ChargeLevel2AttackSpecTag : DefaultAttackSpecTag;
		case EMHWChargeLevel::Overcharged:
			if (OverchargedAttackSpecTag.IsValid())
			{
				return OverchargedAttackSpecTag;
			}
			return ChargeLevel2AttackSpecTag.IsValid() ? ChargeLevel2AttackSpecTag : DefaultAttackSpecTag;
		default:
			return DefaultAttackSpecTag;
		}
	}

	static FGameplayTag ResolveEffectiveAttackSpecTagForTask(
		AMHWCharacter* Character,
		const FGameplayTag& DefaultAttackSpecTag,
		const bool bUsePendingChargeLevel,
		const bool bClearPendingChargeLevelOnEnter,
		const FGameplayTag& ChargeLevel2AttackSpecTag,
		const FGameplayTag& ChargeLevel3AttackSpecTag,
		const FGameplayTag& OverchargedAttackSpecTag)
	{
		UMHWAttackComponent* AttackComponent = GetAttackComponent(Character);
		if (!AttackComponent)
		{
			return DefaultAttackSpecTag;
		}

		if (bUsePendingChargeLevel)
		{
			EMHWChargeLevel PendingChargeLevel;
			if (AttackComponent->PeekPendingChargeLevel(PendingChargeLevel))
			{
				return ResolveAttackSpecTagForChargeLevel(
					PendingChargeLevel,
					DefaultAttackSpecTag,
					ChargeLevel2AttackSpecTag,
					ChargeLevel3AttackSpecTag,
					OverchargedAttackSpecTag);
			}

			return DefaultAttackSpecTag;
		}

		if (bClearPendingChargeLevelOnEnter)
		{
			AttackComponent->ClearPendingChargeLevel();
		}

		return DefaultAttackSpecTag;
	}
}

FSTT_ComboMontagePlay::FSTT_ComboMontagePlay()
{
	bRemoveTagOnEnter = true;

	RemoveTagsOnEnter.Reset();

	RemoveTagsOnEnter.AddTagFast(MHWStateTags::ComboWindow);
}

EStateTreeRunStatus FSTT_ComboMontagePlay::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FSTT_ComboMontagePlayInstanceData& InstanceData = Context.GetInstanceData<FSTT_ComboMontagePlayInstanceData>(*this);
	ComboMontagePlayTask::ResetInstanceData(InstanceData);

	AMHWCharacter* Character = InstanceData.MHWCharacter;
	if (!Character || !ComboMontage)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.RuntimeState = MakeShared<FComboMontagePlayRuntimeState>();

	if (UMHWAttackComponent* AttackComponent = ComboMontagePlayTask::GetAttackComponent(Character))
	{
		AttackComponent->ClearActiveAttackSpecTagOverride();
	}

	const FGameplayTag EffectiveAttackSpecTag = ComboMontagePlayTask::ResolveEffectiveAttackSpecTagForTask(
		Character,
		AttackSpecTag,
		bUsePendingChargeLevel,
		bClearPendingChargeLevelOnEnter,
		ChargeLevel2AttackSpecTag,
		ChargeLevel3AttackSpecTag,
		OverchargedAttackSpecTag);

	if (!ComboMontagePlayTask::TryConsumeConfiguredStamina(Character, EffectiveAttackSpecTag))
	{
		return EStateTreeRunStatus::Failed;
	}

	ComboMontagePlayTask::RemoveConfiguredTags(Character, bRemoveTagOnEnter, bRemoveTagAndChildrenOnEnter, RemoveTagsOnEnter);

	FName MotionWarpingNameToApply = NAME_None;
	FTransform MotionWarpingTargetTransform = FTransform::Identity;
	if (ComboMontagePlayTask::ResolveMotionWarpingTarget(
		Character,
		InstanceData.bUseMotionWarping,
		InstanceData.MotionWarpingName,
		InstanceData.TargetTransform,
		MotionWarpingNameToApply,
		MotionWarpingTargetTransform))
	{
		if (UMotionWarpingComponent* MotionWarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>())
		{
			MotionWarpingComp->AddOrUpdateWarpTargetFromTransform(MotionWarpingNameToApply, MotionWarpingTargetTransform);
			InstanceData.ActiveMotionWarpingName = MotionWarpingNameToApply;
			InstanceData.bAppliedMotionWarping = true;
		}
	}

	USkeletalMeshComponent* MeshComponent = Character->GetMesh();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		ComboMontagePlayTask::CleanupTaskState(
			Character,
			ComboMontage,
			false,
			InstanceData.bClearMotionWarpingOnExit,
			InstanceData.ActiveMotionWarpingName,
			false,
			false,
			InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	float MontageStartTime = 0.0f;
	if (!InstanceData.StartSection.IsNone())
	{
		const int32 SectionIndex = ComboMontage->GetSectionIndex(InstanceData.StartSection);
		if (SectionIndex != INDEX_NONE)
		{
			const float SectionStartTime = ComboMontage->GetAnimCompositeSection(SectionIndex).GetTime();
			MontageStartTime = FMath::Max(0.0f, SectionStartTime);

			// Avoid hard section jump: stop current montage first, then play from target section time.
			AnimInstance->Montage_Stop(ComboMontage->GetDefaultBlendOutTime(), ComboMontage);
		}
	}

	const float PlayedLength = AnimInstance->Montage_Play(
		ComboMontage,
		PlayRate,
		EMontagePlayReturnType::Duration,
		MontageStartTime,
		bShouldStopAllMontages);
	if (PlayedLength <= 0.0f)
	{
		ComboMontagePlayTask::CleanupTaskState(
			Character,
			ComboMontage,
			false,
			InstanceData.bClearMotionWarpingOnExit,
			InstanceData.ActiveMotionWarpingName,
			false,
			false,
			InstanceData);
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CachedAnimInstance = AnimInstance;
	InstanceData.PlayedMontageLength = PlayedLength;
	InstanceData.LastMontagePosition = AnimInstance->Montage_GetPosition(ComboMontage);
	InstanceData.bMontageStarted = true;

	if (bUsePendingChargeLevel)
	{
		if (UMHWAttackComponent* AttackComponent = ComboMontagePlayTask::GetAttackComponent(Character))
		{
			EMHWChargeLevel CurrentChargeLevel;
			if (AttackComponent->PeekPendingChargeLevel(CurrentChargeLevel))
			{
				const FGameplayTag ResolvedAttackSpecTag = ComboMontagePlayTask::ResolveAttackSpecTagForChargeLevel(
					CurrentChargeLevel,
					AttackSpecTag,
					ChargeLevel2AttackSpecTag,
					ChargeLevel3AttackSpecTag,
					OverchargedAttackSpecTag);

				if (ResolvedAttackSpecTag.IsValid() && ResolvedAttackSpecTag != AttackSpecTag)
				{
					AttackComponent->SetActiveAttackSpecTagOverride(ResolvedAttackSpecTag);
					InstanceData.bAppliedAttackSpecTagOverride = true;
				}
				else
				{
					AttackComponent->ClearActiveAttackSpecTagOverride();
				}
			}
		}
	}

	const TSharedPtr<FComboMontagePlayRuntimeState> RuntimeState = InstanceData.RuntimeState;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindWeakLambda(
		Character,
		[CharacterPtr = TWeakObjectPtr<AMHWCharacter>(Character),
		 RuntimeState,
		 bClearMotionWarpingOnExitValue = InstanceData.bClearMotionWarpingOnExit,
		 MotionWarpingNameValue = InstanceData.ActiveMotionWarpingName,
		 bClearPreInputOnExitValue = bClearPreInputOnExit,
		 bDisablePreInputOnExitValue = bDisablePreInputOnExit](UAnimMontage* Montage, bool bInterrupted)
	{
		if (RuntimeState.IsValid())
		{
			RuntimeState->bMontageEnded = true;
			RuntimeState->bMontageInterrupted = bInterrupted;
		}

		if (!bInterrupted || !RuntimeState.IsValid() || RuntimeState->bInterruptedCleanupDone)
		{
			return;
		}

		AMHWCharacter* CharacterInstance = CharacterPtr.Get();
		if (!CharacterInstance)
		{
			return;
		}

		RuntimeState->bInterruptedCleanupDone = true;
		ComboMontagePlayTask::CleanupCharacterState(
			CharacterInstance,
			bClearMotionWarpingOnExitValue,
			MotionWarpingNameValue,
			bClearPreInputOnExitValue,
			bDisablePreInputOnExitValue);
	});
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ComboMontage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindWeakLambda(
		Character,
		[RuntimeState](UAnimMontage* Montage, bool bInterrupted)
	{
		if (RuntimeState.IsValid())
		{
			RuntimeState->bMontageEnded = true;
			RuntimeState->bMontageInterrupted = bInterrupted;
		}
	});
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ComboMontage);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_ComboMontagePlay::Tick(FStateTreeExecutionContext& Context, const float /*DeltaTime*/) const
{
	FSTT_ComboMontagePlayInstanceData& InstanceData = Context.GetInstanceData<FSTT_ComboMontagePlayInstanceData>(*this);

	AMHWCharacter* Character = InstanceData.MHWCharacter;
	if (!Character || !ComboMontage)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAnimInstance* AnimInstance = InstanceData.CachedAnimInstance.Get();
	if (!AnimInstance)
	{
		USkeletalMeshComponent* MeshComponent = Character->GetMesh();
		AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
		InstanceData.CachedAnimInstance = AnimInstance;
	}
	if (!AnimInstance)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (AnimInstance->Montage_IsPlaying(ComboMontage))
	{
		InstanceData.LastMontagePosition = AnimInstance->Montage_GetPosition(ComboMontage);
		return EStateTreeRunStatus::Running;
	}

	if (!InstanceData.bMontageStarted)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.RuntimeState.IsValid() && InstanceData.RuntimeState->bMontageEnded)
	{
		return InstanceData.RuntimeState->bMontageInterrupted ? EStateTreeRunStatus::Failed : EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Succeeded;
}

void FSTT_ComboMontagePlay::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FSTT_ComboMontagePlayInstanceData& InstanceData = Context.GetInstanceData<FSTT_ComboMontagePlayInstanceData>(*this);
	AMHWCharacter* Character = InstanceData.MHWCharacter;
	const bool bHadAppliedAttackSpecTagOverride = InstanceData.bAppliedAttackSpecTagOverride;

	ComboMontagePlayTask::CleanupTaskState(
		Character,
		ComboMontage,
		bStopMontageOnExit,
		InstanceData.bClearMotionWarpingOnExit,
		InstanceData.ActiveMotionWarpingName,
		bClearPreInputOnExit,
		bDisablePreInputOnExit,
		InstanceData);

	if (bHadAppliedAttackSpecTagOverride)
	{
		if (UMHWAttackComponent* AttackComponent = ComboMontagePlayTask::GetAttackComponent(Character))
		{
			AttackComponent->ClearActiveAttackSpecTagOverride();
		}
	}

	if (bUsePendingChargeLevel && bClearPendingChargeLevelOnExit)
	{
		if (UMHWAttackComponent* AttackComponent = ComboMontagePlayTask::GetAttackComponent(Character))
		{
			AttackComponent->ClearPendingChargeLevel();
		}
	}
}
