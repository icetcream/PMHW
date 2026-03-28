#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "MHWGameplayEffectContext.generated.h"

USTRUCT()
struct PMHW_API FMHWGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual FMHWGameplayEffectContext* Duplicate() const override;
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	void SetCriticalHitType(EMHWCriticalHitType InCriticalHitType) { CriticalHitType = InCriticalHitType; }
	EMHWCriticalHitType GetCriticalHitType() const { return CriticalHitType; }

	void SetDamageNumberWorldLocation(const FVector& InDamageNumberWorldLocation)
	{
		DamageNumberWorldLocation = InDamageNumberWorldLocation;
		bHasDamageNumberWorldLocation = true;
	}

	bool HasDamageNumberWorldLocation() const { return bHasDamageNumberWorldLocation; }
	const FVector& GetDamageNumberWorldLocation() const { return DamageNumberWorldLocation; }

	void SetAttackDisplayName(const FString& InAttackDisplayName)
	{
		AttackDisplayName = InAttackDisplayName;
		bHasAttackDisplayName = !AttackDisplayName.IsEmpty();
	}

	bool HasAttackDisplayName() const { return bHasAttackDisplayName; }
	const FString& GetAttackDisplayName() const { return AttackDisplayName; }

private:
	UPROPERTY()
	EMHWCriticalHitType CriticalHitType = EMHWCriticalHitType::None;

	UPROPERTY()
	bool bHasDamageNumberWorldLocation = false;

	UPROPERTY()
	FVector DamageNumberWorldLocation = FVector::ZeroVector;

	UPROPERTY()
	bool bHasAttackDisplayName = false;

	UPROPERTY()
	FString AttackDisplayName;
};

template <>
struct TStructOpsTypeTraits<FMHWGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FMHWGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
