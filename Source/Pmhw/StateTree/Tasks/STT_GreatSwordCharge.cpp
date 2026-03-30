#include "STT_GreatSwordCharge.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
// 必须包含 GAS 相关的头文件
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
		if (!Character || !Character->Implements<UMHWCharacterInterface>())
		{
			return nullptr;
		}

		const UMHWEquipmentManagerComponent* EquipmentManager = IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(Character);
		if (!EquipmentManager)
		{
			return nullptr;
		}

		UMHWEquipmentManagerComponent* MutableEquipmentManager = const_cast<UMHWEquipmentManagerComponent*>(EquipmentManager);
		TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass = Task.ChargeFeedback.WeaponInstanceClass;
		if (!WeaponInstanceClass)
		{
			WeaponInstanceClass = UMHWEquipmentInstance::StaticClass();
		}

		if (UMHWEquipmentInstance* EquipmentInstance = MutableEquipmentManager->GetFirstInstanceOfType(WeaponInstanceClass))
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
			if (USkeletalMeshComponent* WeaponMesh = ResolveChargeWeaponMesh(Task, Character))
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
				if (USkeletalMeshComponent* WeaponMesh = ResolveChargeWeaponMesh(Task, Character))
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

EStateTreeRunStatus FSTT_GreatSwordCharge::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;
	if (!Character) return EStateTreeRunStatus::Failed;
	
	Character->StopAnimMontage();

	if (AMHWCharacter* MHWCharacter = Cast<AMHWCharacter>(Character))
	{
		// Clear stale request so the next montage won't consume an old target by mistake.
		MHWCharacter->ClearPendingMotionWarpTarget();
	}

	InstanceData.CurrentTurnYaw = 0.0f;
	InstanceData.CurrentChargeTime = 0.0f;
	InstanceData.bOwnsSpecificChargeTag = false;
	GreatSwordChargeTask::ClearActiveChargeVFX(InstanceData);

	// 【添加具体的蓄力 Tag】
	// 如果你填的是 State.Combat.Charging.Level2，角色身上就会有这个 Tag。
	// 根据 GAS 规则，拥有子 Tag 等同于拥有父 Tag (State.Combat.Charging)。
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
	{
		// 只在本任务需要时添加，并记录所有权，退出时只清理自己加的标签
		if (SpecificChargeTag.IsValid() && !ASC->HasMatchingGameplayTag(SpecificChargeTag))
		{
			ASC->AddLooseGameplayTag(SpecificChargeTag);
			InstanceData.bOwnsSpecificChargeTag = true;
		}
	}

	if (Character->Implements<UMHWCharacterInterface>())
	{
		if (UMHWAttackComponent* AttackComponent = IMHWCharacterInterface::Execute_GetAttackComponent(Character))
		{
			// A new charge attempt supersedes any unresolved previous charge result.
			AttackComponent->ClearPendingChargeLevel();
		}
	}

	GreatSwordChargeTask::UpdateChargeLevelVFX(*this, InstanceData, Character);

	return EStateTreeRunStatus::Running;
}

