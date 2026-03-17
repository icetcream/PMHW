#include "MeleeTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Data/MHWHitStopData.h"
#include "Engine/World.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "Interface/MHWCharacterInterface.h"


UMeleeTraceComponent::UMeleeTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; 
}


USkeletalMeshComponent* UMeleeTraceComponent::GetWeaponMesh()
{
	AActor* MyCharacter = GetOwner();
	if (!OwnerMeshComp && MyCharacter)
	{
		if (MyCharacter->Implements<UMHWCharacterInterface>())
		{
			if (UMHWEquipmentManagerComponent* EquipmentManagerComponent = const_cast<UMHWEquipmentManagerComponent*>(IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(MyCharacter)))
			{
				if (UMHWEquipmentInstance* EquipmentInstance = EquipmentManagerComponent->GetFirstInstanceOfType(WeaponInstanceClass))
				{
					CachedEquipmentInstance = EquipmentInstance;
					AActor* WeaponActor = EquipmentInstance->GetSpawnedActor();
					if (WeaponActor)
					{
						OwnerMeshComp = WeaponActor->GetComponentByClass<USkeletalMeshComponent>();
					}
				}
			}
		}
	}
	return OwnerMeshComp;
}

void UMeleeTraceComponent::ApplyHitStop(AActor* HitActor)
{
	// 1. 数据驱动拦截：如果没有配数据，或者配置了关闭，直接返回
	if (!CurrentHitstopData || !CurrentHitstopData->bEnableHitstop || !CurrentHitstopData->HitstopCurve) 
		return;
	
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor) return;

	// 2. 如果这是这招砍中的第一只怪（刚开始卡肉）
	if (!bIsHitstopping)
	{
		// 记录攻击者原本的速度
		DefaultTimeDilation = OwnerActor->CustomTimeDilation;
		bIsHitstopping = true;
		
		// 记录此时的【真实世界时间】（不受 TimeDilation 影响的时间）
		HitstopStartTime = World->GetRealTimeSeconds();

		HitstopAffectedActors.Empty();
		HitstopAffectedActors.Add(OwnerActor); // 攻击者自己必须卡肉
	}

	// 3. 把新砍中的怪物也加入卡肉大军
	// （即使已经卡肉了，刀刃扫到第二只怪时，第二只怪也要瞬间被时间静止）
	if (HitActor && !HitstopAffectedActors.Contains(HitActor))
	{
		HitstopAffectedActors.Add(HitActor);
	}

	// 确保组件的 Tick 是开启的，否则曲线读不下去
	SetComponentTickEnabled(true); 
}

void UMeleeTraceComponent::ResetHitStop()
{
	bIsHitstopping = false;

	// 把所有被我们放慢的 Actor（主角和怪物）的速度恢复到打人前的状态
	for (auto& WeakActor : HitstopAffectedActors)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->CustomTimeDilation = DefaultTimeDilation;
		}
	}
	HitstopAffectedActors.Empty();

	// 如果挥砍判定也早就结束了，为了省性能，关掉 Tick
	if (!bIsTracing)
	{
		SetComponentTickEnabled(false);
	}
}

void UMeleeTraceComponent::StartTrace(FName InBaseSocket, const TArray<FName>& InTraceSockets)
{
	// 【核心改动】：在这里进行懒加载。如果之前没获取过，这里会顺着接口去找。找过了就直接返回缓存。
	USkeletalMeshComponent* WeaponMesh = GetWeaponMesh();

	// 如果武器没拿到，或者没配插槽，直接退出
	if (!WeaponMesh || InTraceSockets.Num() == 0 || InBaseSocket.IsNone()) return;
	
	CurrentHitstopData = nullptr;
	if (CachedEquipmentInstance)
	{
		// 这里调用你之前在 MHWEquipmentInstance 里写好的方法！
		CurrentHitstopData = CachedEquipmentInstance->GetHitStopData();
	}

	BaseSocketName = InBaseSocket;
	ActiveTraceSockets = InTraceSockets;
	HitActors.Empty();
	TraceSocketsLocalOffsets.Empty();

	// 获取武器基底的第一帧世界 Transform
	PreviousBaseTransform = WeaponMesh->GetSocketTransform(BaseSocketName);

	// 预计算局部坐标
	for (const FName& SocketName : ActiveTraceSockets)
	{
		FVector WorldLoc = WeaponMesh->GetSocketLocation(SocketName);
		FVector LocalLoc = PreviousBaseTransform.InverseTransformPosition(WorldLoc);
		TraceSocketsLocalOffsets.Add(LocalLoc);
	}

	bIsTracing = true;
	SetComponentTickEnabled(true); 
}

