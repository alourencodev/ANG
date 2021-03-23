#include <catch.hpp>

#include <Core/Math/Math.hpp>


constexpr static char k_tag[] = "[Math]";

TEST_CASE("Linear Interpolations", k_tag)
{
	SECTION("Normal Interpolation")
	{
		REQUIRE(lerp(1, 2, 0.5f) == 1.5f);
		REQUIRE(lerp(0, 10, 0.75) == 7.5);
		REQUIRE(lerp(0, 10, 1) == 10.0f);
		REQUIRE(lerp(0, 10, 0) == 0.0f);
	}

	SECTION("Linear Extrapolation")
	{
		REQUIRE(lerp(0, 10, 2) == 20.0f);
		REQUIRE(lerp(0, 10, -1) == -10.f);
	}
}

