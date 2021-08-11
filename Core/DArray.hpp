#pragma once

#include <iostream>

#include "Core/Log/Log.h"
#include "Core/Math/Math.hpp"
#include "Core/Memory/Allocator.hpp"
#include "Core/SArray.hpp"
#include "Core/Range.hpp"

namespace age
{

template<typename t_type, class t_allocator = DefaultHeapAllocator<t_type>>
class DArray
{
	constexpr static char k_tag[] = "DArray";

public:
	using Iterator = t_type *;
	using ConstIterator = const t_type *;


	DArray() = default;

	DArray(size_t capacity) 
	{ 
		_data = t_allocator::alloc(capacity); 
		_capacity = capacity;
	}

	DArray(const Range<t_type> &range) : DArray(range.count())
	{
		static_assert(meta::isCopyable<t_type>::value, "Trying to copy DArray of non copyable type.");

		memcpy(_data, range.data(), range.count() * sizeof(t_type));
		_count = _capacity;
	}

	template<size_t t_size>
	DArray(const SArray<t_type, t_size> &array)
	{
	  resize(t_size);
	  memcpy(_data, array.data(), t_size * sizeof(t_type));
	  _count = t_size;
	}

	DArray(std::initializer_list<t_type> &&list) : DArray(list.size())
	{
		memcpy(_data, list.begin(), list.size() * sizeof(t_type));
		_count = _capacity;
	}

	DArray(const DArray &other) : DArray(other._capacity)
	{
		static_assert(meta::isCopyable<t_type>::value, "Trying to copy DArray of non copyable type.");

		memcpy(_data, other._data, _capacity * sizeof(t_type));
		_count = other._count;
	}

	DArray(DArray &&other)
	{
		t_allocator::dealloc(_data);

		_data = other._data;
		_capacity = other._capacity;
		_count = other._count;

		other._data = nullptr;
		other._capacity = 0;
		other._count = 0;
	}

	~DArray() { t_allocator::dealloc(_data); }


	void operator = (const DArray<t_type, t_allocator> &other)
	{
		static_assert(meta::isCopyable<t_type>::value, "Trying to copy DArray of non copyable type.");

		if (!t_allocator::realloc(&_data, other._capacity))
			age_error(k_tag, "Unable to reallocate memory during copy assignment.");

		_capacity = other._capacity;
		_count = other._count;
		memcpy(_data, other._data, _capacity * sizeof(t_type));
	}

	void operator = (DArray<t_type, t_allocator> &&other)
	{
		t_allocator::dealloc(_data);

		_data = other._data;
		_capacity = other._capacity;
		_count = other._count;

		other._data = nullptr;
		other._capacity = 0;
		other._count = 0;
	}

	/**
	@brief	Number of elements in the array.
	**/
	size_t count() const { return _count; }

	/**
	@brief	Number of elements the array has allocated memory to support.
	**/
	size_t capacity() const { return _capacity; }

	/**
	@brief	Last valid index.
			If array is empty, this has undefined behavior
	**/
	size_t lastIndex() const { return _count - 1;}


	bool isEmpty() const { return _count == 0; }

	/**
	@brief	Reserve a given number of slots, additionally to the current capacity.
	**/
	void reserve(size_t slotCount) { resize(_capacity + slotCount); }

	/**
	@brief	Frees the memory that is currently empty.
	**/
	void shrinkToFit() { resize(_count); }

	/**
	@brief	Empties the entire array. Keeps memory allocated.
	**/
	void clear() { _count = 0; }

	/**
	@brief	Empties array and frees allocated memory.
	**/
	void free() { resize(0); }

	/**
	@brief	Sets the array's capacity to the given value. Allocates or Frees memory if necessary
	**/
	void resize(size_t capacity)
	{
		if(!t_allocator::realloc(&_data, capacity)) {
			t_type *newPtr = t_allocator::alloc(capacity);
			memcpy(newPtr, _data, _capacity * sizeof(t_type));
			t_allocator::dealloc(_data);
			_data = newPtr;
		}

		_capacity = capacity;
		_count = std::min(_count, _capacity);
	}


	/**
	@brief	Returns pointer for the first element
	**/
	_force_inline const t_type *data() const { return _data; }
	_force_inline t_type *data() { return _data; }

	_force_inline explicit operator const t_type *() { return _data; }
	_force_inline explicit operator t_type *() { return _data; }

	_force_inline operator Range<t_type> () { return Range<t_type>(_data, _count); }
	_force_inline operator Range<const t_type> () const { return Range<const t_type>(_data, _count); }


	_force_inline const t_type &operator[](size_t index) const
	{
		age_assertFatal(index < _count, "Trying to access index %d of a DArray with count %d.", index, _count);
		return _data[index];
	}

	_force_inline t_type &operator[](size_t index)
	{
		age_assertFatal(index < _count, "Trying to access index %d of a DArray with count %d.", index, _count);
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


	/**
	@brief	Add element to the end of the array. Allocates memory if necessary.
	**/
	_force_inline void add(const t_type *ptr, size_t count)
	{
		static_assert(meta::isCopyable<t_type>::value, "Trying to add non copyable type into DArray.");
		_add(ptr, count);
	}

	void add(const Range<t_type> &range)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to add non copyable type into DArray.");
		_add(range.data(), range.count()); 
	}

