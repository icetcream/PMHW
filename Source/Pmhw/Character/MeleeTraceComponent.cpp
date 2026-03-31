#include "MeleeTraceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystem/MHWCombatBlueprintLibrary.h"
#include "Data/MHWAttackDataTable.h"
#include "Data/MHWHitStopData.h"
#include "Engine/World.h"
#include "Equipment/MHWEquipmentDefinition.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "Interface/MHWCharacterInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/MHWCombatComponent.h"
#include "Character/MHWMonsterCombatComponent.h"
#include "Character/MHWPlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Player/MHWPlayerController.h"


UMeleeTraceComponent::UMeleeTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; 
}


USkeletalMeshComponent* UMeleeTraceComponent::GetWeaponMesh()
{
	AActor* MyCharacter = GetOwner();
	if (!WeaponMeshComp && MyCharacter)
	{
		if (MyCharacter->Implements<UMHWCharacterInterface>())
		{
			if (UMHWEquipmentManagerComponent* EquipmentManagerComponent = const_cast<UMHWEquipmentManagerComponent*>(IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(MyCharacter)))
			{
				if (UMHWEquipmentInstance* EquipmentInstance = EquipmentManagerComponent->GetFirstInstanceOfType(WeaponInstanceClass))
				{
					CachedEquipmentInstance = EquipmentInstance;
					AActor* WeaponActor = EquipmentInstance->GetSpawnedActor();
					if (WeaponActor)
					{
						WeaponMeshComp = WeaponActor->GetComponentByClass<USkeletalMeshComponent>();
					}
				}
			}
		}
	}
	return WeaponMeshComp;
}

USkeletalMeshComponent* UMeleeTraceComponent::GetOwnerCharacterMesh()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerCharacterMeshComp && OwnerActor)
	{
		if (const ACharacter* Character = Cast<ACharacter>(OwnerActor))
		{
			OwnerCharacterMeshComp = Character->GetMesh();
		}

		if (!OwnerCharacterMeshComp)
		{
			OwnerCharacterMeshComp = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
		}
	}

	return OwnerCharacterMeshComp;
}

UCapsuleComponent* UMeleeTraceComponent::GetOwnerCapsuleComponent()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerCapsuleComp && OwnerActor)
	{
		OwnerCapsuleComp = OwnerActor->FindComponentByClass<UCapsuleComponent>();
	}

	return OwnerCapsuleComp;
}

void UMeleeTraceComponent::HandleHitResult(const FHitResult& Hit)
{
	AActor* HitActor = Hit.GetActor();
	if (!HitActor || HitActors.Contains(HitActor))
	{
		return;
	}

	HitActors.Add(HitActor);
	OnMeleeHit.Broadcast(Hit);
	SpawnHitVFX(Hit);

	const bool bUseCachedPhysicalDamageSpec = HasCachedPhysicalDamageSpec();
	const bool bHasDamagePayload = bUseCachedPhysicalDamageSpec ? true : (BaseDamage > 0.0f);
	if (bApplyDamageOnHit && bHasDamagePayload)
	{
		FMHWPhysicalDamageSpec RuntimeDamageSpec = bUseCachedPhysicalDamageSpec
			? CachedPhysicalDamageSpec
			: FMHWPhysicalDamageSpec();
		if (!bUseCachedPhysicalDamageSpec)
		{
			RuntimeDamageSpec.TrueRawAttack = BaseDamage;
		}

		UMHWPlayerCombatComponent* SourceCombatComponent = GetOwner()
			? GetOwner()->FindComponentByClass<UMHWPlayerCombatComponent>()
			: nullptr;
		UMHWMonsterCombatComponent* TargetCombatComponent = HitActor
			? HitActor->FindComponentByClass<UMHWMonsterCombatComponent>()
			: nullptr;

		const FMHWPhysicalDamageSpec ResolvedDamageSpec = SourceCombatComponent
			? SourceCombatComponent->BuildResolvedOutgoingPhysicalDamageSpec(RuntimeDamageSpec, TargetCombatComponent)
			: RuntimeDamageSpec;

		UMHWCombatBlueprintLibrary::ApplyPhysicalDamageToActor(
			HitActor,
			GetOwner(),
			ResolvedDamageSpec,
			true,
			Hit.ImpactPoint,
			GetCachedAttackDisplayName());
	}

	ApplyHitStop(HitActor);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AMHWPlayerController* PlayerController = OwnerPawn ? Cast<AMHWPlayerController>(OwnerPawn->GetController()) : nullptr;
	if (PlayerController && PlayerController->IsLocalController() && CachedHitCameraShake.IsEnabled())
	{
		PlayerController->PlayCombatCameraSpringShake(CachedHitCameraShake);
	}
}

