#include "Animation/AnimState/ANS_PlayWeaponRibbonTrail.h"

#include "Components/SkeletalMeshComponent.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/MHWCharacterInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

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
			NiagaraComponent->Deactivate();
			if (bDestroyAtEnd)
			{
				NiagaraComponent->DestroyComponent();
			}
		}
		ActiveTrailComponents.Remove(MeshKey);
	}

	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraSystem,
		WeaponMesh,
		AttachSocketName,
		LocationOffset,
		RotationOffset,
		EAttachLocation::KeepRelativeOffset,
		bAutoActivate,
		true);

	if (!NiagaraComponent)
	{
		return;
	}

	NiagaraComponent->SetRelativeScale3D(Scale);
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
		NiagaraComponent->Deactivate();
		if (bDestroyAtEnd)
		{
			NiagaraComponent->DestroyComponent();
		}
	}

	ActiveTrailComponents.Remove(MeshKey);
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

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor || !OwnerActor->Implements<UMHWCharacterInterface>())
	{
		return nullptr;
	}

	const UMHWEquipmentManagerComponent* EquipmentManager = IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(OwnerActor);
	if (!EquipmentManager)
	{
		return nullptr;
	}

	TSubclassOf<UMHWEquipmentInstance> QueryType = WeaponInstanceClass;
	if (!QueryType)
	{
		QueryType = UMHWEquipmentInstance::StaticClass();
	}

	UMHWEquipmentInstance* EquipmentInstance = const_cast<UMHWEquipmentManagerComponent*>(EquipmentManager)->GetFirstInstanceOfType(QueryType);
	if (!EquipmentInstance)
	{
		return nullptr;
	}

	AActor* WeaponActor = EquipmentInstance->GetSpawnedActor();
	return WeaponActor ? WeaponActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
}
