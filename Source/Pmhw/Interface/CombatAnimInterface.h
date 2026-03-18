#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatAnimInterface.generated.h"

// 这个类不包含任何成员或函数，纯粹为了 UE 的反射系统存在
UINTERFACE(MinimalAPI, Blueprintable)
class UCombatAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 战斗动画相关的通信接口
 * 允许外部系统 (如 StateTree, GAS) 将战斗数据注入动画蓝图，而无需知道 AnimBP 的具体类型。
 */
class PMHW_API ICombatAnimInterface
{
	GENERATED_BODY()

public:

	/**
	 * 设置蓄力时的转身角度 (用于驱动 BlendSpace 或 AimOffset)
	 * @param TurnYaw 期望的偏航角 (通常在 -45.0 到 45.0 之间)
	 */
	// 使用 BlueprintNativeEvent，这意味着你可以在 C++ 里写默认实现，也可以在 AnimBP 里重写它！
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat|Animation")
	void SetChargeTurnYaw(float TurnYaw);
};