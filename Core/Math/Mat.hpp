#pragma once

#include <array>
#include <initializer_list>

#include "../Attributes.hpp"
#include "../BuildScheme.hpp"
#include "../Types.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"

namespace
{

template<typename t_type, size_t t_elementCount, int t_index = 0>
force_inline constexpr void addMat(const std::array<t_type, t_elementCount> &lMat, 
								   const std::array<t_type, t_elementCount> &rMat, 
								   std::array<t_type, t_elementCount> &outMat)
{
	outMat[t_index] = lMat[t_index] + rMat[t_index];

	if constexpr (t_index < t_elementCount - 1)
		addMat<t_type, t_elementCount, t_index + 1>(lMat, rMat, outMat);
}

template<typename t_type, size_t t_elementCount, int t_index = 0>
force_inline constexpr void subMat(const std::array<t_type, t_elementCount> &lMat,
			 				       const std::array<t_type, t_elementCount> &rMat,
			 					   std::array<t_type, t_elementCount> &outMat)
{
	outMat[t_index] = lMat[t_index] - rMat[t_index];

	if constexpr (t_index < t_elementCount - 1)
		subMat<t_type, t_elementCount, t_index + 1>(lMat, rMat, outMat);
}

template<typename t_type, typename t_scalarType, typename t_resultType, size_t t_elementCount, int t_index = 0>
force_inline constexpr void scalarMulMat(const std::array<t_type, t_elementCount> &mat, 
										 t_scalarType scalar, 
										 std::array<t_resultType, t_elementCount> &resultMat)
{
	resultMat[t_index] = mat[t_index] * scalar;

	if constexpr (t_index < t_elementCount - 1)
		scalarMulMat<t_type, t_scalarType, t_resultType, t_elementCount, t_index + 1>(mat, scalar, resultMat);
}

template<typename t_type, typename t_scalarType, size_t t_elementCount, int t_index = 0>
force_inline constexpr void scalarMulDiv(const std::array<t_type, t_elementCount> &mat, 
										 t_scalarType scalar, 
										 std::array<float, t_elementCount> &resultMat)
{
	resultMat[t_index] = mat[t_index] / scalar;

	if constexpr (t_index < t_elementCount - 1)
		scalarMulDiv<t_type, t_scalarType, t_elementCount, t_index + 1>(mat, scalar, resultMat);
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

	constexpr t_self operator + (const t_self &mat) const
	{
		t_self result;
		addMat<t_type, t_columns * t_rows>(_elements, mat._elements, result._elements);

		return result;
	}

	void operator += (const t_self &mat)
	{
		*this = *this + mat;
	}

	constexpr t_self operator - (const t_self &mat) const
	{
		t_self result;
		subMat<t_type, t_columns * t_rows>(_elements, mat._elements, result._elements);
		
		return result;
	}

	void operator -= (const t_self &mat)
	{
		*this = *this - mat;
	}

	template<typename t_scalarType>
	constexpr auto operator * (t_scalarType scalar) const
	{
		ASSERT_IS_ARITHMETIC(t_scalarType);
		using resultType = decltype(_elements[0] * scalar);

		matrix<resultType, t_columns, t_rows> result;
		scalarMulMat<t_type, t_scalarType, resultType, t_columns * t_rows>(_elements, scalar, result._elements);

		return result;
	}

	void operator *= (t_type scalar)
	{
		*this = *this * scalar;
	}

	template<typename t_scalarType>
	constexpr auto operator / (t_scalarType scalar) const
	{
		matrix<float, t_columns, t_rows> result;
		scalarMulDiv<t_type, t_scalarType, t_columns * t_rows>(_elements, scalar, result._elements);

		return result;
	}

private:
	// Use grid to access matrix values as a 2D array
	std::array<std::array<t_type, t_columns>, t_rows> _grid;

	// Use elements to access the matrix values as a linear array
	std::array<t_type, t_columns * t_rows> _elements;
};



namespace IF_TEST(test_matrix)
{

template<typename t_lType, typename t_rType, typename t_resultType, 
		 size_t t_lColumns, size_t t_lRows, size_t t_rColumns, 
		 ui32 t_resultColumnIndex, ui32 t_resultRowIndex, ui32 t_lColumnIndex = 0>
force_inline constexpr t_resultType mulMatCalcCell(const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
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
force_inline constexpr void mulMatCalcColumn(const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
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
force_inline constexpr void mulMat(const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
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
constexpr auto operator * (const matrix<t_lType, t_lColumns, t_lRows> &lMat, 
						   const matrix<t_rType, t_rColumns, t_lColumns> &rMat)
{
	using resultType = decltype(lMat.at(0, 0) * rMat.at(0, 0));

	matrix<resultType, t_rColumns, t_lColumns> result;
	mulMat(lMat, rMat, result);

	return result;
}


template<typename t_type, typename t_scalarType, int t_columns, int t_rows>
matrix<t_type, t_columns, t_rows> operator * (t_scalarType scalar, const matrix<t_type, t_columns, t_rows> &lMat)
{
	return lMat * scalar;
}

using mat2 = matrix<float, 2, 2>;
using mat3 = matrix<float, 3, 3>;
using mat4 = matrix<float, 4, 4>;

