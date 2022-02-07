#pragma once

#include <Core/HashMap.hpp>

#include <algorithm>

namespace age
{

template<typename t_first, typename t_second>
struct Pair
{
	t_first first;
	t_second second;

	bool operator == (const Pair &other) const { return first == other.first && second == other.second; }
	bool operator != (const Pair &other) const { return first != other.first || second != other.second; }
};

namespace
{

template<typename t_first, typename t_second>
class PairHashMapBehavior
{
public:
	bool isEqual(const Pair<t_first, t_second> &a, const Pair<t_first, t_second> &b) const { return a == b; }

	size_t hash(const Pair<t_first, t_second> &value) const 
	{ 
		return std::hash<t_first>(value.first) ^ std::hash<t_second>(value.second); 
	}
};

}


}
