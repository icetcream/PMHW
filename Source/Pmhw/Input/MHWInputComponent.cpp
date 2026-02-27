// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/MHWInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWInputComponent)

UMHWInputComponent::UMHWInputComponent(const FObjectInitializer& ObjectInitializer)
{
	
}

void UMHWInputComponent::AddInputMappings(const UMHWInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UMHWInputComponent::RemoveInputMappings(const UMHWInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UMHWInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
}
