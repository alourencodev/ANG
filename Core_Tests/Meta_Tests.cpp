#include <catch.hpp>

#include <Core/Meta.hpp>

using namespace meta;

TEST_CASE("Templated constants")
{
	REQUIRE(vConst<int, 1>::value == 1);
	REQUIRE(vTrue::value);
	REQUIRE_FALSE(vFalse::value);
}

TEST_CASE("Templated Type comparison")
{
	REQUIRE(isSame<int, int>::value);
	REQUIRE_FALSE(isSame<int, float>::value);

	REQUIRE(areSame<int, int, int, int>::value);
	REQUIRE_FALSE(areSame<int, char, float, int>::value);
}