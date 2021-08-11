#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/StackArray.hpp>
#include <Core/DArray.hpp>


using namespace age;

constexpr static char k_tag[] = "[StackArray]";

TEST_CASE("StackArray Constructor", k_tag)
{
	SECTION("Default Constructor")
	{
		StackArray<int, 8> a;
	
		REQUIRE(a.count() == 0);
		REQUIRE(a.capacity() == 8);
	}

	SECTION("Range contructor")
	{
		DArray<int> da = {1, 2, 3};
		StackArray<int, 8> sa(da);

		REQUIRE(sa.count() == 3);
		REQUIRE(sa.capacity() == 8);

		REQUIRE(sa[0] == 1);
		REQUIRE(sa[1] == 2);
		REQUIRE(sa[2] == 3);
	}

	SECTION("Initializer list constructor")
	{
		StackArray<int, 8> a = {1, 2, 3, 4};

		REQUIRE(a.count() == 4);
		REQUIRE(a.capacity() == 8);

		REQUIRE(a[0] == 1);
		REQUIRE(a[1] == 2);
		REQUIRE(a[2] == 3);
		REQUIRE(a[3] == 4);
	}

	SECTION("Copy Constructor ")
	{
		StackArray<int, 8> a1 = {1, 2, 3, 4};
		StackArray<int, 16> a2(a1);

		REQUIRE(a2.count() == 4);
		REQUIRE(a2.capacity() == 16);

		REQUIRE(a2[0] == 1);
		REQUIRE(a2[1] == 2);
		REQUIRE(a2[2] == 3);
		REQUIRE(a2[3] == 4);
	}
}

TEST_CASE("StackArray add elements", k_tag)
{
	StackArray<int, 8> a = {1, 2, 3, 4};

	REQUIRE(a.count() == 4);
	REQUIRE(a.capacity() == 8);

	REQUIRE(a[0] == 1);
	REQUIRE(a[1] == 2);
	REQUIRE(a[2] == 3);
	REQUIRE(a[3] == 4);

	a.add({5, 6, 7});

	REQUIRE(a.count() == 7);
	REQUIRE(a.capacity() == 8);

	REQUIRE(a[0] == 1);
	REQUIRE(a[1] == 2);
	REQUIRE(a[2] == 3);
	REQUIRE(a[3] == 4);
	REQUIRE(a[4] == 5);
	REQUIRE(a[5] == 6);
	REQUIRE(a[6] == 7);

	a.add(101);

	REQUIRE(a[a.lastIndex()] == 101);
}

TEST_CASE("StackArray insert element", k_tag)
{
	StackArray<int, 8> a = {1, 2, 3, 4};

	REQUIRE(a.count() == 4);
	REQUIRE(a.capacity() == 8);

	REQUIRE(a[0] == 1);
	REQUIRE(a[1] == 2);
	REQUIRE(a[2] == 3);
	REQUIRE(a[3] == 4);

	a.insert({5, 6, 7}, 2);

	REQUIRE(a[0] == 1);
	REQUIRE(a[1] == 2);
	REQUIRE(a[2] == 5);
	REQUIRE(a[3] == 6);
	REQUIRE(a[4] == 7);
	REQUIRE(a[5] == 3);
	REQUIRE(a[6] == 4);

	REQUIRE(a.count() == 7);
	REQUIRE(a.capacity() == 8);
}

TEST_CASE("StackArray remove elements", k_tag)
{

}

