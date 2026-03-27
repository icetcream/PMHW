// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MHWEquipmentInstance.generated.h"

class UMHWHitStopData;
class UMHWEquipmentDefinition;
class AActor;
class APawn;
struct FFrame;
struct FMHWEquipmentActorToSpawn;
struct FMHWMeleeTraceConfig;



/**
 * UMHWEquipmentInstance
 *
 * A piece of equipment spawned and applied to a pawn
 */
UCLASS(BlueprintType, Blueprintable)
class UMHWEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	UMHWEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// 【新增 1】：给外部（比如 AddEntry）调用的初始化方法
	virtual void SetEquipmentDefinition(TSubclassOf<UMHWEquipmentDefinition> InDef);

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return false; }
	virtual UWorld* GetWorld() const override final;
	//~End of UObject interface

	UFUNCTION(BlueprintPure, Category=Equipment)
	UObject* GetInstigator() const { return Instigator; }

	void SetInstigator(UObject* InInstigator) { Instigator = InInstigator; }

	UFUNCTION(BlueprintPure, Category=Equipment)
	APawn* GetPawn() const;

	// 可以用来显示蓝图中的输出引脚为传入的类型
	UFUNCTION(BlueprintPure, Category=Equipment, meta=(DeterminesOutputType=PawnType))
	APawn* GetTypedPawn(TSubclassOf<APawn> PawnType) const;

	UFUNCTION(BlueprintPure, Category=Equipment)
	AActor* GetSpawnedActor() const { return SpawnedActor; }
	
	UFUNCTION(BlueprintCallable, Category=Equipment)
	void UpdateAttachment(FName NewSocketName);
	
	UFUNCTION(BlueprintPure, Category = "Combat Feel")
	const UMHWHitStopData* GetHitStopData() const;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	const UMHWEquipmentDefinition* GetEquipmentDefinition() const;

	const FMHWMeleeTraceConfig* GetDefaultMeleeTraceConfig() const;
	

	virtual void SpawnEquipmentActors(const TArray<FMHWEquipmentActorToSpawn>& ActorsToSpawn);
	virtual void DestroyEquipmentActors();

	virtual void OnEquipped();
	virtual void OnUnequipped();

protected:
	
	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnEquipped"))
	void K2_OnEquipped();

	UFUNCTION(BlueprintImplementableEvent, Category=Equipment, meta=(DisplayName="OnUnequipped"))
	void K2_OnUnequipped();

private:
	UPROPERTY()
	TObjectPtr<UObject> Instigator;

	UPROPERTY()
	TObjectPtr<AActor> SpawnedActor;
	
	UPROPERTY()
	TSubclassOf<UMHWEquipmentDefinition> EquipmentDef;
};

