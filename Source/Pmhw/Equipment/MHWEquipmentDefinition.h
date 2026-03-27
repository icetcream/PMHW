// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"
#include "Settings/MovementSettings.h"

#include "MHWEquipmentDefinition.generated.h"

class UMHWHitStopData;
class UInputMappingContext;
class AActor;
class UMHWAbilitySet;
class UMHWEquipmentInstance;

USTRUCT(BlueprintType)
struct PMHW_API FMHWMeleeTraceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Trace")
	FName BaseSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Trace")
	TArray<FName> TraceSockets;
};

USTRUCT()
struct FMHWEquipmentActorToSpawn
{
	GENERATED_BODY()

	FMHWEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category=Equipment)
	
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * UMHWEquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UMHWEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UMHWEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Class to spawn
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UMHWEquipmentInstance> InstanceType;

	// Gameplay ability sets to grant when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UMHWAbilitySet>> AbilitySetsToGrant;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FMHWEquipmentActorToSpawn> ActorsToSpawn;
	
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	UInputMappingContext* InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Feel")
	TObjectPtr<const UMHWHitStopData> HitStopData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FMHWMeleeTraceConfig DefaultMeleeTraceConfig;

	// Weapon state to set on the character when this equipment is equipped.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	FGameplayTag WeaponStateTag = MHWWeaponTags::Sheathed;

	// If enabled, add/update one entry in DA_MovementSettings.WeaponStates on equip.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	bool bAddMovementSettingsOnEquip = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (EditCondition = "bAddMovementSettingsOnEquip"))
	FMHWMovementRotationModeSettings MovementSettingsToAdd;
};                                       
