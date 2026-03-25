// MHWCharacter.cpp
#include "Character/MHWCharacter.h"

#include "AbilitySystemComponent.h"
#include "Input/MHWInputComponent.h" // 引入自定义组件
#include "Input/MHWInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "MeleeTraceComponent.h"
#include "MHWGameplayTags.h"
#include "MHWMovementComponent.h"
#include "MotionWarpingComponent.h"
#include "Animation/MHWAnimInstance.h"
#include "Character/MHWPawnExtensionComponent.h"
#include "Components/StateTreeComponent.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Input/MHWInputComponent.h"
#include "Player/MHWPlayerState.h"
#include "Settings/MovementSettings.h"
#include "Kismet/KismetMathLibrary.h"

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
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AMHWCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MHWMovementComponent = Cast<UMHWMovementComponent>(GetCharacterMovement());

	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->GetAllComponentFromCharacter();
	}
	if (GetMesh())
	{
		AnimationInstance = Cast<UMHWAnimInstance>(GetMesh()->GetAnimInstance());
	}
}


void AMHWCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->CheckInitialization();
	}
	RotationMode = DesiredRotationMode;
	Stance = DesiredStance;
	Gait = DesiredGait;
	SetTargetYawAngle(GetActorRotation().Yaw);
}

void AMHWCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return;
	// 1. 刷新输入数据 (玩家按了什么方向？)
	RefreshInput();

	// 2. 刷新物理运动数据 (角色实际上在往哪走？速度多快？)
	RefreshLocomotion();

	// 3. 刷新步态与状态 (根据速度和输入，决定现在的真实 Gait 和 RotationMode)
	RefreshGait();
	RefreshRotationMode();
	RefreshMovementPhysics();
	SmoothTickRotation(DeltaSeconds);
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

void AMHWCharacter::SetDesiredGait(FGameplayTag NewDesiredGait)
{
	if (DesiredGait != NewDesiredGait)
	{
		DesiredGait = NewDesiredGait;
	}
}

void AMHWCharacter::SetCurrentWeaponState(FGameplayTag NewWeaponState)
{
	if (NewWeaponState.IsValid())
	{
		CurrentWeaponState = NewWeaponState;
	}
}

void AMHWCharacter::SetTargetYawAngle(float TargetYawAngle)
{
	LocomotionState.TargetYawAngle = FMath::UnwindDegrees(TargetYawAngle);
	LocomotionState.SmoothTargetYawAngle = LocomotionState.TargetYawAngle;
}

void AMHWCharacter::SetTargetYawAngleSmooth(float TargetYawAngle, float DeltaTime, float RotationSpeed)
{
	LocomotionState.TargetYawAngle = FMath::UnwindDegrees(TargetYawAngle);
	LocomotionState.SmoothTargetYawAngle = FMath::FixedTurn(
		LocomotionState.SmoothTargetYawAngle,
		LocomotionState.TargetYawAngle,
		RotationSpeed * DeltaTime);
}

void AMHWCharacter::SetRotationCurveCompensation(bool bEnable, FName CurveName, float CurveScale)
{
	const bool bConfigChanged =
		(bEnableRotationCurveCompensation != bEnable) ||
		(RotationCompensationCurveName != CurveName) ||
		!FMath::IsNearlyEqual(RotationCompensationCurveScale, CurveScale);

	bEnableRotationCurveCompensation = bEnable;
	RotationCompensationCurveName = CurveName;
	RotationCompensationCurveScale = CurveScale;

	// Reset curve sampling when compensation setup changes to avoid first-frame jump or accumulation error.
	if (bConfigChanged || !bEnableRotationCurveCompensation || RotationCompensationCurveName.IsNone())
	{
		bHasRotationCompensationCurveSample = false;
		LastRotationCompensationCurveValue = 0.0f;
	}
}

void AMHWCharacter::SetRotationInterpolationSettings(float InRotationTickRLerpSpeed, float InRotationTargetConstantLerpSpeed)
{
	RotationTickRLerpSpeed = FMath::Max(0.0f, InRotationTickRLerpSpeed);
	RotationTargetConstantLerpSpeed = FMath::Max(0.0f, InRotationTargetConstantLerpSpeed);
}

