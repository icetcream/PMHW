// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MHWPawnExtensionComponent.h"

#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Character/MHWPawnData.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "MHWGameplayTags.h"
#include "MHWLogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPawnExtensionComponent)

namespace MHWPawnExtensionComponent
{
	static const TArray<FGameplayTag> InitializationStateChain = {
		MHWInitStateTags::Spawned,
		MHWInitStateTags::DataAvailable,
		MHWInitStateTags::DataInitialized,
		MHWInitStateTags::GameplayReady
	};
}

UMHWPawnExtensionComponent::UMHWPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	ResetInitializationState();
}

void UMHWPawnExtensionComponent::SetPawnData(const UMHWPawnData* InPawnData)
{
	check(InPawnData);
	APawn* Pawn = GetPawnChecked<APawn>();

	if (PawnData)
	{
		UE_LOG(LogPMHW, Error, TEXT("Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(Pawn), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	CheckInitialization();
}

void UMHWPawnExtensionComponent::SetIsInput(bool isInput)
{
	if (bIsInputSet == isInput)
	{
		return;
	}

	bIsInputSet = isInput;
	CheckInitialization();
}

void UMHWPawnExtensionComponent::GetAllComponentFromCharacter()
{
	RefreshPawnComponentCache();
	CheckInitialization();
}

void UMHWPawnExtensionComponent::InitializeAbilitySystem(UMHWAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC && DoesAbilitySystemMatchCurrentContext(InASC, InOwnerActor))
	{
		// The cached ASC already points at this pawn with the expected owner/avatar pairing.
		return;
	}

	if (AbilitySystemComponent)
	{
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogPMHW, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if (ExistingAvatar && ExistingAvatar != Pawn)
	{
		if (UMHWPawnExtensionComponent* OtherExtensionComponent = FindPawnExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	if (ensure(PawnData))
	{
		/*InASC->SetTagRelationshipMapping(PawnData->TagRelationshipMapping);*/
	}

	OnAbilitySystemInitialized.Broadcast();
}

void UMHWPawnExtensionComponent::UninitializeAbilitySystem()
{
	UMHWAbilitySystemComponent* ExistingASC = AbilitySystemComponent;
	if (!ExistingASC)
	{
		return;
	}

	// Only tear down actor info if this pawn is still the active avatar.
	if (ExistingASC->GetAvatarActor() == GetPawn<APawn>())
	{
		/*
		 * Abilities that should be cancelled when the ASC is detached can be filtered here.
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(MHWGameplayTags::Ability_Behavior_SurvivesDeath);

		ExistingASC->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		*/
		ExistingASC->ClearAbilityInput();
		ExistingASC->RemoveAllGameplayCues();

		if (ExistingASC->GetOwnerActor() != nullptr)
		{
			ExistingASC->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing.
			ExistingASC->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

bool UMHWPawnExtensionComponent::CanChangeInitState(FGameplayTag NextState) const
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return false;
	}

	if (NextState == MHWInitStateTags::Spawned)
	{
		return true;
	}

	if (NextState == MHWInitStateTags::DataAvailable)
	{
		// The controller becomes stable after PossessedBy.
		return Pawn->GetController() != nullptr;
	}

	if (NextState == MHWInitStateTags::DataInitialized)
	{
		return true;
	}

	if (NextState == MHWInitStateTags::GameplayReady)
	{
		// AI pawns and remote server copies never receive local input setup, so only the owning local player waits here.
		return !RequiresInputReady() || bIsInputSet;
	}

	return false;
}

void UMHWPawnExtensionComponent::CheckInitialization()
{
	const int32 CurrentIndex = MHWPawnExtensionComponent::InitializationStateChain.IndexOfByKey(CurrentInitState);
	int32 NextIndex = CurrentIndex + 1;

	while (MHWPawnExtensionComponent::InitializationStateChain.IsValidIndex(NextIndex))
	{
		const FGameplayTag NextState = MHWPawnExtensionComponent::InitializationStateChain[NextIndex];
		if (!CanChangeInitState(NextState))
		{
			break;
		}

		CurrentInitState = NextState;
		HandleInitStateChange(CurrentInitState);
		++NextIndex;
	}
}

void UMHWPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (RegisteredMHWComponents.Num() == 0)
	{
		RefreshPawnComponentCache();
	}

	CheckInitialization();
}

void UMHWPawnExtensionComponent::HandleInitStateChange(FGameplayTag NewState)
{
	for (UMHWPawnComponent* Component : RegisteredMHWComponents)
	{
		if (Component)
		{
			Component->OnActorInitStateChanged(NewState);
		}
	}

	UE_LOG(LogPMHW, Warning, TEXT("[%s] State Changed to: %s"), *GetName(), *NewState.ToString());
}

void UMHWPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();
}

void UMHWPawnExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	RegisteredMHWComponents.Reset();
	ResetInitializationState();

	Super::EndPlay(EndPlayReason);
}

bool UMHWPawnExtensionComponent::DoesAbilitySystemMatchCurrentContext(
	const UMHWAbilitySystemComponent* InASC,
	const AActor* InOwnerActor) const
{
	const APawn* Pawn = GetPawn<APawn>();
	return InASC && Pawn &&
		InASC->GetOwnerActor() == InOwnerActor &&
		InASC->GetAvatarActor() == Pawn;
}

bool UMHWPawnExtensionComponent::RequiresInputReady() const
{
	const APawn* Pawn = GetPawn<APawn>();
	const APlayerController* PlayerController = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	return PlayerController && PlayerController->IsLocalPlayerController();
}

void UMHWPawnExtensionComponent::RefreshPawnComponentCache()
{
	RegisteredMHWComponents.Reset();

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	TArray<UMHWPawnComponent*> FoundComponents;
	Pawn->GetComponents(FoundComponents);
	RegisteredMHWComponents.Reserve(FoundComponents.Num());

	for (UMHWPawnComponent* Component : FoundComponents)
	{
		if (Component && Component != this)
		{
			RegisteredMHWComponents.AddUnique(Component);
		}
	}
}

void UMHWPawnExtensionComponent::ResetInitializationState()
{
	CurrentInitState = FGameplayTag();
	bIsInputSet = false;
}
