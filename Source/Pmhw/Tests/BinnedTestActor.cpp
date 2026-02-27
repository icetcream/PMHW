// Fill out your copyright notice in the Description page of Project Settings.


#include "BinnedTestActor.h"
#include "HAL/UnrealMemory.h"

// Sets default values
ABinnedTestActor::ABinnedTestActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABinnedTestActor::BeginPlay()
{
	Super::BeginPlay();
	// 1. 获取分配器名称 (修正后)
	if (GMalloc)
	{
		FString AllocName = GMalloc->GetDescriptiveName();
		UE_LOG(LogTemp, Warning, TEXT("=== 手动调试: %s ==="), *AllocName);
	}

	// 2. 测试你图片中定义的桶大小
	// 48 字节是一个典型的非 2 的幂次档位
	TArray<int32> SizesToTest = { 8, 48, 1168, 32768 }; 

	for (int32 Size : SizesToTest)
	{
		// 这里的第二个参数是 Alignment。在你的配置中，默认对齐是 16
		void* Ptr = FMemory::Malloc(Size, 16);

		if (Ptr)
		{
			// 验证地址是否符合 16 字节对齐
			uintptr_t Addr = (uintptr_t)Ptr;
			bool bIsAligned = (Addr % 16 == 0);

			UE_LOG(LogTemp, Log, TEXT("申请: %d bytes | 地址: %p | 对齐: %s"), 
				Size, Ptr, bIsAligned ? TEXT("OK") : TEXT("FAIL"));

			// 调试重点：在 Free 这里打断点，观察它如何处理 48 这种档位
			FMemory::Free(Ptr);
		}
	}
}

// Called every frame
void ABinnedTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}




