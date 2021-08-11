#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/Preprocessor/Utils.h>

#include <string>



constexpr static char k_tag[] = "[Preprocessor Utils]";

TEST_CASE("Preprocessor String Conversion", k_tag)
{
	std::string str = PP_STR(TestString);
	REQUIRE(str == "TestString");
}

TEST_CASE("Preprocessor Concatenation", k_tag)
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

