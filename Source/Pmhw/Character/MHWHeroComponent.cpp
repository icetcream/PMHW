// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MHWHeroComponent.h"

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
	enum class EAbilityInputPhase : uint8
	{
		Pressed,
		Held,
		Released
	};

	static UMHWAbilitySystemComponent* GetAbilitySystemFromPawn(APawn* Pawn)
	{
		if (!Pawn)
		{
			return nullptr;
		}

		if (UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			return PawnExtComp->GetMHWAbilitySystemComponent();
		}

		return nullptr;
	}

	static bool ResolveInputTagSet(APawn* Pawn, const FGameplayTag& InputTag, FInputActionTagSet& OutInputTagSet)
	{
		if (!Pawn || !Pawn->Implements<UMHWCharacterInterface>())
		{
			return false;
		}

		UConfigManager* ConfigSystem = Pawn->GetGameInstance() ? Pawn->GetGameInstance()->GetSubsystem<UConfigManager>() : nullptr;
		return ConfigSystem
			&& ConfigSystem->InputActionMappingAsset
			&& ConfigSystem->InputActionMappingAsset->GetTagSetByInput(InputTag, OutInputTagSet);
	}

	static const FInputActionTagAndPriority* GetMappedTagForPhase(const FInputActionTagSet& InputTagSet, const EAbilityInputPhase Phase)
	{
		switch (Phase)
		{
		case EAbilityInputPhase::Pressed:
			return &InputTagSet.InputTag;
		case EAbilityInputPhase::Held:
			return &InputTagSet.HoldTag;
		case EAbilityInputPhase::Released:
			return &InputTagSet.CompleteTag;
		default:
			return nullptr;
		}
	}

	static void ForwardAbilityInputToASC(UMHWAbilitySystemComponent* AbilitySystem, const FGameplayTag& InputTag, const EAbilityInputPhase Phase)
	{
		if (!AbilitySystem)
		{
			return;
		}

		switch (Phase)
		{
		case EAbilityInputPhase::Pressed:
			AbilitySystem->AbilityInputTagPressed(InputTag);
			break;
		case EAbilityInputPhase::Held:
			AbilitySystem->AbilityInputTagHolded(InputTag);
			break;
		case EAbilityInputPhase::Released:
			AbilitySystem->AbilityInputTagReleased(InputTag);
			break;
		default:
			break;
		}
	}

	static void ForwardMappedInputToStateTree(APawn* Pawn, const FInputActionTagAndPriority& MappedTag)
	{
		if (!Pawn || !Pawn->Implements<UMHWCharacterInterface>() || !MappedTag.Tag.IsValid())
		{
			return;
		}

		if (UStateTreeComponent* StateTreeComp = IMHWCharacterInterface::Execute_GetStateTreeComponent(Pawn))
		{
			StateTreeComp->SendStateTreeEvent(FStateTreeEvent(MappedTag.Tag));
		}
	}

	static void ForwardMappedInputToPreInputBuffer(APawn* Pawn, const FInputActionTagAndPriority& MappedTag)
	{
		if (!Pawn || !Pawn->Implements<UMHWCharacterInterface>() || !MappedTag.Tag.IsValid())
		{
			return;
		}

		if (UMHWComboPreInputComponent* PreInputComponent = IMHWCharacterInterface::Execute_GetComboPreInputComponent(Pawn))
		{
			PreInputComponent->BufferInput(MappedTag.Tag, MappedTag.Priority);
		}
	}

	static void ForwardMappedInput(APawn* Pawn, const FGameplayTag& InputTag, const EAbilityInputPhase Phase)
	{
		ForwardAbilityInputToASC(GetAbilitySystemFromPawn(Pawn), InputTag, Phase);

		FInputActionTagSet InputTagSet;
		if (!ResolveInputTagSet(Pawn, InputTag, InputTagSet))
		{
			return;
		}

		const FInputActionTagAndPriority* MappedTag = GetMappedTagForPhase(InputTagSet, Phase);
		if (!MappedTag || !MappedTag->Tag.IsValid())
		{
			return;
		}

		// GAS consumes the raw input tag, while StateTree and combo buffering consume the weapon-context mapped tag.
		ForwardMappedInputToStateTree(Pawn, *MappedTag);
		ForwardMappedInputToPreInputBuffer(Pawn, *MappedTag);
	}
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
					InputComp = MHWIC;

					// Add the key mappings that may have been set by the player
					/*MHWIC->AddInputMappings(InputConfig, Subsystem);*/

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events.
		
					TArray<uint32> BindHandles;
					MHWIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed,&ThisClass::Input_AbilityInputTagHold, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Move, ETriggerEvent::Completed, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::LeftShift, ETriggerEvent::Triggered, this, &ThisClass::Input_Sprint, /*bLogIfNotFound=*/ false);
					MHWIC->BindNativeAction(InputConfig, MHWInputTags::LeftShift, ETriggerEvent::Completed, this, &ThisClass::Input_Sprint, /*bLogIfNotFound=*/ false);
					
				}
			}
		}
	}

	

}

void UMHWHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMHWHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupObservedTagListeners();
	Super::EndPlay(EndPlayReason);
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

		if (UMHWPawnExtensionComponent* PawnExtComp = UMHWPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(MHWPS->GetMHWAbilitySystemComponent(), MHWPS);
		}

		if (GetController<AMHWPlayerController>() != nullptr)
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}

		BindAllObservedTagListeners();

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

void UMHWHeroComponent::BindAllObservedTagListeners()
{
	BindObservedTagListener(MHWStateTags::Movement_BlockMove, MovementBlockMoveTagDelegateHandle, bMovementBlockMoveTagListenerBound);
	BindObservedTagListener(MHWStateTags::Rotation_BlockRotation, RotationBlockTagDelegateHandle, bRotationBlockTagListenerBound);
	ApplyObservedTagStates();
}

void UMHWHeroComponent::BindObservedTagListener(const FGameplayTag TagToObserve, FDelegateHandle& DelegateHandle, bool& bListenerBound)
{
	if (bListenerBound)
	{
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
			DelegateHandle = ASC->RegisterGameplayTagEvent(TagToObserve, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this,&ThisClass::OnObservedTagChanged);
			bListenerBound = true;
		}
	}
}

void UMHWHeroComponent::CleanupObservedTagListeners()
{
	UMHWAbilitySystemComponent* ASC = CachedAbilitySystem.Get();
	if (ASC)
	{
		if (bMovementBlockMoveTagListenerBound)
		{
			ASC->RegisterGameplayTagEvent(MHWStateTags::Movement_BlockMove, EGameplayTagEventType::NewOrRemoved).Remove(MovementBlockMoveTagDelegateHandle);
		}

		if (bRotationBlockTagListenerBound)
		{
			ASC->RegisterGameplayTagEvent(MHWStateTags::Rotation_BlockRotation, EGameplayTagEventType::NewOrRemoved).Remove(RotationBlockTagDelegateHandle);
		}
	}

	MovementBlockMoveTagDelegateHandle.Reset();
	RotationBlockTagDelegateHandle.Reset();
	bMovementBlockMoveTagListenerBound = false;
	bRotationBlockTagListenerBound = false;
	CachedAbilitySystem = nullptr;
}

void UMHWHeroComponent::ApplyObservedTagStates()
{
	APawn* Pawn = GetPawn<APawn>();
	UMHWAbilitySystemComponent* ASC = CachedAbilitySystem.Get();
	AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn);
	if (!ASC || !Character)
	{
		return;
	}

	if (UMHWMovementComponent* MoveComp = Cast<UMHWMovementComponent>(Character->GetCharacterMovement()))
	{
		const bool bBlockMove = ASC->HasMatchingGameplayTag(MHWStateTags::Movement_BlockMove);
		MoveComp->SetMaxWalkSpeedScale(bBlockMove ? 0.0f : 1.0f);
	}

	const bool bBlockRotation = ASC->HasMatchingGameplayTag(MHWStateTags::Rotation_BlockRotation);
	Character->SetBlockRotation(bBlockRotation);
}

void UMHWHeroComponent::OnObservedTagChanged([[maybe_unused]] const FGameplayTag CallbackTag, int32 NewCount)
{
	ApplyObservedTagStates();
}

void UMHWHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		MHWHero::ForwardMappedInput(Pawn, InputTag, MHWHero::EAbilityInputPhase::Pressed);
	}
}

void UMHWHeroComponent::Input_AbilityInputTagHold(FGameplayTag InputTag)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	MHWHero::ForwardMappedInput(Pawn, InputTag, MHWHero::EAbilityInputPhase::Held);
}

void UMHWHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	MHWHero::ForwardMappedInput(Pawn, InputTag, MHWHero::EAbilityInputPhase::Released);
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
		if (AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn))
		{
			if (Character->GetDesiredGait() != MHWGaitTags::Sprinting)
			{
				const float MoveInputStrength = Value.Size();
				Character->SetDesiredGait(MoveInputStrength > 0.5f ? MHWGaitTags::Running : MHWGaitTags::Walking);
			}
		}

		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		
		if (UMHWInputComponent* MHWInputComp = GetMHWInputComponent())
		{
			MHWInputComp->RawMoveInput = MovementRotation.RotateVector(FVector(Value.X, Value.Y, 0.f));
		}
		
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

void UMHWHeroComponent::Input_Sprint(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AMHWCharacter* Character = Cast<AMHWCharacter>(Pawn);
	if (!Character)
	{
		return;
	}

	const bool bSprintPressed = InputActionValue.GetMagnitude() > 0.5f;
	if (bSprintPressed)
	{
		Character->SetDesiredGait(MHWGaitTags::Sprinting);
		return;
	}

	const UMHWInputComponent* MHWInputComp = GetMHWInputComponent();
	const float MoveStrength = MHWInputComp ? MHWInputComp->RawMoveInput.Size2D() : 0.0f;
	Character->SetDesiredGait(MoveStrength > 0.5f ? MHWGaitTags::Running : MHWGaitTags::Walking);
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



UMHWInputComponent* UMHWHeroComponent::GetMHWInputComponent()
{
	if (!IsValid(InputComp))
	{
		if (APawn* Pawn = GetPawn<APawn>())
		{
			InputComp = Pawn->FindComponentByClass<UMHWInputComponent>();
		}
	}
	return InputComp;
}
