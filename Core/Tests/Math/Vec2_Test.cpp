#include <catch.hpp>

#include <cmath>
#include <sstream>

#include <Core/Math/Math.hpp>
#include <Core/Math/Vec2.hpp>
#include <Core/Math/Values.hpp>



constexpr static char k_tag[] = "[Vec2]";

TEST_CASE("2D Vector Constructors", k_tag)
{
	SECTION("Default Constructor")
	{
		vec2i vec;

		REQUIRE(vec.x == 0);
		REQUIRE(vec.y == 0);
	}

	SECTION("Single Parameter Constructor")
	{
		vec2i vec(1);

		REQUIRE(vec.x == 1);
		REQUIRE(vec.y == 1);
	}

	SECTION("Individual Parameter Constructor")
	{
		vec2i vec(2, 3);

		REQUIRE(vec.x == 2);
		REQUIRE(vec.y == 3);
	}
}

TEST_CASE("2D Vector Alternative Names Access", k_tag)
{
	vec2i vec(1, 2);

	SECTION("Access as Size")
	{
		REQUIRE(vec.w == 1);
		REQUIRE(vec.h == 2);
	}

	SECTION("Access as Array")
	{
		REQUIRE(vec[0] == 1);
		REQUIRE(vec[1] == 2);
	}

	SECTION("Assignment as Array")
	{
		vec[0] = 3;
		vec[1] = 4;

		REQUIRE(vec[0] == 3);
		REQUIRE(vec[1] == 4);
	}
}

TEST_CASE("2D Vector Properties", k_tag)
{
	vec2i vec(9, 12);

	SECTION("Vector Length")
	{
		REQUIRE(length(vec) == 15.0f);
	}

	SECTION("Vector Squared Length")
	{
		REQUIRE(sqrLength(vec) == 225.0f);
	}
}

TEST_CASE("2D Vector General Arithmetic Operators", k_tag)
{
	vec2i vecA(1, 2);
	vec2i vecB(3, 4);
	vec2 vecC(5.0f, 6.0f);

	SECTION("Assign Operator")
	{
		vec2i testVec = vecA;

		REQUIRE(testVec.x == 1);
		REQUIRE(testVec.y == 2);
	}

	SECTION("Sum Operator")
	{
		vec2i sum = vecA + vecB;
		
		REQUIRE(sum.x == 4);
		REQUIRE(sum.y == 6);
	}

	SECTION("Cross type sum operator")
	{
		auto sumA = vecA + vecC;
		auto sumB = vecC + vecA;

		REQUIRE(typeid(sumA) == typeid(vec2));
		REQUIRE(typeid(sumB) == typeid(vec2));

		REQUIRE(sumB.x == 6.0f);
		REQUIRE(sumB.y == 8.0f);
	}

	SECTION("Subtraction Operator")
	{
		vec2i sub = vecB - vecA;

		REQUIRE(sub.x == 2);
		REQUIRE(sub.y == 2);
	}

	SECTION("Cross type sub operator")
	{
		auto subA = vecC - vecA;
		auto subB = vecA - vecC;

		REQUIRE(typeid(subA) == typeid(vec2));
		REQUIRE(typeid(subB) == typeid(vec2));
		REQUIRE(subA.x == 4.0f);
		REQUIRE(subA.y == 4.0f);
	}

	SECTION("Scalar Multiplication")
	{
		vec2i mul = vecA * 2;

		REQUIRE(mul.x == 2);
		REQUIRE(mul.y == 4);
	}

	SECTION("Scalar Multiplication Commutation")
	{
		vec2i mul = 2 * vecA;

		REQUIRE(mul.x == 2);
		REQUIRE(mul.y == 4);
	}

	SECTION("Cross type multiplication")
	{
		auto mulA = vecA * 2.0f;
		auto mulB = 2.0f * vecA;

		REQUIRE(typeid(mulA) == typeid(vec2));
		REQUIRE(typeid(mulB) == typeid(vec2));
		REQUIRE(mulA.x == 2.0f);
		REQUIRE(mulA.y == 4.0f);
	}

	SECTION("Scalar Division")
	{
		vec2i testVec(2, 4);
		auto div = testVec / 2;

		REQUIRE(div.x == 1);
		REQUIRE(div.y == 2);
	}

	SECTION("Scalar multiplication must generate a vector of the type of the scalar")
	{
		auto floatVec = vecA * 0.5f;

		REQUIRE(typeid(floatVec) == typeid(vector2<float>));
	}

	SECTION("Scalar division must generate a float vector")
	{
		auto floatVec = vecB / 2;

		REQUIRE(typeid(floatVec) == typeid(vector2<float>));
	}
}

TEST_CASE("2D Vector Vectorial Operators", k_tag)
{
	vec2 vecA(1.0f, 2.0f);
	vec2 vecB(3.0f, 4.0f);

	SECTION("Dot Product")
	{
		float result = dot(vecA, vecB);

		REQUIRE(result == 11);
	}

	SECTION("Normalize")
	{
		vec2 normalized = normalize(vecA);
		float mag = length(normalized);

		REQUIRE((abs(mag) - 1.0f) < k_epsilon);
	}
}

TEST_CASE("2D Vector Boolean Operators", k_tag)
{
	vec2i vecA(1, 2);
	vec2i vecB(1, 2);
	vec2i vecC(2, 1);

	SECTION("Equals")
	{
		REQUIRE(vecA == vecB);
		REQUIRE_FALSE(vecA == vecC);
	}

	SECTION("Different")
	{
		REQUIRE(vecA != vecC);
		REQUIRE_FALSE(vecA != vecB);
	}
}

TEST_CASE("2D Vector Unary Operators", k_tag)
{
	vec2i vecA(1, 2);

	SECTION("Negative")
	{
		vec2i minusVec = -vecA;

		REQUIRE(minusVec.x == -1);
		REQUIRE(minusVec.y == -2);
	}
}

TEST_CASE("2D Vector Stream Operators", k_tag)
{
	SECTION("Stream In")
	{
		vec2i vec;
		std::istringstream is("2 3");

		is >> vec;
		REQUIRE(vec.x == 2);
		REQUIRE(vec.y == 3);
	}

	SECTION("Stream Out")
	{
		vec2i vec(1, 2);
		std::stringstream ss;

		ss << vec;
		REQUIRE(ss.str() == "(1, 2)");
	}
}

TEST_CASE("2D Vector Lerping Vector", k_tag)
{
	vec2i vecA(1, -1);
	vec2i vecB(2, -2);
	auto lerpedVec = math::g_lerp(vecA, vecB, 0.5f);

	REQUIRE(lerpedVec.x == 1.5f);
	REQUIRE(lerpedVec.y == -1.5f);
}

