// MHWCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h" // 需要包含 GameplayTags 模块
#include "MHWComboPreInputComponent.h"
#include "MHWGameplayTags.h"
#include "Interface/MHWCharacterInterface.h"
#include "Settings/MovementSettings.h"
#include "state/MHWLocomotionState.h"
#include "MHWCharacter.generated.h"

struct FMHWMovementGaitSettings;
class UMHWMovementSettings;
class UMHWAnimInstance;
class UMotionWarpingComponent;
class UMeleeTraceComponent;
class UHitboxTraceComponent;
class UStateTreeComponent;
class UMHWEquipmentManagerComponent;
class UMHWPawnExtensionComponent;
class UMHWHeroComponent;
class UInputMappingContext;
class UMHWInputConfig;
struct FInputActionValue;

UCLASS()
class PMHW_API AMHWCharacter : public ACharacter, public IAbilitySystemInterface, public IMHWCharacterInterface
{
	GENERATED_BODY()

public:
	AMHWCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual const UMHWEquipmentManagerComponent* GetEquipmentManagerComponent_Implementation() override;
	virtual UStateTreeComponent* GetStateTreeComponent_Implementation() override;
	virtual UMHWComboPreInputComponent* GetComboPreInputComponent_Implementation() override;
	virtual UMeleeTraceComponent* GetMeleeTraceComponent_Implementation() override;
	
public:
	// 1. 提供给外部或蓝图调用的 Setter (比如你的输入组件按下 Shift 时调用)
	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion")
	void SetDesiredGait(FGameplayTag NewDesiredGait);

	// Rotation data writer for StateTree tasks.
	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Rotation")
	void SetTargetYawAngle(float TargetYawAngle);

	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Rotation")
	void SetTargetYawAngleSmooth(float TargetYawAngle, float DeltaTime, float RotationSpeed);

	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Rotation")
	void SetRotationCurveCompensation(bool bEnable, FName CurveName, float CurveScale);

	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Rotation")
	void SetRotationInterpolationSettings(float InRotationTickRLerpSpeed, float InRotationTargetConstantLerpSpeed);

	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Weapon")
	void SetCurrentWeaponState(FGameplayTag NewWeaponState);

	UFUNCTION(BlueprintPure, Category = "MHW|Locomotion|Weapon")
	FGameplayTag GetCurrentWeaponState() const { return CurrentWeaponState; }

	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Settings")
	bool AddOrUpdateMovementSettingsForWeaponState(FGameplayTag WeaponState, const FMHWMovementRotationModeSettings& InSettings);

	UFUNCTION(BlueprintCallable, Category = "MHW|Locomotion|Settings")
	bool RemoveMovementSettingsForWeaponState(FGameplayTag WeaponState);
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Locomotion|Settings")
	TObjectPtr<UMHWMovementSettings> MovementSettings;

	// ==========================================
	// 期望状态 (Desired State - 玩家意图)
	// ==========================================
	// (移除了 Replicated 和 bDesiredAiming)
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Desired State")
	FGameplayTag DesiredRotationMode{MHWRotationModeTags::VelocityDirection};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Desired State")
	FGameplayTag DesiredStance{MHWStanceTags::Standing};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Desired State")
	FGameplayTag DesiredGait{MHWGaitTags::Running};
	
	// ==========================================
	// 实际状态 (Actual State - 运行时数据)
	// ==========================================
	
	// 动画实例缓存，方便快速调用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Locomotion|State", Transient, Meta = (ShowInnerProperties))
	TWeakObjectPtr<UMHWAnimInstance> AnimationInstance;

	// 运动模式 (地面/空中等)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FGameplayTag LocomotionMode{MHWLocomotionModeTags::Grounded};

	// 当前旋转模式
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FGameplayTag RotationMode{MHWRotationModeTags::VelocityDirection};

	// 当前姿态 (站立/下蹲等)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FGameplayTag Stance{MHWStanceTags::Standing};

	// 当前步态 (走/跑/冲刺)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FGameplayTag Gait{MHWGaitTags::Walking};

	// 动作标签 (翻滚/攻击等，用于打断常规移动)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FGameplayTag LocomotionAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FVector InputDirection{ForceInit};

	// 期望的速度朝向 (去除了 Replicated)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient, Meta = (ClampMin = -180, ClampMax = 180, ForceUnits = "deg"))
	float DesiredVelocityYawAngle{0.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	uint8 bHasDesiredVelocity : 1 {false};

	// 核心运动数据结构体 (速度、加速度等计算结果都存在这里)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State", Transient)
	FMHWLocomotionState LocomotionState;

	// Final actor rotation smoothing speed used in SmoothTickRotation().
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Rotation")
	float RotationTickRLerpSpeed{12.0f};

	// Constant turn speed (deg/s) used to derive TargetRotation from acceleration yaw.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Rotation")
	float RotationTargetConstantLerpSpeed{360.0f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Rotation", Transient)
	bool bEnableRotationCurveCompensation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Rotation", Transient)
	FName RotationCompensationCurveName = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|Rotation", Transient)
	float RotationCompensationCurveScale = 1.0f;

	UPROPERTY(Transient)
	float LastRotationCompensationCurveValue = 0.0f;

	UPROPERTY(Transient)
	bool bHasRotationCompensationCurveSample = false;
	
	// 用于落地时增加摩擦力的计时器 (如果你不做落地打滑的处理，这个也可以删掉)
	FTimerHandle BrakingFrictionFactorResetTimer;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Locomotion|State")
	FGameplayTag CurrentWeaponState{MHWWeaponTags::Sheathed};
	
	bool CanSprint() const;

	// 3. 计算本帧允许的最大步态
	FGameplayTag CalculateMaxAllowedGait() const;
	
	void RefreshInput();
	float CalculateGaitAmount() const;
	void RefreshMovementPhysics();
	void RefreshLocomotion();
	void SmoothTickRotation(float DeltaTime);

	// 3. 刷新步态与状态 (根据速度和输入，决定现在的真实 Gait 和 RotationMode)
	void RefreshGait();
	void RefreshRotationMode();
	
	const FMHWMovementGaitSettings* GetCurrentGaitSettings() const;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWPawnExtensionComponent> MHWPawnExtensionComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWEquipmentManagerComponent> MHWEquipmentManagerComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeComponent> MHWStateTreeComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMHWComboPreInputComponent> MHWComboPreInputComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMeleeTraceComponent> MHWMeleeTraceComponent;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MHW|Character", Meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer CurrentComboState;
	

};


