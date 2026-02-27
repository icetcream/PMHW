#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "MHWStateTreeTask_PlayMontage.generated.h"

// ==========================================
// 1. 数据结构 (输入与参数)
// ==========================================
USTRUCT()
struct FMHWStateTreeTask_PlayMontage_InstanceData
{
	GENERATED_BODY()

	// [输入] 绑定的主角
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AActor> ContextActor; 

	// [参数] 要播放的蒙太奇动画
	UPROPERTY(EditAnywhere, Category = "Parameter")
	UAnimMontage* MontageToPlay = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FName SectionName = NAME_None;

	// [参数] 播放速度
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float PlayRate = 1.0f;
};

// ==========================================
// 2. 逻辑结构 (真正的 Task)
// ==========================================
USTRUCT(meta = (DisplayName = "STT Play Montage (C++)", Category = "MHW|Animation"))
struct PMHW_API FMHWStateTreeTask_PlayMontage : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMHWStateTreeTask_PlayMontage_InstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// ---------------------------------------------------------
	// 第一步：进入状态时 -> 开始播放，并告诉系统“等待我”
	// ---------------------------------------------------------
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		ACharacter* Character = Cast<ACharacter>(InstanceData.ContextActor);

		if (Character && InstanceData.MontageToPlay)
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				// 播放蒙太奇
				AnimInstance->Montage_Play(InstanceData.MontageToPlay, InstanceData.PlayRate);
				
				if (InstanceData.SectionName != NAME_None)
				{
					AnimInstance->Montage_JumpToSection(InstanceData.SectionName, InstanceData.MontageToPlay);
				}
				
				// 【极其关键】返回 Running！告诉状态树：别急着走，停在这里等我！
				return EStateTreeRunStatus::Running; 
			}
		}
		return EStateTreeRunStatus::Failed;
	}

	// ---------------------------------------------------------
	// 第二步：每帧检查 -> 动画播完了吗？
	// ---------------------------------------------------------
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override
	{
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
		ACharacter* Character = Cast<ACharacter>(InstanceData.ContextActor);

		if (Character && InstanceData.MontageToPlay)
		{
			if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
			{
				// 检查这个蒙太奇是否还在播放？
				if (!AnimInstance->Montage_IsPlaying(InstanceData.MontageToPlay))
				{
					// 如果没在播放了，说明播完了！向上级汇报成功！
					return EStateTreeRunStatus::Succeeded;
				}
				if (InstanceData.SectionName != NAME_None)
				{
					FName CurrentSection = AnimInstance->Montage_GetCurrentSection(InstanceData.MontageToPlay);
				
					// 如果当前正在播的段落名字，已经不是你要求的那个段落了（比如滑到 Default 去了）
					// 并且蒙太奇还在播，说明这一段已经播完了！
					if (CurrentSection != InstanceData.SectionName)
					{
						// 掐断它，并汇报成功
						AnimInstance->Montage_Stop(0.1f, InstanceData.MontageToPlay);
						return EStateTreeRunStatus::Succeeded;
					}
				}
				// 还在播，继续维持 Running 状态
				return EStateTreeRunStatus::Running;
			}
		}
		return EStateTreeRunStatus::Failed;
	}

	// ---------------------------------------------------------
	// 第三步：退出状态时 -> 意外打断的善后处理
	// ---------------------------------------------------------
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override
	{
		// 如果因为某些原因（比如被怪物打飞），状态树强制切走了，但动画还没播完
		if (Transition.CurrentRunStatus == EStateTreeRunStatus::Running)
		{
			FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
			ACharacter* Character = Cast<ACharacter>(InstanceData.ContextActor);

			if (Character && InstanceData.MontageToPlay)
			{
				if (UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance())
				{
					// 强行停止这个蒙太奇，防止动作残留
					AnimInstance->Montage_Stop(0.2f, InstanceData.MontageToPlay);
				}
			}
		}
	}
};