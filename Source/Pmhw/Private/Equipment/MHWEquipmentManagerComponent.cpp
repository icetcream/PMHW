// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/MHWEquipmentManagerComponent.h"

#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Equipment/MHWEquipmentDefinition.h"
#include "Equipment/MHWEquipmentInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWEquipmentManagerComponent)

//////////////////////////////////////////////////////////////////////
// FMHWAppliedEquipmentEntry

FString FMHWAppliedEquipmentEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance), *GetNameSafe(EquipmentDefinition.Get()));
}


UMHWAbilitySystemComponent* FMHWEquipmentList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	return Cast<UMHWAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

UMHWEquipmentInstance* FMHWEquipmentList::AddEntry(TSubclassOf<UMHWEquipmentDefinition> EquipmentDefinition)
{
	UMHWEquipmentInstance* Result = nullptr;

	check(EquipmentDefinition != nullptr);
 	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());
	
	const UMHWEquipmentDefinition* EquipmentCDO = GetDefault<UMHWEquipmentDefinition>(EquipmentDefinition);

	TSubclassOf<UMHWEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
	if (InstanceType == nullptr)
	{
		InstanceType = UMHWEquipmentInstance::StaticClass();
	}
	
	FMHWAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.Instance = NewObject<UMHWEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);  //@TODO: Using the actor instead of component as the outer due to UE-127172
	Result = NewEntry.Instance;

	if (UMHWAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (const TObjectPtr<const UMHWAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
		}
	}
	else
	{
		//@TODO: Warning logging?
	}

	Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);

	return Result;
}

void FMHWEquipmentList::RemoveEntry(UMHWEquipmentInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FMHWAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			if (UMHWAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			}

			Instance->DestroyEquipmentActors();

			EntryIt.RemoveCurrent();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// UMHWEquipmentManagerComponent

UMHWEquipmentManagerComponent::UMHWEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	SetIsReplicatedByDefault(false);
	bWantsInitializeComponent = true;
}



UMHWEquipmentInstance* UMHWEquipmentManagerComponent::EquipItem(TSubclassOf<UMHWEquipmentDefinition> EquipmentClass)
{
	UMHWEquipmentInstance* Result = nullptr;
	if (EquipmentClass != nullptr)
	{
		Result = EquipmentList.AddEntry(EquipmentClass);
		if (Result != nullptr)
		{
			Result->OnEquipped();
		}
	}
	return Result;
}

void UMHWEquipmentManagerComponent::UnequipItem(UMHWEquipmentInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		ItemInstance->OnUnequipped();
		EquipmentList.RemoveEntry(ItemInstance);
	}
}



void UMHWEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UMHWEquipmentManagerComponent::UninitializeComponent()
{
	TArray<UMHWEquipmentInstance*> AllEquipmentInstances;

	// gathering all instances before removal to avoid side effects affecting the equipment list iterator	
	for (const FMHWAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		AllEquipmentInstances.Add(Entry.Instance);
	}

	for (UMHWEquipmentInstance* EquipInstance : AllEquipmentInstances)
	{
		UnequipItem(EquipInstance);
	}

	Super::UninitializeComponent();
}



UMHWEquipmentInstance* UMHWEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<UMHWEquipmentInstance> InstanceType)
{
	for (FMHWAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UMHWEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

TArray<UMHWEquipmentInstance*> UMHWEquipmentManagerComponent::GetEquipmentInstancesOfType(TSubclassOf<UMHWEquipmentInstance> InstanceType) const
{
	TArray<UMHWEquipmentInstance*> Results;
	for (const FMHWAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UMHWEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				Results.Add(Instance);
			}
		}
	}
	return Results;
}


