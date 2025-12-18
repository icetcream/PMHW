// MHWCharacter.cpp
#include "Character/MHWCharacter.h"
#include "Input/MHWInputComponent.h" // 引入自定义组件
#include "Input/MHWInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "MHWGameplayTags.h"
#include "Input/MHWInputComponent.h"



AMHWCharacter::AMHWCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 告诉 UE 使用我们自定义的 InputComponent 类，而不是默认的
	// 这是 Lyra 的关键技巧之一
}


