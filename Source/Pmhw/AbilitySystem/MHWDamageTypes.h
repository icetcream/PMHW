#pragma once

#include "CoreMinimal.h"
#include "MHWDamageTypes.generated.h"

UENUM(BlueprintType)
enum class EMHWCriticalHitType : uint8
{
	None UMETA(DisplayName = "普通命中"),
	Positive UMETA(DisplayName = "会心"),
	Negative UMETA(DisplayName = "负会心")
};

USTRUCT(BlueprintType, meta = (DisplayName = "物理伤害参数"))
struct PMHW_API FMHWPhysicalDamageSpec
{
	GENERATED_BODY()

	// True raw only. Convert displayed attack with the weapon coefficient before filling this.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "人物攻击力", ClampMin = "0.0"))
	float TrueRawAttack = 10.0f;

	// Motion value in MH style, e.g. 48 means 48%.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "动作值", ClampMin = "0.0"))
	float MotionValue = 100.0f;

	// Weapon-specific bonus such as spirit level, demon mode, red shield, triple buff.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "武器特有加成", ClampMin = "0.0"))
	float MotionValueScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "斩味补正", ClampMin = "0.0"))
	float SharpnessMultiplier = 1.0f;

	// Affinity in MH style. Positive rolls positive crit, negative rolls weak hits.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "会心率", ClampMin = "-100.0", ClampMax = "100.0"))
	float CriticalChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "正会心补正", ClampMin = "0.0"))
	float PositiveCriticalMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "负会心补正", ClampMin = "0.0"))
	float NegativeCriticalMultiplier = 0.75f;

	// Set >= 0 to bypass affinity rolls for deterministic behavior.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "会心补正覆盖", ClampMin = "-1.0"))
	float CriticalMultiplierOverride = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "刃中补正", ClampMin = "0.0"))
	float BounceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "愤怒补正", ClampMin = "0.0"))
	float EnrageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "异常状态补正", ClampMin = "0.0"))
	float AilmentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "全体防御率", ClampMin = "0.0"))
	float DefenseRate = 1.0f;

	// Physical hitzone in MH style, e.g. 65 means 65%.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "肉质", ClampMin = "0.0"))
	float HitzoneValue = 100.0f;

	// Reserved generic multiplier for project-specific conditions such as combo bonuses.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Damage", meta = (DisplayName = "额外补正", ClampMin = "0.0"))
	float AdditionalMultiplier = 1.0f;
};
