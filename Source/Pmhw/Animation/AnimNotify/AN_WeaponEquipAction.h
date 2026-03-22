#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "AN_WeaponEquipAction.generated.h"

class UMHWEquipmentInstance;
class UAnimInstance;

UCLASS()
class PMHW_API UAN_WeaponEquipAction : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_WeaponEquipAction();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	// Which equipped weapon instance to operate on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass;

	// Update equipped weapon attachment socket.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bUpdateWeaponSocket = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (EditCondition = "bUpdateWeaponSocket"))
	FName TargetWeaponSocket = NAME_None;

	// Link an anim layer class to character mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Layer")
	bool bLinkAnimLayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Layer", meta = (EditCondition = "bLinkAnimLayer"))
	TSubclassOf<UAnimInstance> LinkedAnimLayerClass;

	// Unlink an anim layer class from character mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Layer")
	bool bUnlinkAnimLayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim Layer", meta = (EditCondition = "bUnlinkAnimLayer"))
	TSubclassOf<UAnimInstance> UnlinkedAnimLayerClass;

	// Update character current weapon state.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon State")
	bool bSetCurrentWeaponState = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon State", meta = (EditCondition = "bSetCurrentWeaponState"))
	FGameplayTag NewWeaponStateTag;
};
