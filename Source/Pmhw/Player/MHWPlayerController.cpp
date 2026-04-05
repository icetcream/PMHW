// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWPlayerController.h"
#include "AbilitySystemComponent.h" 
#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Camera/MHWCombatCameraComponent.h"
#include "Camera/MHWCombatCameraModifier.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/MHWCombatComponent.h"
#include "GameplayTagContainer.h"
#include "MHWPlayerState.h"
#include "Input/MHWInputComponent.h"
#include "UI/MHWHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPlayerController)

AMHWPlayerController::AMHWPlayerController()
{
}

void AMHWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	GetOrCreateCombatCameraModifier();
	TryInitOverlay(GetPawn());
}

void AMHWPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	TryInitOverlay(InPawn);
}

UAbilitySystemComponent* AMHWPlayerController::GetAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		AMHWPlayerState* PS = GetPlayerState<AMHWPlayerState>();
		if (PS)
		{
			AbilitySystemComponent = PS->GetAbilitySystemComponent();
			ASC = AbilitySystemComponent.Get();
		}
	}
	return ASC;
}

void AMHWPlayerController::PlayCombatCameraSpringShake(const FMHWCameraSpringShakeSettings& InShakeSettings)
{
	if (UMHWCombatCameraModifier* CameraModifier = GetOrCreateCombatCameraModifier())
	{
		CameraModifier->PlaySpringShake(InShakeSettings);
	}
}

void AMHWPlayerController::ApplyCombatCameraMotion(const FMHWCombatCameraArmMotionSettings& InMotionSettings)
{
	if (UMHWCombatCameraComponent* CameraComponent = GetCombatCameraComponent())
	{
		CameraComponent->ApplyCameraMotion(InMotionSettings);
	}
}

bool AMHWPlayerController::ApplyCombatCameraMotionByTag(const FGameplayTag& CameraMotionTag)
{
	UMHWCombatCameraComponent* CameraComponent = GetCombatCameraComponent();
	return CameraComponent ? CameraComponent->ApplyCameraMotionByTag(CameraMotionTag) : false;
}

void AMHWPlayerController::ClearCombatCameraMotion(const float BlendOutDuration)
{
	if (UMHWCombatCameraComponent* CameraComponent = GetCombatCameraComponent())
	{
		CameraComponent->ClearCameraMotion(BlendOutDuration);
	}
}

UMHWCombatCameraComponent* AMHWPlayerController::GetCombatCameraComponent() const
{
	APawn* ControlledPawn = GetPawn();
	return ControlledPawn ? ControlledPawn->FindComponentByClass<UMHWCombatCameraComponent>() : nullptr;
}

UMHWCombatComponent* AMHWPlayerController::GetCombatComponent(APawn* InPawn) const
{
	return InPawn ? InPawn->FindComponentByClass<UMHWCombatComponent>() : nullptr;
}

void AMHWPlayerController::TryInitOverlay(APawn* InPawn)
{
	if (!IsLocalController())
	{
		return;
	}

	// The overlay only exists for the local player and depends on both player-state and pawn combat data.
	AMHWHUD* MHWHUD = GetHUD<AMHWHUD>();
	AMHWPlayerState* PS = GetPlayerState<AMHWPlayerState>();
	UMHWCombatComponent* CombatComponent = GetCombatComponent(InPawn);
	if (!MHWHUD || !PS || !CombatComponent)
	{
		return;
	}

	MHWHUD->InitOverlay(
		this,
		PS,
		PS->GetMHWAbilitySystemComponent(),
		const_cast<UMHWCombatAttributeSet*>(PS->GetCombatAttributeSet()),
		CombatComponent);
}

UMHWCombatCameraModifier* AMHWPlayerController::GetOrCreateCombatCameraModifier() const
{
	if (!IsLocalController() || !PlayerCameraManager)
	{
		return nullptr;
	}

	if (UMHWCombatCameraModifier* ExistingModifier = CombatCameraModifier.Get())
	{
		return ExistingModifier;
	}

	if (UCameraModifier* FoundModifier = PlayerCameraManager->FindCameraModifierByClass(UMHWCombatCameraModifier::StaticClass()))
	{
		CombatCameraModifier = Cast<UMHWCombatCameraModifier>(FoundModifier);
		return CombatCameraModifier.Get();
	}

	UCameraModifier* NewModifier = PlayerCameraManager->AddNewCameraModifier(UMHWCombatCameraModifier::StaticClass());
	CombatCameraModifier = Cast<UMHWCombatCameraModifier>(NewModifier);
	return CombatCameraModifier.Get();
}

