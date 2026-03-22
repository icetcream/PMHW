#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
// 引入你自定义的组件头文件
// #include "AdvancedLocoMovementComponent.h" 
// #include "CharacterInputComponent.h"
#include "ControllerInputEvaluatorModel.generated.h"

class UMHWInputComponent;
class UMHWMovementComponent;

/**
 * 状态树专用的数据模型 (Data Bus)
 * 作用：集中缓存当前控制的 Pawn 身上的核心组件指针，避免各个 Task 重复 Cast。
 */
UCLASS(BlueprintType)
class PMHW_API UControllerInputEvaluatorModel : public UObject
{
	GENERATED_BODY()

public:
	// 缓存的移动组件指针 (对应你截图里的 USpringMovementComponent)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	UMHWMovementComponent* MovementComponent = nullptr;

	// 缓存的输入组件指针
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	UMHWInputComponent* InputComponent = nullptr;

	// 缓存的处理好的玩家输入方向
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	FVector UserInputDirection = FVector::ZeroVector;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	AActor* ControlledActor = nullptr;

	// ✨ [新增] 缓存角色当前身体的面朝方向 (Forward Vector)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	FVector ActorForwardDirection = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	FGameplayTag CurrentWeaponState;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	FGameplayTagContainer CurrentWeaponStateContainer;
};