void UMeleeTraceComponent::ApplyHitStop(AActor* HitActor)
{
	if (!CurrentHitstopData || !CurrentHitstopData->bEnableHitstop)
	{
		return;
	}

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return;
	}

	if (!IsHitstopActive())
	{
		DefaultTimeDilation = OwnerActor->CustomTimeDilation;
		HitstopAffectedActors.Empty();
		bHasOwnerMontageHitstop = false;
		CachedOwnerAnimInstance.Reset();
		CachedOwnerMontage.Reset();
		CachedOwnerSkeletalMesh.Reset();
		DefaultOwnerMontagePlayRate = 1.0f;
		bOwnerMeshAnimationPausedByHitstop = false;
		CurrentMeshFreezeDuration = 0.0f;

		bHasOwnerMontageHitstop = InitializeOwnerMontageHitstop();
	}

	if (CurrentHitstopData->bAffectOwnerTimeDilation)
	{
		AddHitstopAffectedActor(OwnerActor);
	}

	if (CurrentHitstopData->bAffectHitTargetTimeDilation)
	{
		AddHitstopAffectedActor(HitActor);
	}

	const float FreezeDuration = GetScaledHitstopDuration(CurrentHitstopData->MeshFreezeDuration);
	if (FreezeDuration > 0.0f)
	{
		BeginFreezeHitstop(FreezeDuration);
	}
	else
	{
		BeginRecoverHitstop(GetScaledHitstopDuration(CurrentHitstopData->MontageSlowDuration));
	}
}

void UMeleeTraceComponent::AddHitstopAffectedActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	if (bHasOwnerMontageHitstop && Actor == GetOwner())
	{
		return;
	}

	if (!HitstopAffectedActors.Contains(Actor))
	{
		HitstopAffectedActors.Add(Actor);
	}

	float CurrentMultiplier = 1.0f;
	switch (HitstopPhase)
	{
	case EHitstopRuntimePhase::Freeze:
		CurrentMultiplier = EvaluateCurrentTimeDilationMultiplier(0.0f);
		break;
	case EHitstopRuntimePhase::Recover:
		{
			const UWorld* World = GetWorld();
			const float ElapsedRealTime = World ? (World->GetRealTimeSeconds() - HitstopPhaseStartTime) : 0.0f;
			const float Alpha = HitstopPhaseDuration > 0.0f ? FMath::Clamp(ElapsedRealTime / HitstopPhaseDuration, 0.0f, 1.0f) : 1.0f;
			CurrentMultiplier = EvaluateCurrentTimeDilationMultiplier(Alpha);
		}
		break;
	case EHitstopRuntimePhase::None:
	default:
		break;
	}

	Actor->CustomTimeDilation = DefaultTimeDilation * CurrentMultiplier;
}

void UMeleeTraceComponent::ApplyCurrentHitstopMultiplier(float Multiplier)
{
	for (const TWeakObjectPtr<AActor>& WeakActor : HitstopAffectedActors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->CustomTimeDilation = DefaultTimeDilation * Multiplier;
		}
	}
}

