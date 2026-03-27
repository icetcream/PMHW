#include "UI/Controller/MHWWidgetController.h"

#include "AbilitySystem/Attributes/MHWCombatAttributeSet.h"
#include "Player/MHWPlayerController.h"
#include "Player/MHWPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWWidgetController)

void UMHWWidgetController::SetWidgetControllerParams(const FMHWWidgetControllerParams& InParams)
{
	PlayerController = InParams.PlayerController;
	PlayerState = InParams.PlayerState;
	AbilitySystemComponent = InParams.AbilitySystemComponent;
	AttributeSet = InParams.AttributeSet;
	CombatComponent = InParams.CombatComponent;
}

void UMHWWidgetController::BindCallbacksToDependencies()
{
}

void UMHWWidgetController::BroadcastInitialValues()
{
}

AMHWPlayerController* UMHWWidgetController::GetMHWPlayerController()
{
	if (!MHWPlayerController)
	{
		MHWPlayerController = Cast<AMHWPlayerController>(PlayerController);
	}

	return MHWPlayerController;
}

AMHWPlayerState* UMHWWidgetController::GetMHWPlayerState()
{
	if (!MHWPlayerState)
	{
		MHWPlayerState = Cast<AMHWPlayerState>(PlayerState);
	}

	return MHWPlayerState;
}

UMHWCombatAttributeSet* UMHWWidgetController::GetMHWCombatAttributeSet()
{
	if (!MHWCombatAttributeSet)
	{
		MHWCombatAttributeSet = Cast<UMHWCombatAttributeSet>(AttributeSet);
	}

	return MHWCombatAttributeSet;
}
