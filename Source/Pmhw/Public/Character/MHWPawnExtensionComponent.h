// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHWPawnComponent.h"
#include "MHWPawnExtensionComponent.generated.h"


class UMHWPawnData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWPawnExtensionComponent : public UMHWPawnComponent
{
	GENERATED_BODY()
public:
	UMHWPawnExtensionComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	static UMHWPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UMHWPawnExtensionComponent>() : nullptr); }
	
	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }
	
	void SetPawnData(const UMHWPawnData* InPawnData);
	
	bool CanChangeInitState(FGameplayTag NextState) const;
	void CheckInitialization();
	void HandleInitStateChange(FGameplayTag NewState);
protected:

	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Pawn data used to create the pawn. Specified from a spawn function or on a placed instance. */
	UPROPERTY(EditInstanceOnly, Category = "Lyra|Pawn")
	TObjectPtr<const UMHWPawnData> PawnData;

	UPROPERTY(Transient, VisibleInstanceOnly, Category = "InitState")
	FGameplayTag CurrentInitState;
	
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMHWPawnComponent>> RegisteredMHWComponents;
};