bool UMeleeTraceComponent::InitializeOwnerMontageHitstop()
{
	AActor* OwnerActor = GetOwner();
	USkeletalMeshComponent* OwnerSkeletalMesh = OwnerActor ? OwnerActor->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
	UAnimInstance* OwnerAnimInstance = OwnerSkeletalMesh ? OwnerSkeletalMesh->GetAnimInstance() : nullptr;
	if (!OwnerAnimInstance)
	{
		return false;
	}

	UAnimMontage* OwnerMontage = OwnerAnimInstance->GetCurrentActiveMontage();
	if (!OwnerMontage)
	{
		return false;
	}

	CachedOwnerAnimInstance = OwnerAnimInstance;
	CachedOwnerMontage = OwnerMontage;
	CachedOwnerSkeletalMesh = OwnerSkeletalMesh;
	DefaultOwnerMontagePlayRate = OwnerAnimInstance->Montage_GetPlayRate(OwnerMontage);
	return true;
}

void UMeleeTraceComponent::SetOwnerMeshAnimationPaused(bool bPaused)
{
	if (USkeletalMeshComponent* OwnerSkeletalMesh = CachedOwnerSkeletalMesh.Get())
	{
		OwnerSkeletalMesh->bPauseAnims = bPaused;
		bOwnerMeshAnimationPausedByHitstop = bPaused;
	}
}

void UMeleeTraceComponent::UpdateOwnerMontagePlayRateForCurrentPhase(float PhaseAlpha) const
{
	if (!bHasOwnerMontageHitstop || !CurrentHitstopData)
	{
		return;
	}

	UAnimInstance* OwnerAnimInstance = CachedOwnerAnimInstance.Get();
	UAnimMontage* OwnerMontage = CachedOwnerMontage.Get();
	if (!OwnerAnimInstance || !OwnerMontage)
	{
		return;
	}

	const float TargetPlayRate = EvaluateCurrentMontagePlayRate(PhaseAlpha);
	OwnerAnimInstance->Montage_SetPlayRate(OwnerMontage, DefaultOwnerMontagePlayRate * FMath::Max(0.0001f, TargetPlayRate));
}

float UMeleeTraceComponent::EvaluateCurrentTimeDilationMultiplier(float PhaseAlpha) const
{
	if (!CurrentHitstopData)
	{
		return 1.0f;
	}

	if (!CurrentHitstopData->bAffectOwnerTimeDilation && !CurrentHitstopData->bAffectHitTargetTimeDilation)
	{
		return 1.0f;
	}

	if (HitstopPhase == EHitstopRuntimePhase::Freeze)
	{
		return CurrentHitstopData->TimeDilationStartScale;
	}

	if (CurrentHitstopData->TimeDilationCurve)
	{
		return CurrentHitstopData->TimeDilationCurve->GetFloatValue(FMath::Clamp(PhaseAlpha, 0.0f, 1.0f));
	}

	return FMath::Lerp(CurrentHitstopData->TimeDilationStartScale, 1.0f, FMath::Clamp(PhaseAlpha, 0.0f, 1.0f));
}

float UMeleeTraceComponent::EvaluateCurrentMontagePlayRate(float PhaseAlpha) const
{
	if (!CurrentHitstopData)
	{
		return 1.0f;
	}

	if (HitstopPhase == EHitstopRuntimePhase::Freeze)
	{
		return CurrentHitstopData->MontageSlowStartPlayRate;
	}

	if (CurrentHitstopData->MontageSlowCurve)
	{
		return CurrentHitstopData->MontageSlowCurve->GetFloatValue(FMath::Clamp(PhaseAlpha, 0.0f, 1.0f));
	}

	return FMath::Lerp(CurrentHitstopData->MontageSlowStartPlayRate, 1.0f, FMath::Clamp(PhaseAlpha, 0.0f, 1.0f));
}

