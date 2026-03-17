// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_MeleeTrace.generated.h"

class UMHWEquipmentInstance;
/**
 * 
 */
UCLASS()
class PMHW_API UANS_MeleeTrace : public UAnimNotifyState
{
	GENERATED_BODY()
public:

	UANS_MeleeTrace();

	// ================== 暴露给蓝图配置的变量 ==================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Trace")
	TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass;
	
	// 武器上的插槽名称数组 (至少需要两个，比如剑柄和剑尖)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Trace")
	TArray<FName> Sockets;

	// 追踪的碰撞通道 (比如 WeaponTrace 或 Pawn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	// 是否显示调试射线
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Trace")
	bool bShowDebug;

	// 调试射线持续时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee Trace", meta = (EditCondition = "bShowDebug"))
	float DebugDuration;

	// ================== 重写核心生命周期函数 ==================

	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;

private:

	// ================== 内部状态变量 (不需要暴露) ==================

	// 存储上一帧所有插槽的世界坐标
	TArray<FVector> PreviousLocations;

	// 存储已经在本次挥击中受击的 Actor (防多段伤害)
	TArray<TWeakObjectPtr<AActor>> HitActors;

	// 缓存拥有这个 Mesh 的 Actor
	UPROPERTY()
	AActor* OwnerActor;
	
	UPROPERTY()
	USkeletalMeshComponent* WeaponMesh;
};
