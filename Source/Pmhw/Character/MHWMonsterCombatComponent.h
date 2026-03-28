#pragma once

#include "CoreMinimal.h"
#include "Character/MHWCombatComponent.h"
#include "MHWMonsterCombatComponent.generated.h"

USTRUCT(BlueprintType)
struct PMHW_API FMHWPhysicalDefensePanel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Defense", meta = (DisplayName = "愤怒补正", ClampMin = "0.0"))
	float EnrageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Defense", meta = (DisplayName = "异常状态补正", ClampMin = "0.0"))
	float AilmentMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Defense", meta = (DisplayName = "全体防御率", ClampMin = "0.0"))
	float DefenseRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Defense", meta = (DisplayName = "默认肉质", ClampMin = "0.0"))
	float HitzoneValue = 100.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PMHW_API UMHWMonsterCombatComponent : public UMHWCombatComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "MHW|Combat|Panel")
	const FMHWPhysicalDefensePanel& GetPhysicalDefensePanel() const { return PhysicalDefensePanel; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel")
	FMHWPhysicalDefensePanel PhysicalDefensePanel;
};