void UMeleeTraceComponent::BeginFreezeHitstop(float FreezeDuration)
{
	if (!CurrentHitstopData)
	{
		return;
	}

	HitstopPhase = EHitstopRuntimePhase::Freeze;
	HitstopPhaseStartTime = GetWorld()->GetRealTimeSeconds();
	HitstopPhaseDuration = FMath::Max(0.0f, FreezeDuration);
	ApplyCurrentHitstopMultiplier(EvaluateCurrentTimeDilationMultiplier(0.0f));
	if (bHasOwnerMontageHitstop)
	{
		CurrentMeshFreezeDuration = HitstopPhaseDuration;
		SetOwnerMeshAnimationPaused(true);
	}
	UpdateOwnerMontagePlayRateForCurrentPhase(0.0f);
	SetComponentTickEnabled(true);
}

void UMeleeTraceComponent::BeginRecoverHitstop(float RecoverDuration)
{
	HitstopPhase = EHitstopRuntimePhase::Recover;
	HitstopPhaseStartTime = GetWorld()->GetRealTimeSeconds();
	HitstopPhaseDuration = FMath::Max(0.0f, RecoverDuration);

	if (!CurrentHitstopData || HitstopPhaseDuration <= 0.0f)
	{
		ResetHitStop();
		return;
	}

	if (bOwnerMeshAnimationPausedByHitstop)
	{
		SetOwnerMeshAnimationPaused(false);
	}

	ApplyCurrentHitstopMultiplier(EvaluateCurrentTimeDilationMultiplier(0.0f));
	UpdateOwnerMontagePlayRateForCurrentPhase(0.0f);
	SetComponentTickEnabled(true);
}

float UMeleeTraceComponent::GetScaledHitstopDuration(float BaseDuration) const
{
	return FMath::Max(0.0f, BaseDuration * CurrentHitstopStrengthMultiplier);
}

void UMeleeTraceComponent::SpawnHitVFX(const FHitResult& HitResult) const
{
	if (!CurrentHitVFXSpec.NiagaraSystem)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FRotator BaseRotation = CurrentHitVFXSpec.bOrientToImpactNormal
		? HitResult.ImpactNormal.Rotation()
		: FRotator::ZeroRotator;
	const FRotator SpawnRotation = BaseRotation + CurrentHitVFXSpec.RotationOffset;
	const FVector SpawnLocation = HitResult.ImpactPoint + SpawnRotation.RotateVector(CurrentHitVFXSpec.LocationOffset);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World,
		CurrentHitVFXSpec.NiagaraSystem,
		SpawnLocation,
		SpawnRotation,
		CurrentHitVFXSpec.Scale,
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
}

void UMeleeTraceComponent::ResetHitStop()
{
	HitstopPhase = EHitstopRuntimePhase::None;
	HitstopPhaseStartTime = 0.0f;
	HitstopPhaseDuration = 0.0f;

	// 把所有被我们放慢的 Actor（主角和怪物）的速度恢复到打人前的状态
	for (auto& WeakActor : HitstopAffectedActors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->CustomTimeDilation = DefaultTimeDilation;
		}
	}
	HitstopAffectedActors.Empty();

	if (bHasOwnerMontageHitstop)
	{
		if (UAnimInstance* OwnerAnimInstance = CachedOwnerAnimInstance.Get())
		{
			if (UAnimMontage* OwnerMontage = CachedOwnerMontage.Get())
			{
				OwnerAnimInstance->Montage_SetPlayRate(OwnerMontage, DefaultOwnerMontagePlayRate);
			}
		}
	}

	if (bOwnerMeshAnimationPausedByHitstop)
	{
		SetOwnerMeshAnimationPaused(false);
	}

	bHasOwnerMontageHitstop = false;
	CachedOwnerAnimInstance.Reset();
	CachedOwnerMontage.Reset();
	CachedOwnerSkeletalMesh.Reset();
	DefaultOwnerMontagePlayRate = 1.0f;
	bOwnerMeshAnimationPausedByHitstop = false;
	CurrentMeshFreezeDuration = 0.0f;

	// 如果挥砍判定也早就结束了，为了省性能，关掉 Tick
	if (!bIsTracing)
	{
		CurrentHitstopData = nullptr;
		CurrentHitstopStrengthMultiplier = 1.0f;
		SetComponentTickEnabled(false);
	}
}

