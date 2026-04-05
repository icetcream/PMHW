#include "STT_GreatSwordCharge.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/MHWAttackComponent.h"
#include "Character/MHWCharacter.h"
#include "Components/AudioComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MHWGameplayTags.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/CombatAnimInterface.h"
#include "Interface/MHWCharacterInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

namespace GreatSwordChargeTask
{
	static const FName ChargeSmashTargetName(TEXT("ChargeSmashTarget"));

	static UAbilitySystemComponent* GetASC(AActor* Actor)
	{
		return Actor ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor) : nullptr;
	}

	static UMHWAttackComponent* GetAttackComponent(AActor* Actor)
	{
		if (!Actor || !Actor->Implements<UMHWCharacterInterface>())
		{
			return nullptr;
		}

		return IMHWCharacterInterface::Execute_GetAttackComponent(Actor);
	}

	static UMHWEquipmentManagerComponent* GetEquipmentManager(AActor* Actor)
	{
		if (!Actor || !Actor->Implements<UMHWCharacterInterface>())
		{
			return nullptr;
		}

		return const_cast<UMHWEquipmentManagerComponent*>(IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(Actor));
	}

	static UAnimInstance* GetChargeAnimInstance(ACharacter* Character)
	{
		if (!Character)
		{
			return nullptr;
		}

		USkeletalMeshComponent* MeshComponent = Character->GetMesh();
		return MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	}

	static void SetChargeTurnYaw(ACharacter* Character, const float ChargeTurnYaw)
	{
		UAnimInstance* AnimInstance = GetChargeAnimInstance(Character);
		if (!AnimInstance || !AnimInstance->Implements<UCombatAnimInterface>())
		{
			return;
		}

		ICombatAnimInterface::Execute_SetChargeTurnYaw(AnimInstance, ChargeTurnYaw);
	}

	static void ClearOwnedChargeTag(ACharacter* Character, const FGameplayTag& ChargeTag, const bool bOwnsSpecificChargeTag)
	{
		UAbilitySystemComponent* ASC = GetASC(Character);
		if (!ASC || !bOwnsSpecificChargeTag || !ChargeTag.IsValid())
		{
			return;
		}

		ASC->RemoveLooseGameplayTag(ChargeTag);
	}

	static void UpdatePendingMotionWarpTarget(AMHWCharacter* Character, const bool bUseMotionWarping, const float TurnYaw)
	{
		if (!Character)
		{
			return;
		}

		// Charge warp targets are one-shot data. Clear any leftover request before optionally
		// publishing the new facing target for the follow-up smash montage.
		Character->ClearPendingMotionWarpTarget();

		if (!bUseMotionWarping)
		{
			return;
		}

		FRotator TargetRot = Character->GetActorRotation();
		TargetRot.Yaw += TurnYaw;
		TargetRot.Normalize();

		FTransform TargetTransform = FTransform::Identity;
		TargetTransform.SetLocation(Character->GetActorLocation());
		TargetTransform.SetRotation(TargetRot.Quaternion());

		Character->SetPendingMotionWarpTarget(ChargeSmashTargetName, TargetTransform);
	}

	static void DebugChargeTurnYaw(const float TargetYaw, const float CurrentTurnYaw)
	{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				101,
				0.0f,
				FColor::Yellow,
				FString::Printf(TEXT("[Charge] TargetYaw: %.1f | CurrentTurnYaw: %.1f"), TargetYaw, CurrentTurnYaw));
		}
