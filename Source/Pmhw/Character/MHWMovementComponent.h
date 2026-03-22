#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MHWMovementComponent.generated.h"
USTRUCT(BlueprintType)
struct FMHWCharacterGroundInfo
{
	GENERATED_BODY()

	FMHWCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

UENUM(BlueprintType)
enum class ELocomotionState: uint8
{
	Idle,
	Start,
	Cycle,
	Stop,
	Turn
};
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
	
	UFUNCTION(BlueprintCallable, Category = "Lyra|CharacterMovement")
	const FMHWCharacterGroundInfo& GetGroundInfo();
	
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion|State")
	bool bIsPivot = false;

public:
	// 推荐的做法：提供一个蓝图可调用的 Setter 函数
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void SetLocomotionState(ELocomotionState NewState);

	UFUNCTION(BlueprintPure, Category = "Locomotion")
	ELocomotionState GetLocomotionState() const { return CurrentState; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float  TurnPercent = 0.f;
protected:
	FMHWCharacterGroundInfo CachedGroundInfo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ELocomotionState CurrentState = ELocomotionState::Idle;
};