void UMeleeTraceComponent::StartTrace(FName InBaseSocket, const TArray<FName>& InTraceSockets)
{
	StartSocketTrace(InBaseSocket, InTraceSockets, EMHWTraceMode::WeaponSockets);
}

void UMeleeTraceComponent::StartBodySocketTrace(FName InBaseSocket, const TArray<FName>& InTraceSockets)
{
	StartSocketTrace(InBaseSocket, InTraceSockets, EMHWTraceMode::BodySockets);
}

void UMeleeTraceComponent::StartSocketTrace(FName InBaseSocket, const TArray<FName>& InTraceSockets, EMHWTraceMode TraceMode)
{
	USkeletalMeshComponent* TraceMesh = nullptr;
	switch (TraceMode)
	{
	case EMHWTraceMode::WeaponSockets:
		TraceMesh = GetWeaponMesh();
		break;

	case EMHWTraceMode::BodySockets:
		GetWeaponMesh();
		TraceMesh = GetOwnerCharacterMesh();
		break;

	default:
		return;
	}

	if (!TraceMesh || InTraceSockets.Num() == 0 || InBaseSocket.IsNone())
	{
		return;
	}

	if (!CurrentHitstopData && CachedEquipmentInstance)
	{
		CurrentHitstopData = CachedEquipmentInstance->GetHitStopData();
		CurrentHitstopStrengthMultiplier = 1.0f;
	}

	BaseSocketName = InBaseSocket;
	ActiveTraceSockets = InTraceSockets;
	ActiveTraceMeshComp = TraceMesh;
	HitActors.Empty();
	TraceSocketsLocalOffsets.Empty();
	PreviousBaseTransform = TraceMesh->GetSocketTransform(BaseSocketName);

	for (const FName& SocketName : ActiveTraceSockets)
	{
		const FVector WorldLoc = TraceMesh->GetSocketLocation(SocketName);
		const FVector LocalLoc = PreviousBaseTransform.InverseTransformPosition(WorldLoc);
		TraceSocketsLocalOffsets.Add(LocalLoc);
	}

	CurrentTraceMode = TraceMode;
	bIsTracing = true;
	SetComponentTickEnabled(true);
}

void UMeleeTraceComponent::StartCharacterCollisionTrace()
{
	GetWeaponMesh();
	UCapsuleComponent* CapsuleComponent = GetOwnerCapsuleComponent();
	if (!CapsuleComponent)
	{
		return;
	}

	if (!CurrentHitstopData && CachedEquipmentInstance)
	{
		CurrentHitstopData = CachedEquipmentInstance->GetHitStopData();
		CurrentHitstopStrengthMultiplier = 1.0f;
	}

	HitActors.Empty();
	TraceSocketsLocalOffsets.Empty();
	ActiveTraceSockets.Empty();
	BaseSocketName = NAME_None;
	PreviousOwnerCapsuleTransform = CapsuleComponent->GetComponentTransform();
	ActiveTraceMeshComp = nullptr;

	CurrentTraceMode = EMHWTraceMode::SweepCapsule;
	bIsTracing = true;
	SetComponentTickEnabled(true);
}

bool UMeleeTraceComponent::ResolveTraceConfig(const FName& OverrideBaseSocket, const TArray<FName>& OverrideTraceSockets, FName& OutBaseSocket, TArray<FName>& OutTraceSockets)
{
	GetWeaponMesh();

	OutBaseSocket = OverrideBaseSocket;
	OutTraceSockets = OverrideTraceSockets;

	if (const UMHWEquipmentInstance* EquipmentInstance = CachedEquipmentInstance)
	{
		if (const FMHWMeleeTraceConfig* DefaultTraceConfig = EquipmentInstance->GetDefaultMeleeTraceConfig())
		{
			if (OutBaseSocket.IsNone())
			{
				OutBaseSocket = DefaultTraceConfig->BaseSocket;
			}

			if (OutTraceSockets.IsEmpty())
			{
				OutTraceSockets = DefaultTraceConfig->TraceSockets;
			}
		}
	}

	return !OutBaseSocket.IsNone() && !OutTraceSockets.IsEmpty();
}

