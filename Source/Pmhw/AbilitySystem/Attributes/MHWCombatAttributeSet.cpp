#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"

#include "AbilitySystem/MHWGameplayEffectContext.h"
#include "Character/MHWCombatComponent.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCombatAttributeSet)

UMHWCombatAttributeSet::UMHWCombatAttributeSet()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitStamina(100.0f);
	InitMaxStamina(100.0f);
	InitIncomingDamage(0.0f);
}

void UMHWCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttributeValue(Attribute, NewValue);

	if (Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, NewValue));
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, NewValue));
	}
}

void UMHWCombatAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttributeValue(Attribute, NewValue);
}

void UMHWCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = FMath::Max(GetIncomingDamage(), 0.0f);
		SetIncomingDamage(0.0f);

		if (LocalIncomingDamage > 0.0f)
		{
			const float NewHealth = FMath::Clamp(GetHealth() - LocalIncomingDamage, 0.0f, GetMaxHealth());
			SetHealth(NewHealth);

			AActor* SourceActor = nullptr;
			EMHWCriticalHitType CriticalHitType = EMHWCriticalHitType::None;
			bool bHasDamageNumberWorldLocation = false;
			FVector DamageNumberWorldLocation = FVector::ZeroVector;
			const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
			if (UObject* SourceObject = EffectContext.GetSourceObject())
			{
				SourceActor = Cast<AActor>(SourceObject);
			}

			if (const FGameplayEffectContext* RawEffectContext = EffectContext.Get())
			{
				if (RawEffectContext->GetScriptStruct() == FMHWGameplayEffectContext::StaticStruct())
				{
					const FMHWGameplayEffectContext* MHWEffectContext = static_cast<const FMHWGameplayEffectContext*>(RawEffectContext);
					CriticalHitType = MHWEffectContext->GetCriticalHitType();
					bHasDamageNumberWorldLocation = MHWEffectContext->HasDamageNumberWorldLocation();
					if (bHasDamageNumberWorldLocation)
					{
						DamageNumberWorldLocation = MHWEffectContext->GetDamageNumberWorldLocation();
					}
				}
			}

			if (!SourceActor)
			{
				SourceActor = EffectContext.GetEffectCauser();
			}

			if (!SourceActor)
			{
				SourceActor = EffectContext.GetOriginalInstigator();
			}

			if (AActor* OwningActor = GetOwningActor())
			{
				if (UMHWCombatComponent* CombatComponent = OwningActor->FindComponentByClass<UMHWCombatComponent>())
				{
					CombatComponent->NotifyDamageReceived(LocalIncomingDamage, NewHealth, SourceActor, CriticalHitType, bHasDamageNumberWorldLocation, DamageNumberWorldLocation);
				}
			}
		}
		return;
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}
}

void UMHWCombatAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxHealthAttribute() || Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}
