#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/Math/Mat.hpp>


using namespace age::math;

constexpr static char k_tag[] = "[Mat]";

TEST_CASE("Matrix constructors", k_tag)
{
	SECTION("Default Constructor")
	{
		mat2 matA;

		REQUIRE(matA.at(0, 0) == 0);
		REQUIRE(matA.at(0, 0) == 0);
		REQUIRE(matA.at(0, 0) == 0);
		REQUIRE(matA.at(0, 0) == 0);
	}

	SECTION("Linear array constructor")
	{
		mat2 matA({1.0f, 2.0f, 
				   3.0f, 4.0f});

		REQUIRE(matA.at(0, 0) == 1.0f);
		REQUIRE(matA.at(1, 0) == 2.0f);
		REQUIRE(matA.at(0, 1) == 3.0f);
		REQUIRE(matA.at(1, 1) == 4.0f);
	}

	SECTION("Scalar constructor")
	{
		mat2 matA(8.0f);

		REQUIRE(matA.at(0, 0) == 8.0f);
		REQUIRE(matA.at(1, 0) == 8.0f);
		REQUIRE(matA.at(0, 1) == 8.0f);
		REQUIRE(matA.at(1, 1) == 8.0f);
	}
}

TEST_CASE("Matrix Arithmetic operations", k_tag)
{
	mat2 matA({1.0f, 2.0f, 
			   3.0f, 4.0f});

	mat2 matB({5.0f, 6.0f, 
			   7.0f, 8.0f});

	SECTION("Matrix Assignment")
	{
		matA.at(0, 0) = 5.0f;
		matA.at(1, 0) = 6.0f;
		matA.at(0, 1) = 7.0f;
		matA.at(1, 1) = 8.0f;

		REQUIRE(matA.at(0, 0) == 5.0f);
		REQUIRE(matA.at(1, 0) == 6.0f);
		REQUIRE(matA.at(0, 1) == 7.0f);
		REQUIRE(matA.at(1, 1) == 8.0f);
	}

	SECTION("Matrix sum")
	{
		mat2 result = matA + matB;

		REQUIRE(result.at(0, 0) == 6.0f);
		REQUIRE(result.at(1, 0) == 8.0f);
		REQUIRE(result.at(0, 1) == 10.0f);
		REQUIRE(result.at(1, 1) == 12.0f);
	}

	SECTION("Matrix equal sum")
	{
		matA += matB;

		REQUIRE(matA.at(0, 0) == 6.0f);
		REQUIRE(matA.at(1, 0) == 8.0f);
		REQUIRE(matA.at(0, 1) == 10.0f);
		REQUIRE(matA.at(1, 1) == 12.0f);
	}

	SECTION("Matrix sub")
	{
		mat2 result = matB - matA;

		REQUIRE(result.at(0, 0) == 4.0f);
		REQUIRE(result.at(1, 0) == 4.0f);
		REQUIRE(result.at(0, 1) == 4.0f);
		REQUIRE(result.at(1, 1) == 4.0f);
	}

	SECTION("Matrix equal sub")
	{
		matB -= matA;

		REQUIRE(matB.at(0, 0) == 4.0f);
		REQUIRE(matB.at(1, 0) == 4.0f);
		REQUIRE(matB.at(0, 1) == 4.0f);
		REQUIRE(matB.at(1, 1) == 4.0f);
	}

	SECTION("Scalar mul")
	{
		mat2 result = matA * 2;

		REQUIRE(result.at(0, 0) == 2.0f);
		REQUIRE(result.at(1, 0) == 4.0f);
		REQUIRE(result.at(0, 1) == 6.0f);
		REQUIRE(result.at(1, 1) == 8.0f);
	}

	SECTION("Scalar mul commutation")
	{
		mat2 result = 2 * matA;

		REQUIRE(result.at(0, 0) == 2.0f);
		REQUIRE(result.at(1, 0) == 4.0f);
		REQUIRE(result.at(0, 1) == 6.0f);
		REQUIRE(result.at(1, 1) == 8.0f);
	}

	SECTION("Scalar equal mul")
	{
		matA *= 2;

		REQUIRE(matA.at(0, 0) == 2.0f);
		REQUIRE(matA.at(1, 0) == 4.0f);
		REQUIRE(matA.at(0, 1) == 6.0f);
		REQUIRE(matA.at(1, 1) == 8.0f);
	}

	SECTION("Scalar div")
	{
		mat2 matDiv({2.0f, 4.0f,
					 6.0f, 8.0f});

		mat2 result = matDiv / 2;

		REQUIRE(result.at(0, 0) == 1.0f);
		REQUIRE(result.at(1, 0) == 2.0f);
		REQUIRE(result.at(0, 1) == 3.0f);
		REQUIRE(result.at(1, 1) == 4.0f);
	}
}

