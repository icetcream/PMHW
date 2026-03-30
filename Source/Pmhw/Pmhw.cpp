// Copyright Epic Games, Inc. All Rights Reserved.

#include "Pmhw.h"

#include "GameFramework/InputSettings.h"
#include "Input/MHWInputComponent.h"
#include "Modules/ModuleManager.h"

class FPmhwModule final : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		FDefaultGameModuleImpl::StartupModule();

		if (UInputSettings::GetDefaultInputComponentClass() != UMHWInputComponent::StaticClass())
		{
			UInputSettings::SetDefaultInputComponentClass(UMHWInputComponent::StaticClass());
		}
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FPmhwModule, Pmhw, "Pmhw");
