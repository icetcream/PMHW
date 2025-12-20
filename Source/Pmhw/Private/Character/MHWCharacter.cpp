// MHWCharacter.cpp
#include "Character/MHWCharacter.h"
#include "Input/MHWInputComponent.h" // 引入自定义组件
#include "Input/MHWInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "MHWGameplayTags.h"
#include "Character/MHWPawnExtensionComponent.h"
#include "Input/MHWInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCharacter)

AMHWCharacter::AMHWCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 告诉 UE 使用我们自定义的 InputComponent 类，而不是默认的
	// 这是 Lyra 的关键技巧之一

	MHWPawnExtensionComponent = CreateDefaultSubobject<UMHWPawnExtensionComponent>(TEXT("MHWPawnExtensionComponent"));
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}

void AMHWCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->GetAllComponentFromCharacter();
	}
}


void AMHWCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->CheckInitialization();
	}
}

void AMHWCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->CheckInitialization();
	}
}

void AMHWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->SetIsInput(true);
		MHWPawnExtensionComponent->CheckInitialization();
	}
}