const FMHWAttackDataRow* UMeleeTraceComponent::FindAttackDataRowBySpecTag(const FGameplayTag& AttackSpecTag)
{
	GetWeaponMesh();

	if (CachedEquipmentInstance)
	{
		return CachedEquipmentInstance->FindAttackDataRowBySpecTag(AttackSpecTag);
	}

	return nullptr;
}

const UMHWHitStopData* UMeleeTraceComponent::ResolveDefaultHitStopData(float MotionValue, bool bIsFinisher)
{
	GetWeaponMesh();

	if (CachedEquipmentInstance)
	{
		return CachedEquipmentInstance->ResolveHitStopDataForMotionValue(MotionValue, bIsFinisher);
	}

	return nullptr;
}

float UMeleeTraceComponent::CalculateHitStopStrengthMultiplier(float HitzoneValue, bool bIsSleepHit, bool bIsFinisher)
{
	GetWeaponMesh();

	if (CachedEquipmentInstance)
	{
		return CachedEquipmentInstance->CalculateHitStopStrengthMultiplier(HitzoneValue, bIsSleepHit, bIsFinisher);
	}

	return 1.0f;
}

void UMeleeTraceComponent::SetHitStopConfig(const UMHWHitStopData* InHitStopData, float InStrengthMultiplier)
{
	CurrentHitstopData = InHitStopData;
	CurrentHitstopStrengthMultiplier = FMath::Max(0.0f, InStrengthMultiplier);
}

void UMeleeTraceComponent::ClearHitStopConfig()
{
	if (IsHitstopActive())
	{
		return;
	}

	CurrentHitstopData = nullptr;
	CurrentHitstopStrengthMultiplier = 1.0f;
}

void UMeleeTraceComponent::SetCachedPhysicalDamageSpec(const FMHWPhysicalDamageSpec& InDamageSpec)
{
	CachedPhysicalDamageSpec = InDamageSpec;
	bHasCachedPhysicalDamageSpec = true;
}

void UMeleeTraceComponent::ClearCachedPhysicalDamageSpec()
{
	CachedPhysicalDamageSpec = FMHWPhysicalDamageSpec();
	CachedPhysicalDamageSpec.TrueRawAttack = 0.0f;
	bHasCachedPhysicalDamageSpec = false;
}

bool UMeleeTraceComponent::HasCachedPhysicalDamageSpec() const
{
	return bHasCachedPhysicalDamageSpec;
}

void UMeleeTraceComponent::SetCachedAttackDisplayName(const FText& InAttackDisplayName)
{
	CachedAttackDisplayName = InAttackDisplayName;
}

void UMeleeTraceComponent::ClearCachedAttackDisplayName()
{
	CachedAttackDisplayName = FText::GetEmpty();
}

FString UMeleeTraceComponent::GetCachedAttackDisplayName() const
{
	return CachedAttackDisplayName.IsEmpty() ? FString() : CachedAttackDisplayName.ToString();
}

void UMeleeTraceComponent::SetCachedHitCameraShake(const FMHWCameraSpringShakeSettings& InHitCameraShake)
{
	CachedHitCameraShake = InHitCameraShake;
}

void UMeleeTraceComponent::ClearCachedHitCameraShake()
{
	CachedHitCameraShake = FMHWCameraSpringShakeSettings();
}

void UMeleeTraceComponent::StopTrace()
{
	bIsTracing = false;
	HitActors.Empty();
	TraceSocketsLocalOffsets.Empty();
	ActiveTraceSockets.Empty();
	BaseSocketName = NAME_None;
	ActiveTraceMeshComp = nullptr;
	CurrentTraceMode = EMHWTraceMode::WeaponSockets;
	if (!IsHitstopActive())
	{
		SetComponentTickEnabled(false);
	}
}

