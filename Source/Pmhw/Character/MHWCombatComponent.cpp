#include "Character/MHWCombatComponent.h"

#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWDamageGameplayEffect.h"
#include "AbilitySystem/MHWGameplayEffectContext.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "MHWGameplayTags.h"
#include "UI/MHWDamageNumberActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatComponent)

UMHWCombatComponent::UMHWCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UMHWCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	TryInitializeFromOwner();
	InitializeDamageNumberPool();
}

void UMHWCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TryInitializeFromOwner();
	HandleAutomaticStaminaRegen(DeltaTime);
}

UAbilitySystemComponent* UMHWCombatComponent::GetAbilitySystemComponent() const
{
	if (CachedAbilitySystemComponent)
	{
		return CachedAbilitySystemComponent;
	}

	AActor* OwnerActor = GetOwner();
	return OwnerActor ? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor) : nullptr;
}

const UMHWCombatAttributeSet* UMHWCombatComponent::GetCombatAttributeSet() const
{
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return ASC->GetSet<UMHWCombatAttributeSet>();
	}

	return nullptr;
}

float UMHWCombatComponent::GetHealth() const
{
	if (const UMHWCombatAttributeSet* Attributes = GetCombatAttributeSet())
	{
		return Attributes->GetHealth();
	}

	return 0.0f;
}

float UMHWCombatComponent::GetMaxHealth() const
{
	if (const UMHWCombatAttributeSet* Attributes = GetCombatAttributeSet())
	{
		return Attributes->GetMaxHealth();
	}

	return 0.0f;
}

float UMHWCombatComponent::GetHealthPercent() const
{
	const float MaxHealth = GetMaxHealth();
	return MaxHealth > 0.0f ? (GetHealth() / MaxHealth) : 0.0f;
}

float UMHWCombatComponent::GetStamina() const
{
	if (const UMHWCombatAttributeSet* Attributes = GetCombatAttributeSet())
	{
		return Attributes->GetStamina();
	}

	return 0.0f;
}

float UMHWCombatComponent::GetMaxStamina() const
{
	if (const UMHWCombatAttributeSet* Attributes = GetCombatAttributeSet())
	{
		return Attributes->GetMaxStamina();
	}

	return 0.0f;
}

float UMHWCombatComponent::GetStaminaPercent() const
{
	const float MaxStamina = GetMaxStamina();
	return MaxStamina > 0.0f ? (GetStamina() / MaxStamina) : 0.0f;
}

bool UMHWCombatComponent::IsAlive() const
{
	return GetHealth() > 0.0f;
}

bool UMHWCombatComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return true;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	const float CurrentStamina = ASC->GetNumericAttribute(UMHWCombatAttributeSet::GetStaminaAttribute());
	if (CurrentStamina < Amount)
	{
		return false;
	}

	ASC->ApplyModToAttribute(UMHWCombatAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, -Amount);

	if (UWorld* World = GetWorld())
	{
		LastStaminaConsumeTime = World->GetTimeSeconds();
	}

	return true;
}

void UMHWCombatComponent::RestoreStamina(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->ApplyModToAttribute(UMHWCombatAttributeSet::GetStaminaAttribute(), EGameplayModOp::Additive, Amount);
	}
}

void UMHWCombatComponent::RestoreHealth(float Amount)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->ApplyModToAttribute(UMHWCombatAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, Amount);
	}
}

bool UMHWCombatComponent::ApplyRawDamage(float DamageAmount)
{
	FMHWPhysicalDamageSpec DamageSpec;
	DamageSpec.TrueRawAttack = DamageAmount;
	return ApplyPhysicalDamage(nullptr, DamageSpec);
}

