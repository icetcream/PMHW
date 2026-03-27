#pragma once

#include "CoreMinimal.h"
#include "MHWMeleeHitVFXTypes.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct PMHW_API FMHWMeleeHitVFXSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX")
	bool bOrientToImpactNormal = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX")
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);
};
