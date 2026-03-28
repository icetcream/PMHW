// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/MHWEquipmentInstance.h"

#include "EnhancedInputSubsystems.h"
#include "Character/MHWPlayerCombatComponent.h"
#include "Data/InputActionMappingAsset.h"
#include "Data/MHWAttackDataTable.h"
#include "Equipment/MHWEquipmentDefinition.h"
#include "GameFramework/Character.h"
#include "Player/MHWLocalPlayer.h"
#include "Subsystems/ConfigManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWEquipmentInstance)

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
	APawn* Result = nullptr;
	if (UClass* ActualPawnType = PawnType)
	{
		if (GetOuter()->IsA(ActualPawnType))
		{
			Result = Cast<APawn>(GetOuter());
		}
	}

	return Result;
}

void UMHWEquipmentInstance::UpdateAttachment(FName NewSocketName)
{
	if (SpawnedActor)
	{
		if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
		{
			USkeletalMeshComponent* AttachTarget = MyCharacter->GetMesh();
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
				if (APawn* OwningPawn = GetPawn())
				{
					if (UMHWPlayerCombatComponent* PlayerCombatComponent = OwningPawn->FindComponentByClass<UMHWPlayerCombatComponent>())
					{
						PlayerCombatComponent->ApplyAttackPanelBonus(EquipmentDefinition->PlayerAttackPanelBonus);
						bAppliedPlayerAttackPanelBonus = true;
					}
				}
			}
		}
	}

	if (const UMHWEquipmentDefinition* EquipmentDefinition = GetEquipmentDefinition())
	{
		if (EquipmentDefinition->InputActionMappingAsset)
		{
			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UConfigManager* ConfigManager = GameInstance->GetSubsystem<UConfigManager>())
					{
						ConfigManager->SetInputActionMappingAsset(EquipmentDefinition->InputActionMappingAsset);
					}
				}
			}
		}

		if (EquipmentDefinition->LinkedAnimLayerClass)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
			{
				if (USkeletalMeshComponent* Mesh = Character->GetMesh())
				{
					Mesh->LinkAnimClassLayers(EquipmentDefinition->LinkedAnimLayerClass);
				}
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
			if (APawn* OwningPawn = GetPawn())
			{
				if (UMHWPlayerCombatComponent* PlayerCombatComponent = OwningPawn->FindComponentByClass<UMHWPlayerCombatComponent>())
				{
					PlayerCombatComponent->RemoveAttackPanelBonus(EquipmentDefinition->PlayerAttackPanelBonus);
				}
			}
		}

		bAppliedPlayerAttackPanelBonus = false;
	}

	if (EquipmentDefinition)
	{
		if (EquipmentDefinition->LinkedAnimLayerClass)
		{
			if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
			{
				if (USkeletalMeshComponent* Mesh = Character->GetMesh())
				{
					Mesh->UnlinkAnimClassLayers(EquipmentDefinition->LinkedAnimLayerClass);
				}
			}
		}

		if (EquipmentDefinition->InputActionMappingAsset)
		{
			if (UWorld* World = GetWorld())
			{
				if (UGameInstance* GameInstance = World->GetGameInstance())
				{
					if (UConfigManager* ConfigManager = GameInstance->GetSubsystem<UConfigManager>())
					{
						if (ConfigManager->InputActionMappingAsset == EquipmentDefinition->InputActionMappingAsset)
						{
							ConfigManager->SetInputActionMappingAsset(nullptr);
						}
					}
				}
			}
		}
	}

	K2_OnUnequipped();
}
