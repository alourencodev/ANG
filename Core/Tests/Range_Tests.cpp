#include <catch.hpp>

#include <sstream>

#include <Core/Range.hpp>

constexpr static char k_tag[] = "[Range]";

TEST_CASE("Range Constructor", k_tag)
{
	const int a[4] = {1, 2, 3, 4};
	const Range range(a, 4);

	REQUIRE(range[0] == a[0]);
	REQUIRE(range[1] == a[1]);
	REQUIRE(range[2] == a[2]);
	REQUIRE(range[3] == a[3]);

	REQUIRE(range.lastIndex() == 3);
}

TEST_CASE("Range Iteration", k_tag)
{
	const int a[4] = {1, 2, 3, 4};
	const Range range(a, 4);

	for(int i : range) 
		REQUIRE(range[i - 1] == (i));
}

TEST_CASE("Range Query", k_tag)
{
	const int a[4] = {1, 2, 3, 4};
	const Range range(a, 4);

	SECTION("Find elements")
	{
		REQUIRE(range.find(2) == &range[1]);
		REQUIRE(range.find(1) == &range[0]);
		REQUIRE(range.find(4) == &range[3]);
		REQUIRE(range.find(8) == range.end());

		REQUIRE(range.findBackwards(2) == &range[1]);
		REQUIRE(range.findBackwards(1) == &range[0]);
		REQUIRE(range.findBackwards(4) == &range[3]);
		REQUIRE(range.findBackwards(8) == range.end());
	}

	SECTION("Contains elements")
	{
		REQUIRE(range.contains(2));
		REQUIRE(range.contains(1));
		REQUIRE(range.contains(3));
		REQUIRE_FALSE(range.contains(8));

		REQUIRE(range.containsBackwards(2));
		REQUIRE(range.containsBackwards(1));
		REQUIRE(range.containsBackwards(3));
		REQUIRE_FALSE(range.containsBackwards(8));
	}
}

TEST_CASE("Range Stream Operators", k_tag)
{
	SECTION("Stream In")
	{
		int a[4] = {0, 0, 0, 0};
		Range range(a, 4);

		std::istringstream is("1 2 3 4");
		is >> range;

		for (int i = 0; i < range.count(); i++)
			REQUIRE(range[i] == i+1);
	}

	SECTION("Stream Out")
	{
		const int a[4] = {1, 2, 3, 4};
		const Range range(a, 4);

		std::stringstream ss;
		ss << range;

		REQUIRE(ss.str() == "[1, 2, 3, 4]");
	}
}