bool UMHWCombatComponent::ApplyPhysicalDamage(AActor* SourceActor, const FMHWPhysicalDamageSpec& DamageSpec, bool bHasDamageNumberWorldLocation, FVector DamageNumberWorldLocation, FString AttackDisplayName)
{
	if (DamageSpec.TrueRawAttack <= 0.0f)
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return false;
	}

	UAbilitySystemComponent* SourceASC = SourceActor
		? UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourceActor)
		: nullptr;
	UAbilitySystemComponent* SpecSourceASC = SourceASC ? SourceASC : TargetASC;

	FGameplayEffectContextHandle EffectContext = SpecSourceASC->MakeEffectContext();
	if (SourceActor)
	{
		EffectContext.AddSourceObject(SourceActor);
	}

	if (bHasDamageNumberWorldLocation)
	{
		if (FGameplayEffectContext* RawEffectContext = EffectContext.Get())
		{
			if (RawEffectContext->GetScriptStruct() == FMHWGameplayEffectContext::StaticStruct())
			{
				static_cast<FMHWGameplayEffectContext*>(RawEffectContext)->SetDamageNumberWorldLocation(DamageNumberWorldLocation);
			}
		}
	}

	if (!AttackDisplayName.IsEmpty())
	{
		if (FGameplayEffectContext* RawEffectContext = EffectContext.Get())
		{
			if (RawEffectContext->GetScriptStruct() == FMHWGameplayEffectContext::StaticStruct())
			{
				static_cast<FMHWGameplayEffectContext*>(RawEffectContext)->SetAttackDisplayName(AttackDisplayName);
			}
		}
	}

	FGameplayEffectSpecHandle SpecHandle = SpecSourceASC->MakeOutgoingSpec(UMHWDamageGameplayEffect::StaticClass(), 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	FGameplayEffectSpec* DamageSpecHandle = SpecHandle.Data.Get();
	if (!DamageSpecHandle)
	{
		return false;
	}

	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::TrueRawAttack, DamageSpec.TrueRawAttack);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::MotionValue, DamageSpec.MotionValue);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::MotionValueScale, DamageSpec.MotionValueScale);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::SharpnessMultiplier, DamageSpec.SharpnessMultiplier);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::CriticalChance, DamageSpec.CriticalChance);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::PositiveCriticalMultiplier, DamageSpec.PositiveCriticalMultiplier);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::NegativeCriticalMultiplier, DamageSpec.NegativeCriticalMultiplier);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::CriticalMultiplierOverride, DamageSpec.CriticalMultiplierOverride);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::BounceMultiplier, DamageSpec.BounceMultiplier);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::EnrageMultiplier, DamageSpec.EnrageMultiplier);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::AilmentMultiplier, DamageSpec.AilmentMultiplier);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::DefenseRate, DamageSpec.DefenseRate);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::HitzoneValue, DamageSpec.HitzoneValue);
	DamageSpecHandle->SetSetByCallerMagnitude(MHWDamageDataTags::AdditionalMultiplier, DamageSpec.AdditionalMultiplier);

	TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle);
	return true;
}

void UMHWCombatComponent::ResetVitalsToMax()
{
	RestoreHealth(GetMaxHealth() - GetHealth());
	RestoreStamina(GetMaxStamina() - GetStamina());
}

bool UMHWCombatComponent::ApplyInitialAttributesEffect(bool bForceReapply)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC || !InitialAttributesEffectClass)
	{
		return false;
	}

	if (!bForceReapply && bInitialAttributesEffectApplied && InitialAttributesAppliedASC == ASC)
	{
		return false;
	}

	const UGameplayEffect* InitialEffect = InitialAttributesEffectClass->GetDefaultObject<UGameplayEffect>();
	if (!InitialEffect)
	{
		return false;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetOwner());
	ASC->ApplyGameplayEffectToSelf(InitialEffect, InitialAttributesEffectLevel, EffectContext);

	bInitialAttributesEffectApplied = true;
	InitialAttributesAppliedASC = ASC;
	return true;
}

void UMHWCombatComponent::ResetInitialAttributesEffectApplication()
{
	bInitialAttributesEffectApplied = false;
	InitialAttributesAppliedASC = nullptr;
}

void UMHWCombatComponent::NotifyDamageReceived(float DamageAmount, float NewHealth, AActor* SourceActor, EMHWCriticalHitType CriticalHitType, bool bHasDamageNumberWorldLocation, FVector DamageNumberWorldLocation, const FString& AttackDisplayName)
{
	OnDamagedDetailed.Broadcast(SourceActor, GetOwner(), DamageAmount, NewHealth);
	OnDamagedResultDetailed.Broadcast(SourceActor, GetOwner(), DamageAmount, NewHealth, CriticalHitType, AttackDisplayName);
	SpawnDamageNumberActor(DamageAmount, CriticalHitType, bHasDamageNumberWorldLocation, DamageNumberWorldLocation, AttackDisplayName);
}

void UMHWCombatComponent::InitializeDamageNumberPool()
{
	if (bDamageNumberPoolInitialized)
	{
		return;
	}

	bDamageNumberPoolInitialized = true;

	if (!bSpawnDamageNumberOnDamage || !DamageNumberActorClass || InitialDamageNumberPoolSize <= 0)
	{
		return;
	}

	for (int32 Index = 0; Index < InitialDamageNumberPoolSize; ++Index)
	{
		if (!SpawnPooledDamageNumberActor())
		{
			break;
		}
	}
}

AMHWDamageNumberActor* UMHWCombatComponent::AcquireDamageNumberActor()
{
	InitializeDamageNumberPool();

	for (int32 Index = DamageNumberActorPool.Num() - 1; Index >= 0; --Index)
	{
		AMHWDamageNumberActor* DamageNumberActor = DamageNumberActorPool[Index];
		if (!IsValid(DamageNumberActor))
		{
			DamageNumberActorPool.RemoveAtSwap(Index);
			continue;
		}

		if (DamageNumberActor->IsAvailableForReuse())
		{
			return DamageNumberActor;
		}
	}

	if (MaxDamageNumberPoolSize > 0 && DamageNumberActorPool.Num() >= MaxDamageNumberPoolSize)
	{
		return nullptr;
	}

	return SpawnPooledDamageNumberActor();
}

