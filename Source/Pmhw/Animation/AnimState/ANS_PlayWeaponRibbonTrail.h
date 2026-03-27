#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_PlayWeaponRibbonTrail.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UMHWEquipmentInstance;
class USkeletalMeshComponent;

UCLASS()
class PMHW_API UANS_PlayWeaponRibbonTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	TObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	FName AttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	bool bAutoActivate = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	bool bDestroyAtEnd = false;

private:
	USkeletalMeshComponent* ResolveWeaponMesh(USkeletalMeshComponent* MeshComp) const;

	TMap<TObjectKey<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> ActiveTrailComponents;
};
