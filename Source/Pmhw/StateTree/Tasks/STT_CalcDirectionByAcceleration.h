#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "STT_CalcDirectionByAcceleration.generated.h"

UENUM()
enum class EMHWDirectionOutputMode : uint8
{
	FourWay UMETA(DisplayName = "Direction 4-Way"),
	ThreeWay UMETA(DisplayName = "Direction 3-Way")
};

USTRUCT()
struct FSTT_CalcDirectionByAccelerationInstanceData
{
	GENERATED_BODY()

	// [Context] 必须有的上下文
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACharacter> Character = nullptr;

	// ================= [输出 Output] 给下个节点用 =================

	// 算出来的方向片段名（用于 Montage Starting Section）
	UPROPERTY(EditAnywhere, Category = "Output")
	FName OutDirectionSection = NAME_None;

	// 算出来的精确闪避方向目标 (只转角度不转位移的 Transform)
	UPROPERTY(EditAnywhere, Category = "Output")
	FTransform OutWarpTarget = FTransform::Identity;
};

USTRUCT(meta = (DisplayName = "Calc Direction By Accelration (Math)"))
struct PMHW_API FSTT_CalcDirectionByAcceleration : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_CalcDirectionByAcceleration() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_CalcDirectionByAccelerationInstanceData::StaticStruct(); }

	// ================= [配置项 Input] =================
	UPROPERTY(EditAnywhere, Category = "Settings")
	EMHWDirectionOutputMode OutputMode = EMHWDirectionOutputMode::FourWay;

	// 方向切分阈值（单位：度）
	UPROPERTY(EditAnywhere, Category = "Settings")
	float ForwardHalfAngle = 45.0f;

	// ================= [方向片段命名] =================
	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Direction_Forward = FName("Forward");

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ForceUnits = "deg"))
	float ForwardWarpCompensationYaw = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (EditCondition = "OutputMode == EMHWDirectionOutputMode::FourWay", EditConditionHides))
	FName Direction_Backward = FName("Backward");

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (EditCondition = "OutputMode == EMHWDirectionOutputMode::FourWay", EditConditionHides, ForceUnits = "deg"))
	float BackwardWarpCompensationYaw = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Direction_Left = FName("Left");

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ForceUnits = "deg"))
	float LeftWarpCompensationYaw = -90.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FName Direction_Right = FName("Right");

	// 勾选后：当判定为“右转”时，右转补偿角使用 -90（适配曲线为 -90 -> 0 的资产）。
	// 不勾选时：右转补偿角使用 +90（默认行为）。
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bRightTurnUseNegativeWarpCompensation = false;

	UPROPERTY(EditAnywhere, Category = "Settings", meta = (ForceUnits = "deg"))
	float RightWarpCompensationYaw = 90.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugDrawArrows = false;

	// 核心逻辑只在进入时算一次
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
