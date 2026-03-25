#include "STT_GreatSwordCharge.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
// 必须包含 GAS 相关的头文件
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MHWGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interface/CombatAnimInterface.h"

namespace GreatSwordChargeTask
{
	static const FName ChargeSmashTargetName(TEXT("ChargeSmashTarget"));
}

EStateTreeRunStatus FSTT_GreatSwordCharge::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	ACharacter* Character = InstanceData.Character;
	if (!Character) return EStateTreeRunStatus::Failed;
	
	Character->StopAnimMontage();

	if (UMotionWarpingComponent* WarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>())
	{
		WarpingComp->RemoveWarpTarget(GreatSwordChargeTask::ChargeSmashTargetName);
	}

	InstanceData.CurrentTurnYaw = 0.0f;
	InstanceData.CurrentChargeTime = 0.0f;
	InstanceData.bOwnsSpecificChargeTag = false;

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

	return EStateTreeRunStatus::Running;
}

// ... Tick 函数中的旋转代码保持完全不变 ...
EStateTreeRunStatus FSTT_GreatSwordCharge::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTT_GreatSwordChargeInstanceData& InstanceData = Context.GetInstanceData<FSTT_GreatSwordChargeInstanceData>(*this);
	
	// 1. 累加蓄力时间
	InstanceData.CurrentChargeTime += DeltaTime;
	if (InstanceData.CurrentChargeTime >= MaxChargeDuration)
	{
		return EStateTreeRunStatus::Succeeded; 
	}
	ACharacter* Character = InstanceData.Character;

	if (!Character) return EStateTreeRunStatus::Failed;

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

		if (UMotionWarpingComponent* WarpingComp = Character->FindComponentByClass<UMotionWarpingComponent>())
		{
			WarpingComp->RemoveWarpTarget(GreatSwordChargeTask::ChargeSmashTargetName);

			// a. 计算出我们期望下砸的最终朝向
			FRotator CurrentActorRot = Character->GetActorRotation();
			FRotator TargetRot = CurrentActorRot;
			TargetRot.Yaw += InstanceData.CurrentTurnYaw;
			TargetRot.Normalize();

			// b. 构造一个目标 Transform (位置保持不变，只转朝向)
			FTransform TargetTransform;
			TargetTransform.SetLocation(Character->GetActorLocation());
			TargetTransform.SetRotation(TargetRot.Quaternion());

			// c. 【神之一手】将这个目标喂给 Motion Warping 组件！
			// "ChargeSmashTarget" 是我们随便起的一个名字，记好它，等会要在 Montage 里填！
			if (bUseMotiongWarping)
			{
				WarpingComp->AddOrUpdateWarpTargetFromTransform(GreatSwordChargeTask::ChargeSmashTargetName, TargetTransform);
			}
		}
	}

	InstanceData.CurrentChargeTime = 0.0f;
	InstanceData.bOwnsSpecificChargeTag = false;
}
