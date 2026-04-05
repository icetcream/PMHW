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
	ResetActiveWindowState();
	AttackRuntimeContext.Reset();
	ClearActiveAttackSpecTagOverride();
}

bool UMHWAttackComponent::BeginAttackWindow(const FMHWAttackWindowSpec& WindowSpec)
{
	if (!TryInitializeFromOwner() || !CachedMeleeTraceComponent)
	{
		return false;
	}

	const bool bShouldStartNewAttack =
		WindowSpec.bStartNewAttack ||
		!AttackRuntimeContext.bAttackActive ||
		AttackRuntimeContext.ActiveAttackId != WindowSpec.AttackId;
	const bool bHadActiveWindow = AttackRuntimeContext.bWindowActive;

	FName ResolvedBaseSocket = NAME_None;
	TArray<FName> ResolvedTraceSockets;
	if (!ResolveWindowTraceConfiguration(WindowSpec, ResolvedBaseSocket, ResolvedTraceSockets))
	{
		return false;
	}

	if (bHadActiveWindow)
	{
		StopTraceAndClearDamageSpec();
	}

	if (bShouldStartNewAttack)
	{
		BeginAttack(WindowSpec.AttackId);
	}

	const FGameplayTag ResolvedAttackSpecTag = ResolveAttackSpecTagForCurrentAction(WindowSpec.AttackSpecTag);
	const FMHWPhysicalDamageSpec ResolvedDamageSpec = BuildDamageSpecForWindow(WindowSpec);

	// Keep runtime state untouched until trace validation succeeds, so failed windows do not leave stale active flags behind.
	AttackRuntimeContext.CurrentWindowIndex = WindowSpec.WindowIndex;
	AttackRuntimeContext.bWindowActive = true;
	AttackRuntimeContext.bCurrentWindowHit = false;
	ActiveWindowSpec = WindowSpec;

	ApplyWindowTracePresentation(WindowSpec, ResolvedAttackSpecTag, ResolvedDamageSpec);
	ConfigureHitstopForWindow(WindowSpec, ResolvedAttackSpecTag, ResolvedDamageSpec);

	if (!StartResolvedTrace(WindowSpec, ResolvedBaseSocket, ResolvedTraceSockets))
	{
		StopTraceAndClearDamageSpec();
		ResetActiveWindowState();
		return false;
	}

	return true;
}

void UMHWAttackComponent::EndAttackWindow()
{
	const bool bFinishAttackOnEnd = ActiveWindowSpec.bFinishAttackOnEnd;

	StopTraceAndClearDamageSpec();
	ResetActiveWindowState();

	if (bFinishAttackOnEnd)
	{
		AttackRuntimeContext.Reset();
		ClearActiveAttackSpecTagOverride();
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

void UMHWAttackComponent::ResetActiveWindowState()
{
	AttackRuntimeContext.CurrentWindowIndex = INDEX_NONE;
	AttackRuntimeContext.bWindowActive = false;
	AttackRuntimeContext.bCurrentWindowHit = false;
	ActiveWindowSpec = FMHWAttackWindowSpec();
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
	CachedMeleeTraceComponent->ClearCachedHitCameraShake();
	CachedMeleeTraceComponent->ClearHitVFXSpec();
	CachedMeleeTraceComponent->ClearHitStopConfig();
}

bool UMHWAttackComponent::ResolveWindowTraceConfiguration(
	const FMHWAttackWindowSpec& WindowSpec,
	FName& OutBaseSocket,
	TArray<FName>& OutTraceSockets) const
{
	OutBaseSocket = NAME_None;
	OutTraceSockets.Reset();

	switch (WindowSpec.TraceMode)
	{
	case EMHWAttackTraceMode::Weapon:
		return CachedMeleeTraceComponent &&
			CachedMeleeTraceComponent->ResolveTraceConfig(WindowSpec.BaseSocket, WindowSpec.TraceSockets, OutBaseSocket, OutTraceSockets);

	case EMHWAttackTraceMode::BodySocket:
		OutBaseSocket = WindowSpec.BaseSocket;
		OutTraceSockets = WindowSpec.TraceSockets;
		return !OutBaseSocket.IsNone() && !OutTraceSockets.IsEmpty();

	case EMHWAttackTraceMode::SweepCapsule:
		return true;

	default:
		return false;
	}
}

void UMHWAttackComponent::ApplyWindowTracePresentation(
	const FMHWAttackWindowSpec& WindowSpec,
	const FGameplayTag& ResolvedAttackSpecTag,
	const FMHWPhysicalDamageSpec& ResolvedDamageSpec)
{
	if (!CachedMeleeTraceComponent)
	{
		return;
	}

	CachedMeleeTraceComponent->SetCachedPhysicalDamageSpec(ResolvedDamageSpec);
	CachedMeleeTraceComponent->SetHitVFXSpec(WindowSpec.HitVFX);

	if (const FMHWAttackDataRow* AttackDataRow = CachedMeleeTraceComponent->FindAttackDataRowBySpecTag(ResolvedAttackSpecTag))
	{
		CachedMeleeTraceComponent->SetCachedHitCameraShake(AttackDataRow->HitCameraShake);
		CachedMeleeTraceComponent->SetCachedAttackDisplayName(
			!AttackDataRow->AttackDisplayName.IsEmpty()
				? AttackDataRow->AttackDisplayName
				: FText::FromString(ResolvedAttackSpecTag.ToString()));
		return;
	}

	CachedMeleeTraceComponent->ClearCachedHitCameraShake();
	if (ResolvedAttackSpecTag.IsValid())
	{
		CachedMeleeTraceComponent->SetCachedAttackDisplayName(FText::FromString(ResolvedAttackSpecTag.ToString()));
	}
	else
	{
		CachedMeleeTraceComponent->ClearCachedAttackDisplayName();
	}
}

bool UMHWAttackComponent::StartResolvedTrace(
	const FMHWAttackWindowSpec& WindowSpec,
	const FName& ResolvedBaseSocket,
	const TArray<FName>& ResolvedTraceSockets)
{
	if (!CachedMeleeTraceComponent)
	{
		return false;
	}

	switch (WindowSpec.TraceMode)
	{
	case EMHWAttackTraceMode::SweepCapsule:
		CachedMeleeTraceComponent->StartCharacterCollisionTrace();
		return true;

	case EMHWAttackTraceMode::BodySocket:
		CachedMeleeTraceComponent->StartBodySocketTrace(ResolvedBaseSocket, ResolvedTraceSockets);
		return true;

	case EMHWAttackTraceMode::Weapon:
		CachedMeleeTraceComponent->StartTrace(ResolvedBaseSocket, ResolvedTraceSockets);
		return true;

	default:
		return false;
	}
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
