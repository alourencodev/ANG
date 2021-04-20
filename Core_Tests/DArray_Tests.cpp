#include <catch.hpp>

#include <Core/DArray.hpp>
#include <Core/NonCopyable.hpp>


constexpr static char k_tag[] = "[DArray]";


class TestNonCopyable : public NonCopyable
{
public:
	TestNonCopyable() : n(0) {}
	TestNonCopyable(int inN) : n(inN) {}

	bool operator==(const TestNonCopyable &other) { return other.n == n;}

	int n;
};


TEST_CASE("Dynamic Array Constructors", k_tag)
{
	SECTION("Default constructor")
	{
		DArray<int> da;
		REQUIRE(da.capacity() == 0);
		REQUIRE(da.count() == 0);
	}

	SECTION("Capacity defined constructor")
	{
		DArray<int> da(3);
		REQUIRE(da.capacity() == 3);
		REQUIRE(da.count() == 0);
	}

	SECTION("Constructor from static array")
	{
		SArray<int, 3> sa = {1, 2, 3};
		DArray<int> da(sa);

		REQUIRE(da.capacity() == 3);
		REQUIRE(da.count() == 3);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	SECTION("Constructor from initializer list")
	{
		DArray<int> da = {1, 2, 3};
		REQUIRE(da.capacity() == 3);
		REQUIRE(da.count() == 3);
		
		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	SECTION("Copy Constructor")
	{
		DArray<i32> da(1);
		da.add(1);

		DArray<i32> da2(da);

		REQUIRE(da2.capacity() == 1);
		REQUIRE(da2.count() == 1);
		REQUIRE(da2.data() != nullptr);

		REQUIRE(da.capacity() == 1);
		REQUIRE(da.count() == 1);
		REQUIRE(da.data() != nullptr);

		REQUIRE(da.data() != da2.data());
	}

	SECTION("Move Constructor")
	{
		DArray<TestNonCopyable> da(1);
		da.add(TestNonCopyable());

		DArray<TestNonCopyable> da2(std::move(da));

		REQUIRE(da2.capacity() == 1);
		REQUIRE(da2.count() == 1);
		REQUIRE(da2.data() != nullptr);

		REQUIRE(da.capacity() == 0);
		REQUIRE(da.count() == 0);
		REQUIRE(da.data() == nullptr);
	}
}


TEST_CASE("Dynamic Array Reserve", k_tag)
{
	SECTION("Regular reserve")
	{
		DArray<int> da;
		da.reserve(3);

		REQUIRE(da.count() == 0);
		REQUIRE(da.capacity() == 3);
	}

	SECTION("Reserve less than capacity")
	{
		DArray<int> da(3);
		da.reserve(1);

		REQUIRE(da.count() == 0);
		REQUIRE(da.capacity() == 4);
	}

	SECTION("Reserve twice")
	{
		DArray<int> da;
		da.reserve(3);

		da.add(1);
		da.add(2);
		da.add(3);

		da.reserve(5);

		REQUIRE(da.count() == 3);
		REQUIRE(da.capacity() == 8);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}
}


TEST_CASE("Dynamic Array Resize", k_tag)
{
	SECTION("Standard Resize")
	{
		DArray<int> da;
		da.resize(4);
		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 0);
	}

	SECTION("Resize to less than count")
	{
		DArray<int> da = {1, 2, 3};
		da.resize(2);
		
		REQUIRE(da.capacity() == 2);
		REQUIRE(da.count() == 2);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
	}

	SECTION("Resize to 0")
	{
		DArray<int> da = {1, 2, 3};
		da.resize(0);

		REQUIRE(da.capacity() == 0);
		REQUIRE(da.count() == 0);
	}

	SECTION("Shrink to fit")
	{
		DArray<int> da(4);

		da.add(1);
		da.add(2);

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 2);

		da.shrinkToFit();

		REQUIRE(da.capacity() == 2);
		REQUIRE(da.count() == 2);
	}
}


