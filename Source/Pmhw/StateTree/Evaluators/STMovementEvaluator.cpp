#include "STMovementEvaluator.h"

#include "StateTreeExecutionContext.h"

void FSTMovementEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// 拿到我们的数据篮子
	FSTMovementEvaluator_InstanceData& InstanceData = Context.GetInstanceData<FSTMovementEvaluator_InstanceData>(*this);
	
	if (InstanceData.Controller)
	{
		// 【修改点 2】动态获取当前 Controller 附身的 Character
		ACharacter* CurrentChar = Cast<ACharacter>(InstanceData.Controller->GetPawn());
        
		// 更新缓存，如果玩家死掉或者换人了，这里会自动变空或更新
		InstanceData.ControlledCharacter = CurrentChar;
	}
	// 安全检查：如果角色存在，且他有移动组件
	if (InstanceData.ControlledCharacter && InstanceData.ControlledCharacter->GetCharacterMovement())
	{
		UCharacterMovementComponent* MoveComp = InstanceData.ControlledCharacter->GetCharacterMovement();

		// ✨ 核心：把 C++ 组件里的状态，拷贝到 StateTree 的变量里 ✨
		InstanceData.bIsFalling = MoveComp->IsFalling();
		
		InstanceData.CurrentSpeed = InstanceData.ControlledCharacter->GetVelocity().Size2D();
		
		// 获取玩家"最后一次输入的移动方向" (忽略Z轴，因为是地面移动)
		// 注意：不要用 GetVelocity()，速度有物理惯性。我们要的是玩家的纯粹意图(输入)
		FVector InputVector = InstanceData.ControlledCharacter->GetLastMovementInputVector();
		InputVector.Z = 0.0f; 

		// 计算输入量级 (通常在 0~1 之间)
		InstanceData.InputMagnitude = InputVector.Size2D();

		// 判断是否有输入 (添加一个极小的死区容差，防止手柄漂移)
		InstanceData.bHasMovementInput = InstanceData.InputMagnitude > 0.01f;
	}
	
}
