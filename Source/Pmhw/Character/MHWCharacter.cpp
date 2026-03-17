// MHWCharacter.cpp
#include "Character/MHWCharacter.h"

#include "AbilitySystemComponent.h"
#include "Input/MHWInputComponent.h" // 引入自定义组件
#include "Input/MHWInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "MeleeTraceComponent.h"
#include "MHWGameplayTags.h"
#include "MHWMovementComponent.h"
#include "Character/MHWPawnExtensionComponent.h"
#include "Components/StateTreeComponent.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Input/MHWInputComponent.h"
#include "Player/MHWPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCharacter)


AMHWCharacter::AMHWCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMHWMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// 告诉 UE 使用我们自定义的 InputComponent 类，而不是默认的

	MHWPawnExtensionComponent = CreateDefaultSubobject<UMHWPawnExtensionComponent>(TEXT("MHWPawnExtensionComponent"));
	MHWEquipmentManagerComponent = CreateDefaultSubobject<UMHWEquipmentManagerComponent>(TEXT("MHWEquipmentManagerComponent"));
	MHWStateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>("MHWStateTreeComponent");
	MHWComboPreInputComponent = CreateDefaultSubobject<UMHWComboPreInputComponent>(TEXT("MHWComboPreInputComponent"));
	MHWMeleeTraceComponent = CreateDefaultSubobject<UMeleeTraceComponent>(TEXT("MHWMeleeTraceComponent"));
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
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

void AMHWCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return;
	float CurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration().Size();
	
	bool bIsIntendingToMove = CurrentAcceleration > 10.0f; 
	
	bool bHasMovingTag = AbilitySystemComponent->HasMatchingGameplayTag(MHWTags::State_IsMoving);
	
	if (bIsIntendingToMove && !bHasMovingTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(MHWTags::State_IsMoving);
	}
	else if (!bIsIntendingToMove && bHasMovingTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(MHWTags::State_IsMoving);
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

UAbilitySystemComponent* AMHWCharacter::GetAbilitySystemComponent() const
{
	AMHWPlayerState* PS = GetPlayerState<AMHWPlayerState>();
	if (PS)
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

const UMHWEquipmentManagerComponent* AMHWCharacter::GetEquipmentManagerComponent_Implementation()
{
	return MHWEquipmentManagerComponent;
}

UStateTreeComponent* AMHWCharacter::GetStateTreeComponent_Implementation()
{
	return MHWStateTreeComponent;
}

UMHWComboPreInputComponent* AMHWCharacter::GetComboPreInputComponent_Implementation()
{
	return MHWComboPreInputComponent;
}

UMeleeTraceComponent* AMHWCharacter::GetMeleeTraceComponent_Implementation()
{
	return MHWMeleeTraceComponent;
}



