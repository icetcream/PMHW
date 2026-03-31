#pragma once

#include "CoreMinimal.h"
#include "Character/MHWCombatPawn.h"
#include "MHWTrainingPillar.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;

UCLASS()
class PMHW_API AMHWTrainingPillar : public AMHWCombatPawn
{
	GENERATED_BODY()

public:
	AMHWTrainingPillar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;


};
