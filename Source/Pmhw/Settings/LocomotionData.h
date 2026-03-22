#pragma once

#include "CoreMinimal.h"
#include "MovementSettings.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" // MovementModel (Data Table Row Handle) 需要这个头文件
#include "LocomotionData.generated.h"

// --- 占位符：你的枚举类型 ---
UENUM(BlueprintType)
enum class EMovementState : uint8 { None, Grounded, InAir, Mantling };

UENUM(BlueprintType)
enum class EMovementGait : uint8 { Walking, Running, Sprinting };


USTRUCT(BlueprintType)
struct FMovementSettingsState { GENERATED_BODY() /* 内部变量略 */ };

// ==========================================
// 1. Settings 结构体 (静态配置)
// ==========================================
USTRUCT(BlueprintType)
struct FLocomotionSettings
{
	GENERATED_BODY()

public:
	// 数据表行句柄，用于配置基础移动数据
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion|Settings")
	FDataTableRowHandle MovementModel;

	// 如果你以后还有其他写死的配置（比如最大步幅、转身速度阈值），都放在这里
};

// ==========================================
// 2. Data 结构体 (运行时动态数据)
// ==========================================
USTRUCT(BlueprintType)
struct FLocomotionData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	float Speed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	bool bIsMoving = false; // C++布尔值必须加'b'前缀

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	FRotator LastVelocityRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	float MovementInputAmount = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	bool bHasMovementInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	FRotator LastMovementInputRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	FMHWMovementGaitSettings CurrentMovementSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	FMovementSettingsState MovementData;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	EMovementState CurrentMovementState = EMovementState::Grounded;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion|Data")
	EMovementGait CurrentMovementGait = EMovementGait::Walking;
};