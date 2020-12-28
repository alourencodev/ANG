#include <catch.hpp>

#include <Core/Math/Mat.hpp>

TEST_CASE("Matrix constructors")
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
		mat2 matA({1, 2, 
				   3, 4});

		REQUIRE(matA.at(0, 0) == 1);
		REQUIRE(matA.at(1, 0) == 2);
		REQUIRE(matA.at(0, 1) == 3);
		REQUIRE(matA.at(1, 1) == 4);
	}

	SECTION("Scalar constructor")
	{
		mat2 matA(8);

		REQUIRE(matA.at(0, 0) == 8);
		REQUIRE(matA.at(1, 0) == 8);
		REQUIRE(matA.at(0, 1) == 8);
		REQUIRE(matA.at(1, 1) == 8);
	}
}

TEST_CASE("Matrix Arithmetic operations")
{
	mat2 matA({1, 2, 
			   3, 4});

	mat2 matB({5, 6, 
			   7, 8});

	SECTION("Matrix Assignment")
	{
		matA.at(0, 0) = 5;
		matA.at(1, 0) = 6;
		matA.at(0, 1) = 7;
		matA.at(1, 1) = 8;

		REQUIRE(matA.at(0, 0) == 5);
		REQUIRE(matA.at(1, 0) == 6);
		REQUIRE(matA.at(0, 1) == 7);
		REQUIRE(matA.at(1, 1) == 8);
	}

	SECTION("Matrix sum")
	{
		mat2 result = matA + matB;

		REQUIRE(result.at(0, 0) == 6);
		REQUIRE(result.at(1, 0) == 8);
		REQUIRE(result.at(0, 1) == 10);
		REQUIRE(result.at(1, 1) == 12);
	}

	SECTION("Matrix equal sum")
	{
		matA += matB;

		REQUIRE(matA.at(0, 0) == 6);
		REQUIRE(matA.at(1, 0) == 8);
		REQUIRE(matA.at(0, 1) == 10);
		REQUIRE(matA.at(1, 1) == 12);
	}

	SECTION("Matrix sub")
	{
		mat2 result = matB - matA;

		REQUIRE(result.at(0, 0) == 4);
		REQUIRE(result.at(1, 0) == 4);
		REQUIRE(result.at(0, 1) == 4);
		REQUIRE(result.at(1, 1) == 4);
	}

	SECTION("Matrix equal sub")
	{
		matB -= matA;

		REQUIRE(matB.at(0, 0) == 4);
		REQUIRE(matB.at(1, 0) == 4);
		REQUIRE(matB.at(0, 1) == 4);
		REQUIRE(matB.at(1, 1) == 4);
	}

	SECTION("Scalar mul")
	{
		mat2 result = matA * 2;

		REQUIRE(result.at(0, 0) == 2);
		REQUIRE(result.at(1, 0) == 4);
		REQUIRE(result.at(0, 1) == 6);
		REQUIRE(result.at(1, 1) == 8);
	}

	SECTION("Scalar mul commutation")
	{
		mat2 result = 2 * matA;

		REQUIRE(result.at(0, 0) == 2);
		REQUIRE(result.at(1, 0) == 4);
		REQUIRE(result.at(0, 1) == 6);
		REQUIRE(result.at(1, 1) == 8);
	}

	SECTION("Scalar equal mul")
	{
		matA *= 2;

		REQUIRE(matA.at(0, 0) == 2);
		REQUIRE(matA.at(1, 0) == 4);
		REQUIRE(matA.at(0, 1) == 6);
		REQUIRE(matA.at(1, 1) == 8);
	}

	SECTION("Scalar div")
	{
		mat2 matDiv({2, 4,
					 6, 8});

		mat2 result = matDiv / 2;

		REQUIRE(result.at(0, 0) == 1);
		REQUIRE(result.at(1, 0) == 2);
		REQUIRE(result.at(0, 1) == 3);
		REQUIRE(result.at(1, 1) == 4);
	}
}

TEST_CASE("Matrix Multiplication")
{
	mat2 matA({1, 2, 
			   3, 4});

	mat2 matB({5, 6, 
			   7, 8});

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