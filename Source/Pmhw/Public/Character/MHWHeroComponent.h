#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h" 
#include "InputMappingContext.h"
#include "MHWPawnComponent.h"
#include "MHWHeroComponent.generated.h"


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
	virtual void BeginPlay() override;

	void Input_Move(const FInputActionValue& InputActionValue);
	void Input_LookMouse(const FInputActionValue& InputActionValue);
	void Input_LookStick(const FInputActionValue& InputActionValue);
	void Input_Crouch(const FInputActionValue& InputActionValue);
	void Input_AutoRun(const FInputActionValue& InputActionValue);
	
	UPROPERTY(EditDefaultsOnly, Category = "MHW|Input")
	TObjectPtr<UMHWInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, Category = "MHW|Input")
	TArray<FInputMappingContextAndPriority> DefaultInputMappings;
private:
	
};