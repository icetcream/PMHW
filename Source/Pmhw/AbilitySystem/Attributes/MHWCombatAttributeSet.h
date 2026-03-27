#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MHWCombatAttributeSet.generated.h"

#define MHW_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class PMHW_API UMHWCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMHWCombatAttributeSet();

	MHW_ATTRIBUTE_ACCESSORS(UMHWCombatAttributeSet, Health);
	MHW_ATTRIBUTE_ACCESSORS(UMHWCombatAttributeSet, MaxHealth);
	MHW_ATTRIBUTE_ACCESSORS(UMHWCombatAttributeSet, Stamina);
	MHW_ATTRIBUTE_ACCESSORS(UMHWCombatAttributeSet, MaxStamina);
	MHW_ATTRIBUTE_ACCESSORS(UMHWCombatAttributeSet, IncomingDamage);

	UFUNCTION(BlueprintPure, Category = "MHW|Attributes")
	bool IsAlive() const { return GetHealth() > 0.0f; }

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attributes")
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attributes")
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attributes")
	FGameplayAttributeData Stamina;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attributes")
	FGameplayAttributeData MaxStamina;

	// Damage meta attribute used by damage GameplayEffects.
	UPROPERTY(BlueprintReadOnly, Category = "MHW|Attributes|Meta")
	FGameplayAttributeData IncomingDamage;

private:
	void ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const;
};

#undef MHW_ATTRIBUTE_ACCESSORS
