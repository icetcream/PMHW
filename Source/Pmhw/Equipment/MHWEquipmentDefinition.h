// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Settings/MovementSettings.h"

#include "MHWEquipmentDefinition.generated.h"

class UMHWHitStopData;
class UDataTable;
class UInputMappingContext;
class UInputActionMappingAsset;
class UAnimInstance;
class AActor;
class UMHWAbilitySet;
class UMHWEquipmentInstance;

USTRUCT(BlueprintType)
struct PMHW_API FMHWMeleeTraceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Trace")
	FName BaseSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Trace")
	TArray<FName> TraceSockets;
};

USTRUCT(BlueprintType)
struct PMHW_API FMHWHitStopTierConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "轻卡肉数据", ToolTip = "动作值较低时默认使用的卡肉数据。"))
	TObjectPtr<const UMHWHitStopData> LightHitStopData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "中卡肉数据", ToolTip = "动作值达到中段阈值后默认使用的卡肉数据。"))
	TObjectPtr<const UMHWHitStopData> MediumHitStopData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "重卡肉数据", ToolTip = "动作值达到重段阈值后默认使用的卡肉数据。"))
	TObjectPtr<const UMHWHitStopData> HeavyHitStopData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "终结段卡肉数据", ToolTip = "AttackWindow 勾选终结段时优先使用的卡肉数据。"))
	TObjectPtr<const UMHWHitStopData> FinisherHitStopData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "中段动作值阈值", ToolTip = "动作值达到这个数后，从轻卡肉切到中卡肉。", ClampMin = "0.0"))
	float MediumMotionValueThreshold = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "重段动作值阈值", ToolTip = "动作值达到这个数后，从中卡肉切到重卡肉。", ClampMin = "0.0"))
	float HeavyMotionValueThreshold = 55.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "低肉质阈值", ToolTip = "命中肉质低于这个值时，会套用低肉质倍率。", ClampMin = "0.0"))
	float LowHitzoneThreshold = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "高肉质阈值", ToolTip = "命中肉质高于等于这个值时，会套用高肉质倍率。", ClampMin = "0.0"))
	float HighHitzoneThreshold = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "低肉质倍率", ToolTip = "命中低肉质时对卡肉强度施加的倍率。", ClampMin = "0.0"))
	float LowHitzoneMultiplier = 0.85f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "默认肉质倍率", ToolTip = "普通肉质命中时的卡肉强度倍率。", ClampMin = "0.0"))
	float MidHitzoneMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "高肉质倍率", ToolTip = "命中高肉质时对卡肉强度施加的倍率。", ClampMin = "0.0"))
	float HighHitzoneMultiplier = 1.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "睡眠倍率", ToolTip = "睡眠大伤害等特殊命中时，对卡肉强度施加的倍率。", ClampMin = "0.0"))
	float SleepMultiplier = 1.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat Feel|Hitstop", meta = (DisplayName = "终结段倍率", ToolTip = "终结段命中时，对卡肉强度施加的倍率。", ClampMin = "0.0"))
	float FinisherMultiplier = 1.15f;
};

USTRUCT(BlueprintType)
struct PMHW_API FMHWPhysicalAttackPanelBonus
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "攻击力附加值"))
	float TrueRawAttackBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "武器特有加成附加值"))
	float MotionValueScaleBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "斩味补正附加值"))
	float SharpnessMultiplierBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "会心率附加值"))
	float CriticalChanceBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "正会心补正附加值"))
	float PositiveCriticalMultiplierBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "负会心补正附加值"))
	float NegativeCriticalMultiplierBonus = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MHW|Combat|Panel|Attack Bonus", meta = (DisplayName = "刃中补正附加值"))
	float BounceMultiplierBonus = 0.0f;

	bool HasAnyBonus() const
	{
		return !FMath::IsNearlyZero(TrueRawAttackBonus)
			|| !FMath::IsNearlyZero(MotionValueScaleBonus)
			|| !FMath::IsNearlyZero(SharpnessMultiplierBonus)
			|| !FMath::IsNearlyZero(CriticalChanceBonus)
			|| !FMath::IsNearlyZero(PositiveCriticalMultiplierBonus)
			|| !FMath::IsNearlyZero(NegativeCriticalMultiplierBonus)
			|| !FMath::IsNearlyZero(BounceMultiplierBonus);
	}
};

USTRUCT()
struct FMHWEquipmentActorToSpawn
{
	GENERATED_BODY()

	FMHWEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category=Equipment)
	
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * UMHWEquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class UMHWEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	UMHWEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Class to spawn
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<UMHWEquipmentInstance> InstanceType;

	// Gameplay ability sets to grant when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const UMHWAbilitySet>> AbilitySetsToGrant;

	// Actors to spawn on the pawn when this is equipped
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FMHWEquipmentActorToSpawn> ActorsToSpawn;
	
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Input", meta = (DisplayName = "输入动作映射资产", ToolTip = "装备该武器时，写入 ConfigManager 的输入动作映射数据。"))
	TObjectPtr<UInputActionMappingAsset> InputActionMappingAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Animation", meta = (DisplayName = "装备动画层", ToolTip = "装备该武器时链接到角色 Mesh 的动画层类。"))
	TSubclassOf<UAnimInstance> LinkedAnimLayerClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Feel", meta = (DisplayName = "默认卡肉数据", ToolTip = "当轻/中/重/终结段默认档都未配置时使用的兜底卡肉数据。"))
	TObjectPtr<const UMHWHitStopData> HitStopData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Feel", meta = (DisplayName = "卡肉默认档配置", ToolTip = "根据动作值、肉质和终结段标记自动选择默认卡肉数据。"))
	FMHWHitStopTierConfig HitStopTierConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FMHWMeleeTraceConfig DefaultMeleeTraceConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (DisplayName = "攻击数据表", ToolTip = "按动作标签查找基础伤害参数。攻击窗口只需要填写动作标签即可。"))
	TObjectPtr<UDataTable> AttackDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (DisplayName = "玩家攻击面板附加值", ToolTip = "装备上该武器时，附加到玩家战斗面板上的攻击、会心等数值。"))
	FMHWPhysicalAttackPanelBonus PlayerAttackPanelBonus;

	// Weapon state to set on the character when this equipment is equipped.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	FGameplayTag WeaponStateTag = MHWWeaponTags::Sheathed;

	// If enabled, add/update one entry in DA_MovementSettings.WeaponStates on equip.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	bool bAddMovementSettingsOnEquip = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (EditCondition = "bAddMovementSettingsOnEquip"))
	FMHWMovementRotationModeSettings MovementSettingsToAdd;
};                                       
