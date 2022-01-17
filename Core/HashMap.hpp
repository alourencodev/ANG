#pragma once

#include "Attributes.hpp"
#include "Core/Math/Math.hpp"
#include "Core/Meta.hpp"
#include "Core/Range.hpp"
#include "Core/Types.hpp"
#include "Log/Assert.hpp"
#include "Log/Log.h"
#include "Memory/Allocator.hpp"

#include <functional>

namespace age
{

namespace
{

enum class e_HashNodeState : u8
{
	Empty = 0,	// Needs to be 0
	Full,
	Removed
};

template<typename t_keyType, typename t_valueType>
struct HashNode
{
	static_assert(meta::isCopyable<t_keyType>::value, "A key of a HashMap must be of a copyable type.");
	static_assert(meta::isCopyable<t_valueType>::value, " value of a HashMap must be of a copyable type.");

	t_keyType key;
	t_valueType value;
	e_HashNodeState state = e_HashNodeState::Empty;

	bool isValid() const { return state == e_HashNodeState::Full; }
};

}

template<typename t_keyType, typename t_valueType>
using DefaultHashMapAllocator = DefaultHeapAllocator<HashNode<t_keyType, t_valueType>>;

template<typename t_type>
struct HashMapBehavior
{
public:
	virtual bool isEqual(const t_type &a, const t_type &b) const { return a == b; }
	virtual size_t hash(const t_type &value) const { return std::hash<t_type>{}(value); }
};


template<typename t_keyType, 
		 typename t_valueType,
		 typename t_behavior = HashMapBehavior<t_keyType>, 
		 typename t_allocator = DefaultHashMapAllocator<t_keyType, t_valueType>>
class HashMap
{
	constexpr static char k_tag[] = "HashMap";
	constexpr static size_t k_defaultCapacity = 8;
	constexpr static float k_rehashThreshold = 0.7f;

public:
	using Node = HashNode<t_keyType, t_valueType>;

	HashMap() = default;
	HashMap(size_t capacity)
	{
		_capacity = math::max(math::nextPow2(capacity), k_defaultCapacity);
		_data = t_allocator::alloc(_capacity);
		memset(_data, 0, sizeof(Node) * _capacity);
	}

	HashMap(const HashMap &other)
	{
		
		_data = t_allocator::realloc(&_data, other._capacity);
		_capacity = other._capacity;
		_count = other._count;
		memcpy(_data, other._data, _capacity * sizeof(Node));
	}

	HashMap(HashMap &&other)
	{
		t_allocator::dealloc(_data);

		_data = other._data;
		_capacity = other._capacity;
		_count = other._count;

		other._data = nullptr;
		other._capacity = 0;
	}

	HashMap(std::initializer_list<std::pair<t_keyType, t_valueType>> &&list) : HashMap(list.size())
	{
		for (const auto &pair : list) {
			if (!add(pair.first, pair.second))
				age_error(k_tag, "Failed to add element from initializer list.\n""Be sure that there are no values with repeated keys.");
		}
	}

	~HashMap() { t_allocator::dealloc(_data); }

	void operator = (const HashMap &other)
	{
		if (!t_allocator::realloc(&_data, other._capacity))
			age_error(k_tag, "Unable to reallocate memory during copy assignment");

		_capacity = other._capacity;
		_count = other._count;
		memcpy(_data, other._data, _capacity * sizeof(Node));
	}

	void operator = (HashMap &&other) noexcept
	{
		t_allocator::dealloc(_data);

		_data = other._data;
		_capacity = other._capacity;
		_count = other._count;

		other._data = nullptr;
		other._capacity = 0;
		other._count = 0;
	}

	_force_inline size_t capacity() const { return _capacity; }
	_force_inline size_t count() const { return _count; }
	_force_inline bool isEmpty() const { return _count == 0; }



	/**
	* Returns true if new element added.
	**/
	bool add(const t_keyType &key, const t_valueType &value, t_valueType **valuePtr = nullptr)
	{
		const float loadFactor = (_count + 1) / static_cast<float>(_capacity);
		if (_capacity == 0 || loadFactor >= k_rehashThreshold)
			_grow();

		u32 index = _hashValue(key);
		while (_data[index].state == e_HashNodeState::Full)
		{
			// If already exists
			if (_data[index].key == key)
				return false; 

			index++;
		}

		_data[index] = {t_keyType(key), t_valueType(value), e_HashNodeState::Full };
		_count++;

		if (valuePtr != nullptr)
			*valuePtr = &(_data[index].value);

		return true;
	}

	t_valueType & operator [] (const t_keyType &key) 
	{
		const i32 index = _findExistingIndex(key);
		age_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _data[index].value; 
	}

	const t_valueType & operator [] (const t_keyType &key) const
	{
		const i32 index = _findExistingIndex(key);
		age_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _data[index].value; 
	}

	bool remove(const t_keyType &key)
	{
		const i32 index = _findExistingIndex(key);
		if (index < 0)
			return false;

		_data[index].state = e_HashNodeState::Removed;
		_count--;
		return true;
	}
	
	_force_inline void clear()
	{
		memset(_data, 0, sizeof(Node) * _capacity);
		_count = 0;
	}

	bool contains(const t_keyType &key) const { return _findExistingIndex(key) >= 0; }

	const t_valueType *at(const t_keyType &key) const
	{
		const i32 index = _findExistingIndex(key);
		return index >= 0 ? &(_data[index].value) : nullptr;
	}

	Range<Node> asRange() { return Range<Node>(_data, _capacity); }

private:

	_force_inline void _grow()
	{
		size_t oldCapacity = _capacity;
		Node *oldData = _data;

		_capacity = math::max(k_defaultCapacity, _capacity *2);
		_data = t_allocator::alloc(_capacity);
		_count = 0;
		memset(_data, 0, sizeof(Node) * _capacity);

		for (int i = 0; i < oldCapacity; i++) {
			if (oldData[i].state == e_HashNodeState::Full)
				add(oldData[i].key, oldData[i].value);
		}

		t_allocator::dealloc(oldData);
	}

	_force_inline u32 _hashValue(const t_keyType &key) const
	{
		size_t hash = t_behavior{}.hash(key);
		return static_cast<u32>(hash & (_capacity - 1));
	}

	_force_inline i32 _findExistingIndex(const t_keyType &key) const
	{	
		if (_data != nullptr) {
			i32 index = _hashValue(key);
			while(_data[index].state != e_HashNodeState::Empty) {
				if (_data[index].state == e_HashNodeState::Full && t_behavior{}.isEqual(_data[index].key, key))
					return index;

				// If Removed it continues
				index = (index + 1) % _capacity;
			}
		}

		return -1;
	}

	Node *_data = nullptr;
	size_t _capacity = 0;
	size_t _count = 0;
};

}    // namespace age
