#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "MHWAnimModifier_AddFootLockCurves.generated.h"

class UAnimSequence;

UCLASS(DisplayName = "MHW Add Foot Lock Curves")
class PMHW_API UMHWAnimModifier_AddFootLockCurves : public UAnimationModifier
{
	GENERATED_BODY()

public:
	virtual void OnApply_Implementation(UAnimSequence* AnimationSequence) override;
	virtual void OnRevert_Implementation(UAnimSequence* AnimationSequence) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Lock", meta = (DisplayName = "Left Foot Curve Name"))
	FName LeftFootCurveName = TEXT("FootLock_L");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Lock", meta = (DisplayName = "Right Foot Curve Name"))
	FName RightFootCurveName = TEXT("FootLock_R");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Lock", meta = (DisplayName = "Recreate Existing Curves"))
	bool bRecreateExistingCurves = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Lock", meta = (DisplayName = "Add Default Start/End Keys"))
	bool bAddDefaultKeys = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot Lock", meta = (DisplayName = "Default Curve Value"))
	float DefaultCurveValue = 0.0f;

private:
	void EnsureCurve(UAnimSequence* AnimationSequence, const FName CurveName) const;
};
