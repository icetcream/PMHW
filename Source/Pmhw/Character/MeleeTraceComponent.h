// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "Character/MHWMeleeHitVFXTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MeleeTraceComponent.generated.h"
class UMHWHitStopData;
class UMHWEquipmentInstance;
class UAnimInstance;
class UAnimMontage;
class USkeletalMeshComponent;
struct FMHWAttackDataRow;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeleeHitSignature, const FHitResult&, HitResult);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMeleeTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMeleeTraceComponent();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// ======================== 核心配置参数 ========================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox Trace")
	TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox Trace")
	bool bShowDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage", meta = (ClampMin = "0.0"))
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bApplyDamageOnHit = true;

	// 打中怪物时触发的事件，蓝图可以绑定它来播放特效或扣血
	UPROPERTY(BlueprintAssignable, Category = "Hitbox Trace")
	FOnMeleeHitSignature OnMeleeHit;

	// ======================== API (供 ANS 调用) ========================

	/**
	 * 开始记录轨迹并进行碰撞检测
	 * @param InSockets 武器上用于追踪的插槽名称数组 (建议从剑柄到剑尖多放几个)
	 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Trace")
	void StartTrace(FName InBaseSocket, const TArray<FName>& InTraceSockets);

	UFUNCTION(BlueprintCallable, Category = "Hitbox Trace")
	void StopTrace();

	bool ResolveTraceConfig(const FName& OverrideBaseSocket, const TArray<FName>& OverrideTraceSockets, FName& OutBaseSocket, TArray<FName>& OutTraceSockets);

	const FMHWAttackDataRow* FindAttackDataRowBySpecTag(const FGameplayTag& AttackSpecTag);

	const UMHWHitStopData* ResolveDefaultHitStopData(float MotionValue, bool bIsFinisher);

	float CalculateHitStopStrengthMultiplier(float HitzoneValue, bool bIsSleepHit, bool bIsFinisher);

	void SetHitStopConfig(const UMHWHitStopData* InHitStopData, float InStrengthMultiplier = 1.0f);

	void ClearHitStopConfig();

	void SetCachedPhysicalDamageSpec(const FMHWPhysicalDamageSpec& InDamageSpec);

	void ClearCachedPhysicalDamageSpec();

	bool HasCachedPhysicalDamageSpec() const;

	UFUNCTION(BlueprintCallable, Category = "Hitbox Trace|VFX")
	void SetHitVFXSpec(const FMHWMeleeHitVFXSpec& InHitVFXSpec);

	UFUNCTION(BlueprintCallable, Category = "Hitbox Trace|VFX")
	void ClearHitVFXSpec();

private:
	enum class EHitstopRuntimePhase : uint8
	{
		None,
		Freeze,
		Recover
	};

	// 【新增】：动态获取并缓存武器 Mesh
	USkeletalMeshComponent* GetWeaponMesh();

	bool bIsTracing = false;

	UPROPERTY()
	USkeletalMeshComponent* OwnerMeshComp;

	FName BaseSocketName;
	TArray<FName> ActiveTraceSockets;

	// 武器基底在上一帧的世界 Transform
	FTransform PreviousBaseTransform;
	// 所有追踪点相对于 BaseSocket 的【固定局部坐标偏移】
	TArray<FVector> TraceSocketsLocalOffsets;
	// 防连击机制
	TArray<TWeakObjectPtr<AActor>> HitActors;
	
	UPROPERTY()
	UMHWEquipmentInstance* CachedEquipmentInstance;

	// 自适应补帧常量
	const float MIN_FPS = 60.0f;
	const float MAX_FPS = 120.0f;
	const float DELTA_TIME_60FPS = 1.0f / 60.0f;
	const float DELTA_TIME_120FPS = 1.0f / 120.0f;
	
	// ======================== 卡肉执行逻辑 ========================
	
	// 触发卡肉
	void ApplyHitStop(AActor* HitActor);
	void AddHitstopAffectedActor(AActor* Actor);
	void ApplyCurrentHitstopMultiplier(float Multiplier);
	bool InitializeOwnerMontageHitstop();
	void SetOwnerMeshAnimationPaused(bool bPaused);
	void UpdateOwnerMontagePlayRateForCurrentPhase(float PhaseAlpha = 0.0f) const;
	float EvaluateCurrentTimeDilationMultiplier(float PhaseAlpha) const;
	float EvaluateCurrentMontagePlayRate(float PhaseAlpha) const;
	void BeginFreezeHitstop(float FreezeDuration);
	void BeginRecoverHitstop(float RecoverDuration);
	void UpdateHitstop(float CurrentRealTimeSeconds);
	bool IsHitstopActive() const { return HitstopPhase != EHitstopRuntimePhase::None; }
	float GetScaledHitstopDuration(float BaseDuration) const;
	void SpawnHitVFX(const FHitResult& HitResult) const;
	// 强制结束卡肉并清理状态
	void ResetHitStop();

	// 当前缓存的卡肉数据指针 (从 Instance 获取)
	const UMHWHitStopData* CurrentHitstopData;

	float CurrentHitstopStrengthMultiplier = 1.0f;

	EHitstopRuntimePhase HitstopPhase = EHitstopRuntimePhase::None;

	// 当前卡肉阶段开始时的绝对真实世界时间 (秒)
	float HitstopPhaseStartTime = 0.0f;

	// 当前卡肉阶段时长（秒）
	float HitstopPhaseDuration = 0.0f;

	// 记录攻击者原本的时间膨胀倍率（通常是 1.0）
	float DefaultTimeDilation = 1.0f;

	// 记录当前被卡肉减速影响的所有 Actor（攻击者自己 + 打中的怪物）
	TArray<TWeakObjectPtr<AActor>> HitstopAffectedActors;

	bool bHasOwnerMontageHitstop = false;

	TWeakObjectPtr<UAnimInstance> CachedOwnerAnimInstance;

	TWeakObjectPtr<UAnimMontage> CachedOwnerMontage;

	TWeakObjectPtr<USkeletalMeshComponent> CachedOwnerSkeletalMesh;

	float DefaultOwnerMontagePlayRate = 1.0f;

	bool bOwnerMeshAnimationPausedByHitstop = false;

	float CurrentMeshFreezeDuration = 0.0f;

	UPROPERTY(Transient)
	FMHWMeleeHitVFXSpec CurrentHitVFXSpec;

	UPROPERTY(Transient)
	FMHWPhysicalDamageSpec CachedPhysicalDamageSpec;

	UPROPERTY(Transient)
	bool bHasCachedPhysicalDamageSpec = false;
};
