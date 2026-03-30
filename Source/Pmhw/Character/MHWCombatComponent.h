#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "MHWCombatComponent.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UMHWCombatAttributeSet;
class AMHWDamageNumberActor;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMHWAttributeChangedSignature, float, NewValue, float, MaxValue, float, DeltaValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMHWDamageSignature, float, DamageAmount, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FMHWDamageDetailedSignature, AActor*, SourceActor, AActor*, TargetActor, float, DamageAmount, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FMHWDamageResultDetailedSignature, AActor*, SourceActor, AActor*, TargetActor, float, DamageAmount, float, NewHealth, EMHWCriticalHitType, CriticalHitType, const FString&, AttackDisplayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMHWDeathSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMHWCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	const UMHWCombatAttributeSet* GetCombatAttributeSet() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	float GetStamina() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	bool ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	void RestoreStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	void RestoreHealth(float Amount);

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	bool ApplyRawDamage(float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	bool ApplyPhysicalDamage(AActor* SourceActor, const FMHWPhysicalDamageSpec& DamageSpec, bool bHasDamageNumberWorldLocation = false, FVector DamageNumberWorldLocation = FVector::ZeroVector, FString AttackDisplayName = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	void ResetVitalsToMax();

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat|Init")
	bool ApplyInitialAttributesEffect(bool bForceReapply = false);

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat|Init")
	void ResetInitialAttributesEffectApplication();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Stamina")
	bool bEnableStaminaAutoRegen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenRate = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenDelay = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Init")
	bool bApplyInitialAttributesEffectOnInitialize = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Init")
	TSubclassOf<UGameplayEffect> InitialAttributesEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Init", meta = (ClampMin = "0.0"))
	float InitialAttributesEffectLevel = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "MHW|Combat|Events")
	FMHWAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|Combat|Events")
	FMHWAttributeChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|Combat|Events")
	FMHWDamageSignature OnDamaged;

	UPROPERTY(BlueprintAssignable, Category = "MHW|Combat|Events")
	FMHWDamageDetailedSignature OnDamagedDetailed;

	UPROPERTY(BlueprintAssignable, Category = "MHW|Combat|Events")
	FMHWDamageResultDetailedSignature OnDamagedResultDetailed;

	UPROPERTY(BlueprintAssignable, Category = "MHW|Combat|Events")
	FMHWDeathSignature OnDeath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Damage Number")
	bool bSpawnDamageNumberOnDamage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Damage Number", meta = (EditCondition = "bSpawnDamageNumberOnDamage"))
	TSubclassOf<AMHWDamageNumberActor> DamageNumberActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Damage Number", meta = (EditCondition = "bSpawnDamageNumberOnDamage", ClampMin = "0"))
	int32 InitialDamageNumberPoolSize = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Damage Number", meta = (EditCondition = "bSpawnDamageNumberOnDamage", ClampMin = "0"))
	int32 MaxDamageNumberPoolSize = 24;

	void NotifyDamageReceived(float DamageAmount, float NewHealth, AActor* SourceActor, EMHWCriticalHitType CriticalHitType, bool bHasDamageNumberWorldLocation, FVector DamageNumberWorldLocation, const FString& AttackDisplayName);

private:
	void InitializeDamageNumberPool();
	AMHWDamageNumberActor* AcquireDamageNumberActor();
	AMHWDamageNumberActor* SpawnPooledDamageNumberActor();
	void SpawnDamageNumberActor(float DamageAmount, EMHWCriticalHitType CriticalHitType, bool bHasDamageNumberWorldLocation, const FVector& DamageNumberWorldLocation, const FString& AttackDisplayName);
	bool TryInitializeFromOwner();
	void HandleAutomaticStaminaRegen(float DeltaSeconds);
	void HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void HandleStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData);
	void BroadcastInitialAttributeState();

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedAbilitySystemComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> InitialAttributesAppliedASC;

	UPROPERTY(Transient)
	bool bAttributeDelegatesBound = false;

	UPROPERTY(Transient)
	bool bInitialAttributesEffectApplied = false;

	UPROPERTY(Transient)
	float LastHealthValue = 0.0f;

	UPROPERTY(Transient)
	float LastStaminaValue = 0.0f;

	UPROPERTY(Transient)
	bool bDeathEventBroadcasted = false;

	UPROPERTY(Transient)
	float LastStaminaConsumeTime = -1.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AMHWDamageNumberActor>> DamageNumberActorPool;

	UPROPERTY(Transient)
	bool bDamageNumberPoolInitialized = false;
};