void AMHWCharacter::SetBlockRotation(bool bInBlocked)
{
	bBlockRotation = bInBlocked;
}

void AMHWCharacter::SetDefenseActive(bool bInActive)
{
	bDefenseActive = bInActive;
}

bool AMHWCharacter::AddOrUpdateMovementSettingsForWeaponState(FGameplayTag WeaponState, const FMHWMovementRotationModeSettings& InSettings)
{
	if (!MovementSettings || !WeaponState.IsValid())
	{
		return false;
	}

	MovementSettings->WeaponStates.FindOrAdd(WeaponState) = InSettings;
	return true;
}

bool AMHWCharacter::RemoveMovementSettingsForWeaponState(FGameplayTag WeaponState)
{
	if (!MovementSettings || !WeaponState.IsValid())
	{
		return false;
	}

	return MovementSettings->WeaponStates.Remove(WeaponState) > 0;
}

bool AMHWCharacter::CanSprint() const
{
	// 1. 如果玩家根本没推摇杆/按方向键，不能原地冲刺
	if (!LocomotionState.bHasInput)
	{
		return false;
	}

	// 2. 如果不是站立姿态（比如正在蹲着），不能冲刺
	if (Stance != MHWStanceTags::Standing)
	{
		return false;
	}


	if (CurrentWeaponState == MHWWeaponTags::GreatSword)
	{
		return false;
	}

	return true;
}

bool AMHWCharacter::CanRotation() const
{
	if (bBlockRotation)
	{
		return false;
	}

	if (HasAnyRootMotion())
	{
		return false;
	}

	return true;
}

FGameplayTag AMHWCharacter::CalculateMaxAllowedGait() const
{
	if (DesiredGait != MHWGaitTags::Sprinting)
	{
		return DesiredGait;
	}

	// 如果玩家想冲刺，但系统判定不满足条件，强制降级为跑步
	if (!CanSprint())
	{
		return MHWGaitTags::Running;
	}

	// 一切合法，允许冲刺！
	return MHWGaitTags::Sprinting;
}

void AMHWCharacter::RefreshInput()
{
	UMHWMovementComponent* MoveComp = MHWMovementComponent.Get();
	if (!MoveComp)
	{
		MoveComp = Cast<UMHWMovementComponent>(GetCharacterMovement());
	}
	if (!MoveComp) return;

	// 获取当前玩家的输入加速度方向 (归一化向量)
	InputDirection = MoveComp->GetCurrentAcceleration().GetSafeNormal();

	// 判定：是否有输入？(向量长度大于一丁点就算有输入)
	LocomotionState.bHasInput = InputDirection.SizeSquared() > UE_KINDA_SMALL_NUMBER;

	if (LocomotionState.bHasInput)
	{
		// 计算输入方向的偏航角 (Yaw)，范围 -180 到 180 度。这对动画系统判断四向移动非常重要！
		LocomotionState.InputYawAngle = InputDirection.Rotation().Yaw;
	}
}

float AMHWCharacter::CalculateGaitAmount() const
{
	const FMHWMovementGaitSettings* CurrentSettings = GetCurrentGaitSettings();
	
	// 如果配置资产丢失或没有找到对应的分支，默认返回静止状态 (0.0)
	if (!CurrentSettings)
	{
		return 0.0f;
	}

	// 2. 读取配置表中的三档标准速度
	const float WalkSpeed = CurrentSettings->WalkSpeed;
	const float RunSpeed = CurrentSettings->RunSpeed;
	const float SprintSpeed = CurrentSettings->SprintSpeed;
	
	// 3. 获取角色当前真实的物理速度大小 (XY平面上的速度)
	const float CurrentSpeed = LocomotionState.Speed;

	// ==========================================
	// 4. 将当前速度映射到 0.0 ~ 3.0 的步态量区间
	// ==========================================

	// 区间 1: 停止 (0) -> 走路 (1)
	if (CurrentSpeed <= WalkSpeed)
	{
		// 速度从 0 变到 WalkSpeed，对应步态量从 0.0 变到 1.0
		return FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, WalkSpeed), 
			FVector2D(0.0f, 1.0f), 
			CurrentSpeed
		);
	}
	
	// 区间 2: 走路 (1) -> 跑步 (2)
	if (CurrentSpeed <= RunSpeed)
	{
		// 速度从 WalkSpeed 变到 RunSpeed，对应步态量从 1.0 变到 2.0
		return FMath::GetMappedRangeValueClamped(
			FVector2D(WalkSpeed, RunSpeed), 
			FVector2D(1.0f, 2.0f), 
			CurrentSpeed
		);
	}
	
	// 区间 3: 跑步 (2) -> 冲刺 (3) (如果速度超过了 SprintSpeed，会被 Clamped 限制在 3.0)
	return FMath::GetMappedRangeValueClamped(
		FVector2D(RunSpeed, SprintSpeed), 
		FVector2D(2.0f, 3.0f), 
		CurrentSpeed
	);
}

