#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h" 
#include "InputMappingContext.h"
#include "MHWPawnComponent.h"
#include "MHWHeroComponent.generated.h"


class UMHWInputComponent;
class UMHWAbilitySystemComponent;
class UInputMappingContext;
class UMHWInputConfig;
class UInputComponent;
class UEnhancedInputComponent;
class UInputAction;

USTRUCT()
struct FInputMappingContextAndPriority
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Input", meta=(AssetBundles="Client,Server"))
	TObjectPtr<UInputMappingContext> InputMapping;

	// Higher priority input mappings will be prioritized over mappings with a lower priority.
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;
	
	/** If true, then this mapping context will be registered with the settings when this game feature action is registered. */
	/*UPROPERTY(EditAnywhere, Category="Input")
	bool bRegisterWithSettings = true;*/
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWHeroComponent : public UMHWPawnComponent
{
	GENERATED_BODY()

public:	
	UMHWHeroComponent(const FObjectInitializer& ObjectInitializer);
	
	void InitializePlayerInput(UInputComponent* PlayerInputComponent);

protected:
	// --- Actor Interface ---
	virtual void BeginPlay() override;
	// --- Actor Interface End---

	// --- MHWPawnComponent ---
	virtual void OnActorInitStateChanged(FGameplayTag CurrentState) override;
	// --- MHWPawnComponent End ---
	
	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagHold(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
	
	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_AutoRun(const FInputActionValue& InputActionValue);
	
	void SendInputEventToStateTree(FGameplayTag InputTag, APawn* Pawn, FName SuffixName);
	
	UMHWInputComponent* GetMHWInputComponent();
	void BindMovementBlockMoveTagListener();
	void UpdateMaxWalkSpeedScaleFromTags();
	void OnMovementBlockMoveTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	void BindRotationBlockTagListener();
	void UpdateBlockRotationFromTags();
	void OnRotationBlockTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(EditDefaultsOnly, Category = "MHW|Input")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
private:
	UPROPERTY()
	TObjectPtr<UMHWInputComponent> InputComp;

	UPROPERTY(Transient)
	TObjectPtr<UMHWAbilitySystemComponent> CachedAbilitySystem = nullptr;

	FDelegateHandle MovementBlockMoveTagDelegateHandle;
	FDelegateHandle RotationBlockTagDelegateHandle;

	bool bMovementBlockMoveTagListenerBound = false;
	bool bRotationBlockTagListenerBound = false;
};