#endif
	}

	static float ResolveNormalizedChargeRatio(const FSTT_GreatSwordCharge& Task, const float ChargeTime)
	{
		if (Task.MaxChargeDuration <= KINDA_SMALL_NUMBER)
		{
			return 1.0f;
		}

		return FMath::Clamp(ChargeTime / Task.MaxChargeDuration, 0.0f, 1.0f);
	}

	static bool ResolvePendingChargeLevel(const FSTT_GreatSwordCharge& Task, const float ChargeTime, EMHWChargeLevel& OutChargeLevel)
	{
		const float NormalizedChargeRatio = ResolveNormalizedChargeRatio(Task, ChargeTime);
		const float EffectiveChargeLevel2Threshold = FMath::Clamp(Task.ChargeLevel2Threshold, 0.0f, 1.0f);
		const float EffectiveChargeLevel3Threshold = FMath::Clamp(Task.ChargeLevel3Threshold, EffectiveChargeLevel2Threshold, 1.0f);
		const float EffectiveOverchargeThreshold = FMath::Clamp(Task.OverchargeThreshold, EffectiveChargeLevel3Threshold, 1.0f);

		if (NormalizedChargeRatio >= EffectiveOverchargeThreshold)
		{
			OutChargeLevel = EMHWChargeLevel::Overcharged;
			return true;
		}

		if (NormalizedChargeRatio >= EffectiveChargeLevel3Threshold)
		{
			OutChargeLevel = EMHWChargeLevel::Level3;
			return true;
		}

		if (NormalizedChargeRatio >= EffectiveChargeLevel2Threshold)
		{
			OutChargeLevel = EMHWChargeLevel::Level2;
			return true;
		}

		OutChargeLevel = EMHWChargeLevel::Level1;
		return true;
	}

	static const FMHWChargeStageFeedback& ResolveChargeStageFeedback(const FMHWChargeFeedbackConfig& Config, const EMHWChargeLevel ChargeLevel)
	{
		switch (ChargeLevel)
		{
		case EMHWChargeLevel::Overcharged:
			if (Config.OverchargedFeedback.CharacterVFX || Config.OverchargedFeedback.WeaponVFX || Config.OverchargedFeedback.Sound)
			{
				return Config.OverchargedFeedback;
			}
			if (Config.Level3Feedback.CharacterVFX || Config.Level3Feedback.WeaponVFX || Config.Level3Feedback.Sound)
			{
				return Config.Level3Feedback;
			}
			if (Config.Level2Feedback.CharacterVFX || Config.Level2Feedback.WeaponVFX || Config.Level2Feedback.Sound)
			{
				return Config.Level2Feedback;
			}
			return Config.Level1Feedback;
		case EMHWChargeLevel::Level3:
			if (Config.Level3Feedback.CharacterVFX || Config.Level3Feedback.WeaponVFX || Config.Level3Feedback.Sound)
			{
				return Config.Level3Feedback;
			}
			if (Config.Level2Feedback.CharacterVFX || Config.Level2Feedback.WeaponVFX || Config.Level2Feedback.Sound)
			{
				return Config.Level2Feedback;
			}
			return Config.Level1Feedback;
		case EMHWChargeLevel::Level2:
			if (Config.Level2Feedback.CharacterVFX || Config.Level2Feedback.WeaponVFX || Config.Level2Feedback.Sound)
			{
				return Config.Level2Feedback;
			}
			return Config.Level1Feedback;
		case EMHWChargeLevel::Level1:
		default:
			return Config.Level1Feedback;
		}
	}

	static USkeletalMeshComponent* ResolveChargeWeaponMesh(const FSTT_GreatSwordCharge& Task, ACharacter* Character)
	{
		UMHWEquipmentManagerComponent* EquipmentManager = GetEquipmentManager(Character);
		if (!EquipmentManager)
		{
			return nullptr;
		}

		TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass = Task.ChargeFeedback.WeaponInstanceClass;
		if (!WeaponInstanceClass)
		{
			WeaponInstanceClass = UMHWEquipmentInstance::StaticClass();
		}

		if (UMHWEquipmentInstance* EquipmentInstance = EquipmentManager->GetFirstInstanceOfType(WeaponInstanceClass))
		{
			if (AActor* WeaponActor = EquipmentInstance->GetSpawnedActor())
			{
				return WeaponActor->FindComponentByClass<USkeletalMeshComponent>();
			}
		}

		return nullptr;
	}

	static void ClearActiveChargeVFX(FSTT_GreatSwordChargeInstanceData& InstanceData)
	{
		if (UNiagaraComponent* ActiveComponent = InstanceData.ActiveCharacterChargeVFXComponent)
		{
			ActiveComponent->DeactivateImmediate();
			ActiveComponent->ReleaseToPool();
		}

		if (UNiagaraComponent* ActiveComponent = InstanceData.ActiveWeaponChargeVFXComponent)
		{
			ActiveComponent->DeactivateImmediate();
			ActiveComponent->ReleaseToPool();
		}

		if (UAudioComponent* ActiveAudioComponent = InstanceData.ActiveChargeAudioComponent)
		{
			ActiveAudioComponent->Stop();
			ActiveAudioComponent->DestroyComponent();
		}

		if (InstanceData.Character)
		{
			if (USkeletalMeshComponent* MeshComponent = InstanceData.Character->GetMesh())
			{
				const FMHWChargeDynamicMaterialState& MaterialState = InstanceData.ActiveCharacterOverlayMaterialState;
				if (MaterialState.OriginalOverlayMaterial || MaterialState.AppliedBaseMaterial || MaterialState.DynamicMaterial)
				{
					MeshComponent->SetOverlayMaterial(MaterialState.OriginalOverlayMaterial);
				}
			}
		}

		InstanceData.ActiveCharacterChargeVFXComponent = nullptr;
		InstanceData.ActiveWeaponChargeVFXComponent = nullptr;
		InstanceData.ActiveChargeAudioComponent = nullptr;
		InstanceData.ActiveCharacterOverlayMaterialState = FMHWChargeDynamicMaterialState();
		InstanceData.bHasActiveChargeFeedbackLevel = false;
		InstanceData.ActiveChargeFeedbackLevel = 0;
	}

	static void ResetChargeRuntimeState(FSTT_GreatSwordChargeInstanceData& InstanceData)
	{
		InstanceData.CurrentTurnYaw = 0.0f;
		InstanceData.CurrentChargeTime = 0.0f;
		InstanceData.bOwnsSpecificChargeTag = false;
		ClearActiveChargeVFX(InstanceData);
	}

	static UMaterialInstanceDynamic* FindOrCreateChargeDynamicMaterial(
		FSTT_GreatSwordChargeInstanceData& InstanceData,
		USkeletalMeshComponent* MeshComponent,
		UMaterialInterface* DesiredBaseMaterial)
	{
		if (!MeshComponent)
		{
			return nullptr;
		}

		UMaterialInterface* EffectiveBaseMaterial = DesiredBaseMaterial ? DesiredBaseMaterial : MeshComponent->GetOverlayMaterial();
		if (!EffectiveBaseMaterial)
		{
			return nullptr;
		}

		FMHWChargeDynamicMaterialState& MaterialState = InstanceData.ActiveCharacterOverlayMaterialState;
		if (MaterialState.DynamicMaterial && MaterialState.AppliedBaseMaterial == EffectiveBaseMaterial)
		{
			return MaterialState.DynamicMaterial;
		}

		if (!MaterialState.OriginalOverlayMaterial && !MaterialState.AppliedBaseMaterial && !MaterialState.DynamicMaterial)
		{
			MaterialState.OriginalOverlayMaterial = MeshComponent->GetOverlayMaterial();
		}

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(EffectiveBaseMaterial, MeshComponent);
		if (!DynamicMaterial)
		{
			return nullptr;
		}

		MeshComponent->SetOverlayMaterial(DynamicMaterial);
		MaterialState.AppliedBaseMaterial = EffectiveBaseMaterial;
		MaterialState.DynamicMaterial = DynamicMaterial;
		return MaterialState.DynamicMaterial;
	}

	static void ApplyChargeCharacterMaterialFeedback(
		FSTT_GreatSwordChargeInstanceData& InstanceData,
		USkeletalMeshComponent* MeshComponent,
		const FMHWChargeStageFeedback& Feedback)
	{
		if (!MeshComponent)
		{
			return;
		}

		for (const FMHWChargeCharacterMaterialFeedback& MaterialFeedback : Feedback.CharacterMaterialFeedbacks)
		{
			UMaterialInstanceDynamic* DynamicMaterial = FindOrCreateChargeDynamicMaterial(
				InstanceData,
				MeshComponent,
				MaterialFeedback.OverrideMaterial);
			if (!DynamicMaterial)
			{
				continue;
			}

			for (const FMHWChargeScalarMaterialParameter& ScalarParameter : MaterialFeedback.ScalarParameters)
			{
				if (ScalarParameter.ParameterName != NAME_None)
				{
					DynamicMaterial->SetScalarParameterValue(ScalarParameter.ParameterName, ScalarParameter.Value);
				}
			}

			for (const FMHWChargeVectorMaterialParameter& VectorParameter : MaterialFeedback.VectorParameters)
			{
				if (VectorParameter.ParameterName != NAME_None)
				{
					DynamicMaterial->SetVectorParameterValue(VectorParameter.ParameterName, VectorParameter.Value);
				}
			}
		}
	}

	static void UpdateChargeLevelVFX(const FSTT_GreatSwordCharge& Task, FSTT_GreatSwordChargeInstanceData& InstanceData, ACharacter* Character)
	{
		if (!Character)
		{
			ClearActiveChargeVFX(InstanceData);
			return;
		}

		EMHWChargeLevel CurrentChargeLevel = EMHWChargeLevel::Level1;
		ResolvePendingChargeLevel(Task, InstanceData.CurrentChargeTime, CurrentChargeLevel);

		if (InstanceData.bHasActiveChargeFeedbackLevel && InstanceData.ActiveChargeFeedbackLevel == static_cast<uint8>(CurrentChargeLevel))
		{
			return;
		}

		ClearActiveChargeVFX(InstanceData);
		const FMHWChargeStageFeedback& Feedback = ResolveChargeStageFeedback(Task.ChargeFeedback, CurrentChargeLevel);

		USkeletalMeshComponent* MeshComponent = Character->GetMesh();
		if (!MeshComponent)
		{
			return;
		}

		USkeletalMeshComponent* WeaponMesh = nullptr;
		const bool bNeedsWeaponMesh = Feedback.WeaponVFX || (Feedback.Sound && Task.ChargeFeedback.bAttachSoundToWeapon);
		if (bNeedsWeaponMesh)
		{
			WeaponMesh = ResolveChargeWeaponMesh(Task, Character);
		}

		if (Feedback.CharacterVFX)
		{
			if (UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				Feedback.CharacterVFX,
				MeshComponent,
				Task.ChargeFeedback.CharacterAttachSocketName,
				Task.ChargeFeedback.CharacterLocationOffset,
				Task.ChargeFeedback.CharacterRotationOffset,
				EAttachLocation::KeepRelativeOffset,
				false,
				true,
				ENCPoolMethod::ManualRelease,
				true))
			{
				SpawnedComponent->SetRelativeScale3D(Task.ChargeFeedback.CharacterScale);
				InstanceData.ActiveCharacterChargeVFXComponent = SpawnedComponent;
			}
		}

		ApplyChargeCharacterMaterialFeedback(InstanceData, MeshComponent, Feedback);

		if (Feedback.WeaponVFX)
		{
			if (WeaponMesh)
			{
				if (UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
					Feedback.WeaponVFX,
					WeaponMesh,
					Task.ChargeFeedback.WeaponAttachSocketName,
					Task.ChargeFeedback.WeaponLocationOffset,
					Task.ChargeFeedback.WeaponRotationOffset,
					EAttachLocation::KeepRelativeOffset,
					false,
					true,
					ENCPoolMethod::ManualRelease,
					true))
				{
					SpawnedComponent->SetRelativeScale3D(Task.ChargeFeedback.WeaponScale);
					InstanceData.ActiveWeaponChargeVFXComponent = SpawnedComponent;
				}
			}
		}

		if (Feedback.Sound)
		{
			USceneComponent* SoundAttachComponent = MeshComponent;
			if (Task.ChargeFeedback.bAttachSoundToWeapon)
			{
				if (WeaponMesh)
				{
					SoundAttachComponent = WeaponMesh;
				}
			}

			if (SoundAttachComponent)
			{
				InstanceData.ActiveChargeAudioComponent = UGameplayStatics::SpawnSoundAttached(
					Feedback.Sound,
					SoundAttachComponent,
					Task.ChargeFeedback.SoundAttachSocketName,
					Task.ChargeFeedback.SoundLocationOffset,
					Task.ChargeFeedback.SoundRotationOffset,
					EAttachLocation::KeepRelativeOffset,
					false,
					1.0f,
					1.0f,
					0.0f,
					nullptr,
					nullptr,
					true);
			}
		}

		InstanceData.bHasActiveChargeFeedbackLevel = true;
		InstanceData.ActiveChargeFeedbackLevel = static_cast<uint8>(CurrentChargeLevel);
	}
}

