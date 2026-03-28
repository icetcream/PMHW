#include "AbilitySystem/MHWGameplayEffectContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWGameplayEffectContext)

UScriptStruct* FMHWGameplayEffectContext::GetScriptStruct() const
{
	return StaticStruct();
}

FMHWGameplayEffectContext* FMHWGameplayEffectContext::Duplicate() const
{
	FMHWGameplayEffectContext* NewContext = new FMHWGameplayEffectContext();
	*NewContext = *this;

	if (GetHitResult())
	{
		NewContext->AddHitResult(*GetHitResult(), true);
	}

	return NewContext;
}

bool FMHWGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	const bool bParentSuccess = FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);

	uint8 CriticalHitTypeValue = static_cast<uint8>(CriticalHitType);
	Ar.SerializeBits(&CriticalHitTypeValue, 2);

	uint8 bHasLocationBit = bHasDamageNumberWorldLocation ? 1 : 0;
	Ar.SerializeBits(&bHasLocationBit, 1);

	if (Ar.IsLoading())
	{
		bHasDamageNumberWorldLocation = bHasLocationBit != 0;
	}

	if (bHasDamageNumberWorldLocation)
	{
		Ar << DamageNumberWorldLocation;
	}

	if (Ar.IsLoading())
	{
		CriticalHitType = static_cast<EMHWCriticalHitType>(CriticalHitTypeValue);
	}

	bOutSuccess = bParentSuccess;
	return true;
}
