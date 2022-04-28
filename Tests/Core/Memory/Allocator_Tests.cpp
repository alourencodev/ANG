#include <Tests/Vendor/Catch2/catch.hpp>

#include <Memory/Allocator.hpp>


using namespace age;

constexpr static char k_tag[] = "[Allocator]";

DECLARE_HEAP_ALLOCATOR(TestAllocator);
IMPLEMENT_HEAP_ALLOCATOR(TestAllocator);


constexpr u64 k_testAllocatorKbs = 1;
constexpr u64 k_testAllocatorBytes = k_testAllocatorKbs * 1012;


TEST_CASE("Allocate chunks linearly", k_tag)
{
	TestAllocator::preAllocate(k_testAllocatorBytes);

	int *testPtr = TestAllocator::alloc<int>(4);
	int *testPtr2 = TestAllocator::alloc<int>(2);
	int *testPtr3 = TestAllocator::alloc<int>(8);

	TestAllocator::free(testPtr2);

	TestAllocator::freePreAllocatedMemory();
}