EStateTreeRunStatus FSTT_GreatSwordCharge::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;
	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}

	Character->StopAnimMontage();

	if (AMHWCharacter* MHWCharacter = Cast<AMHWCharacter>(Character))
	{
		MHWCharacter->ClearPendingMotionWarpTarget();
	}

	GreatSwordChargeTask::ResetChargeRuntimeState(InstanceData);

	// The task only removes tags it added itself, so external charge states are left alone.
	if (UAbilitySystemComponent* ASC = GreatSwordChargeTask::GetASC(Character))
	{
		if (SpecificChargeTag.IsValid() && !ASC->HasMatchingGameplayTag(SpecificChargeTag))
		{
			ASC->AddLooseGameplayTag(SpecificChargeTag);
			InstanceData.bOwnsSpecificChargeTag = true;
		}
	}

	if (UMHWAttackComponent* AttackComponent = GreatSwordChargeTask::GetAttackComponent(Character))
	{
		// A new charge attempt supersedes any unresolved previous charge result.
		AttackComponent->ClearPendingChargeLevel();
	}

	GreatSwordChargeTask::UpdateChargeLevelVFX(*this, InstanceData, Character);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSTT_GreatSwordCharge::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	InstanceData.CurrentChargeTime += DeltaTime;
	ACharacter* Character = InstanceData.Character;

	if (!Character)
	{
		return EStateTreeRunStatus::Failed;
	}

	GreatSwordChargeTask::UpdateChargeLevelVFX(*this, InstanceData, Character);

	if (InstanceData.CurrentChargeTime >= MaxChargeDuration)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector CurrentAcceleration = MoveComp->GetCurrentAcceleration();

	if (CurrentAcceleration.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		const FRotator TargetRot = CurrentAcceleration.Rotation();
		const FRotator CharRot = Character->GetActorRotation();
		float TargetYaw = (TargetRot - CharRot).GetNormalized().Yaw;
		TargetYaw = FMath::Clamp(TargetYaw, -MaxTurnAngle, MaxTurnAngle);

		// Turn toward the player input while keeping the charge pose constrained by the task limits.
		InstanceData.CurrentTurnYaw = FMath::FInterpTo(InstanceData.CurrentTurnYaw, TargetYaw, DeltaTime, TurnSpeed);
		GreatSwordChargeTask::DebugChargeTurnYaw(TargetYaw, InstanceData.CurrentTurnYaw);
	}

	GreatSwordChargeTask::SetChargeTurnYaw(Character, InstanceData.CurrentTurnYaw);

	return EStateTreeRunStatus::Running;
}

