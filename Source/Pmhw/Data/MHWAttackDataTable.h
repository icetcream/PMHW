#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MHWAttackDataTable.generated.h"

USTRUCT(BlueprintType)
struct PMHW_API FMHWAttackDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "攻击规格标签", Categories = "Data.AttackSpec"))
	FGameplayTag AttackSpecTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "耐力消耗", ClampMin = "0.0"))
	float StaminaCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "动作值", ClampMin = "0.0"))
	float MotionValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "气绝值", ClampMin = "0.0"))
	float StunValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "减气值", ClampMin = "0.0"))
	float ExhaustValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "骑乘值", ClampMin = "0.0"))
	float MountValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "属性倍率", ClampMin = "0.0"))
	float ElementMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "异常倍率", ClampMin = "0.0"))
	float StatusMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "部位破坏倍率", ClampMin = "0.0"))
	float PartBreakMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MHW|Attack", meta = (DisplayName = "软化值", ClampMin = "0.0"))
	float TenderizeValue = 0.0f;
};
