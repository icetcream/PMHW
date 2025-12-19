// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MHWPawnExtensionComponent.h"
#include "Character/MHWPawnData.h"
#include "GameplayTagContainer.h"
#include "MHWGameplayTags.h"
#include "MHWLogChannels.h"

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

bool UMHWPawnExtensionComponent::CanChangeInitState(FGameplayTag NextState) const
{
	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn) return false;

	if (NextState == FGameplayTag::RequestGameplayTag("State.Spawned"))
		return true;

	if (NextState == FGameplayTag::RequestGameplayTag("State.DataAvailable"))
	{
		// 必须要有控制器 (PossessedBy 触发后此项才会为真)
		return Pawn->GetController() != nullptr;
	}

	if (NextState == FGameplayTag::RequestGameplayTag("State.DataInitialized"))
	{
		// 只要 DataAvailable 过了，通常这一步自动放行，
		// 因为这一步是用来给 CombatComponent 执行 InitAbilityActorInfo 的时机
		return true;
	}

	if (NextState == FGameplayTag::RequestGameplayTag("State.GameplayReady"))
	{
		// 必须等到 SetupPlayerInputComponent 被调用后
		return true;
	}

	return false;
}



void UMHWPawnExtensionComponent::CheckInitialization()
{
	static const TArray<FGameplayTag> StateChain = { MHWTags::InitState_Spawned, MHWTags::InitState_DataAvailable, MHWTags::InitState_DataInitialized, MHWTags::InitState_GameplayReady };
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