void FSTT_GreatSwordCharge::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& /*Transition*/) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;

	if (Character)
	{
		GreatSwordChargeTask::ClearOwnedChargeTag(Character, SpecificChargeTag, InstanceData.bOwnsSpecificChargeTag);

		if (FMath::Abs(InstanceData.CurrentTurnYaw) > KINDA_SMALL_NUMBER)
		{
			GreatSwordChargeTask::SetChargeTurnYaw(Character, 0.0f);
		}

		if (AMHWCharacter* MHWCharacter = Cast<AMHWCharacter>(Character))
		{
			GreatSwordChargeTask::UpdatePendingMotionWarpTarget(MHWCharacter, bUseMotiongWarping, InstanceData.CurrentTurnYaw);
		}

		if (UMHWAttackComponent* AttackComponent = GreatSwordChargeTask::GetAttackComponent(Character))
		{
			EMHWChargeLevel PendingChargeLevel;
			if (GreatSwordChargeTask::ResolvePendingChargeLevel(*this, InstanceData.CurrentChargeTime, PendingChargeLevel))
			{
				AttackComponent->SetPendingChargeLevel(PendingChargeLevel);
			}
			else
			{
				AttackComponent->ClearPendingChargeLevel();
			}
		}
	}

	GreatSwordChargeTask::ResetChargeRuntimeState(InstanceData);
}