AMHWDamageNumberActor* UMHWCombatComponent::SpawnPooledDamageNumberActor()
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || !DamageNumberActorClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerActor;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AMHWDamageNumberActor* DamageNumberActor = World->SpawnActor<AMHWDamageNumberActor>(
		DamageNumberActorClass,
		OwnerActor->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParameters))
	{
		DamageNumberActorPool.Add(DamageNumberActor);
		return DamageNumberActor;
	}

	return nullptr;
}

void UMHWCombatComponent::SpawnDamageNumberActor(float DamageAmount, EMHWCriticalHitType CriticalHitType, bool bHasDamageNumberWorldLocation, const FVector& DamageNumberWorldLocation, const FString& AttackDisplayName)
{
	if (!bSpawnDamageNumberOnDamage || !DamageNumberActorClass)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	const FVector SpawnLocation = bHasDamageNumberWorldLocation ? DamageNumberWorldLocation : OwnerActor->GetActorLocation();
	if (AMHWDamageNumberActor* DamageNumberActor = AcquireDamageNumberActor())
	{
		DamageNumberActor->SetActorLocation(SpawnLocation);
		DamageNumberActor->InitializeDamageNumber(OwnerActor, DamageAmount, CriticalHitType, AttackDisplayName, bHasDamageNumberWorldLocation, DamageNumberWorldLocation);
	}
}

bool UMHWCombatComponent::TryInitializeFromOwner()
{
	if (bAttributeDelegatesBound)
	{
		return true;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	const UMHWCombatAttributeSet* Attributes = ASC->GetSet<UMHWCombatAttributeSet>();
	if (!Attributes)
	{
		return false;
	}

	if (bApplyInitialAttributesEffectOnInitialize)
	{
		ApplyInitialAttributesEffect();
	}

	CachedAbilitySystemComponent = ASC;
	ASC->GetGameplayAttributeValueChangeDelegate(UMHWCombatAttributeSet::GetHealthAttribute())
		.AddUObject(this, &ThisClass::HandleHealthAttributeChanged);
	ASC->GetGameplayAttributeValueChangeDelegate(UMHWCombatAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &ThisClass::HandleStaminaAttributeChanged);

	bAttributeDelegatesBound = true;
	LastStaminaConsumeTime = -StaminaRegenDelay;
	BroadcastInitialAttributeState();
	return true;
}

void UMHWCombatComponent::HandleAutomaticStaminaRegen(float DeltaSeconds)
{
	if (!bAttributeDelegatesBound || !bEnableStaminaAutoRegen || StaminaRegenRate <= 0.0f || !IsAlive())
	{
		return;
	}

	UWorld* World = GetWorld();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!World || !ASC)
	{
		return;
	}

	if (World->GetTimeSeconds() - LastStaminaConsumeTime < StaminaRegenDelay)
	{
		return;
	}

	const float CurrentStamina = ASC->GetNumericAttribute(UMHWCombatAttributeSet::GetStaminaAttribute());
	const float MaxStamina = ASC->GetNumericAttribute(UMHWCombatAttributeSet::GetMaxStaminaAttribute());
	if (CurrentStamina >= MaxStamina - KINDA_SMALL_NUMBER)
	{
		return;
	}

	ASC->ApplyModToAttribute(
		UMHWCombatAttributeSet::GetStaminaAttribute(),
		EGameplayModOp::Additive,
		StaminaRegenRate * DeltaSeconds);
}

void UMHWCombatComponent::HandleHealthAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	const float DeltaValue = ChangeData.NewValue - ChangeData.OldValue;
	LastHealthValue = ChangeData.NewValue;

	OnHealthChanged.Broadcast(ChangeData.NewValue, GetMaxHealth(), DeltaValue);

	if (DeltaValue < 0.0f)
	{
		OnDamaged.Broadcast(-DeltaValue, ChangeData.NewValue);
	}

	if (!bDeathEventBroadcasted && ChangeData.OldValue > 0.0f && ChangeData.NewValue <= 0.0f)
	{
		bDeathEventBroadcasted = true;
		OnDeath.Broadcast();
	}
	else if (ChangeData.NewValue > 0.0f)
	{
		bDeathEventBroadcasted = false;
	}
}

void UMHWCombatComponent::HandleStaminaAttributeChanged(const FOnAttributeChangeData& ChangeData)
{
	LastStaminaValue = ChangeData.NewValue;
	OnStaminaChanged.Broadcast(ChangeData.NewValue, GetMaxStamina(), ChangeData.NewValue - ChangeData.OldValue);
}

void UMHWCombatComponent::BroadcastInitialAttributeState()
{
	LastHealthValue = GetHealth();
	LastStaminaValue = GetStamina();
	bDeathEventBroadcasted = LastHealthValue <= 0.0f;

	OnHealthChanged.Broadcast(LastHealthValue, GetMaxHealth(), 0.0f);
	OnStaminaChanged.Broadcast(LastStaminaValue, GetMaxStamina(), 0.0f);
}
