// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHWPawnComponent.h"
#include "MHWPawnExtensionComponent.generated.h"


class UMHWAbilitySystemComponent;
class UMHWPawnData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWPawnExtensionComponent : public UMHWPawnComponent
{
	GENERATED_BODY()
public:
	UMHWPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "MHW|Pawn")
	static UMHWPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UMHWPawnExtensionComponent>() : nullptr); }
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	
	void SetPawnData(const UMHWPawnData* InPawnData);

	void SetIsInput(bool isInput);
	
	bool CanChangeInitState(FGameplayTag NextState) const;
	void CheckInitialization();
	void HandleInitStateChange(FGameplayTag NewState);

	void GetAllComponentFromCharacter();
	/** Should be called by the owning pawn to become the avatar of the ability system. */
	void InitializeAbilitySystem(UMHWAbilitySystemComponent* InASC, AActor* InOwnerActor);
	/** Should be called by the owning pawn to remove itself as the avatar of the ability system. */
	void UninitializeAbilitySystem();
protected:
	// --- Actor ---
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// --- Actor ---
	
	/** Pawn data used to create the pawn. Specified from a spawn function or on a placed instance. */
	UPROPERTY(EditDefaultsOnly, Category = "MHW|Pawn")
	TObjectPtr<const UMHWPawnData> PawnData;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "InitState")
	FGameplayTag CurrentInitState;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMHWPawnComponent>> RegisteredMHWComponents;
	
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;
	FSimpleMulticastDelegate OnAbilitySystemInitialized;
	/** Pointer to the ability system component that is cached for convenience. */
	UPROPERTY(Transient)
	TObjectPtr<UMHWAbilitySystemComponent> AbilitySystemComponent;

	bool bIsInputSet = false;
};




