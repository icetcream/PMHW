#include "Character/MHWAttackComponent.h"

#include "Character/MeleeTraceComponent.h"
#include "Data/MHWAttackDataTable.h"
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
	ClearActiveAttackSpecTagOverride();
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

	const FGameplayTag ResolvedAttackSpecTag = ResolveAttackSpecTagForCurrentAction(WindowSpec.AttackSpecTag);
	const FMHWPhysicalDamageSpec ResolvedDamageSpec = BuildDamageSpecForWindow(WindowSpec);

	CachedMeleeTraceComponent->SetCachedPhysicalDamageSpec(ResolvedDamageSpec);
	if (const FMHWAttackDataRow* AttackDataRow = CachedMeleeTraceComponent->FindAttackDataRowBySpecTag(ResolvedAttackSpecTag))
	{
		if (!AttackDataRow->AttackDisplayName.IsEmpty())
		{
			CachedMeleeTraceComponent->SetCachedAttackDisplayName(AttackDataRow->AttackDisplayName);
		}
		else
		{
			CachedMeleeTraceComponent->SetCachedAttackDisplayName(FText::FromString(ResolvedAttackSpecTag.ToString()));
		}
	}
	else
	{
		if (ResolvedAttackSpecTag.IsValid())
		{
			CachedMeleeTraceComponent->SetCachedAttackDisplayName(FText::FromString(ResolvedAttackSpecTag.ToString()));
		}
		else
		{
			CachedMeleeTraceComponent->ClearCachedAttackDisplayName();
		}
	}
	CachedMeleeTraceComponent->SetHitVFXSpec(WindowSpec.HitVFX);
	ConfigureHitstopForWindow(WindowSpec, ResolvedAttackSpecTag, ResolvedDamageSpec);
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

void UMHWAttackComponent::SetPendingChargeLevel(const EMHWChargeLevel InChargeLevel)
{
	PendingChargeLevel = InChargeLevel;
	bHasPendingChargeLevel = true;
}

void UMHWAttackComponent::ClearPendingChargeLevel()
{
	bHasPendingChargeLevel = false;
	PendingChargeLevel = EMHWChargeLevel::Level1;
}

bool UMHWAttackComponent::PeekPendingChargeLevel(EMHWChargeLevel& OutChargeLevel) const
{
	OutChargeLevel = PendingChargeLevel;
	return bHasPendingChargeLevel;
}

bool UMHWAttackComponent::ConsumePendingChargeLevel(EMHWChargeLevel& OutChargeLevel)
{
	if (!bHasPendingChargeLevel)
	{
		OutChargeLevel = EMHWChargeLevel::Level1;
		return false;
	}

	OutChargeLevel = PendingChargeLevel;
	ClearPendingChargeLevel();
	return true;
}

void UMHWAttackComponent::SetActiveAttackSpecTagOverride(const FGameplayTag InAttackSpecTag)
{
	ActiveAttackSpecTagOverride = InAttackSpecTag;
}

void UMHWAttackComponent::ClearActiveAttackSpecTagOverride()
{
	ActiveAttackSpecTagOverride = FGameplayTag::EmptyTag;
}

bool UMHWAttackComponent::GetActiveAttackSpecTagOverride(FGameplayTag& OutAttackSpecTag) const
{
	OutAttackSpecTag = ActiveAttackSpecTagOverride;
	return ActiveAttackSpecTagOverride.IsValid();
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
	CachedMeleeTraceComponent->ClearCachedPhysicalDamageSpec();
	CachedMeleeTraceComponent->ClearCachedAttackDisplayName();
	CachedMeleeTraceComponent->ClearHitVFXSpec();
	CachedMeleeTraceComponent->ClearHitStopConfig();
}

FMHWPhysicalDamageSpec UMHWAttackComponent::BuildDamageSpecForWindow(const FMHWAttackWindowSpec& WindowSpec) const
{
	FMHWPhysicalDamageSpec ResolvedDamageSpec;
	ResolvedDamageSpec.TrueRawAttack = 0.0f;
	ResolvedDamageSpec.MotionValue = 0.0f;

	const FGameplayTag EffectiveAttackSpecTag = ResolveAttackSpecTagForCurrentAction(WindowSpec.AttackSpecTag);
	if (CachedMeleeTraceComponent && EffectiveAttackSpecTag.IsValid())
	{
		if (const FMHWAttackDataRow* AttackDataRow = CachedMeleeTraceComponent->FindAttackDataRowBySpecTag(EffectiveAttackSpecTag))
		{
			ResolvedDamageSpec.MotionValue = AttackDataRow->MotionValue;
		}
	}

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

void UMHWAttackComponent::ConfigureHitstopForWindow(const FMHWAttackWindowSpec& WindowSpec, const FGameplayTag& ResolvedAttackSpecTag,
	const FMHWPhysicalDamageSpec& ResolvedDamageSpec)
{
	if (!CachedMeleeTraceComponent)
	{
		return;
	}

	if (!ResolvedAttackSpecTag.IsValid())
	{
		CachedMeleeTraceComponent->ClearHitStopConfig();
		return;
	}

	const UMHWHitStopData* HitStopData = WindowSpec.HitStopDataOverride
		? WindowSpec.HitStopDataOverride.Get()
		: CachedMeleeTraceComponent->ResolveDefaultHitStopData(ResolvedDamageSpec.MotionValue, WindowSpec.bIsFinisher);

	const bool bIsSleepHit = ResolvedDamageSpec.AilmentMultiplier > 1.0f + KINDA_SMALL_NUMBER;
	const float HitStopStrengthMultiplier = CachedMeleeTraceComponent->CalculateHitStopStrengthMultiplier(
		ResolvedDamageSpec.HitzoneValue,
		bIsSleepHit,
		WindowSpec.bIsFinisher);

	CachedMeleeTraceComponent->SetHitStopConfig(HitStopData, HitStopStrengthMultiplier);
}

int32 UMHWAttackComponent::ResolveBonusCheckWindowIndex(const FMHWAttackWindowSpec& WindowSpec) const
{
	return WindowSpec.PreviousWindowIndexOverride != INDEX_NONE
		? WindowSpec.PreviousWindowIndexOverride
		: (WindowSpec.WindowIndex - 1);
}

FGameplayTag UMHWAttackComponent::ResolveAttackSpecTagForCurrentAction(const FGameplayTag& DefaultAttackSpecTag) const
{
	return ActiveAttackSpecTagOverride.IsValid()
		? ActiveAttackSpecTagOverride
		: DefaultAttackSpecTag;
}

void UMHWAttackComponent::HandleMeleeHit(const FHitResult& HitResult)
{
	NotifyCurrentWindowHit(HitResult.GetActor());
}
