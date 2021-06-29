#pragma once

#include "Attributes.hpp"
#include "Log/Assert.hpp"


template<typename t_type, size_t t_maxSize>
class StackArray
{
	constexpr static char k_tag[] = "StackArray";

public:
	using Iterator = t_type *;	
	using ConstIterator = const t_type *;

	StackArray() = default;
	template<size_t t_otherMaxSize>
	StackArray(const StackArray<t_type, t_otherMaxSize> &other)
	{
		static_assert(t_otherMaxSize < t_maxSize, "Source array's max size must be greater or equal to the destination array's mac size.");
		memcpy(_data, other._data, t_maxSize * sizeof(t_type));
		_count = other._count;
	}

	StackArray(StackArray &&other) = delete;

	StackArray(std::initializer_list<t_type> &&list)
	{
		g_assertFatal(list.size() <= t_maxSize, "Trying to initiaize StackArray with an Initializer List bigger than the array size %d", t_maxSize);
		memcpy(_data, list.begin(), list.size() * sizeof(t_type));
		_count = list.size();
	}

	_force_inline const t_type &operator[](size_t index) const
	{
		g_assertFatal(index < _count, "Trying to access index %d of a DArray with count %d.", index, _count);
		return _data[index];
	}

	_force_inline t_type &operator[](size_t index)
	{
		g_assertFatal(index < _count, "Trying to access index %d of a DArray with count %d.", index, _count);
		return _data[index];
	}

	// Iterator
	/**
	@brief	Returns iterator pointing at the first element
	**/
	Iterator begin()					{ return _data; }
	const ConstIterator begin() const	{ return _data; }


	/**
	@brief	Returns iterator for a null element after the last element
	**/
	Iterator end()					{ return _data + _count; }
	const ConstIterator end() const { return _data + _count; }


	/**
	@brief	Returns iterator pointing at the first element
	**/
	t_type &front()				{ return _data[0]; }
	const t_type &front() const { return _data[0]; }


	/**
	@brief	Returns iterator pointing at the last element
	**/
	t_type &back()				{ return _data[lastIndex()]; }
	const t_type &back() const	{ return _data[lastIndex()]; }

	_force_inline void clear() { _count = 0; }

	_force_inline size_t count() const { return _count; }
	_force_inline constexpr size_t capacity() const { return t_maxSize; }
	_force_inline size_t lastIndex() const { return _count - 1;}
	_force_inline bool isEmpty() const { return _count == 0; }


	_force_inline const t_type *data() const { return _data; }
	_force_inline t_type *data() { return _data; }

	_force_inline explicit operator const t_type *() { return _data; }
	_force_inline explicit operator t_type *() { return _data; }

	_force_inline void add(const t_type *ptr, size_t count)
	{
		g_assertFatal((_count + count) < t_maxSize, "StackArray overflow. Array can only take %d elements", t_maxSize);
		memcpy(&_data[_count], ptr, count * sizeof(t_type));
		_count += count;
	}

	template<size_t t_otherMaxSize>
	void add(const StackArray<t_type, t_otherMaxSize> &stackArray)	{ add(stackArray._data, stackArray._count); }
	void add(std::initializer_list<t_type> list)					{ add(list.begin(), list.size()); }
	void add(const t_type &element)									{ add(&element, 1); }

	void add(t_type &&element)
	{
		g_assertFatal(_count < t_maxSize, "StackArray overflow. Array can only take %d elements", t_maxSize);
		_data[_count] = std::move(element);
		_count++;
	}

	void addEmpty(size_t count = 1)
	{
		g_assertFatal(count > 0, "Cannot add 0 empty elements.");
		g_assertFatal((_count + count) < t_maxSize, "StackArray overflow. Array can only take %d elements", t_maxSize);
		_count += count;
	}

	/**
	@brief	Add element to the given index. It allcates memory if necessary
	**/
	_force_inline void insert(const t_type *ptr, size_t count, size_t index)
	{
		g_assertFatal(index < _count, "Trying to insert element in index %d of a DArray with only %d elements.", index, _count);
		g_assertFatal((_count + count) < t_maxSize, "StackArray overflow. Array can only take %d elements", t_maxSize);

		const size_t movingChunkSize = _count - index;
		t_type *insertionPtr = _data + index;

		memcpy(insertionPtr + count, insertionPtr, movingChunkSize * sizeof(t_type));
		memcpy(insertionPtr, ptr, count * sizeof(t_type));

		_count += count;
	}

	template<size_t t_otherMaxSize>
	void insert(const StackArray<t_type, t_otherMaxSize> &stackArray, size_t index) { insert(stackArray._data, stackArray._count, index); }
	void insert(std::initializer_list<t_type> list, size_t index)					{ insert(list.begin(), list.size(), index); }
	void insert(const t_type &element, size_t index)								{ insert(&element, 1, index); }

	void insert(t_type &&element, size_t index)
	{
		g_assertFatal(index < _count, "Trying to insert element in index %d of a DArray with only %d elements.", index, _count);
		g_assertFatal((_count + 1) < t_maxSize, "StackArray overflow. Array can only take %d elements", t_maxSize);

		const size_t movingChunkSize = _count - index;
		t_type *insertionPtr = _data + index;

		memcpy(insertionPtr + 1, insertionPtr, movingChunkSize * sizeof(t_type));
		_data[index] = std::move(element);
		_count++;
	}


