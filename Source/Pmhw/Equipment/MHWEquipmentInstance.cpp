// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/MHWEquipmentInstance.h"

#include "EnhancedInputSubsystems.h"
#include "Equipment/MHWEquipmentDefinition.h"
#include "GameFramework/Character.h"
#include "Player/MHWLocalPlayer.h"

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
	else
	{
		return nullptr;
	}
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
		// 获取 Character 的 Mesh
		if (ACharacter* MyCharacter = Cast<ACharacter>(GetPawn()))
		{
			USkeletalMeshComponent* AttachTarget = MyCharacter->GetMesh();
			// 执行 Attach
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			SpawnedActor->AttachToComponent(AttachTarget, FAttachmentTransformRules::KeepRelativeTransform, NewSocketName);
		}
	}
}

const UMHWHitStopData* UMHWEquipmentInstance::GetHitStopData() const
{
	// 如果之前成功绑定了 Definition
	if (EquipmentDef != nullptr)
	{
		// 通过 GetDefault 获取该类的静态配置实例 (CDO)
		const UMHWEquipmentDefinition* DefCDO = GetDefault<UMHWEquipmentDefinition>(EquipmentDef);
		if (DefCDO)
		{
			// 返回策划在蓝图里配好的卡肉数据资产！
			return DefCDO->HitStopData;
		}
	}
	// 如果没配，或者发生了异常，返回空
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
			SpawnedActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
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
	K2_OnEquipped();
}

void UMHWEquipmentInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

