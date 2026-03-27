#include "UI/Controller/MHWOverlayWidgetController.h"

#include "Character/MHWCombatComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWOverlayWidgetController)

void UMHWOverlayWidgetController::BindCallbacksToDependencies()
{
	if (!CombatComponent)
	{
		return;
	}

	if (BoundCombatComponent == CombatComponent)
	{
		return;
	}

	if (BoundCombatComponent)
	{
		BoundCombatComponent->OnHealthChanged.RemoveDynamic(this, &ThisClass::HandleHealthChanged);
		BoundCombatComponent->OnStaminaChanged.RemoveDynamic(this, &ThisClass::HandleStaminaChanged);
		BoundCombatComponent->OnDamaged.RemoveDynamic(this, &ThisClass::HandleDamaged);
		BoundCombatComponent->OnDeath.RemoveDynamic(this, &ThisClass::HandleDeath);
	}

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

void UMHWOverlayWidgetController::HandleHealthChanged(float NewValue, float MaxValue, float DeltaValue)
{
	OnHealthChanged.Broadcast(NewValue);
	OnMaxHealthChanged.Broadcast(MaxValue);
	OnHealthPercentChanged.Broadcast(MaxValue > 0.0f ? (NewValue / MaxValue) : 0.0f);
}

void UMHWOverlayWidgetController::HandleStaminaChanged(float NewValue, float MaxValue, float DeltaValue)
{
	OnStaminaChanged.Broadcast(NewValue);
	OnMaxStaminaChanged.Broadcast(MaxValue);
	OnStaminaPercentChanged.Broadcast(MaxValue > 0.0f ? (NewValue / MaxValue) : 0.0f);
}

void UMHWOverlayWidgetController::HandleDamaged(float DamageAmount, float NewHealth)
{
	OnDamaged.Broadcast(DamageAmount, NewHealth);
}

void UMHWOverlayWidgetController::HandleDeath()
{
	OnDeath.Broadcast();
}
