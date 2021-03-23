#include <catch.hpp>

#include <cmath>
#include <sstream>

#include <Core/Math/Math.hpp>
#include <Core/Math/Vec4.hpp>
#include <Core/Math/Values.hpp>



constexpr static char k_tag[] = "[Vec4]";

TEST_CASE("4D Vector Constructors", k_tag)
{
	SECTION("Default Constructor")
	{
		vec4i vec;

		REQUIRE(vec.x == 0);
		REQUIRE(vec.y == 0);
		REQUIRE(vec.z == 0);
		REQUIRE(vec.w == 0);
	}

	SECTION("Single Parameter Constructor")
	{
		vec4i vec(1);

		REQUIRE(vec.x == 1);
		REQUIRE(vec.y == 1);
		REQUIRE(vec.z == 1);
		REQUIRE(vec.w == 1);
	}

	SECTION("Individual Parameter Constructor")
	{
		vec4i vec(2, 3, 4, 5);

		REQUIRE(vec.x == 2);
		REQUIRE(vec.y == 3);
		REQUIRE(vec.z == 4);
		REQUIRE(vec.w == 5);
	}

	SECTION("2D Vector Constructor")
	{
		vec4i vecA(vec2i(1, 2));

		REQUIRE(vecA.x == 1);
		REQUIRE(vecA.y == 2);
		REQUIRE(vecA.z == 0);
		REQUIRE(vecA.w == 0);

		vec4i vecB(vec2i(1, 2), 3, 4);

		REQUIRE(vecB.x == 1);
		REQUIRE(vecB.y == 2);
		REQUIRE(vecB.z == 3);
		REQUIRE(vecB.w == 4);
	}

	SECTION("3D Vector Constructor")
	{
		vec4i vecA(vec3i(1, 2, 3));

		REQUIRE(vecA.x == 1);
		REQUIRE(vecA.y == 2);
		REQUIRE(vecA.z == 3);
		REQUIRE(vecA.w == 0);

		vec4i vecB(vec3i(2, 3, 4), 5);

		REQUIRE(vecB.x == 2);
		REQUIRE(vecB.y == 3);
		REQUIRE(vecB.z == 4);
		REQUIRE(vecB.w == 5);
	}
}

TEST_CASE("4D Vector Alternative Names Access", k_tag)
{
	vec4i vec(1, 2, 3, 4);

	SECTION("Access as Color")
	{
		REQUIRE(vec.r == 1);
		REQUIRE(vec.g == 2);
		REQUIRE(vec.b == 3);
		REQUIRE(vec.a == 4);
	}

	SECTION("Access as Array")
	{
		REQUIRE(vec[0] == 1);
		REQUIRE(vec[1] == 2);
		REQUIRE(vec[2] == 3);
		REQUIRE(vec[3] == 4);
	}

	SECTION("Assignment as Array")
	{
		vec[0] = 3;
		vec[1] = 4;
		vec[2] = 5;
		vec[3] = 6;

		REQUIRE(vec[0] == 3);
		REQUIRE(vec[1] == 4);
		REQUIRE(vec[2] == 5);
		REQUIRE(vec[3] == 6);
	}
}

TEST_CASE("4D Vector Properties", k_tag)
{
	vec4i vec(4, 4, 4, 4);

	SECTION("Vector Squared Length")
	{
		REQUIRE(sqrLength(vec) == 64.0f);
	}

	SECTION("Vector Length")
	{
		REQUIRE(length(vec) == 8.0f);
	}
}

