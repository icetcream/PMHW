#include "MHWComboPreInputComponent.h"

UMHWComboPreInputComponent::UMHWComboPreInputComponent(const FObjectInitializer& ObjectInitializer)
:Super(ObjectInitializer)
{
	// 开启 Tick，用于计时器倒计时清空缓存
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 默认关闭，有输入时再开启以节省性能

	ClearBuffer();
}

void UMHWComboPreInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 处理缓存过期逻辑
	if (TimeRemaining > 0.0f)
	{
		TimeRemaining -= DeltaTime;
		
		// 缓存超时，自动清空
		if (TimeRemaining <= 0.0f)
		{
			ClearBuffer();
		}
	}
}

void UMHWComboPreInputComponent::BufferInput(FGameplayTag InputTag, int32 Priority, float CustomLifespan)
{
	// 判断是否允许覆盖：
	// 1. 当前没有缓存 (TimeRemaining <= 0)
	// 2. 或者新输入的优先级 >= 当前缓存的优先级 (使用 >= 可以让玩家连续狂按同一个键时刷新存活时间)
	if (!bCanPreInput) return;
	if (TimeRemaining <= 0.0f || Priority >= BufferedPriority)
	{
		BufferedTag = InputTag;
		BufferedPriority = Priority;
		TimeRemaining = (CustomLifespan > 0.0f) ? CustomLifespan : DefaultBufferLifespan;

		// 开启 Tick 进行倒计时
		SetComponentTickEnabled(true);
	}
}

bool UMHWComboPreInputComponent::ConsumeInput(FGameplayTag& OutTag)
{
	if (TimeRemaining > 0.0f && BufferedTag.IsValid())
	{
		// 取出数据
		OutTag = BufferedTag;
		
		// 消耗掉后，立刻清空缓存
		ClearBuffer();
		return true;
	}

	OutTag = FGameplayTag::EmptyTag;
	return false;
}

bool UMHWComboPreInputComponent::HasBufferedInput(FGameplayTag& OutTag) const
{
	if (TimeRemaining > 0.0f && BufferedTag.IsValid())
	{
		OutTag = BufferedTag;
		return true;
	}
	OutTag = FGameplayTag::EmptyTag;
	return false;
}

void UMHWComboPreInputComponent::ClearBuffer()
{
	BufferedTag = FGameplayTag::EmptyTag;
	BufferedPriority = -1;
	TimeRemaining = 0.0f;

	// 关闭 Tick 节省性能
	SetComponentTickEnabled(false);
}