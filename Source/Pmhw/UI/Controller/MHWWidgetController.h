#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MHWWidgetController.generated.h"

class AMHWPlayerController;
class AMHWPlayerState;
class UAbilitySystemComponent;
class UAttributeSet;
class UMHWCombatComponent;
class UMHWCombatAttributeSet;

USTRUCT(BlueprintType)
struct FMHWWidgetControllerParams
{
	GENERATED_BODY()

public:
	FMHWWidgetControllerParams() = default;

	FMHWWidgetControllerParams(
		APlayerController* InPlayerController,
		APlayerState* InPlayerState,
		UAbilitySystemComponent* InAbilitySystemComponent,
		UAttributeSet* InAttributeSet,
		UMHWCombatComponent* InCombatComponent)
		: PlayerController(InPlayerController)
		, PlayerState(InPlayerState)
		, AbilitySystemComponent(InAbilitySystemComponent)
		, AttributeSet(InAttributeSet)
		, CombatComponent(InCombatComponent)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|UI")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|UI")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|UI")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|UI")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|UI")
	TObjectPtr<UMHWCombatComponent> CombatComponent;
};

UCLASS(BlueprintType, Blueprintable)
class PMHW_API UMHWWidgetController : public UObject
{
	GENERATED_BODY()

public:
	virtual void SetWidgetControllerParams(const FMHWWidgetControllerParams& InParams);
	virtual void BindCallbacksToDependencies();

	UFUNCTION(BlueprintCallable, Category = "MHW|UI")
	virtual void BroadcastInitialValues();

	UFUNCTION(BlueprintPure, Category = "MHW|UI")
	AMHWPlayerController* GetMHWPlayerController();

	UFUNCTION(BlueprintPure, Category = "MHW|UI")
	AMHWPlayerState* GetMHWPlayerState();

	UFUNCTION(BlueprintPure, Category = "MHW|UI")
	UMHWCombatAttributeSet* GetMHWCombatAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UMHWCombatComponent> CombatComponent;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<AMHWPlayerController> MHWPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<AMHWPlayerState> MHWPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UMHWCombatAttributeSet> MHWCombatAttributeSet;
};
