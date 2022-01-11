#pragma once

#include <iostream>

#include "Core/Attributes.hpp"
#include "Core/Log/Assert.hpp"
#include "Core/Meta.hpp"
#include "Core/Range.hpp"
#include "Core/Types.hpp"

#include <array>

namespace age
{

template<typename t_type, size_t t_size>
class SArray
{
public:
	using Iterator = t_type*;
	using ConstIterator = const t_type*;

	SArray() = default;
	SArray(const t_type& value) { fill(value); }
	SArray(const SArray& other) { memcpy(_data, other._data, sizeof(t_type) * t_size); }
	SArray(const Range<t_type>& range) { memcpy(_data, range.daya(), sizeof(t_type) * t_size); }

	template<typename t_type, typename ...t_others>
	SArray(t_type first, t_others ...others) : _data{ std::forward<t_type>(first), std::forward<t_others>(others)... }
	{
		static_assert(sizeof...(t_others) == (t_size - 1), "Trying to initialize a StaticArray with a different amount of elements than the ones defined.");
		static_assert(meta::areSame<t_type, t_others...>::value, "Trying to initialize array with elements of different types.");
	}

	_force_inline const t_type& operator[](size_t index) const
	{
		age_assertFatal(index < t_size, "StaticArray out of bounds! You are trying to access index %d of an array of size %d", index, t_size);
		return _data[index];
	}

	_force_inline t_type& operator[](size_t index)
	{
		age_assertFatal(index < t_size, "StaticArray out of bounds! You are trying to access index %d of an array of size %d", index, t_size);
		return _data[index];
	}

	// Type casting
	_force_inline const t_type* data() const { return _data; }
	_force_inline t_type* data() { return _data; }
	_force_inline explicit constexpr operator const t_type* () { return _data; }
	_force_inline explicit constexpr operator t_type* () { return _data; }

	_force_inline operator Range<t_type>() { return Range<t_type>(_data, t_size); }
	_force_inline operator Range<const t_type>() const { return Range<const t_type>(_data, t_size); }

	// Iterator
	Iterator begin() { return _data; }
	const ConstIterator begin() const { return _data; }
	Iterator end() { return _data + t_size; }
	const ConstIterator end() const { return _data + t_size; }

	t_type& front() { return _data[0]; }
	const t_type& front() const { return _data[0]; }
	t_type& back() { return _data[lastIndex()]; }
	const t_type& back() const { return _data[lastIndex()]; }

	void fill(const t_type& value) { std::fill_n(_data, t_size, value); }

	const t_type* find(const t_type& value) const { return Range<const t_type>(_data, t_size).find(value); }
	t_type* find(const t_type& value) { return Range<t_type>(_data, t_size).find(value); }

	const t_type* findBackwards(const t_type& value) const { return Range<const t_type>(_data, t_size).findBackwards(value); }
	t_type* findBackwards(const t_type& value) { return Range<t_type>(_data, t_size).findBackwards(value); }

	i64 indexOf(const t_type& element) const { return Range<t_type>(_data, t_size).indexOf(element); }
	i64 indexOfBackwards(const t_type& element) const { return Range<t_type>(_data, t_size).indexOfBackwards(element); }

	const bool contains(const t_type& value) const { return find(value) != end(); }
	const bool containsBackwards(const t_type& value) const { return findBackwards(value) != end(); }

	constexpr size_t size() const { return t_size; }
	constexpr size_t lastIndex() const { return t_size - 1; }

private:
	t_type _data[t_size];
};

template<typename t_type, size_t t_size>
static std::ostream& operator<<(std::ostream& os, const SArray<t_type, t_size>& array)
{
	Range<const t_type> range(array.data(), t_size);
	os << range;
	return os;
}

template<typename t_type, size_t t_size>
static std::istream& operator>>(std::istream& is, SArray<t_type, t_size>& array)
{
	is >> Range<t_type>(array.data(), t_size);
	return is;
}

}	// namespace age

