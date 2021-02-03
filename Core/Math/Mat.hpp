#pragma once

#include <array>

#include "../Attributes.hpp"
#include "../BuildScheme.hpp"
#include "../Types.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"



// Matrix constexpr helper functions
namespace
{

template<typename t_type, size_t t_elementCount, int t_index = 0>
force_inline void addMat(const std::array<t_type, t_elementCount> &lMat, 
						 const std::array<t_type, t_elementCount> &rMat, 
						 std::array<t_type, t_elementCount> &outMat)
{
	outMat[t_index] = lMat[t_index] + rMat[t_index];

	if constexpr (t_index < t_elementCount - 1)
		addMat<t_type, t_elementCount, t_index + 1>(lMat, rMat, outMat);
}

template<typename t_type, size_t t_elementCount, int t_index = 0>
force_inline void subMat(const std::array<t_type, t_elementCount> &lMat,
			 				       const std::array<t_type, t_elementCount> &rMat,
			 					   std::array<t_type, t_elementCount> &outMat)
{
	outMat[t_index] = lMat[t_index] - rMat[t_index];

	if constexpr (t_index < t_elementCount - 1)
		subMat<t_type, t_elementCount, t_index + 1>(lMat, rMat, outMat);
}

template<typename t_type, typename t_scalarType, typename t_resultType, size_t t_elementCount, int t_index = 0>
force_inline void scalarMulMat(const std::array<t_type, t_elementCount> &mat, 
										 t_scalarType scalar, 
										 std::array<t_resultType, t_elementCount> &resultMat)
{
	resultMat[t_index] = mat[t_index] * scalar;

	if constexpr (t_index < t_elementCount - 1)
		scalarMulMat<t_type, t_scalarType, t_resultType, t_elementCount, t_index + 1>(mat, scalar, resultMat);
}

template<typename t_type, typename t_scalarType, size_t t_elementCount, int t_index = 0>
force_inline void scalarMulDiv(const std::array<t_type, t_elementCount> &mat, 
										 t_scalarType scalar, 
										 std::array<float, t_elementCount> &resultMat)
{
	resultMat[t_index] = mat[t_index] / scalar;

	if constexpr (t_index < t_elementCount - 1)
		scalarMulDiv<t_type, t_scalarType, t_elementCount, t_index + 1>(mat, scalar, resultMat);
}

template<typename t_type, size_t t_size, int t_index = 0>
force_inline void setDiagonal(std::array<std::array<t_type, t_size>, t_size> &grid, t_type scalar)
{
	grid[t_index][t_index] = scalar;

	if constexpr (t_index < t_size - 1)
		setDiagonal<t_type, t_size, t_index + 1>(grid, scalar);
}

} // namespace Anon



// Row Major Matrix Implementation
template<typename t_type, size_t t_columns, size_t t_rows>
union matrix
{
ASSERT_IS_ARITHMETIC(t_type);

using t_self = matrix<t_type, t_columns, t_rows>;

public:
	matrix() : matrix(static_cast<t_type>(0)) {}
	matrix(std::array<t_type, t_columns * t_rows> &&elements) : _elements(elements) {}
	matrix(t_type value) { _elements.fill(value); }

	const t_type at(ui32 column, ui32 row) const { return _grid[row][column]; }
	t_type &at(ui32 column, ui32 row) { return _grid[row][column]; }
	operator t_type * () const { return &_elements; }

	void operator = (const t_self &mtx) 
	{ 
		_elements = mtx._elements; 
	}

	void operator = (const std::array<t_type, t_columns * t_rows> &&values)
	{
		_elements = values;
	}

	t_self operator + (const t_self &mat) const
	{
		t_self result;
		addMat<t_type, t_columns * t_rows>(_elements, mat._elements, result._elements);

		return result;
	}

	void operator += (const t_self &mat)
	{
		*this = *this + mat;
	}

	t_self operator - (const t_self &mat) const
	{
		t_self result;
		subMat<t_type, t_columns * t_rows>(_elements, mat._elements, result._elements);
		
		return result;
	}

	void operator -= (const t_self &mat)
	{
		*this = *this - mat;
	}

	auto operator * (int scalar)
	{
		return scalarMult<int>(scalar);
	}

