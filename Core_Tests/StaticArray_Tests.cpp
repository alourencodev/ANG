#include <catch.hpp>

#include <Core/StaticArray.hpp>

TEST_CASE("StaticaArray Constructors")
{
	// Default Constructor
	StaticArray<int, 2> a1;
	a1 = {0, 0}; //Surpress warning

	SECTION("Initialize from list")
	{
		StaticArray<int, 4> a2 = {1, 2, 3, 4};

		REQUIRE(a2[0] == 1);
		REQUIRE(a2[1] == 2);
		REQUIRE(a2[2] == 3);
		REQUIRE(a2[3] == 4);
	}
}