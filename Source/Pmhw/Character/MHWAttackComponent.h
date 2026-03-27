#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "Character/MHWMeleeHitVFXTypes.h"
#include "Components/ActorComponent.h"
#include "MHWAttackComponent.generated.h"

class AActor;
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

	FMHWAttackWindowSpec()
	{
		BaseDamageSpec.TrueRawAttack = 0.0f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack")
	FName AttackId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack", meta = (ClampMin = "0"))
	int32 WindowIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack")
	bool bStartNewAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack")
	bool bFinishAttackOnEnd = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Trace Override", meta = (DisplayName = "Override Base Socket", ToolTip = "Optional override. Leave empty to use the equipped weapon's default BaseSocket from EquipmentDefinition."))
	FName BaseSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Trace Override", meta = (DisplayName = "Override Trace Sockets", ToolTip = "Optional override. Leave empty to use the equipped weapon's default TraceSockets from EquipmentDefinition."))
	TArray<FName> TraceSockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack")
	FMHWPhysicalDamageSpec BaseDamageSpec;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|VFX")
	FMHWMeleeHitVFXSpec HitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Conditional")
	bool bBonusIfPreviousWindowHit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Conditional", meta = (EditCondition = "bBonusIfPreviousWindowHit", ClampMin = "0.0"))
	float PreviousWindowHitMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Attack|Conditional", meta = (EditCondition = "bBonusIfPreviousWindowHit"))
	int32 PreviousWindowIndexOverride = INDEX_NONE;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
