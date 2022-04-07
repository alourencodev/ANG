#include <Tests/Vendor/Catch2/catch.hpp>


#include <Core/HashMap.hpp>
#include <Core/String.hpp>


using namespace age;

constexpr static char k_tag[] = "[HashMap]";

TEST_CASE("Hash Map Constructor", k_tag)
{
	SECTION("Default Constructor")
	{
		HashMap<String, int> map;
		
		REQUIRE(map.capacity() == 0);
		REQUIRE(map.count() == 0);
		REQUIRE(map.isEmpty());
	}

	SECTION("Capacity Constructor")
	{
		HashMap<String, int> map(4);
		
		REQUIRE(map.capacity() == 8);
		REQUIRE(map.count() == 0);
		REQUIRE(map.isEmpty());

		HashMap<String, int> map2(10);
		REQUIRE(map2.capacity() == 16);
		REQUIRE(map2.count() == 0);
		REQUIRE(map2.isEmpty());
	}

	SECTION("Initializer List Constructor")
	{
		HashMap<String, int> map = {{"aba", 1}, {"baba", 1}, {"other", 3}};

		REQUIRE(map.capacity() == 8);
		REQUIRE(map.count() == 3);
		REQUIRE_FALSE(map.isEmpty());

		REQUIRE(map["aba"] == 1);
		REQUIRE(map["baba"] == 1);
		REQUIRE(map["other"] == 3);

		REQUIRE(map.contains("aba"));
		REQUIRE_FALSE(map.contains("1337"));
	}
}


TEST_CASE("HashMap Resize", k_tag)
{
	HashMap<String, int> map = {{"aba", 1}, {"baba", 1}, {"other", 3}};
	map.add("first", 4);
	map.add("second", 5);

	REQUIRE(map.capacity() == 8);
	REQUIRE(map.count() == 5);

	map.add("resize", 6);

	REQUIRE(map.capacity() == 16);
	REQUIRE(map.count() == 6);

	REQUIRE(map["aba"] == 1);
	REQUIRE(map["baba"] == 1);
	REQUIRE(map["other"] == 3);
	REQUIRE(map["resize"] == 6);
}

TEST_CASE("HashMap Clear", k_tag)
{
	HashMap<String, int> map = {{"aba", 1}, {"baba", 1}, {"other", 3}};
	map.clear();

	REQUIRE(map.capacity() == 8);
	REQUIRE(map.count() == 0);
}

// TODO: Test move and copy constructors
