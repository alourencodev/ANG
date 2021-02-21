#pragma once

#include "Attributes.hpp"
#include "Log/Log.h"
#include "Meta.hpp"
#include "Types.hpp"

template<typename t_type, size_t t_size>
class StaticArray
{
public:
	
	template<typename t_iterType>
	class _Iterator
	{
	public:
		using iterator_category = std::random_access_iterator_tag;
		using difference_type = ptrdiff_t;
		using value_type = t_type;
		using pointer = t_type *;
		using reference = t_type &;

		_Iterator(t_iterType *ptr) : _ptr(ptr), _index(0) {}
		_Iterator(t_iterType *ptr, size_t index) : _ptr(ptr), _index(index) {}

		_nodiscard constexpr const t_iterType *operator->() const { _checkIndexBounds("derreference"); return _ptr + _index; }
		_nodiscard constexpr t_iterType *operator->() { _checkIndexBounds("derreference"); return _ptr + _index; }

		_nodiscard constexpr const t_iterType &operator*() const { return *operator->(); }
		_nodiscard constexpr t_iterType &operator*() { return *operator->(); }

		_nodiscard constexpr const t_iterType &operator[](ptrdiff_t offset) const { return *(*this + offset); }
		_nodiscard constexpr t_iterType &operator[](ptrdiff_t offset) { return *(*this + offset); }

		constexpr _Iterator &operator++()
		{
			_checkIndexBounds("increment");
			++_index;
			return *this;
		}

		_nodiscard constexpr _Iterator operator++(int)
		{
			_Iterator temp = *this;
			++*this;
			return temp;
		}

		constexpr _Iterator &operator--()
		{
			_checkIndexBounds("decrement");
			--_index;
			return *this;
		}

		_nodiscard constexpr _Iterator operator--(int)
		{
			_Iterator temp = *this;
			--*this;
			return temp;
		}

		constexpr _Iterator &operator+=(ptrdiff_t offset)
		{
			_checkOffset(offset);
			_index += static_cast<size_t>(offset);
			return *this;
		}

		_nodiscard constexpr _Iterator operator+(ptrdiff_t offset) const
		{
			_checkOffset(offset);
			return *this + offset;
		}

		constexpr _Iterator &operator-=(ptrdiff_t offset)
		{
			_checkOffset(-offset);
			_index -= static_cast<size_t>(offset);
			return *this;
		}

		_nodiscard constexpr _Iterator operator-(ptrdiff_t offset) const
		{
			_checkOffset(-offset);
			return *this - offset;
		}

		_nodiscard constexpr ptrdiff_t operator+(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			_checkOffset(other._index);
			return static_cast<ptrdiff_t>(_index + other._index);
		}

		_nodiscard constexpr ptrdiff_t operator-(const _Iterator &other) const 
		{
			_checkIteratorCompatibility(other);
			_checkOffset(-other._index);
			return static_cast<ptrdiff_t>(_index - other._index);
		}

		_nodiscard constexpr bool operator==(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index == other._index;
		}

		_nodiscard constexpr bool operator!=(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index != other._index;
		}

		_nodiscard constexpr bool operator<(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index < other._index;
		}

		_nodiscard constexpr bool operator>(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index > other._index;
		}

		_nodiscard constexpr bool operator<=(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index <= other._index;
		}

		_nodiscard constexpr bool operator>=(const _Iterator &other) const
		{
			_checkIteratorCompatibility(other);
			return _index >= other._index;
		}

	private:
		constexpr void _checkOffset(ptrdiff_t offset) const
		{
			if (offset != 0)
				logAssertFatal(_ptr != nullptr, "Cannot seek uninitialized iterator.");
			if (offset < 0)
				logAssertFatal(_index >= offset, "Cannot seek array iterator before begin.");
			if (offset > 0)
				logAssertFatal((t_size - _index) >= offset, "Cannot seek array iterator after end.");
		}

		constexpr void _checkIndexBounds(const char operationName[]) const
		{
			logAssertFatal(_index < t_size, "Trying to %s iterator out of the array's range.", operationName)
		}

		constexpr void _checkIteratorCompatibility(const _Iterator other) const
		{
			logAssertFatal(_ptr == other._ptr, "Incompatible array iterators. Both iterators must belong to the same array.");
		}

		t_iterType *_ptr = nullptr;
		size_t _index = 0;
	};

	using Iterator = _Iterator<t_type>;
	using ConstIterator = _Iterator<const t_type>;

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

	constexpr void fill(const t_type &value) { std::fill_n(_data, t_size, value); } // TODO: Use memset?

	_nodiscard constexpr Iterator begin() { return Iterator(_data, 0); }
	_nodiscard constexpr const ConstIterator begin() const { return ConstIterator(_data, 0); }
	_nodiscard constexpr Iterator end() { return Iterator(_data, t_size - 1); }
	_nodiscard constexpr const ConstIterator end() const { return ConstIterator(_data, t_size - 1); }

	_nodiscard constexpr t_type &front() { return _data[0]; }
	_nodiscard constexpr const t_type &front() const { return _data[0]; }
	_nodiscard constexpr t_type &back() { return _data[t_size -1]; }
	_nodiscard constexpr const t_type &back() const { return _data[t_size - 1]; }

	constexpr static size_t size = t_size;

private:
	t_type _data[t_size];
};

