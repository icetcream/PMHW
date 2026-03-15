#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "MHWPawnComponent.h"
#include "MHWComboPreInputComponent.generated.h"

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class PMHW_API UMHWComboPreInputComponent : public UMHWPawnComponent
{
	GENERATED_BODY()

public:	
	
	UMHWComboPreInputComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 默认的预输入缓存有效时间（秒）。例如 0.3 秒内按下的键才会被记住
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo Pre-Input")
	float DefaultBufferLifespan = 3.f;

	//-------------------------------------------------------------------------
	// 核心功能接口
	//-------------------------------------------------------------------------

	/**
	 * 尝试缓存一个输入（通常由 Enhanced Input 触发）
	 * @param InputTag 要缓存的动作标签（如 Action.Attack.Light）
	 * @param Priority 优先级（数值越大优先级越高）。同级或高级输入会覆盖低级输入。
	 * @param CustomLifespan 自定义缓存时间。如果 <= 0 则使用 DefaultBufferLifespan
	 */
	UFUNCTION(BlueprintCallable, Category = "Combo Pre-Input")
	void BufferInput(FGameplayTag InputTag, int32 Priority = 0, float CustomLifespan = -1.0f);

	/**
	 * 消耗当前缓存的输入（通常在动画通知 AnimNotify 中调用）
	 * @param OutTag 返回被缓存的标签
	 * @return 如果有有效的缓存输入，返回 true，并清空缓存。
	 */
	UFUNCTION(BlueprintCallable, Category = "Combo Pre-Input")
	bool ConsumeInput(FGameplayTag& OutTag);

	/**
	 * 检查当前是否有某个特定的输入被缓存（但不消耗它）
	 */
	UFUNCTION(BlueprintPure, Category = "Combo Pre-Input")
	bool HasBufferedInput(FGameplayTag& OutTag) const;

	/**
	 * 强制清空缓存（例如角色受击硬直、死亡时调用）
	 */
	UFUNCTION(BlueprintCallable, Category = "Combo Pre-Input")
	void ClearBuffer();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo Pre-Input")
	bool bCanPreInput = false;
private:
	// 当前缓存的数据
	FGameplayTag BufferedTag;
	int32 BufferedPriority;
	float TimeRemaining;
};