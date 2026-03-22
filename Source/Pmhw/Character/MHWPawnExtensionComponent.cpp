// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MHWPawnExtensionComponent.h"
#include "Character/MHWPawnData.h"
#include "GameplayTagContainer.h"
#include "MHWGameplayTags.h"
#include "MHWLogChannels.h"
#include "AbilitySystem/MHWAbilitySystemComponent.h"
#include "Player/MHWPlayerController.h"
#include "Player/MHWPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWPawnExtensionComponent)

UMHWPawnExtensionComponent::UMHWPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	
	CurrentInitState = FGameplayTag();
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
	bIsInputSet = isInput;
}


void UMHWPawnExtensionComponent::GetAllComponentFromCharacter()
{
	APawn* Pawn = GetPawn<APawn>();
	if (Pawn)
	{
		TArray<UMHWPawnComponent*> FoundComponents;
		Pawn->GetComponents(FoundComponents);
        
		for (UMHWPawnComponent* Comp : FoundComponents)
		{
			RegisteredMHWComponents.Add(Comp);
		}
	}
	
	CheckInitialization();
}

void UMHWPawnExtensionComponent::InitializeAbilitySystem(UMHWAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC)
	{
		// The ability system component hasn't changed.
		return;
	}

	if (AbilitySystemComponent)
	{
		// Clean up the old ability system component.
		UninitializeAbilitySystem();
	}

	APawn* Pawn = GetPawnChecked<APawn>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogPMHW, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(Pawn), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != Pawn))
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
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		/*
		 * 卸下AbilityComponent的时候取消的能力
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(MHWGameplayTags::Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		*/
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

bool UMHWPawnExtensionComponent::CanChangeInitState(FGameplayTag NextState) const
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) return false;

	if (NextState == MHWInitStateTags::Spawned)
		return true;

	if (NextState == MHWInitStateTags::DataAvailable)
	{
		// 必须要有控制器 (PossessedBy 触发后此项才会为真)
		return Pawn->GetController() != nullptr;
	}

	if (NextState == MHWInitStateTags::DataInitialized)
	{
		// 只要 DataAvailable 过了，通常这一步自动放行，
		// 因为这一步是用来给 CombatComponent 执行 InitAbilityActorInfo 的时机
		return true;
	}

	if (NextState == MHWInitStateTags::GameplayReady)
	{
		// 必须等到 SetupPlayerInputComponent 被调用后
		return bIsInputSet;
	}

	return false;
}



void UMHWPawnExtensionComponent::CheckInitialization()
{
	static const TArray<FGameplayTag> StateChain = { MHWInitStateTags::Spawned, MHWInitStateTags::DataAvailable, MHWInitStateTags::DataInitialized, MHWInitStateTags::GameplayReady };
	int32 CurrentIndex = StateChain.IndexOfByKey(CurrentInitState);
	int32 NextIndex = CurrentIndex + 1;

	while (StateChain.IsValidIndex(NextIndex))
	{
		FGameplayTag NextState = StateChain[NextIndex];
		
		if (!CanChangeInitState(NextState))
		{
			break; 
		}

		CurrentInitState = NextState;
		
		HandleInitStateChange(CurrentInitState);
		NextIndex++;
	}
}

void UMHWPawnExtensionComponent::BeginPlay()
{
	Super::BeginPlay();
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
	Super::EndPlay(EndPlayReason);
}