#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/String.hpp>
#include <Core/StringBuilder.hpp>

using namespace age;

constexpr static char k_tag[] = "[String]";

TEST_CASE("String Constructors", k_tag)
{
	SECTION("Default constructor")
	{
		String str;
		REQUIRE(str.isEmpty());
	}

	SECTION("C Style string constructor")
	{
		String str("YA String");
		REQUIRE_FALSE(str.isEmpty());
		REQUIRE(str.size() == 9);
	}
}



TEST_CASE("String Boolean Operators", k_tag)
{
	SECTION("Comparison with empty string")
	{
		String str;
		REQUIRE(str != "other");
		REQUIRE_FALSE(str == "other");
		REQUIRE(str == "");
		REQUIRE_FALSE(str != "");
		REQUIRE(str != "1");
		REQUIRE_FALSE(str == "1");
	}

	SECTION("Comparison with string")
	{
		String str("YA String");
		String other("yA String");
		String other2("YA String ");
		String other3("YA String");
		String other4("YA Strin");

		REQUIRE(str != other);
		REQUIRE_FALSE(str == other);
		REQUIRE(str != other2);
		REQUIRE_FALSE(str == other2);
		REQUIRE(str == other3);
		REQUIRE_FALSE(str != other3);
		REQUIRE(str != other4);
		REQUIRE_FALSE(str == other4);

		REQUIRE(str == "YA String");
		REQUIRE_FALSE(str != "YA String");
		REQUIRE(str != "YA String ");
		REQUIRE_FALSE(str == "YA String ");
		REQUIRE(str != "yA String");
		REQUIRE_FALSE(str == "yA String");
	}
}



TEST_CASE("String Assign Operators", k_tag)
{
	String other("YA StrinG");
	String other2("YA Strin");
	String other3("Other String");

	String str;
	str = "YA String";

	REQUIRE(str == "YA String");
	REQUIRE_FALSE(str != "YA String");
	REQUIRE(str.size() == 9);
	REQUIRE(str.size() == strSize(str));

	str = other;
	REQUIRE(str == "YA StrinG");
	REQUIRE_FALSE(str != "YA StrinG");
	REQUIRE(str.size() == 9);
	REQUIRE(str.size() == strSize(str));

	str = other2;
	REQUIRE(str == "YA Strin");
	REQUIRE_FALSE(str != "YA Strin");
	REQUIRE(str.size() == 8);
	REQUIRE(str.size() == strSize(str));

	str = std::move(other3);
	REQUIRE(str == "Other String");
	REQUIRE_FALSE(str != "Other String");
	REQUIRE(str.size() == 12);
	REQUIRE(str.size() == strSize(str));
}

