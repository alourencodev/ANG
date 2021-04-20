#pragma once

#include "Log/Log.h"
#include "Math/Math.hpp"
#include "Memory/Allocator.hpp"
#include "SArray.hpp"


/**
@brief	Dynamic array.
**/
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

	template<size_t t_size>
	DArray(const SArray<t_type, t_size> &sArray) : DArray(t_size)
	{
		memcpy(_data, sArray.data(), t_size * sizeof(t_type));
		_count = _capacity;
	}

	DArray(std::initializer_list<t_type> &&list) : DArray(list.size())
	{
		memcpy(_data, list.begin(), list.size() * sizeof(t_type));
		_count = _capacity;
	}

	DArray(const DArray &other)
	{
		static_assert(meta::isCopyable<t_type>::value, "Trying to copy DArray of non copyable type.");

		_data = t_allocator::alloc(other._capacity);
		_capacity = other._capacity;
		_count = other._count;
		memcpy(_data, other._data, _capacity * sizeof(t_type));
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


	_force_inline const t_type &operator[](size_t index) const
	{
		logAssertFatal(index < _count, "Trying to access index %d of a DArray with count %d.", index, _count);
		return _data[index];
	}

	_force_inline t_type &operator[](size_t index)
	{
		logAssertFatal(index < _count, "Trying to access index %d of a DArray with count %d.", index, _count);
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
	template<size_t t_size>
	void add(const SArray<t_type, t_size> &sArray)	{ _add(sArray.data(), sArray.size); }
	void add(const DArray<t_type> &dArray)			{ _add(dArray.data(), dArray.count()); }
	void add(std::initializer_list<t_type> list)	{ _add(list.begin(), list.size()); }
	void add(const t_type &element)					{ _add(&element, 1); }

	void add(t_type &&element)
	{
		_reserveIfNotEnoughSize();
		_data[_count] = std::move(element);
		_count++;
	}


	/**
	@brief	Add element to the given index. It allcates memory if necessary
	**/
	template<size_t t_size>
	void insert(const SArray<t_type, t_size> &sArray, size_t index) { _insert(sArray.data(), sArray.size, index); }
	void insert(const DArray<t_type> &dArray, size_t index)			{ _insert(dArray.data(), dArray.count(), index); }
	void insert(std::initializer_list<t_type> list, size_t index)	{ _insert(list.begin(), list.size(), index); }
	void insert(const t_type &element, size_t index)				{ _insert(&element, 1, index); }

	void insert(t_type &&element, size_t index)
	{
		logAssertFatal(index < _count, "Trying to insert element in index %d of a DArray with only %d elements.", index, _count);
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
		logAssertFatal(index < _count, "Trying to remove element in index %d from a dynamic array with count %d.", index, _count);
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
		logAssertFatal(_count > 0, "Trying to pop an element of an empty array.");
		_count--; 
	}


	/**
	@brief	Removes the element in the given index.
	**/
	void removeIndex(size_t index)
	{
		logAssertFatal(index < _count, "Trying to remove index %d from a dynamic array with count %d.", index, _count);

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
			reserve(nextPow2(minRequiredCapacity) - _capacity);
	}


	_force_inline void _add(const t_type *ptr, size_t count)
	{
		_reserveIfNotEnoughSize(count);
		memcpy(&_data[_count], ptr, count * sizeof(t_type));
		_count += count;
	}


	_force_inline void _insert(const t_type *ptr, size_t count, size_t index)
	{
		logAssertFatal(index < _count, "Trying to insert element in index %d of a DArray with only %d elements.", index, _count);

		_reserveIfNotEnoughSize(count);

		const size_t movingChunkSize = _count - index;
		t_type *insertionPtr = _data + index;

		memcpy(insertionPtr + count, insertionPtr, movingChunkSize * sizeof(t_type));
		memcpy(insertionPtr, ptr, count * sizeof(t_type));

		_count += count;
	}


	_force_inline void _swapPopPtr(t_type *ptr)
	{
		if (ptr != &back())
			memcpy(ptr, &back(), sizeof(t_type));

		_count--;
	}


	t_type *_data = nullptr;
	size_t _count = 0;
	size_t _capacity = 0;
};
