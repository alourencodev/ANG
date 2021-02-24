#pragma once

#include "Attributes.hpp"
#include "Log/Log.h"
#include "Meta.hpp"
#include "Types.hpp"

template<typename t_type, size_t t_size>
class StaticArray
{
public:
	using Iterator = t_type *;
	using ConstIterator = const t_type *;

	StaticArray() = default;
	StaticArray(const t_type &value) { fill(value); }

	template<typename t_a, typename ...t_others>
	StaticArray(t_a first, t_others ...others)
		: _data{std::forward<t_a>(first), std::forward<t_others>(others)...}
	{
		static_assert(sizeof...(t_others) == (t_size - 1), "Trying to initialize a StaticArray with a different amount of elements than the ones defined.");
		static_assert(meta::areSame<t_a, t_others...>::value, "Trying to initialize array with elements of different types.");
	}

	StaticArray(const StaticArray &other) = delete;

	_force_inline constexpr const t_type &operator[](size_t index) const 
	{ 
		logAssertFatal(index < t_size, "StaticArray out of bounds! You are trying to access index %d of an array of size %d", index, t_size);
		return _data[index]; 
	}

	_force_inline constexpr t_type &operator[](size_t index) 
	{ 
		logAssertFatal(index < t_size, "StaticArray out of bounds! You are trying to access index %d of an array of size %d", index, t_size);
		return _data[index]; 
	}

	_force_inline constexpr const t_type *data() const { return _data; }
	_force_inline constexpr t_type *data() { return _data; }
	_force_inline explicit constexpr operator const t_type *() { return _data; }
	_force_inline explicit constexpr operator t_type *() { return _data; }

	constexpr void fill(const t_type &value) { std::fill_n(_data, t_size, value); }

	_nodiscard constexpr Iterator begin() { return _data; }
	_nodiscard constexpr const ConstIterator begin() const { return _data; }
	_nodiscard constexpr Iterator end() { return _data + lastIndex; }
	_nodiscard constexpr const ConstIterator end() const { return _data + lastIndex; }

	_nodiscard constexpr t_type &front() { return _data[0]; }
	_nodiscard constexpr const t_type &front() const { return _data[0]; }
	_nodiscard constexpr t_type &back() { return _data[lastIndex]; }
	_nodiscard constexpr const t_type &back() const { return _data[lastIndex]; }

	const t_type *find(const t_type &value) const
	{
		for (int i = 0; i < t_size; i++) {
			if (_data[i] == value) return &_data[i];
		}

		return end();
	}

	t_type *find(const t_type &value)
	{
		for (int i = 0; i < t_size; i++) {
			if (_data[i] == value) return &_data[i];
		}

		return end();
	}

	const t_type *findBackwards(const t_type &value) const
	{
		for (int i = lastIndex; i >= 0; i--) {
			if (_data[i] == value) return &_data[i];
		}

		return end();
	}

	t_type *findBackwards(const t_type &value)
	{
		for (int i = lastIndex; i >= 0; i--) {
			if (_data[i] == value) return &_data[i];
		}

		return end();
	}

	const bool contains(const t_type &value) const { return find(value) != end(); }
	const bool containsBackwards(const t_type &value) const { return findBackwards(value) != end(); }

	constexpr static size_t size = t_size;
	constexpr static size_t lastIndex = size - 1;
	
private:
	t_type _data[t_size];
};

template<typename t_type, size_t t_size>
static std::ostream &operator<<(std::ostream &os, const StaticArray<t_type, t_size> &array)
{
	os << "[";
	for (int i = 0; i < array.lastIndex; i++)
		os << array[i] << ", ";

	os << array.back() << "]";

	return os;
}

template<typename t_type, size_t t_size>
static std::istream &operator>>(std::istream &is, StaticArray<t_type, t_size> &array)
{
	for (int i = 0; i < array.size; i++)
		is >> array[i];

	return is;
}
