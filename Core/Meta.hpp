#pragma once

#include "Attributes.hpp"
#include "BuildScheme.hpp"
#include "Types.hpp"

namespace meta
{

/**
@brief	Templated class that defines a constant value to be used in meta programming.
**/
template<typename t_type, t_type t_value>
struct vConst { constexpr static t_type value = t_value; };


/**
@brief	Boolean metaprogramming constants
**/
using vTrue = vConst<bool, true>;
using vFalse = vConst<bool, false>;


/**
@brief	Value is true if type is a class
**/
template<typename t_type>
struct isClass : vConst<bool, __is_class(t_type)> {};


/**
@brief	Value is true if both types are the same
**/
template<typename t_type, typename t_b>
struct isSame : vFalse {};

template<typename t_type>
struct isSame<t_type, t_type> : vTrue {};


/**
@brief	Value is true if every value is the same
**/
template<typename t_type, typename ...>
struct areSame : vTrue {};

template<typename t_a, typename t_b, typename... t_others>
struct areSame<t_a, t_b, t_others...> : vConst<bool, isSame<t_a, t_b>::value && areSame<t_a, t_others...>::value> {};


/**
@brief	Value is true if given type is the same as one of the other types
**/
template<typename t_type, typename ...>
struct isAnyOf : vFalse {};

template<typename t_type, typename t_a, typename... t_others> 
struct isAnyOf<t_type, t_a, t_others...> : vConst<bool, isSame<t_type, t_a>::value || isAnyOf<t_type, t_others...>::value> {};


/**
@brief	Value is true if you can assign a variable of type t_b to a variable of t_a.
**/
template<typename t_a, typename t_b, typename = void>
struct isAssignable : vFalse {};

template<typename t_a, typename t_b>
struct isAssignable<t_a, t_b, decltype(std::declval<t_a>() = std::declval<t_b>(), void())> : vTrue {};


/**
@brief	Value is true if t_a allows to copy through assignment operator.
**/
template<typename t_type>
struct isCopyAssignable : isAssignable<t_type, const t_type &> {};


/**
@brief	Value is true if type has a defined copy constructor
**/
template<typename t_type>
struct isCopyConstructible : vConst<bool, __is_constructible(t_type, const t_type &)> {};


/**
@brief	Value is true if type is not a class or if it has both copy constructor and copy operator defined.
**/
template<typename t_type>
struct isCopyable : vConst<bool, !isClass<t_type>::value || (isCopyConstructible<t_type>::value && isCopyAssignable<t_type>::value)> {};


/**
@brief	Value is true if type is comparable with equal operator.
**/
template<typename t_type, typename = void>
struct isEqualComparable : vFalse {};

template<typename t_type>
struct isEqualComparable<t_type, decltype(std::declval<t_type>() == std::declval<t_type>(), void())> : vTrue {};


/**
@brief	Gives the base type of a given type, without reference const or pointer.
**/
template<typename t_type>
struct baseType { using type = t_type; };

template<typename t_type>
struct baseType<t_type &> { using type = t_type; };

template<typename t_type>
struct baseType<t_type &&> { using type = t_type; };

template<typename t_type>
struct baseType<t_type *> { using type = t_type; };

template<typename t_type>
struct baseType<const t_type> { using type = t_type; };

template<typename t_type>
struct baseType<const t_type &> { using type = t_type; };

template<typename t_type>
struct baseType<const t_type *> { using type = t_type; };


/**
@brief	Value is true if t_a and t_b have both the same base value
**/
template<typename t_a, typename t_b>
struct isSameBaseType : isSame<typename baseType<t_a>::type, typename baseType<t_b>::type> {};


/**
@brief	Value is true if given type is integral
**/
template<typename t_type>
struct isIntegral : isAnyOf<t_type, i8, i16, i32, i64, u8, u16, u32, u64> {};


/**
@brief	Value is true if given type is floating point
**/
template<typename t_type>
struct isFloatingPoint : isAnyOf<t_type, f32, f64> {};


/**
brief	Value is true if given type is arithmetic
**/
template<typename t_type>
struct isArithmetic : vConst<bool, isIntegral<t_type>::value || isFloatingPoint<t_type>::value> {};


#ifdef _DEBUG
struct isDebugBuild : vTrue {};
#else
struct isDebugBuild : vFalse {};
#endif

}