void UMeleeTraceComponent::SetHitVFXSpec(const FMHWMeleeHitVFXSpec& InHitVFXSpec)
{
	CurrentHitVFXSpec = InHitVFXSpec;
}

void UMeleeTraceComponent::ClearHitVFXSpec()
{
	CurrentHitVFXSpec = FMHWMeleeHitVFXSpec();
}

void UMeleeTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UWorld* World = GetWorld();
	if (!World) return;

	UpdateHitstop(World->GetRealTimeSeconds());
	if (!bIsTracing || DeltaTime <= 0.0f) return;

	// ====== 自适应补帧计算 ======
	float CurrentFPS = 1.0f / DeltaTime;
	float SubstepInterval = DELTA_TIME_60FPS; 
	if (CurrentFPS > MIN_FPS)
	{
		float Weight = FMath::Clamp((CurrentFPS - MIN_FPS) / (MAX_FPS - MIN_FPS), 0.0f, 1.0f);
		SubstepInterval = FMath::Lerp(DELTA_TIME_60FPS, DELTA_TIME_120FPS, Weight);
	}
	int32 NumSteps = FMath::CeilToInt(DeltaTime / SubstepInterval);
	NumSteps = FMath::Clamp(NumSteps, 1, 4); 

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (CachedEquipmentInstance)
	{
		if (AActor* WeaponActor = CachedEquipmentInstance->GetSpawnedActor())
		{
			QueryParams.AddIgnoredActor(WeaponActor);
		}
	}
	QueryParams.bTraceComplex = false; 

	if (CurrentTraceMode == EMHWTraceMode::SweepCapsule)
	{
		UCapsuleComponent* CapsuleComponent = GetOwnerCapsuleComponent();
		if (!CapsuleComponent)
		{
			return;
		}

		const FTransform CurrentCapsuleTransform = CapsuleComponent->GetComponentTransform();
		const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
			CapsuleComponent->GetScaledCapsuleRadius(),
			CapsuleComponent->GetScaledCapsuleHalfHeight());

		for (int32 Step = 1; Step <= NumSteps; ++Step)
		{
			const float AlphaPrev = static_cast<float>(Step - 1) / static_cast<float>(NumSteps);
			const float AlphaCurr = static_cast<float>(Step) / static_cast<float>(NumSteps);

			FTransform StepStartCapsuleTransform;
			StepStartCapsuleTransform.Blend(PreviousOwnerCapsuleTransform, CurrentCapsuleTransform, AlphaPrev);

			FTransform StepEndCapsuleTransform;
			StepEndCapsuleTransform.Blend(PreviousOwnerCapsuleTransform, CurrentCapsuleTransform, AlphaCurr);

			TArray<FHitResult> HitResults;
			const bool bHit = World->SweepMultiByChannel(
				HitResults,
				StepStartCapsuleTransform.GetLocation(),
				StepEndCapsuleTransform.GetLocation(),
				StepEndCapsuleTransform.GetRotation(),
				TraceChannel,
				CapsuleShape,
				QueryParams);

			if (bShowDebug)
			{
				const FColor DebugColor = bHit ? FColor::Green : FColor::Red;
				DrawDebugCapsule(
					World,
					StepEndCapsuleTransform.GetLocation(),
					CapsuleComponent->GetScaledCapsuleHalfHeight(),
					CapsuleComponent->GetScaledCapsuleRadius(),
					StepEndCapsuleTransform.GetRotation(),
					DebugColor,
					false,
					2.0f);
			}

			if (bHit)
			{
				for (const FHitResult& Hit : HitResults)
				{
					HandleHitResult(Hit);
				}
			}
		}

		PreviousOwnerCapsuleTransform = CurrentCapsuleTransform;
		return;
	}

	USkeletalMeshComponent* TraceMesh = ActiveTraceMeshComp;
	if (!TraceMesh)
	{
		return;
	}

	const FTransform CurrentBaseTransform = TraceMesh->GetSocketTransform(BaseSocketName);

	for (int32 Step = 1; Step <= NumSteps; ++Step)
	{
		float AlphaPrev = (float)(Step - 1) / (float)NumSteps;
		float AlphaCurr = (float)Step / (float)NumSteps;

		FTransform StepStartBaseTransform;
		StepStartBaseTransform.Blend(PreviousBaseTransform, CurrentBaseTransform, AlphaPrev); 

		FTransform StepEndBaseTransform;
		StepEndBaseTransform.Blend(PreviousBaseTransform, CurrentBaseTransform, AlphaCurr);

		for (int32 i = 0; i < TraceSocketsLocalOffsets.Num(); ++i)
		{
			const FVector& LocalOffset = TraceSocketsLocalOffsets[i];
			FVector StepStartLoc = StepStartBaseTransform.TransformPosition(LocalOffset);
			FVector StepEndLoc   = StepEndBaseTransform.TransformPosition(LocalOffset);

			TArray<FHitResult> HitResults;
			bool bHit = World->LineTraceMultiByChannel(HitResults, StepStartLoc, StepEndLoc, TraceChannel, QueryParams);

			// ====== Debug ======
			if (bShowDebug)
			{
				FColor DebugColor = bHit ? FColor::Green : FColor::Red;
				DrawDebugLine(World, StepStartLoc, StepEndLoc, DebugColor, false, 2.0f, 0, 1.5f);
				if (bHit)
				{
					for (const FHitResult& Hit : HitResults)
					{
						DrawDebugPoint(World, Hit.ImpactPoint, 10.0f, FColor::Yellow, false, 2.0f);
					}
				}
			}

			// ====== 命中逻辑 ======
			if (bHit)
			{
				for (const FHitResult& Hit : HitResults)
				{
					HandleHitResult(Hit);
				}
			}
		}
	}

	PreviousBaseTransform = CurrentBaseTransform;
}

