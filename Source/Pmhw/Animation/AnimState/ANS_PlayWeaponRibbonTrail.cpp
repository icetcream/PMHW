#include "Animation/AnimState/ANS_PlayWeaponRibbonTrail.h"

#include "Components/SkeletalMeshComponent.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/MHWCharacterInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

namespace WeaponRibbonTrailNotify
{
	static UMHWEquipmentManagerComponent* GetEquipmentManager(USkeletalMeshComponent* MeshComp)
	{
		if (!MeshComp)
		{
			return nullptr;
		}

		AActor* OwnerActor = MeshComp->GetOwner();
		if (!OwnerActor || !OwnerActor->Implements<UMHWCharacterInterface>())
		{
			return nullptr;
		}

		return const_cast<UMHWEquipmentManagerComponent*>(IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(OwnerActor));
	}

	static TSubclassOf<UMHWEquipmentInstance> ResolveWeaponQueryType(TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass)
	{
		TSubclassOf<UMHWEquipmentInstance> QueryType = WeaponInstanceClass;
		if (!QueryType)
		{
			QueryType = UMHWEquipmentInstance::StaticClass();
		}

		return QueryType;
	}

	static void ApplyTrailTransform(UNiagaraComponent* NiagaraComponent, const FVector& LocationOffset, const FRotator& RotationOffset, const FVector& Scale)
	{
		if (!NiagaraComponent)
		{
			return;
		}

		NiagaraComponent->SetRelativeLocation(LocationOffset);
		NiagaraComponent->SetRelativeRotation(RotationOffset);
		NiagaraComponent->SetRelativeScale3D(Scale);
	}

	static void DeactivateTrailComponent(UNiagaraComponent* NiagaraComponent, const bool bDestroyAtEnd)
	{
		if (!NiagaraComponent)
		{
			return;
		}

		NiagaraComponent->Deactivate();
		if (bDestroyAtEnd)
		{
			NiagaraComponent->ReleaseToPool();
		}
	}
}

void UANS_PlayWeaponRibbonTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !NiagaraSystem)
	{
		return;
	}

	USkeletalMeshComponent* WeaponMesh = ResolveWeaponMesh(MeshComp);
	if (!WeaponMesh)
	{
		return;
	}

	const TObjectKey<USkeletalMeshComponent> MeshKey(WeaponMesh);
	if (TWeakObjectPtr<UNiagaraComponent>* ExistingComponent = ActiveTrailComponents.Find(MeshKey))
	{
		if (UNiagaraComponent* NiagaraComponent = ExistingComponent->Get())
		{
			WeaponRibbonTrailNotify::DeactivateTrailComponent(NiagaraComponent, bDestroyAtEnd);
			if (!bDestroyAtEnd)
			{
				WeaponRibbonTrailNotify::ApplyTrailTransform(NiagaraComponent, LocationOffset, RotationOffset, Scale);
				NiagaraComponent->Activate(true);
				return;
			}
		}
		if (bDestroyAtEnd)
		{
			ActiveTrailComponents.Remove(MeshKey);
		}
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		WeaponMesh,
		AttachSocketName,
		LocationOffset,
		RotationOffset,
		EAttachLocation::KeepRelativeOffset,
		bAutoActivate,
		true,
		ENCPoolMethod::ManualRelease,
		true);

	if (!NiagaraComponent)
	{
		return;
	}

	WeaponRibbonTrailNotify::ApplyTrailTransform(NiagaraComponent, LocationOffset, RotationOffset, Scale);
	ActiveTrailComponents.Add(MeshKey, NiagaraComponent);
}

void UANS_PlayWeaponRibbonTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	USkeletalMeshComponent* WeaponMesh = ResolveWeaponMesh(MeshComp);
	if (!WeaponMesh)
	{
		return;
	}

	const TObjectKey<USkeletalMeshComponent> MeshKey(WeaponMesh);
	TWeakObjectPtr<UNiagaraComponent>* ActiveComponent = ActiveTrailComponents.Find(MeshKey);
	if (!ActiveComponent)
	{
		return;
	}

	if (UNiagaraComponent* NiagaraComponent = ActiveComponent->Get())
	{
		WeaponRibbonTrailNotify::DeactivateTrailComponent(NiagaraComponent, bDestroyAtEnd);
	}

	if (bDestroyAtEnd)
	{
		ActiveTrailComponents.Remove(MeshKey);
	}
}

FString UANS_PlayWeaponRibbonTrail::GetNotifyName_Implementation() const
{
	return NiagaraSystem
		? FString::Printf(TEXT("WeaponRibbonTrail [%s]"), *NiagaraSystem->GetName())
		: TEXT("WeaponRibbonTrail");
}

USkeletalMeshComponent* UANS_PlayWeaponRibbonTrail::ResolveWeaponMesh(USkeletalMeshComponent* MeshComp) const
{
	if (!MeshComp)
	{
		return nullptr;
	}

	UMHWEquipmentManagerComponent* EquipmentManager = WeaponRibbonTrailNotify::GetEquipmentManager(MeshComp);
	if (!EquipmentManager)
	{
		return nullptr;
	}

	const TSubclassOf<UMHWEquipmentInstance> QueryType = WeaponRibbonTrailNotify::ResolveWeaponQueryType(WeaponInstanceClass);

	UMHWEquipmentInstance* EquipmentInstance = EquipmentManager->GetFirstInstanceOfType(QueryType);
	if (!EquipmentInstance)
	{
		return nullptr;
	}

	AActor* WeaponActor = EquipmentInstance->GetSpawnedActor();
	return WeaponActor ? WeaponActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
}
