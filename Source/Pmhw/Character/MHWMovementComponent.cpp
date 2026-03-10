#include "MHWMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/Character.h"
namespace MHWCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("MHWCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
}
UMHWMovementComponent::UMHWMovementComponent()
{
	// 可以在这里修改默认的物理参数，比如把地面的默认摩擦力调高
	GroundFriction = 8.0f; 
	MaxAcceleration = 2000.0f;
}

void UMHWMovementComponent::BeginPlay()
{
	Super::BeginPlay();
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

const FMHWCharacterGroundInfo& UMHWMovementComponent::GetGroundInfo()
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
	{
		return CachedGroundInfo;
	}

	if (MovementMode == MOVE_Walking)
	{
		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
		CachedGroundInfo.GroundDistance = 0.0f;
	}
	else
	{
		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
		check(CapsuleComp);

		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
		const FVector TraceStart(GetActorLocation());
		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - MHWCharacter::GroundTraceDistance - CapsuleHalfHeight));

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(QueryParams, ResponseParam);

		FHitResult HitResult;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);

		CachedGroundInfo.GroundHitResult = HitResult;
		CachedGroundInfo.GroundDistance = MHWCharacter::GroundTraceDistance;

		if (MovementMode == MOVE_NavWalking)
		{
			CachedGroundInfo.GroundDistance = 0.0f;
		}
		else if (HitResult.bBlockingHit)
		{
			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
		}
	}

	CachedGroundInfo.LastUpdateFrame = GFrameCounter;

	return CachedGroundInfo;
}

