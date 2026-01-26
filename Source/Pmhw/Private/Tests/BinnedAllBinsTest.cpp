#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

// 使用 COMPLEX 宏。注意：它没有直接的测试名称字符串，只有类名和路径前缀
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FBinnedAllBinsTest, "Pmhw.Memory.TestEveryBin", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/**
 * 第一步：GetTests - 告诉引擎你要生成哪些子测试
 */
void FBinnedAllBinsTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	// 这里我们可以直接引用你图片里看到的那个 BlockSizes 数组
	// 为了演示，我们手动列举几个核心档位
	TArray<int32> BinsToTest = { 8, 16, 32, 48, 64, 128, 512, 1024, 32768 };

	for (int32 BinSize : BinsToTest)
	{
		// OutBeautifiedNames: 在 Rider/编辑器 UI 面板里显示的名称
		OutBeautifiedNames.Add(FString::Printf(TEXT("分配测试_大小_%d_字节"), BinSize));
        
		// OutTestCommands: 真正传递给 RunTest 的参数字符串
		OutTestCommands.Add(FString::FromInt(BinSize));
	}
}

/**
 * 第二步：RunTest - 针对每一个子测试运行的逻辑
 */
bool FBinnedAllBinsTest::RunTest(const FString& Parameters)
{
	// Parameters 就是上面 OutTestCommands 里对应的字符串
	int32 CurrentBinSize = FCString::Atoi(*Parameters);

	UE_LOG(LogTemp, Log, TEXT("正在压测桶档位: %d 字节"), CurrentBinSize);

	// 1. 执行分配
	void* Ptr = FMemory::Malloc(CurrentBinSize);
    
	// 2. 验证：由于 Binned 分配器会对齐，地址必须符合要求
	TestNotNull(TEXT("内存不应为空"), Ptr);
    
	// 3. 读写测试：确保这块内存是真实可用的
	uint8* BytePtr = (uint8*)Ptr;
	BytePtr[0] = 0xAA;
	BytePtr[CurrentBinSize - 1] = 0xBB;
    
	TestEqual(TEXT("首字节读写校验"), BytePtr[0], (uint8)0xAA);
	TestEqual(TEXT("末字节读写校验"), BytePtr[CurrentBinSize - 1], (uint8)0xBB);

	// 4. 释放
	FMemory::Free(Ptr);

	return true;
}