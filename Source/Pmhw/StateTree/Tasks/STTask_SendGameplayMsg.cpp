#include "STTask_SendGameplayMsg.h"

#include "STTask_SendGameplayMsg.h"
#include "AbilitySystemBlueprintLibrary.h" // 引入 GAS 蓝图函数库
#include "StateTreeExecutionContext.h"
#include "StateTree/Common/ControllerInputEvaluatorModel.h"

EStateTreeRunStatus FSTTask_SendGameplayMsg::EnterState(FStateTreeExecutionContext& Context,
                                                     const FStateTreeTransitionResult& Transition) const
{
	// 1. 获取我们刚才定义的数据结构体实例
	FSTTask_SendGameplayMsg_InstanceData& Data = Context.GetInstanceData<FSTTask_SendGameplayMsg_InstanceData>(*this);

	const UControllerInputEvaluatorModel* Model = Data.InputEvaluatorModel;

	// 1. 安全检查：仓库存在，且角色(目标)存活，Tag 有效
	if (Model && Model->ControlledActor && Data.EventTag.IsValid())
	{
		// 2. 补全 Payload 信息 (如果策划没填的话)
		if (Data.Payload.Instigator == nullptr)
		{
			// 发起者和目标都设为当前的身体
			Data.Payload.Instigator = Model->ControlledActor; 
			Data.Payload.Target = Model->ControlledActor;
		}

		// 3. 核心执行：向当前身体(TargetActor)的 ASC 发送事件
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Model->ControlledActor, 
			Data.EventTag, 
			Data.Payload
		);
	}

	return EStateTreeRunStatus::Running;
}
