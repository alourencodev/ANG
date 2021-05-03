#include <catch.hpp>

#include <cmath>
#include <sstream>

#include <Core/Math/Math.hpp>
#include <Core/Math/Vec3.hpp>
#include <Core/Math/Values.hpp>


constexpr static char k_tag[] = "[Vec3]"; 

TEST_CASE("3D Vector Constructors", k_tag)
{
	SECTION("Default Constructor")
	{
		vec3i vec;

		REQUIRE(vec.x == 0);
		REQUIRE(vec.y == 0);
		REQUIRE(vec.z == 0);
	}

	SECTION("Single Parameter Constructor")
	{
		vec3i vec(1);

		REQUIRE(vec.x == 1);
		REQUIRE(vec.y == 1);
		REQUIRE(vec.z == 1);
	}

	SECTION("Individual Parameter Constructor")
	{
		vec3i vec(2, 3, 4);

		REQUIRE(vec.x == 2);
		REQUIRE(vec.y == 3);
		REQUIRE(vec.z == 4);
	}

	SECTION("2D Vector Constructor")
	{
		vec3i vecA(vec2i(1, 2));

		REQUIRE(vecA.x == 1);
		REQUIRE(vecA.y == 2);
		REQUIRE(vecA.z == 0);

		vec3i vecB(vec2i(1, 2), 3);

		REQUIRE(vecB.x == 1);
		REQUIRE(vecB.y == 2);
		REQUIRE(vecB.z == 3);
	}
}

TEST_CASE("3D Vector Alternative Names Access", k_tag)
{
	vec3i vec(1, 2, 3);

	SECTION("Access as Color")
	{
		REQUIRE(vec.r == 1);
		REQUIRE(vec.g == 2);
		REQUIRE(vec.b == 3);
	}

	SECTION("Access as Array")
	{
		REQUIRE(vec[0] == 1);
		REQUIRE(vec[1] == 2);
		REQUIRE(vec[2] == 3);
	}

	SECTION("Assignment as Array")
	{
		vec[0] = 3;
		vec[1] = 4;
		vec[2] = 5;

		REQUIRE(vec[0] == 3);
		REQUIRE(vec[1] == 4);
		REQUIRE(vec[2] == 5);
	}
}

TEST_CASE("3D Vector Properties", k_tag)
{
	vec3i vec(2, 4, 4);

	SECTION("Vector Length")
	{
		REQUIRE(length(vec) == 6.0f);
	}

	SECTION("Vector Squared Length")
	{
		REQUIRE(sqrLength(vec) == 36.0f);
	}
}

