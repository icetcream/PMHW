#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Pawn.h"
#include "MHWCombatPawn.generated.h"

class UMHWAbilitySystemComponent;
class UMHWCombatAttributeSet;
class UMHWCombatComponent;
class USceneComponent;

UCLASS()
class PMHW_API AMHWCombatPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMHWCombatPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "MHW|Combatant|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combatant|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combatant|Attributes")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "MHW|Combatant|Attributes")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "MHW|Combatant|Attributes")
	bool ApplyRawDamage(float DamageAmount);

	UFUNCTION(BlueprintPure, Category = "MHW|Combatant|Attributes")
	const UMHWCombatAttributeSet* GetCombatAttributeSet() const { return CombatAttributeSet; }

	UFUNCTION(BlueprintPure, Category = "MHW|Combatant|Attributes")
	UMHWCombatComponent* GetCombatComponent() const { return CombatComponent; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Combatant", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Combatant", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Combatant", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWCombatAttributeSet> CombatAttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Combatant", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWCombatComponent> CombatComponent;
};
