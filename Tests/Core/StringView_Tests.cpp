#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/StringView.hpp>

using namespace age;

constexpr char k_tag[] = "[StringView]";

TEST_CASE("StringView Contruct", k_tag)
{
	const char str[] = "Test String";
	ConstStringView constStringView(str);

	char str2[] = "Test String";
	StringView stringView(str2);
}

TEST_CASE("StringView move operations", k_tag)
{
	SECTION("Move constructor")
	{
		const char str[] = "Test String";

		ConstStringView srcView(str);
		ConstStringView dstView = std::move(srcView);

		REQUIRE(srcView.str() == nullptr);
		REQUIRE(dstView.str() == &(str[0]));
	}

	SECTION("Move operator")
	{
		const char dummy[] = "";
		ConstStringView dstView(dummy);
		
		const char str[] = "Test String";
		ConstStringView srcView(str);

		dstView = std::move(srcView);

		REQUIRE(srcView.str() == nullptr);
		REQUIRE(dstView.str() == &(str[0]));
	}
}

TEST_CASE("StringView Boolean operators", k_tag)
{
	const char strA[] = "Test String";
	const char strB[] = "Test String";
	const char strC[] = "Test Str";
	const char strD[] = "Test String Extra";
	const char strE[] = "Other";

	ConstStringView viewA(strA);
	ConstStringView viewB(strB);
	ConstStringView viewC(strC);
	ConstStringView viewD(strD);
	ConstStringView viewE(strE);

	SECTION("Equal operator")
	{
		// For some reason, catch is not working if we compare the views directly
		const bool isEqual = viewA == viewB;
		REQUIRE(isEqual);

		REQUIRE_FALSE(viewA == viewC);
		REQUIRE_FALSE(viewA == viewD);
		REQUIRE_FALSE(viewA == viewE);
	}

	SECTION("Different operator")
	{
		// For some reason, catch is not working if we compare the views directly
		const bool isDifferent = viewA != viewB;
		REQUIRE_FALSE(isDifferent);

		REQUIRE(viewA != viewC);
		REQUIRE(viewA != viewD);
		REQUIRE(viewA != viewE);
	}
}

TEST_CASE("StringView size calculation", k_tag)
{
	const char str[] = "Test String";
	REQUIRE(ConstStringView(str).calcSize() == 11);
}

TEST_CASE("String View substrings", k_tag)
{
	const char str[] = "SomeRandomFileName.file"; 
	ConstStringView view(str);

	char buffer[32];
	StringView bufferView(buffer);

	// Use a bool since catch messes up the comparisons
	view.getPrefix(buffer, 4);
	bool isEqual = bufferView == "Some";
	REQUIRE(isEqual);

	view.getSufix(buffer, 5);
	isEqual = bufferView == ".file";
	REQUIRE(isEqual);

	view.getSubString(buffer, 10, 4);
	isEqual = bufferView == "RandomFile";
	REQUIRE(isEqual);
}
