#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeTaskBase.h"
#include "STT_GreatSwordCharge.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UMHWEquipmentInstance;
class UAudioComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USoundBase;

USTRUCT(BlueprintType)
struct FMHWChargeScalarMaterialParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "参数名"))
	FName ParameterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "标量值"))
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct FMHWChargeVectorMaterialParameter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "参数名"))
	FName ParameterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "颜色值"))
	FLinearColor Value = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMHWChargeCharacterMaterialFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "Overlay材质"))
	TObjectPtr<UMaterialInterface> OverrideMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "标量参数"))
	TArray<FMHWChargeScalarMaterialParameter> ScalarParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "颜色参数"))
	TArray<FMHWChargeVectorMaterialParameter> VectorParameters;
};

USTRUCT(BlueprintType)
struct FMHWChargeStageFeedback
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX", meta = (DisplayName = "角色特效"))
	TObjectPtr<UNiagaraSystem> CharacterVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX", meta = (DisplayName = "武器特效"))
	TObjectPtr<UNiagaraSystem> WeaponVFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Audio", meta = (DisplayName = "音效"))
	TObjectPtr<USoundBase> Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Material", meta = (DisplayName = "角色材质参数"))
	TArray<FMHWChargeCharacterMaterialFeedback> CharacterMaterialFeedbacks;
};

USTRUCT(BlueprintType)
struct FMHWChargeFeedbackConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX", meta = (DisplayName = "1段反馈"))
	FMHWChargeStageFeedback Level1Feedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX", meta = (DisplayName = "2段反馈"))
	FMHWChargeStageFeedback Level2Feedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX", meta = (DisplayName = "3段反馈"))
	FMHWChargeStageFeedback Level3Feedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX", meta = (DisplayName = "过蓄反馈"))
	FMHWChargeStageFeedback OverchargedFeedback;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Character", meta = (DisplayName = "角色特效附着 Socket"))
	FName CharacterAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Character", meta = (DisplayName = "角色特效位置偏移"))
	FVector CharacterLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Character", meta = (DisplayName = "角色特效旋转偏移"))
	FRotator CharacterRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Character", meta = (DisplayName = "角色特效缩放"))
	FVector CharacterScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Weapon", meta = (DisplayName = "武器实例类型"))
	TSubclassOf<UMHWEquipmentInstance> WeaponInstanceClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Weapon", meta = (DisplayName = "武器特效附着 Socket"))
	FName WeaponAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Weapon", meta = (DisplayName = "武器特效位置偏移"))
	FVector WeaponLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Weapon", meta = (DisplayName = "武器特效旋转偏移"))
	FRotator WeaponRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|VFX|Weapon", meta = (DisplayName = "武器特效缩放"))
	FVector WeaponScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Audio", meta = (DisplayName = "音效挂到武器"))
	bool bAttachSoundToWeapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Audio", meta = (DisplayName = "音效附着 Socket"))
	FName SoundAttachSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Audio", meta = (DisplayName = "音效位置偏移"))
	FVector SoundLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MHW|Audio", meta = (DisplayName = "音效旋转偏移"))
	FRotator SoundRotationOffset = FRotator::ZeroRotator;
};

USTRUCT()
struct FMHWChargeDynamicMaterialState
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> OriginalOverlayMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> AppliedBaseMaterial = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial = nullptr;
};

USTRUCT()
struct FSTT_GreatSwordChargeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ACharacter> Character = nullptr;

	float CurrentTurnYaw = 0.0f;
	
	float CurrentChargeTime = 0.0f;

	bool bOwnsSpecificChargeTag = false;

	TObjectPtr<UNiagaraComponent> ActiveCharacterChargeVFXComponent = nullptr;

	TObjectPtr<UNiagaraComponent> ActiveWeaponChargeVFXComponent = nullptr;

	TObjectPtr<UAudioComponent> ActiveChargeAudioComponent = nullptr;

	UPROPERTY(Transient)
	FMHWChargeDynamicMaterialState ActiveCharacterOverlayMaterialState;

	bool bHasActiveChargeFeedbackLevel = false;

	uint8 ActiveChargeFeedbackLevel = 0;
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

	UPROPERTY(EditAnywhere, Category = "Charge Result", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ChargeLevel2Threshold = 0.32f;

	UPROPERTY(EditAnywhere, Category = "Charge Result", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float ChargeLevel3Threshold = 0.6f;

	UPROPERTY(EditAnywhere, Category = "Charge Result", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float OverchargeThreshold = 0.8f;

	UPROPERTY(EditAnywhere, Category = "VFX", meta = (DisplayName = "蓄力反馈配置"))
	FMHWChargeFeedbackConfig ChargeFeedback;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bUseMotiongWarping = true;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