void UMeleeTraceComponent::StopTrace()
{
	bIsTracing = false;
	HitActors.Empty();
	TraceSocketsLocalOffsets.Empty();
	ActiveTraceSockets.Empty();
	SetComponentTickEnabled(false); 
}

void UMeleeTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UWorld* World = GetWorld();
	if (!World) return;

	// ======================== 1. 动态卡肉曲线处理 ========================
	if (bIsHitstopping && CurrentHitstopData && CurrentHitstopData->HitstopCurve)
	{
		// 算一下从卡肉开始到现在，真实世界过去了多少秒
		float ElapsedRealTime = World->GetRealTimeSeconds() - HitstopStartTime;

		// 看看是不是该结束了
		if (ElapsedRealTime >= CurrentHitstopData->HitstopDuration)
		{
			ResetHitStop();
		}
		else
		{
			// 计算当前卡肉进度 (0.0 代表刚开始，1.0 代表要结束了)
			float Alpha = ElapsedRealTime / CurrentHitstopData->HitstopDuration;

			// 【神来之笔】：去 DataAsset 的曲线图里，查一下这个进度对应的时间缩放率
			float CurrentSpeedMultiplier = CurrentHitstopData->HitstopCurve->GetFloatValue(Alpha);

			// 把这个“极慢”的速度，强行塞给主角和所有被打中的怪物
			for (auto& WeakActor : HitstopAffectedActors)
			{
				if (AActor* Actor = WeakActor.Get())
				{
					Actor->CustomTimeDilation = DefaultTimeDilation * CurrentSpeedMultiplier;
				}
			}
		}
	}
	// 此时 OwnerMeshComp 必定是有效的（在 StartTrace 里拦截过了）
	if (!bIsTracing || !OwnerMeshComp || DeltaTime <= 0.0f) return;

	// ====== 自适应补帧计算 ======
	float CurrentFPS = 1.0f / DeltaTime;
	float SubstepInterval = DELTA_TIME_60FPS; 
	if (CurrentFPS > MIN_FPS)
	{
		float Weight = FMath::Clamp((CurrentFPS - MIN_FPS) / (MAX_FPS - MIN_FPS), 0.0f, 1.0f);
		SubstepInterval = FMath::Lerp(DELTA_TIME_60FPS, DELTA_TIME_120FPS, Weight);
	}
	int32 NumSteps = FMath::CeilToInt(DeltaTime / SubstepInterval);
	NumSteps = FMath::Clamp(NumSteps, 1, 4); 

	// ====== 获取当前帧武器基底的 Transform ======
	FTransform CurrentBaseTransform = OwnerMeshComp->GetSocketTransform(BaseSocketName);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false; 

	// ====== 基于 Transform 曲线插值的纯射线判定网编织 ======
	for (int32 Step = 1; Step <= NumSteps; ++Step)
	{
		float AlphaPrev = (float)(Step - 1) / (float)NumSteps;
		float AlphaCurr = (float)Step / (float)NumSteps;

		FTransform StepStartBaseTransform;
		StepStartBaseTransform.Blend(PreviousBaseTransform, CurrentBaseTransform, AlphaPrev); 

		FTransform StepEndBaseTransform;
		StepEndBaseTransform.Blend(PreviousBaseTransform, CurrentBaseTransform, AlphaCurr);

		for (int32 i = 0; i < TraceSocketsLocalOffsets.Num(); ++i)
		{
			const FVector& LocalOffset = TraceSocketsLocalOffsets[i];
			FVector StepStartLoc = StepStartBaseTransform.TransformPosition(LocalOffset);
			FVector StepEndLoc   = StepEndBaseTransform.TransformPosition(LocalOffset);

			TArray<FHitResult> HitResults;
			bool bHit = World->LineTraceMultiByChannel(HitResults, StepStartLoc, StepEndLoc, TraceChannel, QueryParams);

			// ====== Debug ======
			if (bShowDebug)
			{
				FColor DebugColor = bHit ? FColor::Green : FColor::Red;
				DrawDebugLine(World, StepStartLoc, StepEndLoc, DebugColor, false, 2.0f, 0, 1.5f);
				if (bHit)
				{
					for (const FHitResult& Hit : HitResults)
					{
						DrawDebugPoint(World, Hit.ImpactPoint, 10.0f, FColor::Yellow, false, 2.0f);
					}
				}
			}

			// ====== 命中逻辑 ======
			if (bHit)
			{
				for (const FHitResult& Hit : HitResults)
				{
					AActor* HitActor = Hit.GetActor();
					if (HitActor && !HitActors.Contains(HitActor))
					{
						HitActors.Add(HitActor);
						OnMeleeHit.Broadcast(Hit);
						ApplyHitStop(HitActor);
					}
				}
			}
		}
	}

	PreviousBaseTransform = CurrentBaseTransform;
}