TEST_CASE("Add element to DArray", k_tag)
{
	SECTION("Add Single Elements")
	{
		DArray<int> da;
		da.add(1);

		REQUIRE(da.count() == 1);
		REQUIRE(da.capacity() == 2);

		da.add(2);

		REQUIRE(da.count() == 2);
		REQUIRE(da.capacity() == 2);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
	}

	SECTION("Add Array")
	{
		SArray<int, 3> sa = {1, 2, 3};
		DArray<int> da(2);
		
		REQUIRE(da.count() == 0);
		REQUIRE(da.capacity() == 2);

		da.add(sa);

		REQUIRE(da.count() == 3);
		REQUIRE(da.capacity() == 4);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	SECTION("Add list")
	{
		DArray<int> da(2);

		REQUIRE(da.count() == 0);
		REQUIRE(da.capacity() == 2);

		da.add({1, 2, 3});

		REQUIRE(da.count() == 3);
		REQUIRE(da.capacity() == 4);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	SECTION("Add move semantics")
	{
		DArray<TestNonCopyable> da(3);
		da.add(TestNonCopyable(0));
	}
}


TEST_CASE("Insert Element in DArray", k_tag)
{
	SECTION("Insert element")
	{
		DArray<u32> da = {1, 2, 3}; 

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);

		REQUIRE(da.capacity() == 3);
		REQUIRE(da.count() == 3);

		da.insert(4, 1);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 4);
		REQUIRE(da[2] == 2);
		REQUIRE(da[3] == 3);

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 4);
	}

	SECTION("Insert Darray of elements")
	{
		DArray<u32> daA = {1, 2, 3};
		DArray<u32> daB = {99, 88};

		daA.insert(daB, 2);

		REQUIRE(daA[0] == 1);
		REQUIRE(daA[1] == 2);
		REQUIRE(daA[2] == 99);
		REQUIRE(daA[3] == 88);
		REQUIRE(daA[4] == 3);

		REQUIRE(daA.capacity() == 8);
		REQUIRE(daA.count() == 5);
	}

	SECTION("Insert element built in place")
	{
		DArray<TestNonCopyable> da = {TestNonCopyable(1), 
									  TestNonCopyable(2), 
									  TestNonCopyable(3)};

		REQUIRE(da[0].n == 1);
		REQUIRE(da[1].n == 2);
		REQUIRE(da[2].n == 3);

		REQUIRE(da.capacity() == 3);
		REQUIRE(da.count() == 3);

		da.insert({1337}, 1);

		REQUIRE(da[0].n == 1);
		REQUIRE(da[1].n == 1337);
		REQUIRE(da[2].n == 2);
		REQUIRE(da[3].n == 3);

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 4);

		da.insert(TestNonCopyable(111), 3);

		REQUIRE(da[0].n == 1);
		REQUIRE(da[1].n == 1337);
		REQUIRE(da[2].n == 2);
		REQUIRE(da[3].n == 111);

		REQUIRE(da.capacity() == 8);
		REQUIRE(da.count() == 5);
	}
}


