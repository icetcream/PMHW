#include "MHWMovementComponent.h"

UMHWMovementComponent::UMHWMovementComponent()
{
	// 可以在这里修改默认的物理参数，比如把地面的默认摩擦力调高
	GroundFriction = 8.0f; 
	MaxAcceleration = 2000.0f;
}

bool UMHWMovementComponent::CalculateIsInPivot(FVector VelocityDirection, FVector DesiredDirection) const
{
	// 安全检查：如果速度方向或输入方向几乎为0，说明没在移动或没推摇杆，不可能触发急停
	if (VelocityDirection.IsNearlyZero() || DesiredDirection.IsNearlyZero())
	{
		return false;
	}

	// 核心数学：计算两个向量的点乘 (Dot Product)
	// 点乘结果在 -1(完全反向) 到 1(完全同向) 之间
	float DotResult = FVector::DotProduct(VelocityDirection.GetSafeNormal2D(), DesiredDirection.GetSafeNormal2D());

	// 阈值判断：如果点乘小于 -0.5（约等于夹角大于 120 度），判定为玩家猛拉了摇杆，触发急停！
	// 你可以把 -0.5 提取成一个变量，暴露给策划去调手感
	return DotResult < -0.5f; 
}