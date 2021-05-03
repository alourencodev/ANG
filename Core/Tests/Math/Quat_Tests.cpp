#include <catch.hpp>

#include <cmath>
#include <sstream>

#include <Core/Math/Math.hpp>
#include <Core/Math/Quat.hpp>


constexpr static char k_tag[] = "[Quat]";

TEST_CASE("Quaternion constructors", k_tag)
{
	SECTION("Default Constructor")
	{
		quat q;

		REQUIRE(q.w == 0.0f);
		REQUIRE(q.x == 0.0f);
		REQUIRE(q.y == 0.0f);
		REQUIRE(q.z == 0.0f);
	}

	SECTION("Single Float Constructor")
	{
		quat q(2.0f);

		REQUIRE(q.w == 2.0f);
		REQUIRE(q.x == 2.0f);
		REQUIRE(q.y == 2.0f);
		REQUIRE(q.z == 2.0f);
	}

	SECTION("Multiple Float Constructor")
	{
		quat q(1, 2, 3, 4);

		REQUIRE(q.w == 1.0f);
		REQUIRE(q.x == 2.0f);
		REQUIRE(q.y == 3.0f);
		REQUIRE(q.z == 4.0f);
	}

	SECTION("Angle and Axix Constructor")
	{
		vec3 axis(1, 2, 3);
		quat q(4, axis);

		REQUIRE(q.w == 4.0f);
		REQUIRE(q.x == 1.0f);
		REQUIRE(q.y == 2.0f);
		REQUIRE(q.z == 3.0f);
	}
}

TEST_CASE("Quaternion Element Access", k_tag)
{
	quat q(1, 2, 3, 4);
	vec3 v(2, 3, 4);

	// Access per element is already being done in other tests

	SECTION("Access Per Angle/Axis")
	{
		REQUIRE(q.angle == 1.0f);
		REQUIRE(q.axis == v);
	}

	SECTION("Access as Array")
	{
		REQUIRE(q[0] == 1.0f);
		REQUIRE(q[1] == 2.0f);
		REQUIRE(q[2] == 3.0f);
		REQUIRE(q[3] == 4.0f);
	}
}

TEST_CASE("Quaternion Arithmetic Operations", k_tag)
{
	quat q1(1, 2, 3, 4);
	quat q2(5, 6, 7, 8);

	SECTION("Sum Operator")
	{
		quat result = q1 + q2;

		REQUIRE(result.w == 6.0f);
		REQUIRE(result.x == 8.0f);
		REQUIRE(result.y == 10.0f);
		REQUIRE(result.z == 12.0f);
	}

	SECTION("Sub Operator")
	{
		quat result = q2 - q1;

		REQUIRE(result.w == 4.0f);
		REQUIRE(result.x == 4.0f);
		REQUIRE(result.y == 4.0f);
		REQUIRE(result.z == 4.0f);
	}

	SECTION("Quaternion Multiplication Operator")
	{
		quat result = q1 * q2;

		REQUIRE(result.w == -60.0f);
		REQUIRE(result.x == 12.0f);
		REQUIRE(result.y == 30.0f);
		REQUIRE(result.z == 24.0f); 
	}

	SECTION("Scalar Multiplication Operator")
	{
		quat result = 2 * q1;

		REQUIRE(result.w == 2.0f);
		REQUIRE(result.x == 4.0f);
		REQUIRE(result.y == 6.0f);
		REQUIRE(result.z == 8.0f);

		quat result2 = q1 * 2;
		REQUIRE(result == result2);
	}

	SECTION("Scalar Divisio Operator")
	{
		quat q3(2, 4, 6, 8);
		quat result = q3 / 2;

		REQUIRE(result.w == 1.0f);
		REQUIRE(result.x == 2.0f);
		REQUIRE(result.y == 3.0f);
		REQUIRE(result.z == 4.0f);
	}
}

TEST_CASE("Quaternion Boolean Operations", k_tag)
{
	quat q1(1, 2, 3, 4);
	quat q2(1, 2, 3, 4);
	quat q3(1, 2, 5, 5);

	REQUIRE(q1 == q2);
	REQUIRE(q2 != q3);
}

TEST_CASE("Quaternion to Matrix Conversion", k_tag)
{
	quat q(1, 2, 2, 4);
	mat4 result = q;

	REQUIRE(result.at(0,0) == Approx(-0.6f));
	REQUIRE(result.at(1,0) == Approx(0.64f));
	REQUIRE(result.at(2,0) == Approx(0.48f));
	REQUIRE(result.at(3,0) == Approx(0.0f));

	REQUIRE(result.at(0,1) == Approx(0.0f));
	REQUIRE(result.at(1,1) == Approx(-0.6f));
	REQUIRE(result.at(2,1) == Approx(0.8f));
	REQUIRE(result.at(3,1) == Approx(0.0f));

	REQUIRE(result.at(0,2) == Approx(0.8f));
	REQUIRE(result.at(1,2) == Approx(0.48f));
	REQUIRE(result.at(2,2) == Approx(0.36f));
	REQUIRE(result.at(3,2) == Approx(0.0f));

	REQUIRE(result.at(0,3) == Approx(0.0f));
	REQUIRE(result.at(1,3) == Approx(0.0f));
	REQUIRE(result.at(2,3) == Approx(0.0f));
	REQUIRE(result.at(3,3) == Approx(1.0f));
}

TEST_CASE("Quaternion Properties", k_tag)
{
	quat q(4, 4, 4, 4);

	SECTION("Quaternion Squared Length")
	{
		REQUIRE(sqrLength(q) == 64.0f);
	}

	SECTION("Quaternion Length")
	{
		REQUIRE(length(q) == 8.0f);
	}

	SECTION("Normalize")
	{
		quat normalized = normalize(q);
		float mag = length(normalized);

		REQUIRE(abs(mag) == Approx(1.0f));
	}
}

TEST_CASE("Quaternion Stream Operators", k_tag)
{
	SECTION("Stream In")
	{
		quat q;
		std::istringstream is("2 3 4 5");
		is >> q;

		REQUIRE(q.w == 2);
		REQUIRE(q.x == 3);
		REQUIRE(q.y == 4);
		REQUIRE(q.z == 5);
	}

	SECTION("Stream Out")
	{
		quat q(1, 2, 3, 4);
		std::stringstream ss;
		ss << q;

		REQUIRE(ss.str() == "(1, (2, 3, 4))");
	}
}

TEST_CASE("Quaternion Lerping Vector", k_tag)
{
	quat q1(1, -1, 2, 3);
	quat q2(2, -2, 4, 6);
	auto lerpedQuat = math::g_lerp(q1, q2, 0.5f);

	REQUIRE(lerpedQuat.w == 1.5f);
	REQUIRE(lerpedQuat.x == -1.5f);
	REQUIRE(lerpedQuat.y == 3.0f);
	REQUIRE(lerpedQuat.z == 4.5f);
}
