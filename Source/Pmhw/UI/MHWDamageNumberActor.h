#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/MHWDamageTypes.h"
#include "GameFramework/Actor.h"
#include "MHWDamageNumberActor.generated.h"

class USceneComponent;
class UWidgetComponent;
class UMHWDamageNumberWidget;

UCLASS()
class PMHW_API AMHWDamageNumberActor : public AActor
{
	GENERATED_BODY()

public:
	AMHWDamageNumberActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "MHW|UI|Damage Number")
	void InitializeDamageNumber(AActor* InTargetActor, float InDamageAmount, EMHWCriticalHitType InCriticalHitType, bool bHasCustomSpawnLocation = false, FVector InCustomSpawnLocation = FVector::ZeroVector);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI")
	TSubclassOf<UMHWDamageNumberWidget> DamageNumberWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI", meta = (ClampMin = "0.01"))
	float Lifetime = 0.9f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI")
	float SpawnHeightOffset = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI", meta = (ClampMin = "0.0"))
	float RandomHorizontalOffset = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI", meta = (ClampMin = "0.0"))
	float ImpactPointRightOffset = 24.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|UI", meta = (ClampMin = "0.0"))
	float FloatSpeed = 55.0f;

private:
	void EnsureWidgetReady();
	void PushDamageNumberToWidget();
	void UpdateFacingRotation();
	FVector ResolveSpawnLocation(AActor* InTargetActor) const;
	FVector ResolveSpawnLocationFromImpactPoint(const FVector& ImpactPoint) const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> TargetActor;

	UPROPERTY(Transient)
	bool bHasCustomSpawnLocation = false;

	UPROPERTY(Transient)
	FVector CustomSpawnLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	float DamageAmount = 0.0f;

	UPROPERTY(Transient)
	EMHWCriticalHitType CriticalHitType = EMHWCriticalHitType::None;

	UPROPERTY(Transient)
	FVector StartLocation = FVector::ZeroVector;

	UPROPERTY(Transient)
	float ElapsedLifetime = 0.0f;

	UPROPERTY(Transient)
	bool bInitialized = false;
};
