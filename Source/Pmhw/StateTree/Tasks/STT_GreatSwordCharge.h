#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "STT_GreatSwordCharge.generated.h"

USTRUCT()
struct FSTT_GreatSwordChargeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACharacter> Character = nullptr;

	float CurrentTurnYaw = 0.0f;
	
	float CurrentChargeTime = 0.0f;
};

USTRUCT(meta = (DisplayName = "Great Sword Charge Task (Dynamic Tag)"))
struct PMHW_API FSTT_GreatSwordCharge : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	FSTT_GreatSwordCharge() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FSTT_GreatSwordChargeInstanceData::StaticStruct(); }

	// ==================== 核心配置：具体是哪种蓄力 ====================
	
	// 在 StateTree 编辑器中，你可以为强蓄力节点配 .Level2，为真蓄节点配 .True
	UPROPERTY(EditAnywhere, Category = "Settings")
	FGameplayTag SpecificChargeTag; 

	// ==================================================================

	UPROPERTY(EditAnywhere, Category = "Settings")
	float MaxTurnAngle = 45.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float TurnSpeed = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	float MaxChargeDuration = 2.5f;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};