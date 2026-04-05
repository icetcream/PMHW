// Copyright Epic Games, Inc. All Rights Reserved.

#include "MHWAbilitySet.h"

#include "Abilities/MHWGameplayAbility.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "MHWLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWAbilitySet)

namespace AbilitySet
{
	static bool CanModifyAbilitySet(const UMHWAbilitySystemComponent* MHWASC)
	{
		return MHWASC && MHWASC->IsOwnerActorAuthoritative();
	}

	static void LogInvalidGrant(const UObject* Owner, const TCHAR* Category, const int32 Index)
	{
		UE_LOG(LogPMHWAbilitySystem, Error, TEXT("%s[%d] on ability set [%s] is not valid."), Category, Index, *GetNameSafe(Owner));
	}
}

void FMHWAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FMHWAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FMHWAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* Set)
{
	if (Set)
	{
		GrantedAttributeSets.Add(Set);
	}
}

void FMHWAbilitySet_GrantedHandles::TakeFromAbilitySystem(UMHWAbilitySystemComponent* MHWASC)
{
	check(MHWASC);

	if (!AbilitySet::CanModifyAbilitySet(MHWASC))
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		if (Handle.IsValid())
		{
			MHWASC->ClearAbility(Handle);
		}
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		if (Handle.IsValid())
		{
			MHWASC->RemoveActiveGameplayEffect(Handle);
		}
	}

	for (UAttributeSet* Set : GrantedAttributeSets)
	{
		MHWASC->RemoveSpawnedAttribute(Set);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
	GrantedAttributeSets.Reset();
}

UMHWAbilitySet::UMHWAbilitySet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHWAbilitySet::GiveToAbilitySystem(UMHWAbilitySystemComponent* MHWASC, FMHWAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(MHWASC);

	if (!AbilitySet::CanModifyAbilitySet(MHWASC))
	{
		return;
	}

	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FMHWAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex];

		if (!IsValid(AbilityToGrant.Ability))
		{
			AbilitySet::LogInvalidGrant(this, TEXT("GrantedGameplayAbilities"), AbilityIndex);
			continue;
		}

		UMHWGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UMHWGameplayAbility>();

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.DynamicAbilityTags.AddTag(AbilityToGrant.InputTag);

		const FGameplayAbilitySpecHandle AbilitySpecHandle = MHWASC->GiveAbility(AbilitySpec);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FMHWAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];

		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			AbilitySet::LogInvalidGrant(this, TEXT("GrantedGameplayEffects"), EffectIndex);
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>();
		const FActiveGameplayEffectHandle GameplayEffectHandle = MHWASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, MHWASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}

	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FMHWAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];

		if (!IsValid(SetToGrant.AttributeSet))
		{
			AbilitySet::LogInvalidGrant(this, TEXT("GrantedAttributes"), SetIndex);
			continue;
		}

		UAttributeSet* NewSet = NewObject<UAttributeSet>(MHWASC->GetOwner(), SetToGrant.AttributeSet);
		MHWASC->AddAttributeSetSubobject(NewSet);

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewSet);
		}
	}
}
