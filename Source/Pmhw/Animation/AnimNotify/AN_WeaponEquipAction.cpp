#include "Animation/AnimNotify/AN_WeaponEquipAction.h"

#include "Character/MHWCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "Interface/MHWCharacterInterface.h"

namespace WeaponEquipActionNotify
{
	static AActor* GetOwnerActor(USkeletalMeshComponent* MeshComp)
	{
		return MeshComp ? MeshComp->GetOwner() : nullptr;
	}

	static UMHWEquipmentManagerComponent* GetEquipmentManager(AActor* OwnerActor)
	{
		if (!OwnerActor || !OwnerActor->Implements<UMHWCharacterInterface>())
		{
			return nullptr;
		}

		return const_cast<UMHWEquipmentManagerComponent*>(IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(OwnerActor));
	}

	static UMHWEquipmentInstance* GetWeaponInstance(AActor* OwnerActor, TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass)
	{
		UMHWEquipmentManagerComponent* EquipmentManager = GetEquipmentManager(OwnerActor);
		if (!EquipmentManager)
		{
			return nullptr;
		}

		TSubclassOf<UMHWEquipmentInstance> QueryType = WeaponInstanceClass;
		if (!QueryType)
		{
			QueryType = UMHWEquipmentInstance::StaticClass();
		}

		return EquipmentManager->GetFirstInstanceOfType(QueryType);
	}
}

UAN_WeaponEquipAction::UAN_WeaponEquipAction()
{
}

void UAN_WeaponEquipAction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = WeaponEquipActionNotify::GetOwnerActor(MeshComp);
	if (!Owner)
	{
		return;
	}

	if (bUpdateWeaponSocket)
	{
		if (UMHWEquipmentInstance* EquipmentInstance = WeaponEquipActionNotify::GetWeaponInstance(Owner, WeaponInstanceClass))
		{
			EquipmentInstance->UpdateAttachment(TargetWeaponSocket);
		}
	}

	if (bLinkAnimLayer && LinkedAnimLayerClass)
	{
		MeshComp->LinkAnimClassLayers(LinkedAnimLayerClass);
	}

	if (bUnlinkAnimLayer && UnlinkedAnimLayerClass)
	{
		MeshComp->UnlinkAnimClassLayers(UnlinkedAnimLayerClass);
	}

	if (bSetCurrentWeaponState)
	{
		if (AMHWCharacter* Character = Cast<AMHWCharacter>(Owner))
		{
			Character->SetCurrentWeaponState(NewWeaponStateTag);
		}
	}
}

FString UAN_WeaponEquipAction::GetNotifyName_Implementation() const
{
	return TEXT("WeaponEquipAction");
}
