// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MHWHeroComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MHWGameplayTags.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Character/MHWCharacter.h"
#include "Character/MHWPawnData.h"
#include "Character/MHWPawnExtensionComponent.h"
#include "Character/MHWMovementComponent.h"
#include "Components/StateTreeComponent.h"
#include "Data/InputActionMappingAsset.h"
#include "Input/MHWInputComponent.h"
#include "Interface/MHWCharacterInterface.h"
#include "Player/MHWLocalPlayer.h"
#include "Player/MHWPlayerController.h"
#include "Player/MHWPlayerState.h"
#include "Subsystems/ConfigManager.h"

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
					MHWIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,&ThisClass::Input_AbilityInputTagHold, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Move, ETriggerEvent::Completed, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
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
	if (CurrentState == MHWInitStateTags::GameplayReady)
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

		BindMovementBlockMoveTagListener();
		BindRotationBlockTagListener();

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

void UMHWHeroComponent::BindMovementBlockMoveTagListener()
{
	if (bMovementBlockMoveTagListenerBound)
	{
		UpdateMaxWalkSpeedScaleFromTags();
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMHWAbilitySystemComponent* ASC = PawnExtComp->GetMHWAbilitySystemComponent())
		{
			CachedAbilitySystem = ASC;
			MovementBlockMoveTagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Movement_BlockMove, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this,&ThisClass::OnMovementBlockMoveTagChanged);
			bMovementBlockMoveTagListenerBound = true;
			UpdateMaxWalkSpeedScaleFromTags();
		}
	}
}

void UMHWHeroComponent::UpdateMaxWalkSpeedScaleFromTags()
{
	APawn* Pawn = GetPawn<APawn>();
	UMHWAbilitySystemComponent* ASC = CachedAbilitySystem.Get();
	if (!Pawn || !ASC)
	{
		return;
	}

	AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn);
	UMHWMovementComponent* MoveComp = Character ? Cast<UMHWMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	if (!MoveComp)
	{
		return;
	}

	const bool bBlockMove = ASC->HasMatchingGameplayTag(MHWStateTags::Movement_BlockMove);
	MoveComp->SetMaxWalkSpeedScale(bBlockMove ? 0.0f : 1.0f);
}

void UMHWHeroComponent::OnMovementBlockMoveTagChanged([[maybe_unused]] const FGameplayTag CallbackTag, int32 NewCount)
{
	
	APawn* Pawn = GetPawn<APawn>();
	AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn);
	UMHWMovementComponent* MoveComp = Character ? Cast<UMHWMovementComponent>(Character->GetCharacterMovement()) : nullptr;
	if (!MoveComp)
	{
		return;
	}

	MoveComp->SetMaxWalkSpeedScale(NewCount > 0 ? 0.0f : 1.0f);
}

void UMHWHeroComponent::BindRotationBlockTagListener()
{
	if (bRotationBlockTagListenerBound)
	{
		UpdateBlockRotationFromTags();
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMHWAbilitySystemComponent* ASC = PawnExtComp->GetMHWAbilitySystemComponent())
		{
			CachedAbilitySystem = ASC;
			RotationBlockTagDelegateHandle = ASC->RegisterGameplayTagEvent(MHWStateTags::Rotation_BlockRotation, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this,&ThisClass::OnRotationBlockTagChanged);
			bRotationBlockTagListenerBound = true;
			UpdateBlockRotationFromTags();
		}
	}
}

void UMHWHeroComponent::UpdateBlockRotationFromTags()
{
	APawn* Pawn = GetPawn<APawn>();
	UMHWAbilitySystemComponent* ASC = CachedAbilitySystem.Get();
	AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn);
	if (!ASC || !Character)
	{
		return;
	}

	const bool bBlockRotation = ASC->HasMatchingGameplayTag(MHWStateTags::Rotation_BlockRotation);
	Character->SetBlockRotationByTag(bBlockRotation);
}

void UMHWHeroComponent::OnRotationBlockTagChanged([[maybe_unused]] const FGameplayTag CallbackTag, int32 NewCount)
{
	APawn* Pawn = GetPawn<APawn>();
	AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn);
	if (!Character)
	{
		return;
	}

	Character->SetBlockRotationByTag(NewCount > 0);
}

void UMHWHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UMHWAbilitySystemComponent* MHWASC = PawnExtComp->GetMHWAbilitySystemComponent())
			{
				MHWASC->AbilityInputTagPressed(InputTag);
			}
		}	
		if (Pawn->Implements<UMHWCharacterInterface>())
		{
			UStateTreeComponent* StateTreeComp = IMHWCharacterInterface::Execute_GetStateTreeComponent(Pawn);
			UConfigManager* ConfigSystem = Pawn->GetGameInstance()->GetSubsystem<UConfigManager>();
			FInputActionTagSet OutInputTagSet;
			if (ConfigSystem && ConfigSystem->InputActionMappingAsset&&
				ConfigSystem->InputActionMappingAsset->GetTagSetByInput(InputTag, OutInputTagSet))
			{
				if (OutInputTagSet.InputTag.Tag != FGameplayTag())
				{
					StateTreeComp->SendStateTreeEvent(FStateTreeEvent(OutInputTagSet.InputTag.Tag));
				}
			}
			UMHWComboPreInputComponent* PreInputComponent = IMHWCharacterInterface::Execute_GetComboPreInputComponent(GetOuter());
			PreInputComponent->BufferInput(OutInputTagSet.InputTag.Tag, OutInputTagSet.InputTag.Priority);
		}
	}
}
void UMHWHeroComponent::Input_AbilityInputTagHold(FGameplayTag InputTag)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMHWAbilitySystemComponent* MHWASC = PawnExtComp->GetMHWAbilitySystemComponent())
		{
			MHWASC->AbilityInputTagHolded(InputTag);
		}
	}	
	if (Pawn->Implements<UMHWCharacterInterface>())
	{
		UStateTreeComponent* StateTreeComp = IMHWCharacterInterface::Execute_GetStateTreeComponent(Pawn);
		UConfigManager* ConfigSystem = Pawn->GetGameInstance()->GetSubsystem<UConfigManager>();
		FInputActionTagSet OutInputTagSet;
		if (ConfigSystem && ConfigSystem->InputActionMappingAsset&&
			ConfigSystem->InputActionMappingAsset->GetTagSetByInput(InputTag, OutInputTagSet))
		{
			if (OutInputTagSet.HoldTag.Tag != FGameplayTag())
			{
				StateTreeComp->SendStateTreeEvent(FStateTreeEvent(OutInputTagSet.HoldTag.Tag));
			}
		}
		UMHWComboPreInputComponent* PreInputComponent = IMHWCharacterInterface::Execute_GetComboPreInputComponent(GetOuter());
		PreInputComponent->BufferInput(OutInputTagSet.HoldTag.Tag, OutInputTagSet.HoldTag.Priority);
	}
}

void UMHWHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UMHWAbilitySystemComponent* MHWASC = PawnExtComp->GetMHWAbilitySystemComponent())
		{
			MHWASC->AbilityInputTagReleased(InputTag);
		}
	}	
	if (Pawn->Implements<UMHWCharacterInterface>())
	{
		UStateTreeComponent* StateTreeComp = IMHWCharacterInterface::Execute_GetStateTreeComponent(Pawn);
		UConfigManager* ConfigSystem = Pawn->GetGameInstance()->GetSubsystem<UConfigManager>();
		FInputActionTagSet OutInputTagSet;
		if (ConfigSystem && ConfigSystem->InputActionMappingAsset&&
			ConfigSystem->InputActionMappingAsset->GetTagSetByInput(InputTag, OutInputTagSet))
		{
			if (OutInputTagSet.CompleteTag.Tag != FGameplayTag())
			{
				StateTreeComp->SendStateTreeEvent(FStateTreeEvent(OutInputTagSet.CompleteTag.Tag));
			}
		}
		UMHWComboPreInputComponent* PreInputComponent = IMHWCharacterInterface::Execute_GetComboPreInputComponent(GetOuter());
		PreInputComponent->BufferInput(OutInputTagSet.CompleteTag.Tag, OutInputTagSet.CompleteTag.Priority);
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
		
		UMHWInputComponent* MHWInputComp = GetMHWInputComponent();
		MHWInputComp->RawMoveInput = MovementRotation.RotateVector(FVector(Value.X, Value.Y, 0.f));
		
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

UMHWInputComponent* UMHWHeroComponent::GetMHWInputComponent()
{
	if (!IsValid(InputComp))
	{
		UMHWInputComponent* MHWInputComp = Cast<UMHWInputComponent>(GetPawn<APawn>()->GetComponentByClass(UMHWInputComponent::StaticClass()));
		InputComp = MHWInputComp;
	}
	return InputComp;
}

