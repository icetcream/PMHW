// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/MHWInputComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWInputComponent)

UMHWInputComponent::UMHWInputComponent(const FObjectInitializer& ObjectInitializer)
{
	
}

void UMHWInputComponent::AddInputMappings(const UMHWInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to add something from your input config if required
}

void UMHWInputComponent::RemoveInputMappings(const UMHWInputConfig* InputConfig, UEnhancedInputLocalPlayerSubsystem* InputSubsystem) const
{
	check(InputConfig);
	check(InputSubsystem);

	// Here you can handle any custom logic to remove input mappings that you may have added above
}

void UMHWInputComponent::RemoveBinds(TArray<uint32>& BindHandles)
{
}

void UMHWInputComponent::UpdateSmoothInput(FVector TargetDirection, float InterpSpeed, float DeltaTime)
{
	// 1. 获取玩家推摇杆的力度大小 (0.0 到 1.0)
	// 因为 TargetDirection 可能不是归一化的 (玩家摇杆没推到底)
	float TargetMagnitude = TargetDirection.Size2D();

	// 2. 如果玩家完全没推摇杆 (死区)，直接把平滑输入归零
	if (TargetMagnitude <= KINDA_SMALL_NUMBER)
	{
		// 这里可以使用 FMath::VInterpTo 慢慢归零，产生“刹车滑行”的感觉，
		// 但通常在移动系统中，停止是由 Friction(摩擦力) 控制的，输入可以立刻切断。
		SmoothInputDirection = FVector::ZeroVector;
		return;
	}

	// 3. 将期望方向归一化 (纯粹的角度方向)
	FVector NormalizedTargetDir = TargetDirection.GetSafeNormal2D();

	// 4. 如果当前的平滑方向是 0 (比如角色刚起步的第一帧)
	// 直接把目标方向赋给它，防止从 0,0,0 插值导致第一步的诡异延迟
	if (SmoothInputDirection.IsNearlyZero())
	{
		SmoothInputDirection = TargetDirection;
		return;
	}

	// 5. ✨ 核心插值算法：旋转插值 (RInterpTo) ✨
	// 把当前的输入方向和目标的输入方向，都转换成 Rotator (旋转体)
	FRotator CurrentRot = SmoothInputDirection.Rotation();
	FRotator TargetRot = NormalizedTargetDir.Rotation();

	// 使用虚幻自带的最顺滑的旋转插值函数
	// 它会自动处理 -180 到 180 度的万向节死锁问题，抄近路旋转
	FRotator NewRot = UKismetMathLibrary::RInterpTo(CurrentRot, TargetRot, DeltaTime, InterpSpeed);

	// 6. 把插值算好的新角度，重新转回方向向量 (此时长度为 1)
	FVector NewDirection = NewRot.Vector();

	// 7. 重新乘上玩家推摇杆的力度 (Magnitude)
	// 这样既保证了角度是平滑转过来的，又保证了如果玩家只推了一半摇杆，角色能半速跑
	SmoothInputDirection = NewDirection * TargetMagnitude;

	// 确保 Z 轴始终为 0 (我们只做地面 2D 移动)
	SmoothInputDirection.Z = 0.0f;
}
