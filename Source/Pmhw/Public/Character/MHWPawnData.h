// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"

#include "MHWPawnData.generated.h"

class APawn;
/*class UMHWAbilitySet;
class UMHWAbilityTagRelationshipMapping;
class UMHWCameraMode;*/
class UMHWInputConfig;
class UObject;


/**
 * UMHWPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "MHW Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class PMHW_API UMHWPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UMHWPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// Class to instantiate for this pawn (should usually derive from AMHWPawn or AMHWCharacter).
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Pawn")
	TSubclassOf<APawn> PawnClass;*/

	/*// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Abilities")
	TArray<TObjectPtr<UMHWAbilitySet>> AbilitySets;

	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Abilities")
	TObjectPtr<UMHWAbilityTagRelationshipMapping> TagRelationshipMapping;*/

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Input")
	TObjectPtr<UMHWInputConfig> InputConfig;

	// Default camera mode used by player controlled pawns.
	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Camera")
	TSubclassOf<UMHWCameraMode> DefaultCameraMode;*/
};
