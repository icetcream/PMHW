#include "Character/MHWPlayerCombatComponent.h"

#include "Character/MHWMonsterCombatComponent.h"
#include "Equipment/MHWEquipmentDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPlayerCombatComponent)

void UMHWPlayerCombatComponent::ApplyAttackPanelBonus(const FMHWPhysicalAttackPanelBonus& Bonus)
{
	PhysicalAttackPanel.TrueRawAttack += Bonus.TrueRawAttackBonus;
	PhysicalAttackPanel.MotionValueScale += Bonus.MotionValueScaleBonus;
	PhysicalAttackPanel.SharpnessMultiplier += Bonus.SharpnessMultiplierBonus;
	PhysicalAttackPanel.CriticalChance += Bonus.CriticalChanceBonus;
	PhysicalAttackPanel.PositiveCriticalMultiplier += Bonus.PositiveCriticalMultiplierBonus;
	PhysicalAttackPanel.NegativeCriticalMultiplier += Bonus.NegativeCriticalMultiplierBonus;
	PhysicalAttackPanel.BounceMultiplier += Bonus.BounceMultiplierBonus;

	NormalizePhysicalAttackPanel();
}

void UMHWPlayerCombatComponent::RemoveAttackPanelBonus(const FMHWPhysicalAttackPanelBonus& Bonus)
{
	PhysicalAttackPanel.TrueRawAttack -= Bonus.TrueRawAttackBonus;
	PhysicalAttackPanel.MotionValueScale -= Bonus.MotionValueScaleBonus;
	PhysicalAttackPanel.SharpnessMultiplier -= Bonus.SharpnessMultiplierBonus;
	PhysicalAttackPanel.CriticalChance -= Bonus.CriticalChanceBonus;
	PhysicalAttackPanel.PositiveCriticalMultiplier -= Bonus.PositiveCriticalMultiplierBonus;
	PhysicalAttackPanel.NegativeCriticalMultiplier -= Bonus.NegativeCriticalMultiplierBonus;
	PhysicalAttackPanel.BounceMultiplier -= Bonus.BounceMultiplierBonus;

	NormalizePhysicalAttackPanel();
}

void UMHWPlayerCombatComponent::NormalizePhysicalAttackPanel()
{
	PhysicalAttackPanel.TrueRawAttack = FMath::Max(0.0f, PhysicalAttackPanel.TrueRawAttack);
	PhysicalAttackPanel.MotionValueScale = FMath::Max(0.0f, PhysicalAttackPanel.MotionValueScale);
	PhysicalAttackPanel.SharpnessMultiplier = FMath::Max(0.0f, PhysicalAttackPanel.SharpnessMultiplier);
	PhysicalAttackPanel.CriticalChance = FMath::Clamp(PhysicalAttackPanel.CriticalChance, -100.0f, 100.0f);
	PhysicalAttackPanel.PositiveCriticalMultiplier = FMath::Max(0.0f, PhysicalAttackPanel.PositiveCriticalMultiplier);
	PhysicalAttackPanel.NegativeCriticalMultiplier = FMath::Max(0.0f, PhysicalAttackPanel.NegativeCriticalMultiplier);
	PhysicalAttackPanel.BounceMultiplier = FMath::Max(0.0f, PhysicalAttackPanel.BounceMultiplier);
}

FMHWPhysicalDamageSpec UMHWPlayerCombatComponent::BuildResolvedOutgoingPhysicalDamageSpec(
	const FMHWPhysicalDamageSpec& RuntimeDamageSpec,
	const UMHWMonsterCombatComponent* TargetCombatComponent) const
{
	FMHWPhysicalDamageSpec ResolvedDamageSpec = RuntimeDamageSpec;

	ResolvedDamageSpec.TrueRawAttack = PhysicalAttackPanel.TrueRawAttack;
	ResolvedDamageSpec.MotionValueScale *= PhysicalAttackPanel.MotionValueScale;
	ResolvedDamageSpec.SharpnessMultiplier = PhysicalAttackPanel.SharpnessMultiplier;
	ResolvedDamageSpec.CriticalChance = PhysicalAttackPanel.CriticalChance;
	ResolvedDamageSpec.PositiveCriticalMultiplier = PhysicalAttackPanel.PositiveCriticalMultiplier;
	ResolvedDamageSpec.NegativeCriticalMultiplier = PhysicalAttackPanel.NegativeCriticalMultiplier;
	ResolvedDamageSpec.BounceMultiplier = PhysicalAttackPanel.BounceMultiplier;

	if (TargetCombatComponent)
	{
		const FMHWPhysicalDefensePanel& TargetDefensePanel = TargetCombatComponent->GetPhysicalDefensePanel();
		ResolvedDamageSpec.EnrageMultiplier = TargetDefensePanel.EnrageMultiplier;
		ResolvedDamageSpec.AilmentMultiplier = TargetDefensePanel.AilmentMultiplier;
		ResolvedDamageSpec.DefenseRate = TargetDefensePanel.DefenseRate;
		ResolvedDamageSpec.HitzoneValue = TargetDefensePanel.HitzoneValue;
	}

	return ResolvedDamageSpec;
}