void AMHWCharacter::RefreshMovementPhysics()
{
	UMHWMovementComponent* MoveComp = MHWMovementComponent.Get();
	if (!MoveComp)
	{
		MoveComp = Cast<UMHWMovementComponent>(GetCharacterMovement());
	}
	if (!MoveComp) return;

	// 2. 获取当前状态下（武器 + 旋转模式 + 姿态）真正生效的那组移动配置
	const FMHWMovementGaitSettings* CurrentSettings = GetCurrentGaitSettings();
	if (!CurrentSettings) return;

	// 3. 计算当前的“步态量”(Gait Amount)，范围是 0.0 ~ 3.0
	// 这个值代表了角色当前真实物理速度在 停止->走->跑->冲刺 之间的插值进度
	float CurrentGaitAmount = CalculateGaitAmount();

	// ==========================================
	// 核心步骤 A：应用动态物理曲线
	// ==========================================
	if (CurrentSettings->AccelerationAndDecelerationAndGroundFrictionCurve)
	{
		// 根据当前的步态量，从曲线上读取对应的 X, Y, Z 值
		FVector CurveValue = CurrentSettings->AccelerationAndDecelerationAndGroundFrictionCurve->GetVectorValue(CurrentGaitAmount);

		// ALS 的物理魔法：根据速度动态改变角色的加减速和地面摩擦力！
		// X = 加速度 (Acceleration)
		// Y = 减速度 (Deceleration)
		// Z = 地面摩擦力 (Ground Friction)
		MoveComp->MaxAcceleration = CurveValue.X;
		MoveComp->BrakingDecelerationWalking = CurveValue.Y;
		MoveComp->GroundFriction = CurveValue.Z;
	}

	// ==========================================
	// 核心步骤 B：根据“审批结果”应用最大速度限制
	// ==========================================
	// 绝对不能直接用 DesiredGait！必须用 CalculateMaxAllowedGait() 进行规则核对
	// (例如：如果玩家拿着大剑，哪怕按断了 Shift 键，系统也会把 MaxAllowedGait 降级为 Running 或 Walking)
	FGameplayTag MaxAllowedGait = CalculateMaxAllowedGait();

	if (MaxAllowedGait == MHWGaitTags::Sprinting)
	{
		MoveComp->MaxWalkSpeed = CurrentSettings->SprintSpeed * MoveComp->MaxWalkSpeedScale;
	}
	else if (MaxAllowedGait == MHWGaitTags::Running)
	{
		MoveComp->MaxWalkSpeed = CurrentSettings->RunSpeed * MoveComp->MaxWalkSpeedScale;
	}
	else // 如果是 MHWGaitTags::Walking 或者其他情况
	{
		MoveComp->MaxWalkSpeed = CurrentSettings->WalkSpeed * MoveComp->MaxWalkSpeedScale;
	}

	// ==========================================
	// 附加步骤 C：如果你需要控制角色的转身速度
	// ==========================================
	if (CurrentSettings->RotationInterpolationSpeedCurve)
	{
		// 从曲线上读取当前的转身插值速度
		float RotationSpeed = CurrentSettings->RotationInterpolationSpeedCurve->GetFloatValue(CurrentGaitAmount);
		
		// 你可以把这个值存到 LocomotionState 里，供你的角色转向逻辑 (如 FaceRotation) 使用
		// 例如：LocomotionState.CurrentRotationSpeed = RotationSpeed;
	}
}