	auto operator * (float scalar)
	{
		return scalarMult<float>(scalar);
	}

	void operator *= (t_type scalar)
	{
		*this = *this * scalar;
	}

	template<typename t_scalarType>
	auto operator / (t_scalarType scalar) const
	{
		matrix<float, t_columns, t_rows> result;
		scalarMulDiv<t_type, t_scalarType, t_columns * t_rows>(_elements, scalar, result._elements);

		return result;
	}

	static t_self identity()
	{
		static_assert(t_columns == t_rows, "We can only retrieve an identity matrix from a square matrix.");
		t_self mat;
		setDiagonal(mat._grid, static_cast<t_type>(1));

		return mat;
	}

	friend auto operator * (int scalar, const matrix<t_type, t_columns, t_rows> &rMat)
	{
		return rMat.scalarMult<int>(scalar);
	}

	friend auto operator * (float scalar, const matrix<t_type, t_columns, t_rows> &rMat)
	{
		return rMat.scalarMult<float>(scalar);
	}

private:
	// Use grid to access matrix values as a 2D array
	std::array<std::array<t_type, t_columns>, t_rows> _grid;

	// Use elements to access the matrix values as a linear array
	std::array<t_type, t_columns * t_rows> _elements;

	// We need this private method so the scalar and matrix multiplication
	// operators aren't confused with each other by templating
	template<typename t_scalarType>
	force_inline auto scalarMult(t_scalarType scalar) const
	{
		ASSERT_IS_ARITHMETIC(t_scalarType);
		using resultType = decltype(_elements[0] * scalar);

		matrix<resultType, t_columns, t_rows> result;
		scalarMulMat<t_type, t_scalarType, resultType, t_columns * t_rows>(_elements, scalar, result._elements);

		return result;
	}
};



// Matrix multiplication constexpr helper functions
namespace IF_TEST(test_matrix)
{

template<typename t_lType, typename t_rType, typename t_resultType, 
		 size_t t_lColumns, size_t t_lRows, size_t t_rColumns, 
		 ui32 t_resultColumnIndex, ui32 t_resultRowIndex, ui32 t_lColumnIndex = 0>
force_inline t_resultType mulMatCalcCell(const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
												   const matrix<t_rType, t_rColumns, t_lColumns> &rMat)
{
	if constexpr (t_lColumnIndex < t_lColumns)
	{
		t_resultType result = lMat.at(t_lColumnIndex, t_resultRowIndex) * rMat.at(t_resultColumnIndex, t_lColumnIndex);
		return result + mulMatCalcCell<t_lType, t_rType, t_resultType,
								   t_lColumns, t_lRows, t_rColumns,
								   t_resultColumnIndex,t_resultRowIndex, t_lColumnIndex + 1>
								  (lMat, rMat);
	}

	return static_cast<t_resultType>(0);
}

template<typename t_lType, typename t_rType, typename t_resultType, 
		 size_t t_lColumns, size_t t_lRows, size_t t_rColumns, 
		 ui32 t_resultColumnIndex, ui32 t_resultRowsIndex = 0>
force_inline void mulMatCalcColumn(const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
											 const matrix<t_rType, t_rColumns, t_lColumns> &rMat,
											 matrix<t_resultType, t_rColumns, t_lColumns> &resultMat)
{
	resultMat.at(t_resultColumnIndex, t_resultRowsIndex) = mulMatCalcCell<t_lType, t_rType, t_resultType,
														   t_lColumns, t_lRows, t_rColumns,
														   t_resultColumnIndex, t_resultRowsIndex>
														  (lMat, rMat);  

	if constexpr (t_resultRowsIndex < t_lColumns - 1)
		mulMatCalcColumn<t_lType, t_rType, t_resultType, 
						 t_lColumns, t_lRows, t_rColumns, 
						 t_resultColumnIndex, t_resultRowsIndex + 1>
						(lMat, rMat, resultMat);
}


template<typename t_lType, typename t_rType, typename t_resultType, 
		 size_t t_lColumns, size_t t_lRows, size_t t_rColumns, 
		 int t_rColumnIndex = 0>
force_inline void mulMat(const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
								   const matrix<t_rType, t_rColumns, t_lColumns> &rMat,
								  matrix<t_resultType, t_rColumns, t_lColumns> &resultMat)
{
	mulMatCalcColumn<t_lType, t_rType, t_resultType,
					 t_lColumns, t_lRows, t_rColumns,
					 t_rColumnIndex>
					(lMat, rMat, resultMat);

	if constexpr (t_rColumnIndex < t_rColumns - 1)
		mulMat<t_lType, t_rType, t_resultType, 
			   t_lColumns, t_lRows, t_rColumns,
			   t_rColumnIndex + 1>
			  (lMat, rMat, resultMat);
}

} IF_TEST(using namespace test_matrix);



