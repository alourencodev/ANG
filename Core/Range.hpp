#pragma once

#include <algorithm>
#include <iostream>

#include "Core/Attributes.hpp"
#include "Core/Log/Assert.hpp"
#include "Core/Meta.hpp"
#include "Core/Types.hpp"

template<typename t_type>
class Range
{
public:
	using Iterator = t_type *;
	using ConstIterator = const t_type *;

	Range(t_type *data, size_t count) : _data(data), _count(count) {}
	Range(Range && other) : Range(other._data, other._count) {}
	Range(const Range &other) : Range(other._data, other._count) {}

	_force_inline const t_type &operator[](u32 index) const
	{
		age_assertFatal(index < _count, "Range out of bounds! Trying to access index %d of Range with size %d", index, _count);
		return _data[index];
	}

	_force_inline t_type &operator[](u32 index)
	{
		age_assertFatal(index < _count, "Range out of bounds! Trying to access index %d of Range with size %d", index, _count);
		return _data[index];
	}

	_force_inline const t_type *data() const { return _data; }
	_force_inline const size_t count() const { return _count; }
	_force_inline size_t lastIndex() const { return _count - 1; }

	_force_inline explicit operator const t_type *() const { return _data; }
	_force_inline explicit operator t_type *() { return _data; }

	Iterator begin() { return _data; }
	ConstIterator begin() const { return _data; }
	Iterator end() { return _data + _count; }
	ConstIterator end() const { return _data + _count; }

	t_type &front() { return _data[0]; }
	const t_type &front() const { return _data[0]; }
	t_type &back() { return _data[lastIndex()]; }
	const t_type &back() const { return _data[lastIndex()]; }

	void fill(const t_type &value) { std::fill_n(_data, _count, value); }

	_force_inline const t_type *find(const t_type &value) const
	{
		static_assert(meta::isEqualComparable<t_type>::value, "Can't call find, if Range type is not equal comparable.");
		for (u32 i = 0; i < _count; i++) {
			if (_data[i] == value)
				return &_data[i];
		}

		return end();
	}

	_force_inline t_type *find(const t_type &value)
	{
		static_assert(meta::isEqualComparable<t_type>::value, "Can't call find, if Range type is not equal comparable.");
		for (u32 i = 0; i < _count; i++) {
			if (_data[i] == value)
				return &_data[i];
		}

		return end();
	}

	_force_inline const t_type *findBackwards(const t_type &value) const
	{
		static_assert(meta::isEqualComparable<t_type>::value, "Can't call findBackwards, if Range type is not equal comparable.");
		for (i32 i = static_cast<i32>(lastIndex()); i >= 0; i--) {
			if (_data[i] == value) 
				return &_data[i];
		}

		return end();
	}

	_force_inline t_type *findBackwards(const t_type &value)
	{
		static_assert(meta::isEqualComparable<t_type>::value, "Can't call findBackwards, if Range type is not equal comparable.");
		for (i32 i = static_cast<i32>(lastIndex()); i >= 0; i--) {
			if (_data[i] == value) 
				return &_data[i];
		}

		return end();
	}

	_force_inline i64 indexOf(const t_type &element) const 
	{ 
		static_assert(meta::isEqualComparable<t_type>::value, "Can't call indexOf, if Rnge type is not equal comparable");

		for (i64 i = 0; i < static_cast<i64>(_count); i++) {
			if (_data[i] == element) 
				return i;
		}

		return -1;
	}

	_force_inline i64 indexOfBackwards(const t_type &element) const
	{
		static_assert(meta::isEqualComparable<t_type>::value, "Can't call indexOfBackwards, if Rnge type is not equal comparable");

		for (i64 i = lastIndex(); i >= 0; i--) {
			if (_data[i] == element) 
				return i;
		}

		return -1;
	}

	_force_inline bool contains(const t_type &value) const { return find(value) != end(); }
	_force_inline bool containsBackwards(const t_type &value) const { return findBackwards(value) != end(); }

private:
	t_type * const _data = nullptr;
	const size_t _count = 0;
};

template<typename t_type>
static std::ostream &operator<<(std::ostream &os, const Range<const t_type> &range)
{
	os << "[";
	for (u32 i = 0; i < range.lastIndex(); i++)
		os << range[i] << ", ";

	os << range.back() << "]";

	return os;
}

template<typename t_type>
static std::istream &operator>>(std::istream &is, Range<t_type> &range)
{
	for (u32 i = 0; i < range.count(); i++)
		is >> range[i];

	return is;
}

template<typename t_type>
static std::istream &operator>>(std::istream &is, Range<t_type> &&range)
{
	for (u32 i = 0; i < range.count(); i++)
		is >> range[i];

	return is;
}
