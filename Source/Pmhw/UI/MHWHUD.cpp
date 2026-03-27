#include "UI/MHWHUD.h"

#include "UI/Controller/MHWOverlayWidgetController.h"
#include "UI/Controller/MHWWidgetController.h"
#include "UI/Widget/MHWUserWidgetBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWHUD)

AMHWHUD::AMHWHUD()
{
	OverlayWidgetControllerClass = UMHWOverlayWidgetController::StaticClass();
}

void AMHWHUD::InitOverlay(
	APlayerController* PlayerController,
	APlayerState* PlayerState,
	UAbilitySystemComponent* AbilitySystemComponent,
	UAttributeSet* AttributeSet,
	UMHWCombatComponent* CombatComponent)
{
	if (!ensure(OverlayWidgetClass) || !ensure(OverlayWidgetControllerClass))
	{
		return;
	}

	if (!OverlayWidget)
	{
		OverlayWidget = CreateWidget<UMHWUserWidgetBase>(PlayerController, OverlayWidgetClass);
		if (OverlayWidget)
		{
			OverlayWidget->AddToViewport();
		}
	}

	if (!OverlayWidget)
	{
		return;
	}

	const FMHWWidgetControllerParams WidgetControllerParams(
		PlayerController,
		PlayerState,
		AbilitySystemComponent,
		AttributeSet,
		CombatComponent);

	OverlayWidgetController = GetOverlayWidgetController(WidgetControllerParams);
	OverlayWidget->SetWidgetController(OverlayWidgetController);
	OverlayWidgetController->BroadcastInitialValues();
}

UMHWOverlayWidgetController* AMHWHUD::GetOverlayWidgetController(const FMHWWidgetControllerParams& WidgetControllerParams)
{
	if (!OverlayWidgetController)
	{
		OverlayWidgetController = NewObject<UMHWOverlayWidgetController>(this, OverlayWidgetControllerClass);
	}

	if (OverlayWidgetController)
	{
		OverlayWidgetController->SetWidgetControllerParams(WidgetControllerParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}

	return OverlayWidgetController;
}