void UMeleeTraceComponent::UpdateHitstop(float CurrentRealTimeSeconds)
{
	if (!IsHitstopActive() || !CurrentHitstopData)
	{
		return;
	}

	const float ElapsedRealTime = CurrentRealTimeSeconds - HitstopPhaseStartTime;

	switch (HitstopPhase)
	{
	case EHitstopRuntimePhase::Freeze:
		ApplyCurrentHitstopMultiplier(EvaluateCurrentTimeDilationMultiplier(0.0f));
		UpdateOwnerMontagePlayRateForCurrentPhase(0.0f);

		if (bOwnerMeshAnimationPausedByHitstop && ElapsedRealTime >= CurrentMeshFreezeDuration)
		{
			SetOwnerMeshAnimationPaused(false);
		}

		if (ElapsedRealTime >= HitstopPhaseDuration)
		{
			const float RecoverDuration = GetScaledHitstopDuration(CurrentHitstopData->MontageSlowDuration);
			if (RecoverDuration > 0.0f)
			{
				BeginRecoverHitstop(RecoverDuration);
			}
			else
			{
				ResetHitStop();
			}
		}
		break;

	case EHitstopRuntimePhase::Recover:
		if (HitstopPhaseDuration <= 0.0f)
		{
			ResetHitStop();
			return;
		}

		if (ElapsedRealTime >= HitstopPhaseDuration)
		{
			ResetHitStop();
			return;
		}

		{
			const float PhaseAlpha = ElapsedRealTime / HitstopPhaseDuration;
			ApplyCurrentHitstopMultiplier(EvaluateCurrentTimeDilationMultiplier(PhaseAlpha));
			UpdateOwnerMontagePlayRateForCurrentPhase(PhaseAlpha);
		}
		break;

	case EHitstopRuntimePhase::None:
	default:
		break;
	}
}
