// MHWInputComponent.h
#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "MHWInputConfig.h"
#include "MHWInputComponent.generated.h"

class UEnhancedInputLocalPlayerSubsystem;

//原本是IA绑定FUNCTION，多一层实现Tag对于FUNCTION
UCLASS(Config = Input)
class PMHW_API UMHWInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	UMHWInputComponent(const FObjectInitializer& ObjectInitializer);
	
	void AddInputMappings(const UMHWInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	void RemoveInputMappings(const UMHWInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const;
	
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UMHWInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound);
	
	// 按下按键 -> 发送 InputTag 给 GAS 组件 -> GAS 激活拥有该 Tag 的技能
	template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
	void BindAbilityActions(const UMHWInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles);

	void RemoveBinds(TArray<uint32>& BindHandles);
};


template<class UserClass, typename FuncType>
void UMHWInputComponent::BindNativeAction(const UMHWInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename PressedFuncType, typename ReleasedFuncType>
void UMHWInputComponent::BindAbilityActions(const UMHWInputConfig* InputConfig, UserClass* Object, PressedFuncType PressedFunc, ReleasedFuncType ReleasedFunc, TArray<uint32>& BindHandles)
{
	check(InputConfig);

	for (const FMHWInputAction& Action : InputConfig->AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag.IsValid())
		{
			if (PressedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Triggered, Object, PressedFunc, Action.InputTag).GetHandle());
			}
			
			if (ReleasedFunc)
			{
				BindHandles.Add(BindAction(Action.InputAction, ETriggerEvent::Completed, Object, ReleasedFunc, Action.InputTag).GetHandle());
			}
		}
	}
}