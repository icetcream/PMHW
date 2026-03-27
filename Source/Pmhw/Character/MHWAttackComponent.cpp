#include "Character/MHWAttackComponent.h"

#include "Character/MeleeTraceComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/MHWCharacterInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWAttackComponent)

UMHWAttackComponent::UMHWAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMHWAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	TryInitializeFromOwner();
}

void UMHWAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bMeleeHitDelegateBound && CachedMeleeTraceComponent)
	{
		CachedMeleeTraceComponent->OnMeleeHit.RemoveDynamic(this, &ThisClass::HandleMeleeHit);
		bMeleeHitDelegateBound = false;
	}

	Super::EndPlay(EndPlayReason);
}

void UMHWAttackComponent::BeginAttack(FName InAttackId)
{
	AttackRuntimeContext.Reset();
	AttackRuntimeContext.ActiveAttackId = InAttackId;
	AttackRuntimeContext.bAttackActive = true;
}

void UMHWAttackComponent::EndAttack()
{
	StopTraceAndClearDamageSpec();
	ActiveWindowSpec = FMHWAttackWindowSpec();
	AttackRuntimeContext.Reset();
}

bool UMHWAttackComponent::BeginAttackWindow(const FMHWAttackWindowSpec& WindowSpec)
{
	if (!TryInitializeFromOwner() || !CachedMeleeTraceComponent)
	{
		return false;
	}

	if (WindowSpec.bStartNewAttack || !AttackRuntimeContext.bAttackActive || AttackRuntimeContext.ActiveAttackId != WindowSpec.AttackId)
	{
		BeginAttack(WindowSpec.AttackId);
	}

	if (AttackRuntimeContext.bWindowActive)
	{
		StopTraceAndClearDamageSpec();
	}

	AttackRuntimeContext.CurrentWindowIndex = WindowSpec.WindowIndex;
	AttackRuntimeContext.bWindowActive = true;
	AttackRuntimeContext.bCurrentWindowHit = false;
	ActiveWindowSpec = WindowSpec;

	FName ResolvedBaseSocket = NAME_None;
	TArray<FName> ResolvedTraceSockets;
	if (!CachedMeleeTraceComponent->ResolveTraceConfig(WindowSpec.BaseSocket, WindowSpec.TraceSockets, ResolvedBaseSocket, ResolvedTraceSockets))
	{
		return false;
	}

	CachedMeleeTraceComponent->bUsePhysicalDamageSpec = true;
	CachedMeleeTraceComponent->PhysicalDamageSpec = BuildDamageSpecForWindow(WindowSpec);
	CachedMeleeTraceComponent->SetHitVFXSpec(WindowSpec.HitVFX);
	CachedMeleeTraceComponent->StartTrace(ResolvedBaseSocket, ResolvedTraceSockets);
	return true;
}

void UMHWAttackComponent::EndAttackWindow()
{
	const bool bFinishAttackOnEnd = ActiveWindowSpec.bFinishAttackOnEnd;

	StopTraceAndClearDamageSpec();
	AttackRuntimeContext.CurrentWindowIndex = INDEX_NONE;
	AttackRuntimeContext.bWindowActive = false;
	AttackRuntimeContext.bCurrentWindowHit = false;
	ActiveWindowSpec = FMHWAttackWindowSpec();

	if (bFinishAttackOnEnd)
	{
		EndAttack();
	}
}

void UMHWAttackComponent::NotifyCurrentWindowHit(AActor* HitActor)
{
	if (!AttackRuntimeContext.bWindowActive || !HitActor)
	{
		return;
	}

	AttackRuntimeContext.bCurrentWindowHit = true;
	AttackRuntimeContext.RegisterWindowHit(AttackRuntimeContext.CurrentWindowIndex);
	++AttackRuntimeContext.TotalHitCount;
}

bool UMHWAttackComponent::HasWindowHit(int32 WindowIndex) const
{
	return AttackRuntimeContext.HasWindowHit(WindowIndex);
}

bool UMHWAttackComponent::TryInitializeFromOwner()
{
	if (CachedMeleeTraceComponent)
	{
		if (!bMeleeHitDelegateBound)
		{
			CachedMeleeTraceComponent->OnMeleeHit.AddDynamic(this, &ThisClass::HandleMeleeHit);
			bMeleeHitDelegateBound = true;
		}

		return true;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->Implements<UMHWCharacterInterface>())
	{
		return false;
	}

	CachedMeleeTraceComponent = IMHWCharacterInterface::Execute_GetMeleeTraceComponent(OwnerActor);
	if (!CachedMeleeTraceComponent)
	{
		return false;
	}

	CachedMeleeTraceComponent->OnMeleeHit.AddDynamic(this, &ThisClass::HandleMeleeHit);
	bMeleeHitDelegateBound = true;
	return true;
}

void UMHWAttackComponent::StopTraceAndClearDamageSpec()
{
	if (!TryInitializeFromOwner() || !CachedMeleeTraceComponent)
	{
		return;
	}

	CachedMeleeTraceComponent->StopTrace();
	CachedMeleeTraceComponent->bUsePhysicalDamageSpec = false;
	CachedMeleeTraceComponent->ClearHitVFXSpec();

	FMHWPhysicalDamageSpec ClearedDamageSpec;
	ClearedDamageSpec.TrueRawAttack = 0.0f;
	CachedMeleeTraceComponent->PhysicalDamageSpec = ClearedDamageSpec;
}

FMHWPhysicalDamageSpec UMHWAttackComponent::BuildDamageSpecForWindow(const FMHWAttackWindowSpec& WindowSpec) const
{
	FMHWPhysicalDamageSpec ResolvedDamageSpec = WindowSpec.BaseDamageSpec;

	if (WindowSpec.bBonusIfPreviousWindowHit)
	{
		const int32 BonusCheckWindowIndex = ResolveBonusCheckWindowIndex(WindowSpec);
		if (AttackRuntimeContext.HasWindowHit(BonusCheckWindowIndex))
		{
			ResolvedDamageSpec.AdditionalMultiplier *= WindowSpec.PreviousWindowHitMultiplier;
		}
	}

	return ResolvedDamageSpec;
}

int32 UMHWAttackComponent::ResolveBonusCheckWindowIndex(const FMHWAttackWindowSpec& WindowSpec) const
{
	return WindowSpec.PreviousWindowIndexOverride != INDEX_NONE
		? WindowSpec.PreviousWindowIndexOverride
		: (WindowSpec.WindowIndex - 1);
}

void UMHWAttackComponent::HandleMeleeHit(const FHitResult& HitResult)
{
	NotifyCurrentWindowHit(HitResult.GetActor());
}
