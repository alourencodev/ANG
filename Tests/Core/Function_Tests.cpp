#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/Function.hpp>


using namespace age;

constexpr char k_tag[] = "[Function]";

bool testFunction(bool value) { return value; }

TEST_CASE("Function Constructor", k_tag)
{
	Function<bool(bool)> test(testFunction);

	REQUIRE(test(true));
	REQUIRE_FALSE(test(false));
}

TEST_CASE("Function assignment operator", k_tag)
{
	Function<bool()> test;
	
	test = []() -> bool { return true; };
	REQUIRE(test());
	
	test = []() -> bool { return false; };
	REQUIRE_FALSE(test());
}