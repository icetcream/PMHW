#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MHWHUD.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UMHWCombatComponent;
class UMHWOverlayWidgetController;
class UMHWUserWidgetBase;
struct FMHWWidgetControllerParams;

UCLASS()
class PMHW_API AMHWHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMHWHUD();

	void InitOverlay(
		APlayerController* PlayerController,
		APlayerState* PlayerState,
		UAbilitySystemComponent* AbilitySystemComponent,
		UAttributeSet* AttributeSet,
		UMHWCombatComponent* CombatComponent);

	UFUNCTION(BlueprintPure, Category = "MHW|UI")
	UMHWOverlayWidgetController* GetOverlayWidgetController(const FMHWWidgetControllerParams& WidgetControllerParams);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UMHWOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|UI")
	TObjectPtr<UMHWUserWidgetBase> OverlayWidget;

private:
	UPROPERTY(EditDefaultsOnly, Category = "MHW|UI")
	TSubclassOf<UMHWUserWidgetBase> OverlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "MHW|UI")
	TSubclassOf<UMHWOverlayWidgetController> OverlayWidgetControllerClass;
};
