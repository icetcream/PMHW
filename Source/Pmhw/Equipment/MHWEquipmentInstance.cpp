// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/MHWEquipmentInstance.h"

#include "Character/MHWPlayerCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Data/InputActionMappingAsset.h"
#include "Data/MHWAttackDataTable.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Equipment/MHWEquipmentDefinition.h"
#include "GameFramework/Character.h"
#include "Subsystems/ConfigManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWEquipmentInstance)

namespace EquipmentInstance
{
	static USkeletalMeshComponent* GetCharacterMesh(APawn* Pawn)
	{
		if (ACharacter* Character = Cast<ACharacter>(Pawn))
		{
			return Character->GetMesh();
		}

		return nullptr;
	}

	static UMHWPlayerCombatComponent* GetPlayerCombatComponent(APawn* Pawn)
	{
		return Pawn ? Pawn->FindComponentByClass<UMHWPlayerCombatComponent>() : nullptr;
	}

	static UConfigManager* GetConfigManager(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		UGameInstance* GameInstance = World->GetGameInstance();
		return GameInstance ? GameInstance->GetSubsystem<UConfigManager>() : nullptr;
	}
}

class UClass;
class USceneComponent;

UMHWEquipmentInstance::UMHWEquipmentInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHWEquipmentInstance::SetEquipmentDefinition(TSubclassOf<UMHWEquipmentDefinition> InDef)
{
	EquipmentDef = InDef;
}

UWorld* UMHWEquipmentInstance::GetWorld() const
{
	if (APawn* OwningPawn = GetPawn())
	{
		return OwningPawn->GetWorld();
	}

	return nullptr;
}

APawn* UMHWEquipmentInstance::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

APawn* UMHWEquipmentInstance::GetTypedPawn(TSubclassOf<APawn> PawnType) const
{
	APawn* OwningPawn = GetPawn();
	if (!OwningPawn)
	{
		return nullptr;
	}

	if (UClass* ActualPawnType = PawnType)
	{
		return OwningPawn->IsA(ActualPawnType) ? OwningPawn : nullptr;
	}

	return OwningPawn;
}

void UMHWEquipmentInstance::UpdateAttachment(FName NewSocketName)
{
	if (SpawnedActor)
	{
		if (USkeletalMeshComponent* AttachTarget = EquipmentInstance::GetCharacterMesh(GetPawn()))
		{
			SpawnedActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName);
		}
	}
}

const UMHWHitStopData* UMHWEquipmentInstance::GetHitStopData() const
{
	if (const UMHWEquipmentDefinition* DefCDO = GetEquipmentDefinition())
	{
		return DefCDO->HitStopData;
	}

	return nullptr;
}

const UMHWHitStopData* UMHWEquipmentInstance::ResolveHitStopDataForMotionValue(float MotionValue, bool bIsFinisher) const
{
	const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition();
	if (!EquipmentDefinition)
	{
		return nullptr;
	}

	const FMHWHitStopTierConfig& TierConfig = EquipmentDefinition->HitStopTierConfig;

	if (bIsFinisher && TierConfig.FinisherHitStopData)
	{
		return TierConfig.FinisherHitStopData;
	}

	if (MotionValue >= TierConfig.HeavyMotionValueThreshold && TierConfig.HeavyHitStopData)
	{
		return TierConfig.HeavyHitStopData;
	}

	if (MotionValue >= TierConfig.MediumMotionValueThreshold && TierConfig.MediumHitStopData)
	{
		return TierConfig.MediumHitStopData;
	}

	if (TierConfig.LightHitStopData)
	{
		return TierConfig.LightHitStopData;
	}

	return EquipmentDefinition->HitStopData;
}

float UMHWEquipmentInstance::CalculateHitStopStrengthMultiplier(float HitzoneValue, bool bIsSleepHit, bool bIsFinisher) const
{
	const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition();
	if (!EquipmentDefinition)
	{
		return 1.0f;
	}

	const FMHWHitStopTierConfig& TierConfig = EquipmentDefinition->HitStopTierConfig;
	float Multiplier = TierConfig.MidHitzoneMultiplier;

	if (HitzoneValue > 0.0f)
	{
		if (HitzoneValue < TierConfig.LowHitzoneThreshold)
		{
			Multiplier *= TierConfig.LowHitzoneMultiplier;
		}
		else if (HitzoneValue >= TierConfig.HighHitzoneThreshold)
		{
			Multiplier *= TierConfig.HighHitzoneMultiplier;
		}
	}

	if (bIsSleepHit)
	{
		Multiplier *= TierConfig.SleepMultiplier;
	}

	if (bIsFinisher)
	{
		Multiplier *= TierConfig.FinisherMultiplier;
	}

	return FMath::Max(0.0f, Multiplier);
}

