#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MHWMovementComponent.generated.h"

/**
 * 自定义的高级移动组件
 * 用于处理平滑加速、急停(Pivot)检测等 3A 级物理逻辑
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UMHWMovementComponent();

	// ==========================================
	// 1. 暴露给外部(比如 StateTree Task)的变量
	// ==========================================
	
	// 当前是否处于急转弯状态
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	bool bIsPivot = false;
	
	float CurrentLocomotionAngle = 0.f;

	// ==========================================
	// 2. 实现截图中的核心逻辑：检测是否急转弯
	// ==========================================
	
	/**
	 * 计算是否触发了急停(Pivot)
	 * @param VelocityDirection 角色的实际物理运动方向(带惯性)
	 * @param DesiredDirection 玩家摇杆推的期望方向
	 */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Math")
	bool CalculateIsInPivot(FVector VelocityDirection, FVector DesiredDirection) const;

	// 提供一个 Setter 给 StateTree 使用
	UFUNCTION(BlueprintCallable, Category = "Locomotion|State")
	void SetIsPivot(bool bInIsPivot) { bIsPivot = bInIsPivot; }
};