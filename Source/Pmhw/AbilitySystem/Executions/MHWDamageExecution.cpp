#include "AbilitySystem/Executions/MHWDamageExecution.h"

#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWGameplayEffectContext.h"
#include "GameplayEffectExtension.h"
#include "MHWGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWDamageExecution)

UMHWDamageExecution::UMHWDamageExecution()
{
}

void UMHWDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& OwningSpec = ExecutionParams.GetOwningSpec();

	const float TrueRawAttack = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::TrueRawAttack, false, 0.0f));
	const float MotionValue = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::MotionValue, false, 100.0f));
	const float MotionValueScale = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::MotionValueScale, false, 1.0f));
	const float SharpnessMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::SharpnessMultiplier, false, 1.0f));
	const float CriticalChance = FMath::Clamp(OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::CriticalChance, false, 0.0f), -100.0f, 100.0f);
	const float PositiveCriticalMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::PositiveCriticalMultiplier, false, 1.25f));
	const float NegativeCriticalMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::NegativeCriticalMultiplier, false, 0.75f));
	const float CriticalMultiplierOverride = OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::CriticalMultiplierOverride, false, -1.0f);
	const float BounceMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::BounceMultiplier, false, 1.0f));
	const float EnrageMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::EnrageMultiplier, false, 1.0f));
	const float AilmentMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::AilmentMultiplier, false, 1.0f));
	const float DefenseRate = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::DefenseRate, false, 1.0f));
	const float HitzoneValue = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::HitzoneValue, false, 100.0f));
	const float AdditionalMultiplier = FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::AdditionalMultiplier, false, 1.0f));

	float CriticalMultiplier = 1.0f;
	EMHWCriticalHitType CriticalHitType = EMHWCriticalHitType::None;
	if (CriticalMultiplierOverride >= 0.0f)
	{
		CriticalMultiplier = CriticalMultiplierOverride;
		if (!FMath::IsNearlyEqual(CriticalMultiplierOverride, 1.0f))
		{
			CriticalHitType = CriticalMultiplierOverride > 1.0f
				? EMHWCriticalHitType::Positive
				: EMHWCriticalHitType::Negative;
		}
	}
	else if (CriticalChance > 0.0f)
	{
		if (FMath::FRandRange(0.0f, 100.0f) <= CriticalChance)
		{
			CriticalMultiplier = PositiveCriticalMultiplier;
			CriticalHitType = EMHWCriticalHitType::Positive;
		}
	}
	else if (CriticalChance < 0.0f)
	{
		if (FMath::FRandRange(0.0f, 100.0f) <= FMath::Abs(CriticalChance))
		{
			CriticalMultiplier = NegativeCriticalMultiplier;
			CriticalHitType = EMHWCriticalHitType::Negative;
		}
	}

	if (FGameplayEffectContext* EffectContext = OwningSpec.GetContext().Get())
	{
		if (EffectContext->GetScriptStruct() == FMHWGameplayEffectContext::StaticStruct())
		{
			static_cast<FMHWGameplayEffectContext*>(EffectContext)->SetCriticalHitType(CriticalHitType);
		}
	}

	const float FinalDamage =
		TrueRawAttack *
		MotionValueScale *
		(MotionValue / 100.0f) *
		SharpnessMultiplier *
		CriticalMultiplier *
		BounceMultiplier *
		EnrageMultiplier *
		AilmentMultiplier *
		DefenseRate *
		(HitzoneValue / 100.0f) *
		AdditionalMultiplier;

	const float ClampedDamage = FMath::Max(0.0f, FMath::FloorToFloat(FinalDamage));
	if (ClampedDamage <= 0.0f)
	{
		return;
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UMHWCombatAttributeSet::GetIncomingDamageAttribute(),
		EGameplayModOp::Additive,
		ClampedDamage));
}
