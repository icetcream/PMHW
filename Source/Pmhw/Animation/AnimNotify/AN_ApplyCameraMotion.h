#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Camera/MHWCombatCameraTypes.h"
#include "GameplayTagContainer.h"
#include "AN_ApplyCameraMotion.generated.h"

UCLASS()
class PMHW_API UAN_ApplyCameraMotion : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bResetToDefault = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "!bResetToDefault", EditConditionHides, Categories = "Data.CameraMotion"))
	FGameplayTag CameraMotionTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "!bResetToDefault", EditConditionHides))
	FMHWCombatCameraArmMotionSettings CameraMotion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (EditCondition = "bResetToDefault", EditConditionHides, ClampMin = "0.0", UIMin = "0.0"))
	float ResetBlendDuration = 0.2f;
};
