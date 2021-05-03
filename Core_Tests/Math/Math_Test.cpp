#include <catch.hpp>

#include <Core/Math/Math.hpp>


using namespace math;

constexpr static char k_tag[] = "[Math]";


TEST_CASE("Linear Interpolations", k_tag)
{
	SECTION("Normal Interpolation")
	{
		REQUIRE(g_lerp(1, 2, 0.5f) == 1.5f);
		REQUIRE(g_lerp(0, 10, 0.75) == 7.5);
		REQUIRE(g_lerp(0, 10, 1) == 10.0f);
		REQUIRE(g_lerp(0, 10, 0) == 0.0f);
	}

	SECTION("Linear Extrapolation")
	{
		REQUIRE(g_lerp(0, 10, 2) == 20.0f);
		REQUIRE(g_lerp(0, 10, -1) == -10.f);
	}
}


TEST_CASE("Next Power of 2", k_tag)
{
	REQUIRE(g_nextPow2(0) == 2);		// n=0 edge case
	REQUIRE(g_nextPow2(1) == 2);
	REQUIRE(g_nextPow2(2) == 2);		// Does it work 2 itself
	REQUIRE(g_nextPow2(3) == 4);
	REQUIRE(g_nextPow2(6) == 8);
	REQUIRE(g_nextPow2(16) == 16);	// Does it work in a power of 2 different that 2
	REQUIRE(g_nextPow2(25) == 32);
}


TEST_CASE("Absolute value")
{
	SECTION("Integral Absolute Value")
	{
		REQUIRE(math::g_abs(1) == 1);
		REQUIRE(math::g_abs(-1) == 1);
		REQUIRE(math::g_abs(0) == 0);
	}

	SECTION("Float Absolute Value")
	{
		REQUIRE(math::g_fabs(1.0f) == Approx(1.0f));
		REQUIRE(math::g_fabs(-1.0f) == Approx(1.0f));
		REQUIRE(math::g_fabs(0.0f) == Approx(0.0f));
	}
}

TEST_CASE("Check if floats are approximated")
{
	REQUIRE(math::g_approx(0.0f, 0.0f));
	REQUIRE(math::g_approx(1.0f, 1.0f));
	REQUIRE(math::g_approx(-1.0f, -1.0f));
	REQUIRE(math::g_approx(1.0f, 1.000001f));

	REQUIRE_FALSE(math::g_approx(-1.0f, 1.0f));
	REQUIRE_FALSE(math::g_approx(0.0f, 1.0f));
	REQUIRE_FALSE(math::g_approx(1.0f, 1.0001f));
}
