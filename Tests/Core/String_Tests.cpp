#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/String.hpp>

using namespace age;

constexpr static char k_tag[] = "[String]";

TEST_CASE("String Constructors", k_tag)
{
	SECTION("Default constructor")
	{
		String str;
		REQUIRE(str.isEmpty());
	}

}
