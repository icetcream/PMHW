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
		return;
	}

	Data.CurrentWeaponState = Data.Character->GetCurrentWeaponState();
	if (Data.CurrentWeaponState.IsValid())
	{
		Data.CurrentWeaponStateContainer.AddTag(Data.CurrentWeaponState);
	}

	if (const UCharacterMovementComponent* MoveComp = Data.Character->GetCharacterMovement())
	{
		Data.bHasMovementInput = MoveComp->GetCurrentAcceleration().SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
	}
	else
	{
		Data.bHasMovementInput = false;
	}
}