TEST_CASE("Check element in DArray")
{
	DArray<int> da = {1, 2, 3, 4};
	const DArray<int> cda = {1, 2, 3, 4};

	SECTION("Find Forward")
	{
		int *firstPtr = da.find(1);
		int *middlePtr = da.find(2);
		int *lastPtr = da.find(4);

		REQUIRE(firstPtr == &da.front());
		REQUIRE(middlePtr == &da[1]);
		REQUIRE(lastPtr == &da.back());

		const int *constFirstPtr = cda.find(1);
		const int *constMiddlePtr = cda.find(2);
		const int *constLastPtr = cda.find(4);

		REQUIRE(constFirstPtr == &cda.front());
		REQUIRE(constMiddlePtr == &cda[1]);
		REQUIRE(constLastPtr == &cda.back());
	}

	SECTION("Find Backwards")
	{
		int *firstPtr = da.findBackwards(1);
		int *middlePtr = da.findBackwards(2);
		int *lastPtr = da.findBackwards(4);

		REQUIRE(firstPtr == &da.front());
		REQUIRE(middlePtr == &da[1]);
		REQUIRE(lastPtr == &da.back());

		const int *constFirstPtr = cda.findBackwards(1);
		const int *constMiddlePtr = cda.findBackwards(2);
		const int *constLastPtr = cda.findBackwards(4);

		REQUIRE(constFirstPtr == &cda.front());
		REQUIRE(constMiddlePtr == &cda[1]);
		REQUIRE(constLastPtr == &cda.back());
	}

	SECTION("Find non existent element")
	{
		int *missPtr = da.find(1337);
		REQUIRE(missPtr == da.end());

		missPtr = da.findBackwards(1337);
		REQUIRE(missPtr == da.end());
	}

	SECTION("Find element index")
	{
		size_t firstIndex = da.indexOf(1);
		size_t middleIndex = da.indexOf(2);
		size_t lastIndex = da.indexOf(4);

		REQUIRE(firstIndex == 0);
		REQUIRE(middleIndex == 1);
		REQUIRE(lastIndex == da.lastIndex());

		size_t firstBackwardsIndex = da.indexOfBackwards(1);
		size_t middleBackwardsIndex = da.indexOfBackwards(2);
		size_t lastBackwardsIndex = da.indexOfBackwards(4);

		REQUIRE(firstBackwardsIndex == 0);
		REQUIRE(middleBackwardsIndex == 1);
		REQUIRE(lastBackwardsIndex == da.lastIndex());
	}

	DArray<TestNonCopyable> arrayOfNonCopyables {{1}, {2}, {3}};
	TestNonCopyable element(3);
	TestNonCopyable NonElement(111);

	SECTION("Find Forwrd element withouth any copy")
	{
		const TestNonCopyable *elementPtrA = arrayOfNonCopyables.find(TestNonCopyable(2));
		const TestNonCopyable *elementPtrB = arrayOfNonCopyables.find(element);
		REQUIRE(elementPtrA == &arrayOfNonCopyables[1]);
		REQUIRE(elementPtrB == &arrayOfNonCopyables[2]);
	}

	SECTION("Find Backwrds element withouth any copy")
	{
		const TestNonCopyable *elementPtrA = arrayOfNonCopyables.findBackwards(TestNonCopyable(2));
		const TestNonCopyable *elementPtrB = arrayOfNonCopyables.findBackwards(element);
		REQUIRE(elementPtrA == &arrayOfNonCopyables[1]);
		REQUIRE(elementPtrB == &arrayOfNonCopyables[2]);
	}

	SECTION("Check if element is in array")
	{
		REQUIRE(arrayOfNonCopyables.contains(TestNonCopyable(2)));
		REQUIRE(arrayOfNonCopyables.contains(element));

		REQUIRE_FALSE(arrayOfNonCopyables.contains(TestNonCopyable(111)));
		REQUIRE_FALSE(arrayOfNonCopyables.contains(NonElement));
	}

	SECTION("Check if element is in array backwards")
	{
		REQUIRE(arrayOfNonCopyables.containsBackwards(TestNonCopyable(2)));
		REQUIRE(arrayOfNonCopyables.containsBackwards(element));

		REQUIRE_FALSE(arrayOfNonCopyables.containsBackwards(TestNonCopyable(111)));
		REQUIRE_FALSE(arrayOfNonCopyables.containsBackwards(NonElement));
	}
}

