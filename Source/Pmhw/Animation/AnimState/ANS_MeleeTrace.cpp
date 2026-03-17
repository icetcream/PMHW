#include "ANS_MeleeTrace.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Equipment/MHWEquipmentInstance.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "GameFramework/Actor.h"
#include "Interface/MHWCharacterInterface.h"

UANS_MeleeTrace::UANS_MeleeTrace()
{
    // 设置默认值
    TraceChannel = ECC_Pawn; // 默认打 Pawn
    bShowDebug = false;
    DebugDuration = 2.0f;
}

void UANS_MeleeTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (!MeshComp) return;
    
    AActor* MyCharacter = MeshComp->GetOwner();

    if (MyCharacter->Implements<UMHWCharacterInterface>())
    {
        if (UMHWEquipmentManagerComponent* EquipmentManagerComponent = const_cast<UMHWEquipmentManagerComponent*>(IMHWCharacterInterface::Execute_GetEquipmentManagerComponent(MyCharacter)))
        {
            if (UMHWEquipmentInstance* EquipmentInstance = EquipmentManagerComponent->GetFirstInstanceOfType(WeaponInstanceClass))
            {
                OwnerActor = EquipmentInstance->GetSpawnedActor();
            }
        }
    }
    WeaponMesh = OwnerActor->GetComponentByClass<USkeletalMeshComponent>();
    // 清理上一轮的脏数据
    HitActors.Empty();
    PreviousLocations.Empty();

    // 记录挥剑第一帧所有插槽的初始位置
    for (const FName& SocketName : Sockets)
    {
        FVector InitialLoc = WeaponMesh->GetSocketLocation(SocketName);
        PreviousLocations.Add(InitialLoc);
    }
}

void UANS_MeleeTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    if (!WeaponMesh || !OwnerActor) return;

    UWorld* World = WeaponMesh->GetWorld();
    if (!World) return;

    // 确保插槽数量和上一帧记录的位置数量一致
    if (Sockets.Num() != PreviousLocations.Num()) return;

    for (int32 Index = 0; Index < Sockets.Num(); ++Index)
    {
        FVector CurrentLoc = WeaponMesh->GetSocketLocation(Sockets[Index]);
        FVector PreviousLoc = PreviousLocations[Index];

        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(OwnerActor);
        // 为了更高的精度，通常开启复杂碰撞追踪 (可选)
        QueryParams.bTraceComplex = false; 

        TArray<FHitResult> HitResults;

        // 【关键替换】：使用纯射线追踪代替球体扫描
        bool bHit = World->LineTraceMultiByChannel(
            HitResults,
            PreviousLoc, // 起点：上一帧的位置
            CurrentLoc,  // 终点：这一帧的位置
            TraceChannel,
            QueryParams
        );

        // 绘制调试射线
        if (bShowDebug)
        {
            FColor DebugColor = bHit ? FColor::Green : FColor::Red;
            DrawDebugLine(World, PreviousLoc, CurrentLoc, DebugColor, false, DebugDuration, 0, 1.5f);
            // 在击中点画个小红叉
            if (bHit)
            {
                for (const FHitResult& Hit : HitResults)
                {
                    DrawDebugPoint(World, Hit.ImpactPoint, 10.0f, FColor::Yellow, false, DebugDuration);
                }
            }
        }

        // 处理命中逻辑
        if (bHit)
        {
            for (const FHitResult& Hit : HitResults)
            {
                AActor* HitActor = Hit.GetActor();

                // 检查是否合法且尚未被击中过
                if (HitActor && !HitActors.Contains(HitActor))
                {
                    // 1. 将其加入已击中列表，防止连击
                    HitActors.Add(HitActor);

                    // 2. 触发伤害事件 (这里你可以根据你的架构来)
                    
                    // 方案A: 原生应用伤害
                    /* 
                    UGameplayStatics::ApplyPointDamage(
                        HitActor, 
                        10.0f, // 伤害值
                        (CurrentLoc - PreviousLoc).GetSafeNormal(), // 击退方向
                        Hit, 
                        OwnerActor->GetInstigatorController(), 
                        OwnerActor, 
                        nullptr
                    ); 
                    */

                    // 方案B: 如果你用了接口 (推荐)
                    // ICombatInterface::Execute_OnMeleeHit(OwnerActor, Hit);

                    // 方案C: 如果你用 GAS，在这里通过 TargetData 发送 GameplayEvent
                }
            }
        }

        // 核心步：将当前帧位置更新为“上一帧”，供下一 Tick 使用
        PreviousLocations[Index] = CurrentLoc;
    }
}

void UANS_MeleeTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    // 挥砍结束，清空内存防止泄漏或影响下一次挥砍
    HitActors.Empty();
    PreviousLocations.Empty();
}