TEST_CASE("3D Vector General Arithmetic Operators", k_tag)
{
	vec3i vecA(1, 2, 3);
	vec3i vecB(4, 5, 6);
	vec3 vecC(7.0f, 8.0f, 9.0f);

	SECTION("Assign Operator")
	{
		vec3i testVec = vecA;

		REQUIRE(testVec.x == 1);
		REQUIRE(testVec.y == 2);
		REQUIRE(testVec.z == 3);
	}

	SECTION("Assign to 2D Vector")
	{
		vec2i vec2D = static_cast<vec2i>(vecA);
		
		REQUIRE(vec2D.x == 1);
		REQUIRE(vec2D.y == 2);
	}

	SECTION("Sum Operator")
	{
		vec3i sum = vecA + vecB;
		
		REQUIRE(sum.x == 5);
		REQUIRE(sum.y == 7);
		REQUIRE(sum.z == 9);
	}

	SECTION("Cross type sum operator")
	{
		auto sumA = vecA + vecC;
		auto sumB = vecC + vecA;

		REQUIRE(typeid(sumA) == typeid(vec3));
		REQUIRE(typeid(sumB) == typeid(vec3));

		REQUIRE(sumB.x == 8.0f);
		REQUIRE(sumB.y == 10.0f);
		REQUIRE(sumB.z == 12.0f);
	}

	SECTION("Subtraction Operator")
	{
		auto sub = vecB - vecA;

		REQUIRE(sub.x == 3);
		REQUIRE(sub.y == 3);
		REQUIRE(sub.z == 3);
	}

	SECTION("Cross type sub operator")
	{
		auto subA = vecC - vecA;
		auto subB = vecA - vecC;

		REQUIRE(typeid(subA) == typeid(vec3));
		REQUIRE(typeid(subB) == typeid(vec3));
		REQUIRE(subA.x == 6.0f);
		REQUIRE(subA.y == 6.0f);
		REQUIRE(subA.z == 6.0f);
	}

	SECTION("Scalar Multiplication")
	{
		auto mul = vecA * 2;

		REQUIRE(mul.x == 2);
		REQUIRE(mul.y == 4);
		REQUIRE(mul.z == 6);
	}

	SECTION("Scalar Multiplication Commutation")
	{
		auto mul = 2 * vecA;

		REQUIRE(mul.x == 2);
		REQUIRE(mul.y == 4);
		REQUIRE(mul.z == 6);
	}

	SECTION("Cross type multiplication")
	{
		auto mulA = vecA * 2.0f;
		auto mulB = 2.0f * vecA;

		REQUIRE(typeid(mulA) == typeid(vec3));
		REQUIRE(typeid(mulB) == typeid(vec3));
		REQUIRE(mulA.x == 2.0f);
		REQUIRE(mulA.y == 4.0f);
		REQUIRE(mulA.z == 6.0f);
	}

	SECTION("Scalar Division")
	{
		vec3i testVec(2, 4, 6);
		auto div = testVec / 2;

		REQUIRE(typeid(div) == typeid(vec3));
		REQUIRE(div.x == 1.0f);
		REQUIRE(div.y == 2.0f);
		REQUIRE(div.z == 3.0f);
	}

	SECTION("Scalar multiplication must generate a vector of the type of the scalar")
	{
		auto floatVec = vecA * 0.5f;

		REQUIRE(typeid(floatVec) == typeid(vector3<float>));
	}

	SECTION("Scalar division must generate a float vector")
	{
		auto floatVec = vecB / 2;

		REQUIRE(typeid(floatVec) == typeid(vector3<float>));
	}
}

TEST_CASE("3D Vector Vectorial Operators", k_tag)
{
	vec3 vecA(1.0f, 2.0f, 3.0f);
	vec3 vecB(4.0f, 5.0f, 6.0f);

	SECTION("Dot Product")
	{
		float result = dot(vecA, vecB);

		REQUIRE(result == 32.0f);
	}

	SECTION("Cross Product")
	{
		vec3 crossVec = cross(vecA, vecB);

		REQUIRE(crossVec.x == -3.0f);
		REQUIRE(crossVec.y == 6.0f);
		REQUIRE(crossVec.z == -3.0f);
	}

	SECTION("Normalize")
	{
		vec3 normalized = normalize(vecA);
		float mag = length(normalized);

		REQUIRE((abs(mag) - 1.0f) < k_epsilon);
	}
}

TEST_CASE("3D Vector Boolean Operators", k_tag)
{
	vec3i vecA(1, 2, 3);
	vec3i vecB(1, 2, 3);
	vec3i vecC(2, 3, 1);

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

TEST_CASE("3D Vector Unary Operators", k_tag)
{
	vec3i vecA(1, 2, 3);

	SECTION("Negative")
	{
		vec3i minusVec = -vecA;

		REQUIRE(minusVec.x == -1);
		REQUIRE(minusVec.y == -2);
		REQUIRE(minusVec.z == -3);
	}
}

TEST_CASE("3D Vector Stream Operators", k_tag)
{
	SECTION("Stream In")
	{
		vec3i vec;
		std::istringstream is("2 3 4");

		is >> vec;
		REQUIRE(vec.x == 2);
		REQUIRE(vec.y == 3);
		REQUIRE(vec.z == 4);
	}

	SECTION("Stream Out")
	{
		vec3i vec(1, 2, 3);
		std::stringstream ss;

		ss << vec;
		REQUIRE(ss.str() == "(1, 2, 3)");
	}
}

TEST_CASE("3D Vector Lerping Vector", k_tag)
{
	vec3i vecA(1, -1, 2);
	vec3i vecB(2, -2, 4);
	auto lerpedVec = math::g_lerp(vecA, vecB, 0.5f);

	REQUIRE(lerpedVec.x == 1.5f);
	REQUIRE(lerpedVec.y == -1.5f);
	REQUIRE(lerpedVec.z == 3.0f);
}
