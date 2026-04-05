#pragma once

#include "CoreMinimal.h"
#include "Character/MHWCombatComponent.h"
#include "MHWPlayerCombatComponent.generated.h"

struct FMHWPhysicalAttackPanelBonus;
class UMHWMonsterCombatComponent;

USTRUCT(BlueprintType)
struct PMHW_API FMHWPhysicalAttackPanel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "人物攻击力", ClampMin = "0.0"))
	float TrueRawAttack = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "武器特有加成", ClampMin = "0.0"))
	float MotionValueScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "斩味补正", ClampMin = "0.0"))
	float SharpnessMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "会心率", ClampMin = "-100.0", ClampMax = "100.0"))
	float CriticalChance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "正会心补正", ClampMin = "0.0"))
	float PositiveCriticalMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "负会心补正", ClampMin = "0.0"))
	float NegativeCriticalMultiplier = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel|Attack", meta = (DisplayName = "刃中补正", ClampMin = "0.0"))
	float BounceMultiplier = 1.0f;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PMHW_API UMHWPlayerCombatComponent : public UMHWCombatComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "MHW|Combat|Panel")
	const FMHWPhysicalAttackPanel& GetPhysicalAttackPanel() const { return PhysicalAttackPanel; }

	UFUNCTION(BlueprintCallable, Category = "MHW|Combat|Damage")
	FMHWPhysicalDamageSpec BuildResolvedOutgoingPhysicalDamageSpec(const FMHWPhysicalDamageSpec& RuntimeDamageSpec, const UMHWMonsterCombatComponent* TargetCombatComponent) const;

	void ApplyAttackPanelBonus(const FMHWPhysicalAttackPanelBonus& Bonus);
	void RemoveAttackPanelBonus(const FMHWPhysicalAttackPanelBonus& Bonus);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Combat|Panel")
	FMHWPhysicalAttackPanel PhysicalAttackPanel;

private:
	void ApplyAttackPanelBonusInternal(const FMHWPhysicalAttackPanelBonus& Bonus, float Sign);
	void NormalizePhysicalAttackPanel();
};
