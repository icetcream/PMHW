// MHWCharacter.cpp
#include "Character/MHWCharacter.h"
#include "Input/MHWInputComponent.h" // 引入自定义组件
#include "Input/MHWInputConfig.h"
#include "EnhancedInputSubsystems.h"
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
	/*if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		// 直接从源头（AnimInstance）抓取这一帧提取到的 Root Motion
		FRootMotionMovementParams ExtractedRootMotion = GetMesh()->GetAnimInstance()->ConsumeExtractedRootMotion(1.0f); // 注意：Consume 会清空数据，仅供 Debug 测试！
        
		// 注意！上面的 Consume 会导致真正的移动失效（因为被你偷吃掉了）
		// 所以这个方法只能用来“看有没有数据”，看完你的角色肯定动不了了。
		// 测完记得删掉。ss

		if (!ExtractedRootMotion.GetRootMotionTransform().GetRotation().IsIdentity())
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, 
				FString::Printf(TEXT("AnimInstance 产出了旋转: %s"), *ExtractedRootMotion.GetRootMotionTransform().GetRotation().Rotator().ToString()));
		}
	}*/
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