void AMHWCharacter::RefreshLocomotion()
{
	// 获取角色当前的真实物理速度
	LocomotionState.Velocity = GetVelocity();

	// 只计算水平面 (XY轴) 的速度大小，忽略掉落速度
	LocomotionState.Speed = LocomotionState.Velocity.Size2D();

	// 判定：角色是否有物理速度？ (比如速度大于 1.0f 就算有)
	LocomotionState.bHasVelocity = LocomotionState.Speed >= 1.0f;

	if (LocomotionState.bHasVelocity)
	{
		// 计算真实运动方向的偏航角 (Yaw)
		LocomotionState.VelocityYawAngle = LocomotionState.Velocity.Rotation().Yaw;
	}

	// 判定：角色是否处于“移动中”？ (有输入且有速度，或者速度足够大，用于排除被轻微碰撞导致的速度)
	LocomotionState.bMoving = (LocomotionState.bHasInput && LocomotionState.bHasVelocity) || LocomotionState.Speed > 3.0f;
}

void AMHWCharacter::SmoothTickRotation(float DeltaTime)
{
	auto ResetCurveSampling = [this]()
	{
		bHasRotationCompensationCurveSample = false;
		LastRotationCompensationCurveValue = 0.0f;
	};

	if (!CanRotation())
	{
		// Root motion phase should own rotation. Reset curve sampling to avoid re-enable spikes.
		ResetCurveSampling();

		// Keep rotation targets aligned with the current actor yaw while rotation is blocked,
		// so we do not snap to a stale target when rotation gets re-enabled.
		const float CurrentYaw = FMath::UnwindDegrees(GetActorRotation().Yaw);
		LocomotionState.TargetYawAngle = CurrentYaw;
		LocomotionState.SmoothTargetYawAngle = CurrentYaw;
		return;
	}

	const FRotator CurrentRotation = GetActorRotation();
	float DebugCurrentCurveValue = 0.0f;
	float DebugCurveRawDelta = 0.0f;
	float DebugCurveClampedDelta = 0.0f;
	bool bDebugUsingCurve = false;

	// 1) Build target yaw from acceleration using constant turn speed.
	UMHWMovementComponent* MoveComp = MHWMovementComponent.Get();
	if (!MoveComp)
	{
		MoveComp = Cast<UMHWMovementComponent>(GetCharacterMovement());
	}
	if (MoveComp)
	{
		const FVector CurrentAcceleration = MoveComp->GetCurrentAcceleration();
		if (CurrentAcceleration.SizeSquared2D() > UE_KINDA_SMALL_NUMBER)
		{
			LocomotionState.TargetYawAngle = FMath::UnwindDegrees(CurrentAcceleration.Rotation().Yaw);
			LocomotionState.SmoothTargetYawAngle = FMath::FixedTurn(
				CurrentRotation.Yaw,
				LocomotionState.TargetYawAngle,
				RotationTargetConstantLerpSpeed * DeltaTime);
		}
	}

	// 2) Smooth final actor yaw by RLerp shortest path.
	const FRotator TargetRotation(0.0f, LocomotionState.SmoothTargetYawAngle, 0.0f);
	const float RotationAlpha = FMath::Clamp(RotationTickRLerpSpeed * DeltaTime, 0.0f, 1.0f);
	const FRotator SmoothedRotation = UKismetMathLibrary::RLerp(CurrentRotation, TargetRotation, RotationAlpha, true);

	// 3) Optional curve delta compensation.
	float CurveRotationCompensation = 0.0f;
	const bool bCanUseCurveCompensation =
		bEnableRotationCurveCompensation &&
		!RotationCompensationCurveName.IsNone() &&
		AnimationInstance.IsValid();
	if (bCanUseCurveCompensation)
	{
		const float CurrentCurveValue = FMath::UnwindDegrees(
			AnimationInstance->GetCurveValue(RotationCompensationCurveName) * RotationCompensationCurveScale);
		DebugCurrentCurveValue = CurrentCurveValue;
		bDebugUsingCurve = true;

		if (bHasRotationCompensationCurveSample)
		{
			// Use shortest-angle delta and clamp by per-second speed to avoid large blend spikes.
			const float RawDelta = FMath::FindDeltaAngleDegrees(
				LastRotationCompensationCurveValue,
				CurrentCurveValue);
			const float MaxDeltaThisFrame = FMath::Max(0.0f, MaxRotationCompensationDeltaSpeed) * DeltaTime;
			CurveRotationCompensation = FMath::Clamp(RawDelta, -MaxDeltaThisFrame, MaxDeltaThisFrame);
			DebugCurveRawDelta = RawDelta;
			DebugCurveClampedDelta = CurveRotationCompensation;
		}
		else
		{
			CurveRotationCompensation = 0.0f;
			bHasRotationCompensationCurveSample = true;
		}

		LastRotationCompensationCurveValue = CurrentCurveValue;
	}
	else
	{
		ResetCurveSampling();
	}

	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw = FMath::UnwindDegrees(SmoothedRotation.Yaw + CurveRotationCompensation);
	SetActorRotation(NewRotation);

	if (bDebugRotationCompensationLog)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[RotComp] UsingCurve=%d Curve=%s CurrentCurve=%.2f RawDelta=%.2f ClampedDelta=%.2f TargetYaw=%.2f ResultYaw=%.2f FinalYaw=%.2f"),
			bDebugUsingCurve ? 1 : 0,
			*RotationCompensationCurveName.ToString(),
			DebugCurrentCurveValue,
			DebugCurveRawDelta,
			DebugCurveClampedDelta,
			LocomotionState.SmoothTargetYawAngle,
			SmoothedRotation.Yaw,
			NewRotation.Yaw);
	}
}

