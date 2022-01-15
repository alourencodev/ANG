#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/StringBuilder.hpp>

using namespace age;

constexpr char k_tag[] = "[StringBuilder]";


TEST_CASE("Build String With StringBuilder", k_tag)
{
	String testString = "str1";
	const char anotherString[] = "str2";
	const char str[] = "str3";
	ConstStringView view(str);

	StringBuilder builder;
	builder.append(testString);
	builder.append(anotherString);
	builder.append(view);
	builder.append("str4");
	
	String result = builder.build();

	REQUIRE(result == "str1str2str3str4");
}

