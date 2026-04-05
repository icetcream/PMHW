#include "AbilitySystem/MHWCombatBlueprintLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/MHWDamageGameplayEffect.h"
#include "AbilitySystem/MHWGameplayEffectContext.h"
#include "Character/MHWCombatComponent.h"
#include "MHWGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatBlueprintLibrary)

namespace CombatBlueprintLibrary
{
	static UMHWCombatComponent* GetCombatComponent(AActor* Actor)
	{
		return Actor ? Actor->FindComponentByClass<UMHWCombatComponent>() : nullptr;
	}

	static FMHWGameplayEffectContext* GetMHWEffectContext(FGameplayEffectContextHandle& EffectContext)
	{
		FGameplayEffectContext* RawEffectContext = EffectContext.Get();
		if (!RawEffectContext || RawEffectContext->GetScriptStruct() != FMHWGameplayEffectContext::StaticStruct())
		{
			return nullptr;
		}

		return static_cast<FMHWGameplayEffectContext*>(RawEffectContext);
	}

	static void ApplyOptionalDamageMetadata(
		FGameplayEffectContextHandle& EffectContext,
		const bool bHasDamageNumberWorldLocation,
		const FVector& DamageNumberWorldLocation,
		const FString& AttackDisplayName)
	{
		FMHWGameplayEffectContext* CombatEffectContext = GetMHWEffectContext(EffectContext);
		if (!CombatEffectContext)
		{
			return;
		}

		if (bHasDamageNumberWorldLocation)
		{
			CombatEffectContext->SetDamageNumberWorldLocation(DamageNumberWorldLocation);
		}

		if (!AttackDisplayName.IsEmpty())
		{
			CombatEffectContext->SetAttackDisplayName(AttackDisplayName);
		}
	}

	static void SetDamageSpecMagnitudes(FGameplayEffectSpec& GameplayEffectSpec, const FMHWPhysicalDamageSpec& DamageSpec)
	{
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::TrueRawAttack, DamageSpec.TrueRawAttack);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::MotionValue, DamageSpec.MotionValue);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::MotionValueScale, DamageSpec.MotionValueScale);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::SharpnessMultiplier, DamageSpec.SharpnessMultiplier);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::CriticalChance, DamageSpec.CriticalChance);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::PositiveCriticalMultiplier, DamageSpec.PositiveCriticalMultiplier);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::NegativeCriticalMultiplier, DamageSpec.NegativeCriticalMultiplier);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::CriticalMultiplierOverride, DamageSpec.CriticalMultiplierOverride);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::BounceMultiplier, DamageSpec.BounceMultiplier);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::EnrageMultiplier, DamageSpec.EnrageMultiplier);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::AilmentMultiplier, DamageSpec.AilmentMultiplier);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::DefenseRate, DamageSpec.DefenseRate);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::HitzoneValue, DamageSpec.HitzoneValue);
		GameplayEffectSpec.SetSetByCallerMagnitude(MHWDamageDataTags::AdditionalMultiplier, DamageSpec.AdditionalMultiplier);
	}
}

bool UMHWCombatBlueprintLibrary::ApplyRawDamageToActor(AActor* TargetActor, float DamageAmount)
{
	FMHWPhysicalDamageSpec DamageSpec;
	DamageSpec.TrueRawAttack = DamageAmount;
	return ApplyPhysicalDamageToActor(TargetActor, nullptr, DamageSpec, false, FVector::ZeroVector, FString());
}

bool UMHWCombatBlueprintLibrary::ApplyPhysicalDamageToActor(AActor* TargetActor, AActor* SourceActor, const FMHWPhysicalDamageSpec& DamageSpec, bool bHasDamageNumberWorldLocation, FVector DamageNumberWorldLocation, FString AttackDisplayName)
{
	if (!IsValid(TargetActor) || DamageSpec.TrueRawAttack <= 0.0f)
	{
		return false;
	}

	if (UMHWCombatComponent* CombatComponent = CombatBlueprintLibrary::GetCombatComponent(TargetActor))
	{
		return CombatComponent->ApplyPhysicalDamage(SourceActor, DamageSpec, bHasDamageNumberWorldLocation, DamageNumberWorldLocation, AttackDisplayName);
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
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

	CombatBlueprintLibrary::ApplyOptionalDamageMetadata(
		EffectContext,
		bHasDamageNumberWorldLocation,
		DamageNumberWorldLocation,
		AttackDisplayName);

	FGameplayEffectSpecHandle SpecHandle = SpecSourceASC->MakeOutgoingSpec(UMHWDamageGameplayEffect::StaticClass(), 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return false;
	}

	FGameplayEffectSpec* GameplayEffectSpec = SpecHandle.Data.Get();
	if (!GameplayEffectSpec)
	{
		return false;
	}

	CombatBlueprintLibrary::SetDamageSpecMagnitudes(*GameplayEffectSpec, DamageSpec);

	TargetASC->ApplyGameplayEffectSpecToSelf(*GameplayEffectSpec);
	return true;
}
