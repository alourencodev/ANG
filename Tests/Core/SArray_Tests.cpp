#include <Tests/Vendor/Catch2/catch.hpp>

#include <sstream>

#include <Core/SArray.hpp>


constexpr static char k_tag[] = "[SArray]";

TEST_CASE("StaticaArray Constructors", k_tag)
{
	// Default Constructor
	SECTION ("Default Cnstructor")
	{
		SArray<i32, 2> a1;
		a1 = {0, 0}; //Surpress warning
	}

	SECTION("Initialize from list")
	{
		SArray<i32, 3> a2 = {1, 2, 3};

		REQUIRE(a2[0] == 1);
		REQUIRE(a2[1] == 2);
		REQUIRE(a2[2] == 3);
	}

	SECTION("Initialize with filling value")
	{
		SArray <i32, 3> a3(100);

		REQUIRE(a3[0] == 100);
		REQUIRE(a3[1] == 100);
		REQUIRE(a3[2] == 100);
	}
}

TEST_CASE("StaticArray Iteration", k_tag)
{
	SArray<i32, 4> testArray = {0, 1, 2, 3};

	SECTION("Iterate by index")
	{
		for (i32 i = 0; i < testArray.size(); i++)
			REQUIRE(testArray[i] == i);
	}

	SECTION("Iterate for each")
	{
		i32 counter = 0;
		for(i32 value : testArray)
			REQUIRE(value == counter++);
	}

	SECTION("Change value in iterator for loop")
	{
		for (auto &value : testArray)
			value = 999;

		for (i32 value : testArray)
			REQUIRE(value == 999);
	}
}

TEST_CASE("Static Array type cast operators", k_tag)
{
	SECTION("Cast to Range")
	{
		SArray<int, 4> sa = {1, 2, 3, 4};
		Range<int> range = sa;
	}

	SECTION("Cast to const Range")
	{
		const SArray<int, 4> sa = {1, 2, 3, 4};
		Range<const int> range = sa;
	}
}

class NoCopiable
{
public:
	NoCopiable() = default;
	NoCopiable(int n) : num(n) {}
	NoCopiable(const NoCopiable &) = delete;

	int num;
};

TEST_CASE("StaticArray set", k_tag)
{
	SECTION("Set from constructor")
	{
		SArray<NoCopiable, 3> testArray;
		
		testArray[0] = NoCopiable(1337);
		REQUIRE(testArray[0].num == 1337);
	}
}

TEST_CASE("StaticArray Iterator Access", k_tag)
{
	SArray<i32, 4> testArray = {0, 1, 2, 3};

	REQUIRE(*testArray.begin() == 0);
	REQUIRE(*(testArray.end() - 1) == 3);

	const SArray<i32, 4> constTestArray = {0, 1, 2, 3};

	REQUIRE(*constTestArray.begin() == 0);
	REQUIRE(*(constTestArray.end()- 1) == 3);
}

TEST_CASE("StaticArray Access", k_tag)
{
	SArray<i32, 4> testArray = {0, 1, 2, 3};

	REQUIRE(testArray.front() == 0);
	REQUIRE(testArray.back() == 3);

	const SArray<i32, 4> constTestArray = {0, 1, 2, 3};

	REQUIRE(constTestArray.front() == 0);
	REQUIRE(constTestArray.back() == 3);

	// Access by square brackets is already implicitly tested in the other test cases
}

TEST_CASE("StaticArray Query", k_tag)
{
	const SArray<i32, 4> testArray = {1, 2, 3, 4};
	
	SECTION("Find elements")
	{
		REQUIRE(testArray.find(2) == &testArray[1]);
		REQUIRE(testArray.find(1) == &testArray[0]);
		REQUIRE(testArray.find(4) == &testArray[3]);
		REQUIRE(testArray.find(8) == testArray.end());

		REQUIRE(testArray.findBackwards(2) == &testArray[1]);
		REQUIRE(testArray.findBackwards(1) == &testArray[0]);
		REQUIRE(testArray.findBackwards(4) == &testArray[3]);
		REQUIRE(testArray.findBackwards(8) == testArray.end());
	}

	SECTION("Contains elements")
	{
		REQUIRE(testArray.contains(2));
		REQUIRE(testArray.contains(1));
		REQUIRE(testArray.contains(3));
		REQUIRE_FALSE(testArray.contains(8));

		REQUIRE(testArray.containsBackwards(2));
		REQUIRE(testArray.containsBackwards(1));
		REQUIRE(testArray.containsBackwards(3));
		REQUIRE_FALSE(testArray.containsBackwards(8));
	}
}

TEST_CASE("StaticArray Compile Time Asserts", k_tag)
{
	SArray<i32, 4> testArray = {1, 2, 3, 4};
	static_assert(testArray.size() == 4);
	static_assert(testArray.lastIndex() == 3);
}

TEST_CASE("StaticArray Stream Operators", k_tag)
{
	SECTION("Stream In")
	{
		SArray<i32, 4> testArray;
		std::istringstream is("1 2 3 4");
		is >> testArray;

		for (int i = 0; i < testArray.size(); i++)
			REQUIRE(testArray[i] == i+1);
	}

	SECTION("Stream Out")
	{
		const SArray<i32, 4> testArray = {1, 2, 3, 4};
		std::stringstream ss;
		ss << testArray;

		REQUIRE(ss.str() == "[1, 2, 3, 4]");
	}
}

TEST_CASE("StaticArray Copy", k_tag)
{
	SArray<i32, 4> original = {1, 2, 3, 4};
	SArray<i32, 4> copied = original;

	REQUIRE(original.data() != copied.data());

	for (int i = 0; i < original.size(); i++)
		REQUIRE(copied[i] == original[i]);
}
