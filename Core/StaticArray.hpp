#pragma once

#include "Attributes.hpp"
#include "Log/Log.h"
#include "Meta.hpp"
#include "Types.hpp"

#include <iterator>

template<typename t_type, size_t t_size>
class StaticArray
{
public:
	
	class Iterator : public std::iterator<std::random_access_iterator_tag, t_type>
	{
	public:
		_nodiscard constexpr const t_type *operator->() const { _checkIndexBounds("derreference"); return _ptr + _index; }
		_nodiscard constexpr t_type *operator->() { _checkIndexBounds("derreference"); return _ptr + _index; }

		_nodiscard constexpr const t_type &operator*() const { return *operator->(); }
		_nodiscard constexpr t_type &operator*() { return *operator->(); }

		_nodiscard constexpr const t_type &operator[](ptrdiff_t offset) const { return *(*this + offset); }
		_nodiscard constexpr t_type &operator[](ptrdiff_t offset) { return *(*this + offset); }

		constexpr Iterator &operator++()
		{
			_checkIndexBounds("increment");
			++_index;
			return *this;
		}

		constexpr Iterator operator++(int)
		{
			Iterator temp = *this;
			++*this;
			return temp;
		}

		constexpr Iterator &operator--()
		{
			_checkIndexBounds("decrement");
			--_index;
			return *this;
		}

		constexpr Iterator operator--(int)
		{
			Iterator temp = *this;
			--*this;
			return temp;
		}

		constexpr Iterator &operator+=(ptrdiff_t offset)
		{
			_checkOffset(offset);
			_index += static_cast<size_t>(offset);
			return *this;
		}

		_nodiscard constexpr Iterator operator+(ptrdiff_t offset) const
		{
			_checkOffset(offset);
			return *this + offset;
		}

		constexpr Iterator &operator-=(ptrdiff_t offset)
		{
			_checkOffset(-offset);
			_index -= static_cast<size_t>(offset);
			return *this;
		}

		_nodiscard constexpr Iterator operator-(ptrdiff_t offset) const
		{
			_checkOffset(-offset);
			return *this - offset;
		}

		_nodiscard constexpr ptrdiff_t operator+(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			_checkOffset(other._index);
			return static_cast<ptrdiff_t>(_index + other._index);
		}

		_nodiscard constexpr ptrdiff_t operator-(const Iterator &other) const 
		{
			_checkIteratorCompatibility(other);
			_checkOffset(-other._index);
			return static_cast<ptrdiff_t>(_index - other._index);
		}

		_nodiscard constexpr bool operator==(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index == other._index;
		}

		_nodiscard constexpr bool operator!=(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index != other._index;
		}

		_nodiscard constexpr bool operator<(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index < other._index;
		}

		_nodiscard constexpr bool operator>(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index > other._index;
		}

		_nodiscard constexpr bool operator<=(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index <= other._index;
		}

		_nodiscard constexpr bool operator>=(const Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index >= other._index;
		}

	private:
		constexpr void _checkOffset(ptrdiff_t offset) const
		{
			if (offset != 0)
				assertFatal(_ptr != nullptr, "Cannot seek uninitialized iterator.");
			if (offset < 0)
				assertFatal(_index >= offset, "Cannot seek array iterator before begin.");
			if (offset > 0)
				assertFatal((t_size - _index) >= offset, "Cannot seek array iterator after end.");
		}

		constexpr void _checkIndexBounds(const char operationName[]) const
		{
			assertFatal(_index < t_size, "Trying to %s iterator out of the array's range.", operationName)
		}

		constexpr void _checkIteratorCompatibility(const Iterator other) const
		{
			assertFatal(_ptr == other._ptr, "Incompatible array iterators. Both iterators must belong to the same array.");
		}

		t_type *_ptr = nullptr;
		size_t _index = 0;
	};

	using ReverseIterator = std::reverse_iterator<Iterator>;

	StaticArray() = default;
	
	template<typename t_a, typename ...t_others>
	StaticArray(t_a first, t_others ...others)
		: _data{std::forward<t_a>(first), std::forward<t_others>(others)...}
	{
		static_assert(sizeof...(t_others) == (t_size - 1), "Trying to initialize a StaticArray with a different amount of elements than the ones defined.");
		static_assert(meta::areSame<t_a, t_others...>::value, "Trying to initialize array with elements of different types.");
	}

	_force_inline constexpr const t_type &operator[](u32 index) const 
	{ 
		assertFatal(index < t_size, "StaticArray out of bounds! You are trying to access index %d of an array of size %d", index, t_size);
		return _data[index]; 
	}

	_force_inline constexpr t_type &operator[](u32 index) 
	{ 
		assertFatal(index < t_size, "StaticArray out of bounds! You are trying to access index %d of an array of size %d", index, t_size);
		return _data[index]; 
	}

	_force_inline constexpr const t_type *data() const { return _data; }
	_force_inline constexpr t_type *data() { return _data; }
	_force_inline explicit constexpr operator const t_type *() { return _data; }
	_force_inline explicit constexpr operator t_type *() { return _data; }

	constexpr void fill(const t_type &value) { std::fill_n(_data, t_size, value); }

	_nodiscard constexpr const Iterator begin() const { return Iterator(_data, 0); }
	_nodiscard constexpr Iterator begin() { return Iterator(_data, 0); }
	_nodiscard constexpr const Iterator end() const { return Iterator(_data, t_size - 1); }
	_nodiscard constexpr Iterator end() { return Iterator(_data, t_size - 1);}

	_nodiscard constexpr const ReverseIterator rBegin() const { return ReverseIterator(end()); }
	_nodiscard constexpr ReverseIterator rBegin() { return ReverseIterator(end()); }
	_nodiscard constexpr const ReverseIterator rEnd() const { return ReverseIterator(begin()); }
	_nodiscard constexpr ReverseIterator rEnd() { return ReverseIterator(begin()); }

	_nodiscard constexpr t_type &front() { return _data[0]; }
	_nodiscard constexpr const t_type &front() const { return _data[0]; }
	_nodiscard constexpr t_type &back() { return _data[t_size -1]; }
	_nodiscard constexpr const t_type &back() const { return _data[t_size - 1]; }

	constexpr static size_t size = t_size;

private:
	t_type _data[t_size];
};

#undef CHECK_STATIC_ARRAY_ITERATOR(OpName) 