	void add(const Range<const t_type> &range)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to add non copyable type into DArray.");
		_add(range.data(), range.count()); 
	}

	template<size_t t_size>
	void add(const SArray<t_type, t_size> &array)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to add non copyable type into DArray.");
		_add(array.data(), t_size); 
	}

	template<size_t t_size>
	void add(const SArray<const t_type, t_size> &array)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to add non copyable type into DArray.");
		_add(array.data(), t_size); 
	}

	void add(const t_type &element)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to add non copyable type into DArray.");
		_add(&element, 1); 
	}

	void add(std::initializer_list<t_type> list) { _add(list.begin(), list.size()); }
	void add(t_type &&element)
	{
		_reserveIfNotEnoughSize();
		_data[_count] = std::move(element);
		_count++;
	}


	/**
	@brief	Add an empty element to the end of the array. Allocates memory if necessary.
	**/
	void addEmpty(size_t count = 1)
	{
		age_assertFatal(count > 0, "Cannot add 0 empty elements.");
		_reserveIfNotEnoughSize(count);
		_count += count;
	}


	/**
	@brief	Add element to the given index. It allcates memory if necessary
	**/
	_force_inline void insert(const t_type *ptr, size_t count, size_t index)
	{
		static_assert(meta::isCopyable<t_type>::value, "Trying to insert non copyable type into DArray.");
		_insert(ptr, count, index);
	}

	void insert(const Range<t_type> &range, size_t index)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to insert non copyable type into DArray.");
		_insert(range.data(), range.count(), index); 
	}

	void insert(const t_type &element, size_t index)
	{ 
		static_assert(meta::isCopyable<t_type>::value, "Trying to insert non copyable type into DArray.");
		_insert(&element, 1, index); 
	}

	void insert(std::initializer_list<t_type> list, size_t index) { _insert(list.begin(), list.size(), index); }
	void insert(t_type &&element, size_t index)
	{
		age_assertFatal(index < _count, "Trying to insert element in index %d of a DArray with only %d elements.", index, _count);
		_reserveIfNotEnoughSize();
		
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
	const t_type *find(const t_type &element) const { return Range<t_type>(_data, _count).find(element); }
	t_type *find(const t_type &element) { return Range<t_type>(_data, _count).find(element); }


	/**
	@brief	Returns a pointer to the first element that is equal to the given element, by searching from the end to the beginning.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	const t_type *findBackwards(const t_type &element) const { return Range<t_type>(_data, _count).findBackwards(element); }
	t_type *findBackwards(const t_type &element) { return Range<t_type>(_data, _count).findBackwards(element); }


	/**
	@brief	Returns the index in the array for first element that is equal to the given element.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	i64 indexOf(const t_type &element) const { return Range<t_type>(_data, _count).indexOf(element); }


	/**
	@brief	Returns the index in the array for first element that is equal to the given element, by searching from the end to the beginning.
			If there is no such element in the array -1 is returned.
			The type of the DArrat must have operator == defined.
	**/
	i64 indexOfBackwards(const t_type &element) const { return Range<t_type>(_data, _count).indexOfBackwards(element); }


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
		age_assertFatal(index < _count, "Trying to remove element in index %d from a dynamic array with count %d.", index, _count);
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
		age_assertFatal(_count > 0, "Trying to pop an element of an empty array.");
		_count--; 
	}


	/**
	@brief	Removes the element in the given index.
	**/
	void removeIndex(size_t index)
	{
		age_assertFatal(index < _count, "Trying to remove index %d from a dynamic array with count %d.", index, _count);

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
	_force_inline void _reserveIfNotEnoughSize(size_t aditionalSlots = 1)
	{
		const u64 minRequiredCapacity = _count + aditionalSlots;
		if (minRequiredCapacity > _capacity)
			reserve(math::nextPow2(minRequiredCapacity) - _capacity);
	}


	_force_inline void _swapPopPtr(t_type *ptr)
	{
		if (ptr != &back())
			memcpy(ptr, &back(), sizeof(t_type));

		_count--;
	}

	_force_inline void _add(const t_type *ptr, size_t count)
	{
		_reserveIfNotEnoughSize(count);
		memcpy(&_data[_count], ptr, count * sizeof(t_type));
		_count += count;
	}

	_force_inline void _insert(const t_type *ptr, size_t count, size_t index)
	{
		age_assertFatal(index < _count, "Trying to insert element in index %d of a DArray with only %d elements.", index, _count);

		_reserveIfNotEnoughSize(count);

		const size_t movingChunkSize = _count - index;
		t_type *insertionPtr = _data + index;

		memcpy(insertionPtr + count, insertionPtr, movingChunkSize * sizeof(t_type));
		memcpy(insertionPtr, ptr, count * sizeof(t_type));

		_count += count;
	}

	t_type *_data = nullptr;
	size_t _count = 0;
	size_t _capacity = 0;
};


template<typename t_type, class t_allocator>
static std::ostream &operator<<(std::ostream &os, const DArray<t_type, t_allocator> &array)
{
	Range range(array.data(), array.count());
	os << range;

	return os;
}


template<typename t_type, class t_allocator>
static std::istream &operator>>(std::istream &is, DArray<t_type, t_allocator> &array)
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

}    // namespace age
