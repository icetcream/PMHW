#include "Animation/AnimNotify/AN_WeaponEquipAction.h"

#include "Character/MHWCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "Interface/MHWCharacterInterface.h"

UAN_WeaponEquipAction::UAN_WeaponEquipAction()
{
}

void UAN_WeaponEquipAction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	if (bUpdateWeaponSocket && Owner->Implements<UMHWCharacterInterface>())
	{
		if (UMHWEquipmentManagerComponent* EquipmentManager = const_cast<UMHWEquipmentManagerComponent*>(
			IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(Owner)))
		{
			TSubclassOf<UMHWEquipmentInstance> QueryType = WeaponInstanceClass;
			if (!QueryType)
			{
				QueryType = UMHWEquipmentInstance::StaticClass();
			}
			if (UMHWEquipmentInstance* EquipmentInstance = EquipmentManager->GetFirstInstanceOfType(QueryType))
			{
				EquipmentInstance->UpdateAttachment(TargetWeaponSocket);
			}
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
