// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWAbilitySet.h"
#include "Character/MHWPawnComponent.h"
#include "MHWEquipmentManagerComponent.generated.h"


class UMHWEquipmentDefinition;
class UMHWEquipmentManagerComponent;
class UMHWEquipmentInstance;
struct FMHWEquipmentList;

USTRUCT(BlueprintType)
struct FMHWAppliedEquipmentEntry
{
	GENERATED_BODY()

	FMHWAppliedEquipmentEntry()
	{}

	FString GetDebugString() const;

private:
	friend FMHWEquipmentList;
	friend UMHWEquipmentManagerComponent;

	// The equipment class that got equipped
	UPROPERTY()
	TSubclassOf<UMHWEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UMHWEquipmentInstance> Instance = nullptr;

	// Authority-only list of granted handles
	UPROPERTY()
	FMHWAbilitySet_GrantedHandles GrantedHandles;
};


/** List of applied equipment */
USTRUCT(BlueprintType)
struct FMHWEquipmentList
{
	GENERATED_BODY()

	FMHWEquipmentList()
		: OwnerComponent(nullptr)
	{
	}

	FMHWEquipmentList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

public:
	UMHWEquipmentInstance* AddEntry(TSubclassOf<UMHWEquipmentDefinition> EquipmentDefinition);
	void RemoveEntry(UMHWEquipmentInstance* Instance);

private:
	UMHWAbilitySystemComponent* GetAbilitySystemComponent() const;

	friend UMHWEquipmentManagerComponent;

private:
	// Replicated list of equipment entries
	UPROPERTY()
	TArray<FMHWAppliedEquipmentEntry> Entries;

	UPROPERTY()
	TObjectPtr<UActorComponent> OwnerComponent;
};


class UMHWEquipmentInstance;
/**
 * Manages equipment applied to a pawn
 */
UCLASS(BlueprintType, Const)
class UMHWEquipmentManagerComponent : public UMHWPawnComponent
{
	GENERATED_BODY()

public:
	UMHWEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	UMHWEquipmentInstance* EquipItem(TSubclassOf<UMHWEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItem(UMHWEquipmentInstance* ItemInstance);
	

	//~UActorComponent interface
	virtual void BeginPlay() override;
	//virtual void EndPlay() override;
	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	//~End of UActorComponent interface

	/** Returns the first equipped instance of a given type, or nullptr if none are found */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UMHWEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UMHWEquipmentInstance> InstanceType);

	/** Returns all equipped instances of a given type, or an empty array if none are found */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UMHWEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UMHWEquipmentInstance> InstanceType) const;
	
	void OnWeaponDrawnStateChanged(const FGameplayTag Tag, int32 NewCount);
	
	template <typename T>
	T* GetFirstInstanceOfType()
	{
		return (T*)GetFirstInstanceOfType(T::StaticClass());
	}

private:
	UPROPERTY()
	FMHWEquipmentList EquipmentList;
};