void AMHWCharacter::RefreshGait()
{
	if (LocomotionMode != MHWLocomotionModeTags::Grounded) return;

	// 1. 拿到经过审批的“最大允许步态”
	FGameplayTag MaxAllowedGait = CalculateMaxAllowedGait();

	// 2. 读取配置表里的速度
	const FMHWMovementGaitSettings* CurrentSettings = GetCurrentGaitSettings();
	if (!CurrentSettings) return;

	const float WalkSpeed = CurrentSettings->WalkSpeed;
	const float RunSpeed = CurrentSettings->RunSpeed;
	const float SpeedTolerance = 10.0f; // 容差值

	FGameplayTag ActualGait;

	// 3. 根据【实际速度】和【最大允许步态】计算真正的状态
	if (LocomotionState.Speed < WalkSpeed + SpeedTolerance)
	{
		ActualGait = MHWGaitTags::Walking;
	}
	// 如果实际速度不到冲刺的速度，或者系统根本不允许冲刺，那就是跑步
	else if (LocomotionState.Speed < RunSpeed + SpeedTolerance || MaxAllowedGait != MHWGaitTags::Sprinting)
	{
		ActualGait = MHWGaitTags::Running;
	}
	else
	{
		ActualGait = MHWGaitTags::Sprinting;
	}

	// 4. 应用真正的 Gait
	if (Gait != ActualGait)
	{
		Gait = ActualGait;
	}
}

void AMHWCharacter::RefreshRotationMode()
{
	FGameplayTag ActualRotationMode = DesiredRotationMode;

	if (Gait == MHWGaitTags::Sprinting)
	{
		ActualRotationMode = MHWRotationModeTags::VelocityDirection;
	}

	if (RotationMode != ActualRotationMode)
	{
		RotationMode = ActualRotationMode;
	}
}

const FMHWMovementGaitSettings* AMHWCharacter::GetCurrentGaitSettings() const
{
	if (!MovementSettings) return nullptr;

	// 1. 根据当前【武器状态】查找 (收刀 还是 拔出了大剑?)
	const FMHWMovementRotationModeSettings* WeaponSettings = MovementSettings->WeaponStates.Find(CurrentWeaponState);
	if (!WeaponSettings) return nullptr;

	// 2. 根据当前【旋转模式】查找 (自由视角 还是 锁定了怪物?)
	const FMHWMovementStanceSettings* StanceSettings = WeaponSettings->RotationModes.Find(RotationMode);
	if (!StanceSettings) return nullptr;

	// 3. 根据当前【姿态】查找 (站立 还是 蹲下?)
	const FMHWMovementGaitSettings* GaitSettings = StanceSettings->Stances.Find(Stance);
	
	return GaitSettings;
}