TEST_CASE("Matrix Multiplication", k_tag)
{
	mat2 matA({1.0f, 2.0f, 
			   3.0f, 4.0f});

	mat2 matB({5.0f, 6.0f, 
			   7.0f, 8.0f});

	SECTION("Matrix cell multiplication")
	{
		float leftUpperResult = test_matrix::mulMatCalcCell<float, float, float, 2, 2, 2, 0, 0>(matA, matB);
		float leftBottomResult = test_matrix::mulMatCalcCell<float, float, float, 2, 2, 2, 0, 1>(matA, matB);
		float rightUpperResult = test_matrix::mulMatCalcCell<float, float, float, 2, 2, 2, 1, 0>(matA, matB);
		float rightBottomResult = test_matrix::mulMatCalcCell<float, float, float, 2, 2, 2, 1, 1>(matA, matB);

		REQUIRE(leftUpperResult == 19.0f);
		REQUIRE(leftBottomResult == 43.0f);
		REQUIRE(rightUpperResult == 22.0f);
		REQUIRE(rightBottomResult == 50.0f);
	}

	SECTION("Matrix Row Multiplication")
	{
		mat2 resultMat;

		{	// Calculate the left side
			test_matrix::mulMatCalcColumn<float, float, float, 2, 2, 2, 0>(matA, matB, resultMat);

			REQUIRE(resultMat.at(0, 0) == 19.0f);
			REQUIRE(resultMat.at(0, 1) == 43.0f);
			
			// Test if roght side is not yet calculated
			REQUIRE(resultMat.at(1, 0) == 0.0f);
			REQUIRE(resultMat.at(1, 1) == 0.0f);
		}

		{ 	// Calculate the right side
			test_matrix::mulMatCalcColumn<float, float, float, 2, 2, 2, 1>(matA, matB, resultMat);

			REQUIRE(resultMat.at(1, 0) == 22.0f);
			REQUIRE(resultMat.at(1, 1) == 50.0f);
		}
	}

	SECTION("Matrix multiplication operator")
	{
		mat2 resultMat = matA * matB;

		REQUIRE(resultMat.at(0, 0) == 19.0f);
		REQUIRE(resultMat.at(0, 1) == 43.0f);
		REQUIRE(resultMat.at(1, 0) == 22.0f);
		REQUIRE(resultMat.at(1, 1) == 50.0f);
	}
}

TEST_CASE("Matrix vector multiplication", k_tag)
{
	SECTION("Mat2 * Vec2")
	{
		vec2 vec(1.0f, 2.0f);
		mat2 mat({1.0f, 2.0f, 
				  3.0f, 4.0f});

		auto result = mat * vec;

		REQUIRE(typeid(result) == typeid(vec2));

		REQUIRE(result.x == 5.0f);
		REQUIRE(result.y == 11.0f);
	}

	SECTION("Mat3 * Vec3")
	{
		vec3 vec(1.0f, 2.0f, 3.0f);
		mat3 mat({1.0f, 2.0f, 3.0f, 
				  4.0f, 5.0f, 6.0f,
				  7.0f, 8.0f, 9.0f});

		auto result = mat * vec;

		REQUIRE(typeid(result) == typeid(vec3));

		REQUIRE(result.x == 14.0f);
		REQUIRE(result.y == 32.0f);
		REQUIRE(result.z == 50.0f);
	}

	SECTION("Mat3 * Vec3")
	{
		vec4 vec(1.0f, 2.0f, 3.0f, 4.0f);
		mat4 mat({ 1.0f,  2.0f,  3.0f,  4.0f, 
				   5.0f,  6.0f,  7.0f,  8.0f, 
				   9.0f, 10.0f, 11.0f, 12.0f,
				  13.0f, 14.0f, 15.0f, 16.0f});

		auto result = mat * vec;

		REQUIRE(typeid(result) == typeid(vec4));

		REQUIRE(result.x == 30.0f);
		REQUIRE(result.y == 70.0f);
		REQUIRE(result.z == 110.0f);
		REQUIRE(result.w == 150.0f);
	}
}

TEST_CASE("Generating Matrices", k_tag)
{
	SECTION("Identity Matrix")
	{
		mat3 id = mat3::identity();

		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				if (y == x)
					REQUIRE(id.at(x, y) == 1.0f);
				else
					REQUIRE(id.at(x, y) == 0.0f);
			}
		}
	}

	SECTION("Translation Matrix")
	{
		vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
		vec4 movedVec = mat::translate(1.0f, -1.0f, 2.0f) * vec;

		REQUIRE(movedVec.x ==  2.0f);
		REQUIRE(movedVec.y == -1.0f);
		REQUIRE(movedVec.z ==  2.0f);
		REQUIRE(movedVec.w ==  1.0f);
	}

	SECTION("Scale Matrix")
	{
		vec4 vec(2.0f, 0.0, 2.0f, 1.0f);
		vec4 scaledVec = mat::scale(1.0f, 2.0f, 2.0f) * vec;

		REQUIRE(scaledVec.x == 2.0f);
		REQUIRE(scaledVec.y == 0.0f);
		REQUIRE(scaledVec.z == 4.0f);
		REQUIRE(scaledVec.w == 1.0f);
	}
}
