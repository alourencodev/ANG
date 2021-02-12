#include <catch.hpp>

#include <Core/Preprocessor/Utils.h>

#include <string>

TEST_CASE("Preprocessor String Conversion")
{
	std::string str = PP_STR(TestString);
	REQUIRE(str == "TestString");
}

TEST_CASE("Preprocessor Concatenation")
{
	SECTION("Normal Concatenation")
	{
		int PP_CAT(my, Variable) = 1;
		REQUIRE(myVariable == 1);
	}

	SECTION("Converting Concatenation to string")
	{
		std::string str = PP_STR(PP_CAT(Test, String));
		REQUIRE(str == "TestString");
	}
}

