// Fill out your copyright notice in the Description page of Project Settings.


#include "MHWAnimInstance.h"

#include "AbilitySystemGlobals.h"
#include "Character/MHWCharacter.h"
#include "Character/MHWMovementComponent.h"

UMHWAnimInstance::UMHWAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMHWAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* ASC)
{
}

void UMHWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	if (AActor* OwningActor = GetOwningActor())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
		{
			InitializeWithAbilitySystem(ASC);
		}
	}
}

void UMHWAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	const AMHWCharacter* Character = Cast<AMHWCharacter>(GetOwningActor());
	if (!Character)
	{
		return;
	}

	UMHWMovementComponent* CharMoveComp = CastChecked<UMHWMovementComponent>(Character->GetCharacterMovement());
	const FMHWCharacterGroundInfo& GroundInfo = CharMoveComp->GetGroundInfo();
	GroundDistance = GroundInfo.GroundDistance;
}
