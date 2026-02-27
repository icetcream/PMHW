#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MallocBinned, "Pmhw.Pmhw.Public.MallocBinned",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MallocBinned::RunTest(const FString& Parameters)
{
	// Make the test pass by returning true, or fail by returning false.
	// 1. 验证当前分配器类型
	const TCHAR* AllocName = GMalloc->GetDescriptiveName();
	TestTrue(TEXT("必须使用 Binned 分配器进行测试"), FString(AllocName).Contains(TEXT("Binned")));

	// 2. 测试小块内存分配与对齐 (Small Block)
	// 申请 12 字节，默认对齐应该是 16
	void* Ptr1 = FMemory::Malloc(12, 16);
	TestNotNull(TEXT("小块内存分配失败"), Ptr1);
	TestTrue(TEXT("指针地址应 16 字节对齐"), ((uintptr_t)Ptr1 % 16) == 0);

	// 3. 测试桶分级 (Binning Logic)
	// 申请 20 字节，在你的 BlockSizes 数组中应对应到 32 字节桶
	// 我们可以通过分配两个连续块并观察间距来“侧写”分配器行为
	void* Ptr2 = FMemory::Malloc(20, 1);
	void* Ptr3 = FMemory::Malloc(20, 1);
	// 注意：如果它们在同一个 Pool 里，间距通常是 BlockSize
    
	// 4. 测试大内存 (Large Block)
	void* BigPtr = FMemory::Malloc(64 * 1024, 16);
	TestNotNull(TEXT("大块内存分配失败"), BigPtr);

	// 释放内存，测试 Free 逻辑
	FMemory::Free(Ptr1);
	FMemory::Free(Ptr2);
	FMemory::Free(Ptr3);
	FMemory::Free(BigPtr);

	return true;
}

