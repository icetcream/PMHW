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
	// ==========================================
	// 核心输入平滑逻辑 (Input Smoothing)
	// ==========================================

	// 缓存的、经过平滑处理后的输入方向。供动画蓝图读取来驱动转身补偿
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion|Input")
	FVector SmoothedInputDirection = FVector::ZeroVector;

	/**
	 * 每一帧调用，平滑过渡玩家的输入方向
	 * @param RawInputDirection  玩家这一帧的原始摇杆输入 (生硬的突变值)
	 * @param InterpSpeed        平滑速度 (例如 Start 传 10.0, Cycle 传 12.0)
	 * @param DeltaTime          帧时间
	 */
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void UpdateSmoothInputDirection(FVector RawInputDirection, float InterpSpeed, float DeltaTime);

	// 瞬间重置平滑方向 (通常在刚进入 Start 或 Cycle 状态时调用，防止上一状态的残留导致瞬间掉头)
	UFUNCTION(BlueprintCallable, Category = "Locomotion|Input")
	void ResetSmoothInputDirection(FVector NewDirection);
public:
	// 推荐的做法：提供一个蓝图可调用的 Setter 函数
	UFUNCTION(BlueprintCallable, Category = "Locomotion")
	void SetLocomotionState(ELocomotionState NewState);
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float  TurnPercent = 0.f;
protected:
	FMHWCharacterGroundInfo CachedGroundInfo;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ELocomotionState CurrentState;
};