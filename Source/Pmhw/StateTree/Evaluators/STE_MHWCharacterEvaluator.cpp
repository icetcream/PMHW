#include "STE_MHWCharacterEvaluator.h"

#include "Character/MHWCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"

void FSTE_MHWCharacterEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTE_MHWCharacterEvaluator_InstanceData& Data = Context.GetInstanceData<FSTE_MHWCharacterEvaluator_InstanceData>(*this);

	Data.CurrentWeaponStateContainer.Reset();

	if (!Data.Character)
	{
		Data.CurrentWeaponState = FGameplayTag();
		Data.bHasMovementInput = false;
		Data.bHasVelocity = false;
		Data.AccelerationFacingTransform = FTransform::Identity;
		return;
	}

	Data.CurrentWeaponState = Data.Character->GetCurrentWeaponState();
	if (Data.CurrentWeaponState.IsValid())
	{
		Data.CurrentWeaponStateContainer.AddTag(Data.CurrentWeaponState);
	}

	if (const UCharacterMovementComponent* MoveComp = Data.Character->GetCharacterMovement())
	{
		const FVector CurrentAcceleration = MoveComp->GetCurrentAcceleration();
		Data.bHasMovementInput = CurrentAcceleration.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
		Data.bHasVelocity = MoveComp->Velocity.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;

		FRotator FacingRot = Data.Character->GetActorRotation();
		if (Data.bHasMovementInput)
		{
			FacingRot = CurrentAcceleration.GetSafeNormal2D().Rotation();
		}

		Data.AccelerationFacingTransform.SetLocation(Data.Character->GetActorLocation());
		Data.AccelerationFacingTransform.SetRotation(FacingRot.Quaternion());
		Data.AccelerationFacingTransform.SetScale3D(FVector::OneVector);
	}
	else
	{
		Data.bHasMovementInput = false;
		Data.bHasVelocity = false;
		Data.AccelerationFacingTransform.SetLocation(Data.Character->GetActorLocation());
		Data.AccelerationFacingTransform.SetRotation(Data.Character->GetActorRotation().Quaternion());
		Data.AccelerationFacingTransform.SetScale3D(FVector::OneVector);
	}
}