	/**
	@brief	Returns a pointer to the first element that is equal to the given element.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	t_type *find(const t_type &element)
	{
		static_assert(meta::isEqualComparable<t_type>::value, "It is not possible to search for a type that doesn't have the equal operator defined.");

		for (size_t i = 0; i < _count; i++) {
			if (_data[i] == element) 
				return _data + i;
		}

		return end();
	}

	const t_type *find(const t_type &element) const
	{
		static_assert(meta::isEqualComparable<t_type>::value, "It is not possible to search for a type that doesn't have the equal operator defined.");

		for (size_t i = 0; i < _count; i++) {
			if (_data[i] == element) 
				return _data + i;
		}

		return end();
	}


	/**
	@brief	Returns a pointer to the first element that is equal to the given element, by searching from the end to the beginning.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	t_type *findBackwards(const t_type &element)
	{
		static_assert(meta::isEqualComparable<t_type>::value, "It is not possible to search for a type that doesn't have the equal operator defined.");

		for (i64 i = lastIndex(); i >= 0; i--) {
			if (_data[i] == element) 
				return _data + i;
		}

		return end();
	}

	const t_type *findBackwards(const t_type &element) const
	{
		static_assert(meta::isEqualComparable<t_type>::value, "It is not possible to search for a type that doesn't have the equal operator defined.");

		for (i64 i = lastIndex(); i >= 0; i--) {
			if (_data[i] == element) 
				return _data + i;
		}

		return end();
	}


	/**
	@brief	Returns the index in the array for first element that is equal to the given element.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	i64 indexOf(const t_type &element) const 
	{ 
		static_assert(meta::isEqualComparable<t_type>::value, "It is not possible to search for a type that doesn't have the equal operator defined.");

		for (i64 i = 0; i < static_cast<i64>(_count); i++) {
			if (_data[i] == element) 
				return i;
		}

		return -1;
	}


	/**
	@brief	Returns the index in the array for first element that is equal to the given element, by searching from the end to the beginning.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	i64 indexOfBackwards(const t_type &element) const
	{
		static_assert(meta::isEqualComparable<t_type>::value, "It is not possible to search for a type that doesn't have the equal operator defined.");

		for (i64 i = lastIndex(); i >= 0; i--) {
			if (_data[i] == element) 
				return i;
		}

		return -1;
	}


	/**
	@brief	Returns true if the array contains an element equal to the given element.
	**/
	bool contains(const t_type &element) const { return find(element) != end(); }

	/**
	@brief	Returns true if the array contains an element equal to the given element, by searching from the end to the beginning.
	**/
	bool containsBackwards(const t_type &element) const { return findBackwards(element) != end(); }


	/**
	@brief	Removes the element in the given index.
			Does not preserve the order of the elements.
	**/
	void swapPopIndex(size_t index) 
	{ 
		g_assertFatal(index < _count, "Trying to remove element in index %d from a dynamic array with count %d.", index, _count);
		_swapPopPtr(_data + index); 
	}

	/**
	@brief	Removes the element equal to the given element.
			Returns true if successful.
			Does not preserve the order of the elements.
	**/
	bool swapPop(const t_type &element)
	{
		t_type *ptr = find(element);
		if (ptr == end())
			return false;

		_swapPopPtr(ptr);
		return true;
	}

	/**
	@brief	Removes the last element of the array
	**/
	_force_inline void pop() 
	{ 
		g_assertFatal(_count > 0, "Trying to pop an element of an empty array.");
		_count--; 
	}


	/**
	@brief	Removes the element in the given index.
	**/
	void removeIndex(size_t index)
	{
		g_assertFatal(index < _count, "Trying to remove index %d from a dynamic array with count %d.", index, _count);

		if (index == lastIndex()) {
			_count--;
			return;
		}

		const size_t movingChunkSize = lastIndex() - index;
		t_type *dstPtr = _data + index;
		memcpy(dstPtr, dstPtr + 1, movingChunkSize * sizeof(t_type));
		_count--;
	}

	/**
	@brief	removes the first element equal to the given element
	**/
	bool remove(const t_type &element)
	{
		i64 index = indexOf(element);
		if (index < 0)
			return false;

		removeIndex(index);
		return true;
	}


private:
	_force_inline void _swapPopPtr(t_type *ptr)
	{
		if (ptr != &back())
			memcpy(ptr, &back(), sizeof(t_type));

		_count--;
	}

	t_type _data[t_maxSize];
	size_t _count = 0;
};


template<typename t_type, size_t t_maxSize>
static std::ostream &operator<<(std::ostream &os, const StackArray<t_type, t_maxSize> &array)
{
	os << "[";
	for (i64 i = 0; i < static_cast<i64>(array.lastIndex()); i++)
		os << array[i] << ", ";

	os << array.back() << "]";

	return os;
}


template<typename t_type, size_t t_maxSize>
static std::istream &operator>>(std::istream &is, StackArray<t_type, t_maxSize> &array)
{
	is.seekg(0, is.end);
	const size_t length = is.tellg();
	is.seekg(0, is.beg);

	size_t i = 0;
	while (static_cast<size_t>(is.tellg()) < length) {
		array.addEmpty();
		is >> array[i++];
	}

	return is;
}
