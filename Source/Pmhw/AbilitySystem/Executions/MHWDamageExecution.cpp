#include "AbilitySystem/Executions/MHWDamageExecution.h"

#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWGameplayEffectContext.h"
#include "GameplayEffectExtension.h"
#include "MHWGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWDamageExecution)

namespace MHWDamageExecution
{
	static float GetNonNegativeSetByCallerMagnitude(const FGameplayEffectSpec& OwningSpec, const FGameplayTag& DataTag, const float DefaultValue)
	{
		return FMath::Max(0.0f, OwningSpec.GetSetByCallerMagnitude(DataTag, false, DefaultValue));
	}

	static float GetClampedSetByCallerMagnitude(
		const FGameplayEffectSpec& OwningSpec,
		const FGameplayTag& DataTag,
		const float DefaultValue,
		const float MinValue,
		const float MaxValue)
	{
		return FMath::Clamp(OwningSpec.GetSetByCallerMagnitude(DataTag, false, DefaultValue), MinValue, MaxValue);
	}

	static EMHWCriticalHitType ResolveCriticalHitType(
		const float CriticalChance,
		const float PositiveCriticalMultiplier,
		const float NegativeCriticalMultiplier,
		const float CriticalMultiplierOverride,
		float& OutCriticalMultiplier)
	{
		OutCriticalMultiplier = 1.0f;

		if (CriticalMultiplierOverride >= 0.0f)
		{
			OutCriticalMultiplier = CriticalMultiplierOverride;
			if (FMath::IsNearlyEqual(CriticalMultiplierOverride, 1.0f))
			{
				return EMHWCriticalHitType::None;
			}

			return CriticalMultiplierOverride > 1.0f
				? EMHWCriticalHitType::Positive
				: EMHWCriticalHitType::Negative;
		}

		if (CriticalChance > 0.0f && FMath::FRandRange(0.0f, 100.0f) <= CriticalChance)
		{
			OutCriticalMultiplier = PositiveCriticalMultiplier;
			return EMHWCriticalHitType::Positive;
		}

		if (CriticalChance < 0.0f && FMath::FRandRange(0.0f, 100.0f) <= FMath::Abs(CriticalChance))
		{
			OutCriticalMultiplier = NegativeCriticalMultiplier;
			return EMHWCriticalHitType::Negative;
		}

		return EMHWCriticalHitType::None;
	}

	static void SetCriticalHitTypeOnContext(FGameplayEffectContextHandle EffectContextHandle, const EMHWCriticalHitType CriticalHitType)
	{
		FGameplayEffectContext* EffectContext = EffectContextHandle.Get();
		if (!EffectContext || EffectContext->GetScriptStruct() != FMHWGameplayEffectContext::StaticStruct())
		{
			return;
		}

		static_cast<FMHWGameplayEffectContext*>(EffectContext)->SetCriticalHitType(CriticalHitType);
	}

	static float CalculateFinalDamage(
		const float TrueRawAttack,
		const float MotionValue,
		const float MotionValueScale,
		const float SharpnessMultiplier,
		const float CriticalMultiplier,
		const float BounceMultiplier,
		const float EnrageMultiplier,
		const float AilmentMultiplier,
		const float DefenseRate,
		const float HitzoneValue,
		const float AdditionalMultiplier)
	{
		return
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
	}
}

UMHWDamageExecution::UMHWDamageExecution()
{
}

void UMHWDamageExecution::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& OwningSpec = ExecutionParams.GetOwningSpec();

	const float TrueRawAttack = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::TrueRawAttack, 0.0f);
	const float MotionValue = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::MotionValue, 100.0f);
	const float MotionValueScale = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::MotionValueScale, 1.0f);
	const float SharpnessMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::SharpnessMultiplier, 1.0f);
	const float CriticalChance = MHWDamageExecution::GetClampedSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::CriticalChance, 0.0f, -100.0f, 100.0f);
	const float PositiveCriticalMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::PositiveCriticalMultiplier, 1.25f);
	const float NegativeCriticalMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::NegativeCriticalMultiplier, 0.75f);
	const float CriticalMultiplierOverride = OwningSpec.GetSetByCallerMagnitude(MHWDamageDataTags::CriticalMultiplierOverride, false, -1.0f);
	const float BounceMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::BounceMultiplier, 1.0f);
	const float EnrageMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::EnrageMultiplier, 1.0f);
	const float AilmentMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::AilmentMultiplier, 1.0f);
	const float DefenseRate = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::DefenseRate, 1.0f);
	const float HitzoneValue = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::HitzoneValue, 100.0f);
	const float AdditionalMultiplier = MHWDamageExecution::GetNonNegativeSetByCallerMagnitude(OwningSpec, MHWDamageDataTags::AdditionalMultiplier, 1.0f);

	float CriticalMultiplier = 1.0f;
	const EMHWCriticalHitType CriticalHitType = MHWDamageExecution::ResolveCriticalHitType(
		CriticalChance,
		PositiveCriticalMultiplier,
		NegativeCriticalMultiplier,
		CriticalMultiplierOverride,
		CriticalMultiplier);

	MHWDamageExecution::SetCriticalHitTypeOnContext(OwningSpec.GetContext(), CriticalHitType);

	const float FinalDamage = MHWDamageExecution::CalculateFinalDamage(
		TrueRawAttack,
		MotionValue,
		MotionValueScale,
		SharpnessMultiplier,
		CriticalMultiplier,
		BounceMultiplier,
		EnrageMultiplier,
		AilmentMultiplier,
		DefenseRate,
		HitzoneValue,
		AdditionalMultiplier);

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
