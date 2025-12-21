// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"

#include "MHWEquipmentDefinition.generated.h"

class AActor;
class UMHWAbilitySet;
class UMHWEquipmentInstance;

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
	/*UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UMHWEquipmentInstance> InstanceType;*/

	// Gameplay ability sets to grant when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UMHWAbilitySet>> AbilitySetsToGrant;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FMHWEquipmentActorToSpawn> ActorsToSpawn;
};
