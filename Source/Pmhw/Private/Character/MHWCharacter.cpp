// MHWCharacter.cpp
#include "Character/MHWCharacter.h"
#include "Input/MHWInputComponent.h" // 引入自定义组件
#include "Input/MHWInputConfig.h"
#include "EnhancedInputSubsystems.h"
#include "MHWGameplayTags.h"
#include "Character/MHWPawnExtensionComponent.h"
#include "Equipment/MHWEquipmentManagerComponent.h"
#include "Input/MHWInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MHWCharacter)

AMHWCharacter::AMHWCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 告诉 UE 使用我们自定义的 InputComponent 类，而不是默认的
	// 这是 Lyra 的关键技巧之一

	MHWPawnExtensionComponent = CreateDefaultSubobject<UMHWPawnExtensionComponent>(TEXT("MHWPawnExtensionComponent"));
	MHWEquipmentManagerComponent = CreateDefaultSubobject<UMHWEquipmentManagerComponent>(TEXT("MHWEquipmentManagerComponent"));
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AMHWCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->GetAllComponentFromCharacter();
	}
}


void AMHWCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->CheckInitialization();
	}
	/*// 场景：观察堆内存的重新分配
	UE_LOG(LogTemp, Warning, TEXT("=== 测试默认分配器 ==="));

	TArray<int32> HeapArray; // 默认 FDefaultAllocator

	// 打印初始状态
	UE_LOG(LogTemp, Log, TEXT("Start: Ptr=%p, Num=%d, Max=%d"), HeapArray.GetData(), HeapArray.Num(), HeapArray.Max());

	// 逐步添加元素，观察指针变化
	for (int32 i = 0; i < 10; ++i)
	{
		int32* OldPtr = HeapArray.GetData();
		HeapArray.Add(i);
		int32* NewPtr = HeapArray.GetData();

		// 如果指针变了，说明发生了 Realloc（搬家）
		if (OldPtr != NewPtr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Trigger Realloc! Index: %d | Old: %p -> New: %p | Capacity: %d"), 
				i, OldPtr, NewPtr, HeapArray.Max());
		}
	}*/
}

void AMHWCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->CheckInitialization();
	}
}

void AMHWCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (MHWPawnExtensionComponent)
	{
		MHWPawnExtensionComponent->SetIsInput(true);
		MHWPawnExtensionComponent->CheckInitialization();
	}
}


