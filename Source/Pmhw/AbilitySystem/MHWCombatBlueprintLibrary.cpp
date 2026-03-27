#include "AbilitySystem/MHWCombatBlueprintLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/MHWDamageGameplayEffect.h"
#include "Character/MHWCombatComponent.h"
#include "MHWGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatBlueprintLibrary)

bool UMHWCombatBlueprintLibrary::ApplyRawDamageToActor(AActor* TargetActor, float DamageAmount)
{
	FMHWPhysicalDamageSpec DamageSpec;
	DamageSpec.TrueRawAttack = DamageAmount;
	return ApplyPhysicalDamageToActor(TargetActor, nullptr, DamageSpec);
}

bool UMHWCombatBlueprintLibrary::ApplyPhysicalDamageToActor(AActor* TargetActor, AActor* SourceActor, const FMHWPhysicalDamageSpec& DamageSpec)
{
	if (!IsValid(TargetActor) || DamageSpec.TrueRawAttack <= 0.0f)
	{
		return false;
	}

	if (UMHWCombatComponent* CombatComponent = TargetActor->FindComponentByClass<UMHWCombatComponent>())
	{
		return CombatComponent->ApplyPhysicalDamage(SourceActor, DamageSpec);
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

	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::TrueRawAttack, DamageSpec.TrueRawAttack);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::MotionValue, DamageSpec.MotionValue);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::MotionValueScale, DamageSpec.MotionValueScale);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::SharpnessMultiplier, DamageSpec.SharpnessMultiplier);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::AffinityChance, DamageSpec.AffinityChance);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::PositiveCriticalMultiplier, DamageSpec.PositiveCriticalMultiplier);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::NegativeCriticalMultiplier, DamageSpec.NegativeCriticalMultiplier);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::CriticalMultiplierOverride, DamageSpec.CriticalMultiplierOverride);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::BounceMultiplier, DamageSpec.BounceMultiplier);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::EnrageMultiplier, DamageSpec.EnrageMultiplier);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::AilmentMultiplier, DamageSpec.AilmentMultiplier);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::DefenseRate, DamageSpec.DefenseRate);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::HitzoneValue, DamageSpec.HitzoneValue);
	GameplayEffectSpec->SetSetByCallerMagnitude(MHWDamageDataTags::AdditionalMultiplier, DamageSpec.AdditionalMultiplier);

	TargetASC->ApplyGameplayEffectSpecToSelf(*GameplayEffectSpec);
	return true;
}
