#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "Character/MHWMeleeHitVFXTypes.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MHWAttackComponent.generated.h"

class AActor;
class UMHWHitStopData;
class UMeleeTraceComponent;
struct FHitResult;

USTRUCT(BlueprintType)
struct PMHW_API FMHWAttackRuntimeContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	FName ActiveAttackId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	int32 CurrentWindowIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	bool bAttackActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	bool bWindowActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	bool bCurrentWindowHit = false;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	int32 TotalHitCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attack")
	TArray<int32> HitWindowIndices;

	void Reset()
	{
		ActiveAttackId = NAME_None;
		CurrentWindowIndex = INDEX_NONE;
		bAttackActive = false;
		bWindowActive = false;
		bCurrentWindowHit = false;
		TotalHitCount = 0;
		HitWindowIndices.Reset();
	}

	bool HasWindowHit(int32 WindowIndex) const
	{
		return HitWindowIndices.Contains(WindowIndex);
	}

	void RegisterWindowHit(int32 WindowIndex)
	{
		if (WindowIndex != INDEX_NONE && !HitWindowIndices.Contains(WindowIndex))
		{
			HitWindowIndices.Add(WindowIndex);
		}
	}
};

USTRUCT(BlueprintType)
struct PMHW_API FMHWAttackWindowSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack", meta = (DisplayName = "攻击ID"))
	FName AttackId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack", meta = (DisplayName = "窗口索引", ClampMin = "0"))
	int32 WindowIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack", meta = (DisplayName = "开始新攻击"))
	bool bStartNewAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack", meta = (DisplayName = "结束时完成攻击"))
	bool bFinishAttackOnEnd = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Trace Override", meta = (DisplayName = "覆盖基准 Socket", ToolTip = "可选覆盖。不填写时使用当前武器 EquipmentDefinition 中的默认 BaseSocket。"))
	FName BaseSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Trace Override", meta = (DisplayName = "覆盖追踪 Sockets", ToolTip = "可选覆盖。不填写时使用当前武器 EquipmentDefinition 中的默认 TraceSockets。"))
	TArray<FName> TraceSockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack", meta = (DisplayName = "攻击规格标签", ToolTip = "从当前武器的攻击数据表中查找这一段攻击的动作数据。", Categories = "Data.AttackSpec"))
	FGameplayTag AttackSpecTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|VFX", meta = (DisplayName = "命中特效"))
	FMHWMeleeHitVFXSpec HitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Hitstop", meta = (DisplayName = "是否终结段"))
	bool bIsFinisher = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Hitstop", meta = (DisplayName = "卡肉数据覆盖"))
	TObjectPtr<const UMHWHitStopData> HitStopDataOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Conditional", meta = (DisplayName = "上一段命中时加成"))
	bool bBonusIfPreviousWindowHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Conditional", meta = (DisplayName = "上一段命中倍率", EditCondition = "bBonusIfPreviousWindowHit", ClampMin = "0.0"))
	float PreviousWindowHitMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Conditional", meta = (DisplayName = "检查窗口索引覆盖", EditCondition = "bBonusIfPreviousWindowHit"))
	int32 PreviousWindowIndexOverride = INDEX_NONE;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PMHW_API UMHWAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMHWAttackComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "MHW|Attack")
	void BeginAttack(FName InAttackId);

	UFUNCTION(BlueprintCallable, Category = "MHW|Attack")
	void EndAttack();

	UFUNCTION(BlueprintCallable, Category = "MHW|Attack")
	bool BeginAttackWindow(const FMHWAttackWindowSpec& WindowSpec);

	UFUNCTION(BlueprintCallable, Category = "MHW|Attack")
	void EndAttackWindow();

	UFUNCTION(BlueprintCallable, Category = "MHW|Attack")
	void NotifyCurrentWindowHit(AActor* HitActor);

	UFUNCTION(BlueprintPure, Category = "MHW|Attack")
	FMHWAttackRuntimeContext GetAttackRuntimeContext() const { return AttackRuntimeContext; }

	UFUNCTION(BlueprintPure, Category = "MHW|Attack")
	bool HasWindowHit(int32 WindowIndex) const;

private:
	bool TryInitializeFromOwner();
	void StopTraceAndClearDamageSpec();
	FMHWPhysicalDamageSpec BuildDamageSpecForWindow(const FMHWAttackWindowSpec& WindowSpec) const;
	void ConfigureHitstopForWindow(const FMHWAttackWindowSpec& WindowSpec, const FMHWPhysicalDamageSpec& ResolvedDamageSpec);
	int32 ResolveBonusCheckWindowIndex(const FMHWAttackWindowSpec& WindowSpec) const;

	UFUNCTION()
	void HandleMeleeHit(const FHitResult& HitResult);

	UPROPERTY(Transient)
	TObjectPtr<UMeleeTraceComponent> CachedMeleeTraceComponent;

	UPROPERTY(Transient)
	bool bMeleeHitDelegateBound = false;

	UPROPERTY(Transient)
	FMHWAttackRuntimeContext AttackRuntimeContext;

	UPROPERTY(Transient)
	FMHWAttackWindowSpec ActiveWindowSpec;
};
