#pragma once

#include "Attributes.hpp"

namespace meta
{

template<typename t_a, t_a t_value>
struct vConst { constexpr static t_a value = t_value; };


using vTrue = vConst<bool, true>;
using vFalse = vConst<bool, false>;


template<typename t_a, typename t_b>
struct isSame : vFalse {};

template<typename t_a>
struct isSame<t_a, t_a> : vTrue {};


template<typename t_a, typename ...>
struct areSame : vTrue {};

template<typename t_a, typename t_b, typename... t_others>
struct areSame<t_a, t_b, t_others...> : vConst<bool, isSame<t_a, t_b>::value && areSame<t_a, t_others...>::value> {};


template<typename t_a, typename t_b, typename = void>
struct isAssignable : vFalse {};

template<typename t_a, typename t_b>
struct isAssignable<t_a, t_b, decltype(std::declval<t_a>() = std::declval<t_b>(), void())> : vTrue {};


template<typename t_a>
struct isCopyAssignable : isAssignable<t_a, const t_a &> {};

}

