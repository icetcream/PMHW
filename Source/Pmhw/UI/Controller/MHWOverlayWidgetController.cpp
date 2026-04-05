#include "UI/Controller/MHWOverlayWidgetController.h"

#include "Character/MHWCombatComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWOverlayWidgetController)

void UMHWOverlayWidgetController::UnbindFromCombatComponent()
{
	if (!BoundCombatComponent)
	{
		return;
	}

	BoundCombatComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
	BoundCombatComponent->OnStaminaChanged.RemoveDynamic(this, &ThisClass::HandleStaminaChanged);
	BoundCombatComponent->OnDamaged.RemoveDynamic(this, &ThisClass::HandleDamaged);
	BoundCombatComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleDeath);
	BoundCombatComponent = nullptr;
}

void UMHWOverlayWidgetController::BindCallbacksToDependencies()
{
	if (!CombatComponent)
	{
		UnbindFromCombatComponent();
		return;
	}

	if (BoundCombatComponent == CombatComponent)
	{
		return;
	}

	// Local HUD can be rebound during pawn/controller changes, so stale combat delegates must be released first.
	UnbindFromCombatComponent();
	BoundCombatComponent = CombatComponent;
	CombatComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);
	CombatComponent->OnStaminaChanged.AddDynamic(this, &ThisClass::HandleStaminaChanged);
	CombatComponent->OnDamaged.AddDynamic(this, &ThisClass::HandleDamaged);
	CombatComponent->OnDeath.AddDynamic(this, &ThisClass::HandleDeath);
}

void UMHWOverlayWidgetController::BroadcastInitialValues()
{
	if (!CombatComponent)
	{
		return;
	}

	HandleHealthChanged(CombatComponent->GetHealth(), CombatComponent->GetMaxHealth(), 0.0f);
	HandleStaminaChanged(CombatComponent->GetStamina(), CombatComponent->GetMaxStamina(), 0.0f);
}

void UMHWOverlayWidgetController::BroadcastResourceChanged(
	const float NewValue,
	const float MaxValue,
	FMHWOnFloatValueChangedSignature& OnValueChanged,
	FMHWOnFloatValueChangedSignature& OnMaxValueChanged,
	FMHWOnFloatValueChangedSignature& OnPercentChanged) const
{
	OnValueChanged.Broadcast(NewValue);
	OnMaxValueChanged.Broadcast(MaxValue);
	OnPercentChanged.Broadcast(MaxValue > 0.0f ? (NewValue / MaxValue) : 0.0f);
}

void UMHWOverlayWidgetController::HandleHealthChanged(const float NewValue, const float MaxValue, float /*DeltaValue*/)
{
	BroadcastResourceChanged(NewValue, MaxValue, OnHealthChanged, OnMaxHealthChanged, OnHealthPercentChanged);
}

void UMHWOverlayWidgetController::HandleStaminaChanged(const float NewValue, const float MaxValue, float /*DeltaValue*/)
{
	BroadcastResourceChanged(NewValue, MaxValue, OnStaminaChanged, OnMaxStaminaChanged, OnStaminaPercentChanged);
}

void UMHWOverlayWidgetController::HandleDamaged(const float DamageAmount, const float NewHealth)
{
	OnDamaged.Broadcast(DamageAmount, NewHealth);
}

void UMHWOverlayWidgetController::HandleDeath()
{
	OnDeath.Broadcast();
}
