#include "STE_InputEvaluator.h"

#include "Character/MHWCharacter.h"
#include "MHWLogChannels.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "Input/MHWInputComponent.h"
#include "Character/MHWMovementComponent.h"

void FSTE_InputEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FSTE_InputEvaluator_InstanceData& Data = Context.GetInstanceData<FSTE_InputEvaluator_InstanceData>(*this);

	if (!Data.EvaluatorModel)
	{
		Data.EvaluatorModel = NewObject<UControllerInputEvaluatorModel>();
	}
}

void FSTE_InputEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FSTE_InputEvaluator_InstanceData& Data = Context.GetInstanceData<FSTE_InputEvaluator_InstanceData>(*this);

	if (Data.Controller && Data.EvaluatorModel)
	{
		if (ACharacter* CurrentChar = Cast<ACharacter>(Data.Controller->GetPawn()))
		{
			if (const AMHWCharacter* MHWCharacter = Cast<AMHWCharacter>(CurrentChar))
			{
				Data.CurrentWeaponState = MHWCharacter->GetCurrentWeaponState();
				Data.EvaluatorModel->CurrentWeaponState = Data.CurrentWeaponState;
				Data.CurrentWeaponStateContainer.Reset();
				if (Data.CurrentWeaponState.IsValid())
				{
					Data.CurrentWeaponStateContainer.AddTag(Data.CurrentWeaponState);
				}
				Data.EvaluatorModel->CurrentWeaponStateContainer = Data.CurrentWeaponStateContainer;
			}
			else
			{
				Data.CurrentWeaponState = FGameplayTag();
				Data.EvaluatorModel->CurrentWeaponState = FGameplayTag();
				Data.CurrentWeaponStateContainer.Reset();
				Data.EvaluatorModel->CurrentWeaponStateContainer.Reset();
			}

			UMHWMovementComponent* MoveComp = CurrentChar->GetComponentByClass<UMHWMovementComponent>();
			UMHWInputComponent* InputComp = CurrentChar->GetComponentByClass<UMHWInputComponent>();

			if (MoveComp && InputComp)
			{
				Data.EvaluatorModel->MovementComponent = MoveComp;
				Data.EvaluatorModel->InputComponent = InputComp;
				Data.EvaluatorModel->ControlledActor = CurrentChar;
				Data.EvaluatorModel->ActorForwardDirection = CurrentChar->GetActorForwardVector().GetSafeNormal2D();
				Data.EvaluatorModel->UserInputDirection = MoveComp->GetLastInputVector();

				const FVector Acceleration = MoveComp->GetCurrentAcceleration();
				Data.bHasMovementInput = Acceleration.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
				Data.GroundVelocity = MoveComp->Velocity.SizeSquared2D();
				Data.MaxGroundVelocity = MoveComp->MaxWalkSpeed;
				Data.bHasVelocity = Data.GroundVelocity > 0.01f;
				Data.bIsFalling = MoveComp->IsFalling();
				Data.bHasRootMotion = CurrentChar->HasAnyRootMotion();

				if (Data.bHasVelocity)
				{
					FRotator VelocityRot = MoveComp->Velocity.Rotation();
					FRotator ActorRot = CurrentChar->GetActorRotation();
					FRotator DeltaRot = VelocityRot - ActorRot;
					DeltaRot.Normalize();
					Data.VelocityLocalAngle = DeltaRot.Yaw;
				}
				else
				{
					Data.VelocityLocalAngle = 0.f;
				}

				if (Acceleration.SizeSquared2D() > UE_KINDA_SMALL_NUMBER)
				{
					FRotator AccelRot = Acceleration.Rotation();
					FRotator ActorRot = CurrentChar->GetActorRotation();
					FRotator DeltaRot = AccelRot - ActorRot;
					DeltaRot.Normalize();
					Data.AccelerationLocalAngle = DeltaRot.Yaw;
				}
				else
				{
					Data.AccelerationLocalAngle = 0.f;
				}
			}
		}
		else
		{
			Data.EvaluatorModel->ControlledActor = nullptr;
			Data.bHasMovementInput = false;
			Data.bIsFalling = false;
			Data.bHasVelocity = false;
			Data.VelocityLocalAngle = 0.f;
			Data.AccelerationLocalAngle = 0.f;
			Data.CurrentWeaponState = FGameplayTag();
			Data.EvaluatorModel->CurrentWeaponState = FGameplayTag();
			Data.CurrentWeaponStateContainer.Reset();
			Data.EvaluatorModel->CurrentWeaponStateContainer.Reset();
		}
	}
}