TEST_CASE("4D Vector General Arithmetic Operators", k_tag)
{
	vec4i vecA(1, 2, 3, 4);
	vec4i vecB(5, 6, 7, 8);
	vec4 vecC(9.0f, 10.0f, 11.0f, 12.0f);

	SECTION("Assign Operator")
	{
		vec4i testVec = vecA;

		REQUIRE(testVec.x == 1);
		REQUIRE(testVec.y == 2);
		REQUIRE(testVec.z == 3);
		REQUIRE(testVec.w == 4);
	}

	SECTION("Assign to a 2D Vector")
	{
		vec2i vec2D = static_cast<vec2i>(vecA);
		
		REQUIRE(vec2D.x == 1);
		REQUIRE(vec2D.y == 2);
	}

	SECTION("Assign to a 3D Vector")
	{
		vec3i vec3D = static_cast<vec3i>(vecA);

		REQUIRE(vec3D.x == 1);
		REQUIRE(vec3D.y == 2);
		REQUIRE(vec3D.z == 3);
	}

	SECTION("Sum Operator")
	{
		vec4i sum = vecA + vecB;
		
		REQUIRE(sum.x == 6);
		REQUIRE(sum.y == 8);
		REQUIRE(sum.z == 10);
		REQUIRE(sum.w == 12);
	}

	SECTION("Cross type sum operator")
	{
		auto sumA = vecA + vecC;
		auto sumB = vecC + vecA;

		REQUIRE(typeid(sumA) == typeid(vec4));
		REQUIRE(typeid(sumB) == typeid(vec4));

		REQUIRE(sumB.x == 10.0f);
		REQUIRE(sumB.y == 12.0f);
		REQUIRE(sumB.z == 14.0f);
		REQUIRE(sumB.w == 16.0f);
	}

	SECTION("Subtraction Operator")
	{
		auto sub = vecB - vecA;

		REQUIRE(sub.x == 4);
		REQUIRE(sub.y == 4);
		REQUIRE(sub.z == 4);
		REQUIRE(sub.w == 4);
	}

	SECTION("Cross type sub operator")
	{
		auto subA = vecC - vecA;
		auto subB = vecA - vecC;

		REQUIRE(typeid(subA) == typeid(vec4));
		REQUIRE(typeid(subB) == typeid(vec4));
		REQUIRE(subA.x == 8.0f);
		REQUIRE(subA.y == 8.0f);
		REQUIRE(subA.z == 8.0f);
		REQUIRE(subA.w == 8.0f);
	}

	SECTION("Scalar Multiplication")
	{
		auto mul = vecA * 2;

		REQUIRE(mul.x == 2);
		REQUIRE(mul.y == 4);
		REQUIRE(mul.z == 6);
		REQUIRE(mul.w == 8);
	}

	SECTION("Scalar Multiplication Commutation")
	{
		auto mul = 2 * vecA;

		REQUIRE(mul.x == 2);
		REQUIRE(mul.y == 4);
		REQUIRE(mul.z == 6);
		REQUIRE(mul.w == 8);
	}

	SECTION("Cross type multiplication")
	{
		auto mulA = vecA * 2.0f;
		auto mulB = 2.0f * vecA;

		REQUIRE(typeid(mulA) == typeid(vec4));
		REQUIRE(typeid(mulB) == typeid(vec4));
		REQUIRE(mulA.x == 2.0f);
		REQUIRE(mulA.y == 4.0f);
		REQUIRE(mulA.z == 6.0f);
		REQUIRE(mulA.w == 8.0f);
	}

	SECTION("Scalar Division")
	{
		vec4i testVec(2, 4, 6, 8);
		auto div = testVec / 2;

		REQUIRE(typeid(div) == typeid(vec4));
		REQUIRE(div.x == 1.0f);
		REQUIRE(div.y == 2.0f);
		REQUIRE(div.z == 3.0f);
		REQUIRE(div.w == 4.0f);
	}

	SECTION("Scalar multiplication must generate a vector of the type of the scalar")
	{
		auto floatVec = vecA * 0.5f;

		REQUIRE(typeid(floatVec) == typeid(vector4<float>));
	}

	SECTION("Scalar division must generate a float vector")
	{
		auto floatVec = vecB / 2;

		REQUIRE(typeid(floatVec) == typeid(vector4<float>));
	}
}

TEST_CASE("4D Vector Vectorial Operators", k_tag)
{
	vec4 vecA(1.0f, 2.0f, 3.0f, 4.0f);
	vec4 vecB(5.0f, 6.0f, 7.0f, 8.0f);

	SECTION("Dot Product")
	{
		float result = dot(vecA, vecB);

		REQUIRE(result == 70.0f);
	}

	SECTION("Normalize")
	{
		vec4 normalized = normalize(vecA);
		float mag = length(normalized);

		REQUIRE((abs(mag) - 1.0f) < k_epsilon);
	}
}

TEST_CASE("4D Vector Boolean Operators", k_tag)
{
	vec4i vecA(1, 2, 3, 4);
	vec4i vecB(1, 2, 3, 4);
	vec4i vecC(1, 3, 1, 6);

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

TEST_CASE("4D Vector Unary Operators", k_tag)
{
	vec4i vecA(1, 2, 3, 4);

	SECTION("Negative")
	{
		vec4i minusVec = -vecA;

		REQUIRE(minusVec.x == -1);
		REQUIRE(minusVec.y == -2);
		REQUIRE(minusVec.z == -3);
		REQUIRE(minusVec.w == -4);
	}
}

TEST_CASE("4D Vector Stream Operators", k_tag)
{
	SECTION("Stream In")
	{
		vec4i vec;
		std::istringstream is("2 3 4 5");
		is >> vec;

		REQUIRE(vec.x == 2);
		REQUIRE(vec.y == 3);
		REQUIRE(vec.z == 4);
		REQUIRE(vec.w == 5);
	}

	SECTION("Stream Out")
	{
		vec4i vec(1, 2, 3, 4);
		std::stringstream ss;
		ss << vec;

		REQUIRE(ss.str() == "(1, 2, 3, 4)");
	}
}

TEST_CASE("4D Vector Lerping Vector", k_tag)
{
	vec4i vecA(1, -1, 2, 3);
	vec4i vecB(2, -2, 4, 6);
	auto lerpedVec = lerp(vecA, vecB, 0.5f);

	REQUIRE(lerpedVec.x == 1.5f);
	REQUIRE(lerpedVec.y == -1.5f);
	REQUIRE(lerpedVec.z == 3.0f);
	REQUIRE(lerpedVec.w == 4.5f);
}