// ... Tick 函数中的旋转代码保持完全不变 ...
EStateTreeRunStatus FSTT_GreatSwordCharge::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	
	// 1. 累加蓄力时间
	InstanceData.CurrentChargeTime += DeltaTime;
	ACharacter* Character = InstanceData.Character;

	if (!Character) return EStateTreeRunStatus::Failed;

	GreatSwordChargeTask::UpdateChargeLevelVFX(*this, InstanceData, Character);

	if (InstanceData.CurrentChargeTime >= MaxChargeDuration)
	{
		return EStateTreeRunStatus::Succeeded; 
	}

	// ===================== 核心角度计算 (基于真实物理意图) =====================
	
	// 【终极解法】：获取移动组件的当前加速度！这是玩家真实意图在物理层的最终映射。
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp) return EStateTreeRunStatus::Failed;

	FVector CurrentAcceleration = MoveComp->GetCurrentAcceleration();

	// 判断加速度是否大于一个极小值 (防止摇杆漂移导致原地抽搐)
	if (CurrentAcceleration.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		// 1. 算出加速度指示的绝对世界旋转方向 (即玩家想往哪里走)
		FRotator TargetRot = CurrentAcceleration.Rotation();
		
		// 2. 获取角色当前的绝对世界旋转 (胶囊体起手时是被锁死的)
		FRotator CharRot = Character->GetActorRotation();
		
		// 3. 计算相对差值：玩家推的方向，相对于角色正前方偏移了多少度？
		// GetNormalized() 会自动把角度转换并限制到 -180 到 180 之间
		float TargetYaw = (TargetRot - CharRot).GetNormalized().Yaw;
		
		TargetYaw = FMath::Clamp(TargetYaw, -MaxTurnAngle, MaxTurnAngle);

		// 4. 【平滑插值】：模拟大剑沉重转身的力量感
		// 注意：不要在这里写 Clamp，让 BlendSpace 自动去卡死极限量 (-45 到 45)。
		// 这样即便玩家往身后推 (-180度)，数值也会向 -180 狂奔，
		// 但动画蓝图会极其自然地死死卡在 -45度的极限扭腰动作上！
		InstanceData.CurrentTurnYaw = FMath::FInterpTo(InstanceData.CurrentTurnYaw, TargetYaw, DeltaTime, TurnSpeed);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				101, // 使用固定 ID 101，确保每帧覆盖更新，而不是满屏刷字幕
				0.0f, // 0.0f 表示只显示一帧
				FColor::Yellow, 
				FString::Printf(TEXT("[推摇杆中] 目标偏角: %.1f | 当前扭曲值(CurrentTurnYaw): %.1f"), TargetYaw, InstanceData.CurrentTurnYaw)
			);		}
	}
	// 如果玩家松开了摇杆 (CurrentAcceleration 几乎为 0)，上面的 if 不执行。
	// CurrentTurnYaw 就会像钉子一样死死钉在最后那一刻的角度，完美实现“维持当前姿势继续蓄力”！

	// ===================== 将角度传给 AnimBP =====================
	
	if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			if (AnimInstance->Implements<UCombatAnimInterface>())
			{
				ICombatAnimInterface::Execute_SetChargeTurnYaw(AnimInstance, InstanceData.CurrentTurnYaw);
			}
		}
	}
	
	return EStateTreeRunStatus::Running;
}

void FSTT_GreatSwordCharge::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;

	if (Character)
	{
		// 【移除这个具体的蓄力 Tag】
		// 必须保证退出时清理干净，否则角色的动画会被卡死在蓄力状态里
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character))
		{
			if (InstanceData.bOwnsSpecificChargeTag && SpecificChargeTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(SpecificChargeTag);
			}
		}

		if (FMath::Abs(InstanceData.CurrentTurnYaw) > KINDA_SMALL_NUMBER)
		{
			if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
			{
				if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
				{
					if (AnimInstance->Implements<UCombatAnimInterface>())
					{
						ICombatAnimInterface::Execute_SetChargeTurnYaw(AnimInstance, 0.0f);
					}
				}
			}
		}

		if (AMHWCharacter* MHWCharacter = Cast<AMHWCharacter>(Character))
		{
			// Always clear old request first to keep one-shot semantics.
			MHWCharacter->ClearPendingMotionWarpTarget();

			// a. 计算出我们期望下砸的最终朝向
			FRotator CurrentActorRot = Character->GetActorRotation();
			FRotator TargetRot = CurrentActorRot;
			TargetRot.Yaw += InstanceData.CurrentTurnYaw;
			TargetRot.Normalize();

			// b. 构造一个目标 Transform (位置保持不变，只转朝向)
			FTransform TargetTransform;
			TargetTransform.SetLocation(Character->GetActorLocation());
			TargetTransform.SetRotation(TargetRot.Quaternion());

			// c. 将目标写入 Character 缓存，由下游 Montage 任务统一执行 MotionWarping。
			// "ChargeSmashTarget" 是我们随便起的一个名字，记好它，等会要在 Montage 里填！
			if (bUseMotiongWarping)
			{
				MHWCharacter->SetPendingMotionWarpTarget(GreatSwordChargeTask::ChargeSmashTargetName, TargetTransform);
			}
		}

		if (Character->Implements<UMHWCharacterInterface>())
		{
			if (UMHWAttackComponent* AttackComponent = IMHWCharacterInterface::Execute_GetAttackComponent(Character))
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
	}

	InstanceData.CurrentChargeTime = 0.0f;
	InstanceData.bOwnsSpecificChargeTag = false;
	GreatSwordChargeTask::ClearActiveChargeVFX(InstanceData);
}
