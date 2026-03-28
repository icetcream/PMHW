#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MHWCombatBlueprintLibrary.generated.h"

UCLASS()
class PMHW_API UMHWCombatBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	static bool ApplyRawDamageToActor(AActor* TargetActor, float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat")
	static bool ApplyPhysicalDamageToActor(AActor* TargetActor, AActor* SourceActor, const FMHWPhysicalDamageSpec& DamageSpec, bool bHasDamageNumberWorldLocation = false, FVector DamageNumberWorldLocation = FVector::ZeroVector, FString AttackDisplayName = TEXT(""));
};
