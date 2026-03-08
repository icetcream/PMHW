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
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Locomotion|Math")
	bool CalculateIsInPivot(FVector VelocityDirection, FVector DesiredDirection) const;
	
	UFUNCTION(BlueprintCallable, Category = "Locomotion|State")
	void SetIsPivot(bool bInIsPivot) { bIsPivot = bInIsPivot; }
	
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	bool bIsPivot = false;
	
	float CurrentLocomotionAngle = 0.f;
	
};