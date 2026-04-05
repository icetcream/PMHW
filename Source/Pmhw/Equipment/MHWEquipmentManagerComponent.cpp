// Copyright Epic Games, Inc. All Rights Reserved.

#include "Equipment/MHWEquipmentManagerComponent.h"

#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EnhancedInputSubsystems.h"
#include "Character/MHWCharacter.h"
#include "Equipment/MHWEquipmentDefinition.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Player/MHWLocalPlayer.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWEquipmentManagerComponent)

#define WEAPON_PRIORITY 1
//////////////////////////////////////////////////////////////////////
// FMHWAppliedEquipmentEntry

class UEnhancedInputLocalPlayerSubsystem;
class UMHWLocalPlayer;

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

APawn* FMHWEquipmentList::GetPawn() const
{
	return OwnerComponent ? Cast<APawn>(OwnerComponent->GetOwner()) : nullptr;
}

void FMHWEquipmentList::AddInputMappingContext(const UMHWEquipmentDefinition* EquipmentCDO)
{
	if (!EquipmentCDO || !EquipmentCDO->InputMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSystem())
	{
		FModifyContextOptions Options;
		Options.bIgnoreAllPressedKeysUntilRelease = false;
		Subsystem->AddMappingContext(EquipmentCDO->InputMappingContext, WEAPON_PRIORITY, Options);
	}
}

void FMHWEquipmentList::RemoveInputMappingContext(const UMHWEquipmentDefinition* EquipmentCDO)
{
	if (!EquipmentCDO || !EquipmentCDO->InputMappingContext)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSystem())
	{
		Subsystem->RemoveMappingContext(EquipmentCDO->InputMappingContext);
	}
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
	if (!Result)
	{
		Entries.RemoveAt(Entries.Num() - 1);
		return nullptr;
	}

	Result->SetEquipmentDefinition(EquipmentDefinition);

	if (AMHWCharacter* Character = Cast<AMHWCharacter>(GetPawn()))
	{
		if (EquipmentCDO->WeaponStateTag.IsValid())
		{
			NewEntry.AppliedWeaponStateTag = EquipmentCDO->WeaponStateTag;
		}

		if (EquipmentCDO->bAddMovementSettingsOnEquip && NewEntry.AppliedWeaponStateTag.IsValid())
		{
			NewEntry.bAppliedMovementSettingsOnEquip =
				Character->AddOrUpdateMovementSettingsForWeaponState(NewEntry.AppliedWeaponStateTag, EquipmentCDO->MovementSettingsToAdd);
		}
	}

	// Input mappings are only meaningful for locally controlled pawns. Server-only or AI-owned
	// equipment can still grant actors/abilities without requiring a local player subsystem.
	AddInputMappingContext(EquipmentCDO);

	if (UMHWAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (const TObjectPtr<const UMHWAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
			}
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
			const UMHWEquipmentDefinition* EquipmentCDO = GetDefault<UMHWEquipmentDefinition>(Entry.EquipmentDefinition);

			RemoveInputMappingContext(EquipmentCDO);

			if (AMHWCharacter* Character = Cast<AMHWCharacter>(GetPawn()))
			{
				if (Entry.bAppliedMovementSettingsOnEquip && Entry.AppliedWeaponStateTag.IsValid())
				{
					Character->RemoveMovementSettingsForWeaponState(Entry.AppliedWeaponStateTag);
				}
			}

			if (UMHWAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			}

			Instance->DestroyEquipmentActors();

			EntryIt.RemoveCurrent();
		}
	}
}

UEnhancedInputLocalPlayerSubsystem* FMHWEquipmentList::GetEnhancedInputSystem()
{
	if (EnhancedInputSubsystem)
	{
		return EnhancedInputSubsystem.Get();
	}

	APawn* Pawn = GetPawn();
	if (!Pawn)
	{
		return nullptr;
	}

	const APlayerController* PC = Pawn->GetController<APlayerController>();
	if (!PC)
	{
		return nullptr;
	}

	const UMHWLocalPlayer* LP = Cast<UMHWLocalPlayer>(PC->GetLocalPlayer());
	if (!LP)
	{
		return nullptr;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	EnhancedInputSubsystem = Subsystem;
	return EnhancedInputSubsystem.Get();
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

void UMHWEquipmentManagerComponent::EquipInitialEquipment()
{
	if (bInitialEquipmentEquipped || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UMHWEquipmentDefinition> EquipmentDefinition : InitialEquipmentDefinitions)
	{
		if (EquipmentDefinition)
		{
			EquipItem(EquipmentDefinition);
		}
	}

	bInitialEquipmentEquipped = true;
}

void UMHWEquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();
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

void UMHWEquipmentManagerComponent::OnActorInitStateChanged(FGameplayTag CurrentState)
{
	Super::OnActorInitStateChanged(CurrentState);

	if (CurrentState == MHWInitStateTags::GameplayReady)
	{
		EquipInitialEquipment();
	}
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

void UMHWEquipmentManagerComponent::OnWeaponDrawnStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	bool bIsDrawn = (NewCount > 0);
	
    
	if (bIsDrawn)
	{
		/*UMHWEquipmentInstance* Weapon = GetFirstInstanceOfType(UAnimInstance::StaticClass());
		TArray<AActor*> SpawnActors = Weapon->GetSpawnedActors();
		if (SpawnActors.IsEmpty()) return;
		Weapon->GetSpawnedActors()[0]->AttachToActor(Weapon->GetPawn(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Hand_R_Socket"));
		*/
		// 链接战斗动画层
		/*GetPawn<ACharacter>()->GetMesh()->LinkAnimClassLayers(Weapon);*/
	}
	else
	{
		// 收刀了！
		// 挂载回后背
		/*WeaponMesh->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Back_Socket"));
		// 断开战斗动画层，恢复空手
		CharacterMesh->UnlinkAnimClassLayers(WeaponAnimLayerClass);*/
	}
}
