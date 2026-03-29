// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Camera/MHWCombatCameraTypes.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "MHWPlayerController.generated.h"

class UMHWCombatCameraModifier;

/**
 * 
 */
UCLASS()
class PMHW_API AMHWPlayerController : public APlayerController, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AMHWPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void PlayCombatCameraSpringShake(const FMHWCameraSpringShakeSettings& InShakeSettings);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void ApplyCombatCameraMotion(const FMHWCombatCameraArmMotionSettings& InMotionSettings);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	bool ApplyCombatCameraMotionByTag(const FGameplayTag& CameraMotionTag);

	UFUNCTION(BlueprintCallable, Category = "MHW|Camera")
	void ClearCombatCameraMotion(float BlendOutDuration = 0.12f);

private:
	void TryInitOverlay(APawn* InPawn);
	UMHWCombatCameraModifier* GetOrCreateCombatCameraModifier() const;

	mutable TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	mutable TWeakObjectPtr<UMHWCombatCameraModifier> CombatCameraModifier;
};