TEST_CASE("Swap Pop Remove elements", k_tag)
{
	DArray<int> da = {1, 2, 3, 4};

	SECTION("Remove first")
	{
		da.swapPop(1);

		REQUIRE(da[0] == 4);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 3);
	}

	SECTION("Remove middle")
	{
		da.swapPop(2);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 4);
		REQUIRE(da[2] == 3);

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 3);
	}

	SECTION("Remove last")
	{
		da.swapPop(4);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	SECTION("Remove the only element")
	{
		DArray<int> singleDa = {1};
		singleDa.swapPop(1);
	}

	SECTION("Remove return value")
	{
		REQUIRE(da.swapPop(4));
		REQUIRE_FALSE(da.swapPop(111));

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 3);
	}

	SECTION("Remove first index")
	{
		da.swapPopIndex(0);

		REQUIRE(da[0] == 4);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	SECTION("Remove middle index")
	{
		da.swapPopIndex(2);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 4);
	}

	SECTION("Remove last index")
	{
		da.swapPopIndex(3);

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);
	}

	DArray<TestNonCopyable> arrayOfNonCopyables {{1}, {2}, {3}};
	TestNonCopyable element(3);

	SECTION("Remove without unnecessary copy")
	{
		REQUIRE(arrayOfNonCopyables.swapPop(TestNonCopyable(1)));
		REQUIRE(arrayOfNonCopyables[0].n == 3);
		REQUIRE(arrayOfNonCopyables[1].n == 2);

		REQUIRE(arrayOfNonCopyables.count() == 2);

		REQUIRE(arrayOfNonCopyables.swapPop(element));
		REQUIRE(arrayOfNonCopyables[0].n == 2);

		REQUIRE(arrayOfNonCopyables.count() == 1);

		REQUIRE(arrayOfNonCopyables.capacity() == 3);
	}
}


TEST_CASE("Remove elements", k_tag)
{
	DArray<int> da = {1, 2, 3, 4};

	SECTION("Remove first element")
	{
		REQUIRE(da.remove(1));

		REQUIRE(da[0] == 2);
		REQUIRE(da[1] == 3);
		REQUIRE(da[2] == 4);

		REQUIRE(da.count() == 3);
		REQUIRE(da.capacity() == 4);
	}

	SECTION("Remove middle element")
	{
		REQUIRE(da.remove(3));

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 4);

		REQUIRE(da.count() == 3);
		REQUIRE(da.capacity() == 4);
	}

	SECTION("Remove last element")
	{
		REQUIRE(da.remove(4));

		REQUIRE(da[0] == 1);
		REQUIRE(da[1] == 2);
		REQUIRE(da[2] == 3);

		REQUIRE(da.count() == 3);
		REQUIRE(da.capacity() == 4);
	}

	SECTION("Remove return value")
	{
		REQUIRE(da.remove(4));
		REQUIRE_FALSE(da.remove(111));

		REQUIRE(da.capacity() == 4);
		REQUIRE(da.count() == 3);
	}

	DArray<TestNonCopyable> arrayOfNonCopyables {{1}, {2}, {3}};
	TestNonCopyable element(3);

	SECTION("Remove without unnecessary copy")
	{
		REQUIRE(arrayOfNonCopyables.remove(TestNonCopyable(1)));
		REQUIRE(arrayOfNonCopyables[0].n == 2);
		REQUIRE(arrayOfNonCopyables[1].n == 3);

		REQUIRE(arrayOfNonCopyables.count() == 2);

		REQUIRE(arrayOfNonCopyables.remove(element));
		REQUIRE(arrayOfNonCopyables[0].n == 2);

		REQUIRE(arrayOfNonCopyables.count() == 1);

		REQUIRE(arrayOfNonCopyables.capacity() == 3);
	}
}


TEST_CASE("DArray Stream Operators")
{
	SECTION("Stream In")
	{
		DArray<i32> testArray;

		std::istringstream is("1 2 3 4");
		is >> testArray;

		for (int i = 0; i < testArray.count(); i++)
			REQUIRE(testArray[i] == i+1);

		REQUIRE(testArray.count() == 4);
		REQUIRE(testArray.capacity() == 4);
	}

	SECTION("Stream Out")
	{
		const DArray<i32> testArray = {1, 2, 3, 4};
		std::stringstream ss;
		ss << testArray;

		REQUIRE(ss.str() == "[1, 2, 3, 4]");
	}
}