template <typename t_lType, typename t_rType, size_t t_lColumns, size_t t_lRows, size_t t_rColumns>
auto operator * (const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
				 const matrix<t_rType, t_rColumns, t_lColumns> &rMat)
{
	using resultType = decltype(lMat.at(0, 0) * rMat.at(0, 0));

	matrix<resultType, t_rColumns, t_lColumns> result;
	mulMat(lMat, rMat, result);

	return result;
}



// Other multiplication operator
template<typename t_lType, typename t_rType>
auto operator * (const matrix<t_lType, 2, 2> &mat, const vector2<t_rType> &vec)
{
	using resultType = decltype(mat.at(0, 0) * vec.x);

	vector2<resultType> result;
	result.x = (mat.at(0, 0) * vec.x) + (mat.at(1, 0) * vec.y);
	result.y = (mat.at(0, 1) * vec.x) + (mat.at(1, 1) * vec.y);

	return result;
}

template<typename t_lType, typename t_rType>
auto operator * (const matrix<t_lType, 3, 3> &mat, const vector3<t_rType> &vec)
{
	using resultType = decltype(mat.at(0, 0) * vec.x);

	vector3<resultType> result;
	result.x = (mat.at(0, 0) * vec.x) + (mat.at(1, 0) * vec.y) + (mat.at(2, 0) * vec.z);
	result.y = (mat.at(0, 1) * vec.x) + (mat.at(1, 1) * vec.y) + (mat.at(2, 1) * vec.z);
	result.z = (mat.at(0, 2) * vec.x) + (mat.at(1, 2) * vec.y) + (mat.at(2, 2) * vec.z);

	return result;
}

template<typename t_lType, typename t_rType>
auto operator * (const matrix<t_lType, 4, 4> &mat, const vector4<t_rType> &vec)
{
	using resultType = decltype(mat.at(0, 0) * vec.x);

	vector4<resultType> result;
	result.x = (mat.at(0, 0) * vec.x) + (mat.at(1, 0) * vec.y) + (mat.at(2, 0) * vec.z) + (mat.at(3, 0) * vec.w);
	result.y = (mat.at(0, 1) * vec.x) + (mat.at(1, 1) * vec.y) + (mat.at(2, 1) * vec.z) + (mat.at(3, 1) * vec.w);
	result.z = (mat.at(0, 2) * vec.x) + (mat.at(1, 2) * vec.y) + (mat.at(2, 2) * vec.z) + (mat.at(3, 2) * vec.w);
	result.w = (mat.at(0, 3) * vec.x) + (mat.at(1, 3) * vec.y) + (mat.at(2, 3) * vec.z) + (mat.at(3, 3) * vec.w);

	return result;
}



// Aliases
using mat2 = matrix<float, 2, 2>;
using mat3 = matrix<float, 3, 3>;
using mat4 = matrix<float, 4, 4>;



// Matrix generators
namespace mat
{

force_inline mat4 translate(float x, float y, float z)
{
	return mat4({1.0f, 0.0f, 0.0f,    x,
				 0.0f, 1.0f, 0.0f,    y,
				 0.0f, 0.0f, 1.0f,	  z,
				 0.0f, 0.0f, 0.0f, 1.0f});
}

force_inline mat4 scale(float x, float y, float z)
{
	return mat4({	x, 0.0f, 0.0f, 0.0f,
				 0.0f,	  y, 0.0f, 0.0f,
				 0.0f, 0.0f,	z, 0.0f,
				 0.0f, 0.0f, 0.0f, 1.0f});
}

}	// namespace mat
