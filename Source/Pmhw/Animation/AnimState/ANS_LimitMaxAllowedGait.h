#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MHWGameplayTags.h"
#include "ANS_LimitMaxAllowedGait.generated.h"

UCLASS()
class PMHW_API UANS_LimitMaxAllowedGait : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gait", meta = (Categories = "Als.Gait"))
	FGameplayTag MaxAllowedGait = MHWGaitTags::Walking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gait")
	bool bClearOverrideOnEnd = true;
};
