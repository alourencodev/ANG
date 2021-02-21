#include <catch.hpp>

#include <Core/StaticArray.hpp>


TEST_CASE("StaticaArray Constructors")
{
	// Default Constructor
	SECTION ("Default Cnstructor")
	{
		StaticArray<i32, 2> a1;
		a1 = {0, 0}; //Surpress warning
	}

	SECTION("Initialize from list")
	{
		StaticArray<i32, 3> a2 = {1, 2, 3};

		REQUIRE(a2[0] == 1);
		REQUIRE(a2[1] == 2);
		REQUIRE(a2[2] == 3);
	}

	SECTION("Initialize with filling value")
	{
		StaticArray <i32, 3> a3(100);

		REQUIRE(a3[0] == 100);
		REQUIRE(a3[1] == 100);
		REQUIRE(a3[2] == 100);
	}
}

TEST_CASE("StaticArray Iteration")
{
	StaticArray<i32, 4> testArray = {0, 1, 2, 3};

	SECTION("Iterate by index")
	{
		for (i32 i = 0; i < testArray.size; i++)
			REQUIRE(testArray[i] == i);
	}

	SECTION("Iterate for each")
	{
		i32 counter = 0;
		for(i32 value : testArray)
			REQUIRE(value == counter++);
	}

	// TODO: Iterate backwards
}

TEST_CASE("StaticArray Iterators")
{
	StaticArray<i32, 4> testArray = {0, 1, 2, 3};

	REQUIRE(*testArray.begin() == 0);
	REQUIRE(*testArray.end() == 3);

	const StaticArray<i32, 4> constTestArray = {0, 1, 2, 3};

	REQUIRE(*constTestArray.begin() == 0);
	REQUIRE(*constTestArray.end() == 3);
}

TEST_CASE("StaticArray Access")
{
	StaticArray<i32, 4> testArray = {0, 1, 2, 3};

	REQUIRE(testArray.front() == 0);
	REQUIRE(testArray.back() == 3);

	// Access by square brackets is already implicitly tested in the other test cases
}

TEST_CASE("StaticArray Compile Time Asserts")
{
	StaticArray<i32, 4> testArray = {1, 2, 3, 4};
	static_assert(testArray.size == 4);
}
