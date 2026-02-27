// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MHWHeroComponent.h"

#include "EnhancedInputSubsystems.h"
#include "MHWGameplayTags.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Character/MHWPawnData.h"
#include "Character/MHWPawnExtensionComponent.h"
#include "Input/MHWInputComponent.h"
#include "Player/MHWLocalPlayer.h"
#include "Player/MHWPlayerController.h"
#include "Player/MHWPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWHeroComponent)

class UMHWPawnExtensionComponent;

namespace MHWHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
}


UMHWHeroComponent::UMHWHeroComponent(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer)
{
}

void UMHWHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);
	
	const UMHWLocalPlayer* LP = Cast<UMHWLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UMHWPawnData* PawnData = PawnExtComp->GetPawnData<UMHWPawnData>())
		{
			if (const UMHWInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping.Get())
					{
						FModifyContextOptions Options = {};
						Options.bIgnoreAllPressedKeysUntilRelease = false;
						// Actually add the config to the local player							
						Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
					}
				}
				UMHWInputComponent* MHWIC = Cast<UMHWInputComponent>(PlayerInputComponent);
				if (ensureMsgf(MHWIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UMHWInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					/*MHWIC->AddInputMappings(InputConfig, Subsystem);*/

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events.
		
					TArray<uint32> BindHandles;
					MHWIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					MHWIC->BindNativeAction(InputConfig, MHWTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}

	

}

void UMHWHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMHWHeroComponent::OnActorInitStateChanged(FGameplayTag CurrentState)
{
	//TODO；可能之后修改这个初始化的阶段
	if (CurrentState == MHWTags::InitState_GameplayReady)
	{
		APawn* Pawn = GetPawn<APawn>();
		AMHWPlayerState* MHWPS = GetPlayerState<AMHWPlayerState>();
		if (!ensure(Pawn && MHWPS))
		{
			return;
		}

		const UMHWPawnData* PawnData = nullptr;

		if (UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnData = PawnExtComp->GetPawnData<UMHWPawnData>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(MHWPS->GetMHWAbilitySystemComponent(), MHWPS);
		}

		if (AMHWPlayerController* MHWPC = GetController<AMHWPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		// Hook up the delegate for all pawns, in case we spectate later
		/*if (PawnData)
		{
			if (UMHWCameraComponent* CameraComponent = UMHWCameraComponent::FindCameraComponent(Pawn))
			{
				CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
			}
		}*/
	}
}

void UMHWHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UMHWAbilitySystemComponent* MHWASC = PawnExtComp->GetLyraAbilitySystemComponent())
			{
				MHWASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UMHWHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMHWAbilitySystemComponent* MHWASC = PawnExtComp->GetLyraAbilitySystemComponent())
		{
			MHWASC->AbilityInputTagReleased(InputTag);
		}
	}	
	
}

void UMHWHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	// If the player has attempted to move again then cancel auto running
	/*if (AMHWPlayerController* MHWController = Cast<AMHWPlayerController>(Controller))
	{
		MHWController->SetIsAutoRunning(false);
	}*/
	
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UMHWHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UMHWHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * MHWHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * MHWHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

void UMHWHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	/*if (AMHWCharacter* Character = GetPawn<AMHWCharacter>())
	{
		Character->ToggleCrouch();
	}*/
}

void UMHWHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	/*if (APawn* Pawn = GetPawn<APawn>())
	{
		if (AMHWPlayerController* Controller = Cast<AMHWPlayerController>(Pawn->GetController()))
		{
			// Toggle auto running
			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
		}	
	}*/
}