const FMHWAttackDataRow* UMHWEquipmentInstance::FindAttackDataRowBySpecTag(const FGameplayTag& AttackSpecTag) const
{
	if (!AttackSpecTag.IsValid())
	{
		return nullptr;
	}

	const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition();
	if (!EquipmentDefinition || !EquipmentDefinition->AttackDataTable)
	{
		return nullptr;
	}

	static const FString ContextString = TEXT("MHWAttackDataLookup");
	TArray<FMHWAttackDataRow*> AllRows;
	EquipmentDefinition->AttackDataTable->GetAllRows<FMHWAttackDataRow>(ContextString, AllRows);
	for (const FMHWAttackDataRow* Row : AllRows)
	{
		if (Row && Row->AttackSpecTag == AttackSpecTag)
		{
			return Row;
		}
	}

	return nullptr;
}

const UMHWEquipmentDefinition* UMHWEquipmentInstance::GetEquipmentDefinition() const
{
	return EquipmentDef ? GetDefault<UMHWEquipmentDefinition>(EquipmentDef) : nullptr;
}

const FMHWMeleeTraceConfig* UMHWEquipmentInstance::GetDefaultMeleeTraceConfig() const
{
	if (const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition())
	{
		return &EquipmentDefinition->DefaultMeleeTraceConfig;
	}

	return nullptr;
}

void UMHWEquipmentInstance::SpawnEquipmentActors(const TArray<FMHWEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		USceneComponent* AttachTarget = OwningPawn->GetRootComponent();
		if (ACharacter* Char = Cast<ACharacter>(OwningPawn))
		{
			AttachTarget = Char->GetMesh();
		}

		for (const FMHWEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
		{
			SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
			SpawnedActor->FinishSpawning(FTransform::Identity, true);
			SpawnedActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
			SpawnedActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
		}
	}
}

void UMHWEquipmentInstance::DestroyEquipmentActors()
{
	if (SpawnedActor)
	{
		SpawnedActor->Destroy();
	}
}

void UMHWEquipmentInstance::OnEquipped()
{
	if (!bAppliedPlayerAttackPanelBonus)
	{
		if (const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition())
		{
			if (EquipmentDefinition->PlayerAttackPanelBonus.HasAnyBonus())
			{
				if (UMHWPlayerCombatComponent* PlayerCombatComponent = EquipmentInstance::GetPlayerCombatComponent(GetPawn()))
				{
					PlayerCombatComponent->ApplyAttackPanelBonus(EquipmentDefinition->PlayerAttackPanelBonus);
					bAppliedPlayerAttackPanelBonus = true;
				}
			}
		}
	}

	if (const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition())
	{
		if (EquipmentDefinition->InputActionMappingAsset)
		{
			if (UConfigManager* ConfigManager = EquipmentInstance::GetConfigManager(GetWorld()))
			{
				ConfigManager->SetInputActionMappingAsset(EquipmentDefinition->InputActionMappingAsset);
			}
		}

		if (EquipmentDefinition->LinkedAnimLayerClass)
		{
			if (USkeletalMeshComponent* Mesh = EquipmentInstance::GetCharacterMesh(GetPawn()))
			{
				Mesh->LinkAnimClassLayers(EquipmentDefinition->LinkedAnimLayerClass);
			}
		}
	}

	K2_OnEquipped();
}

void UMHWEquipmentInstance::OnUnequipped()
{
	const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition();

	if (bAppliedPlayerAttackPanelBonus)
	{
		if (EquipmentDefinition)
		{
			if (UMHWPlayerCombatComponent* PlayerCombatComponent = EquipmentInstance::GetPlayerCombatComponent(GetPawn()))
			{
				PlayerCombatComponent->RemoveAttackPanelBonus(EquipmentDefinition->PlayerAttackPanelBonus);
			}
		}

		bAppliedPlayerAttackPanelBonus = false;
	}

	if (EquipmentDefinition)
	{
		if (EquipmentDefinition->LinkedAnimLayerClass)
		{
			if (USkeletalMeshComponent* Mesh = EquipmentInstance::GetCharacterMesh(GetPawn()))
			{
				Mesh->UnlinkAnimClassLayers(EquipmentDefinition->LinkedAnimLayerClass);
			}
		}

		if (EquipmentDefinition->InputActionMappingAsset)
		{
			if (UConfigManager* ConfigManager = EquipmentInstance::GetConfigManager(GetWorld()))
			{
				if (ConfigManager->InputActionMappingAsset == EquipmentDefinition->InputActionMappingAsset)
				{
					ConfigManager->SetInputActionMappingAsset(nullptr);
				}
			}
		}
	}

	K2_OnUnequipped();
}
