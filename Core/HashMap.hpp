#pragma once

#include "Core/Math/Math.hpp"
#include "Core/Meta.hpp"
#include "Core/Types.hpp"
#include "Log/Assert.hpp"
#include "Log/Log.h"
#include "Memory/Allocator.hpp"

#include <functional>

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

	t_keyType key = default;
	t_valueType value = default;
	e_HashNodeState state = e_HashNodeState::Empty;
};

}


template<typename t_keyType, typename t_valueType, typename t_allocator = DefaultHeapAllocator<HashNode<t_keyType, t_valueType>>>
class HashMap
{
	constexpr static char k_tag[] = "HashMap";
	constexpr static size_t k_defaultCapacity = 8;
	constexpr static float k_rehashThreshold = 1 / 0.7f;

public:
	HashMap() = default;
	HashMap(size_t capacity)
	{
		_capacity = math::g_max(math::g_nextPow2(capacity), k_defaultCapacity);
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
				g_error(k_tag, "Failed to add element from initializer list.\n""Be sure that there are no values with repeated keys.");
		}
	}

	~HashMap() { t_allocator::dealloc(_data); }

	void operator = (const HashMap &other)
	{
		if (!t_allocator::realloc(&_data, other._capacity))
			g_error(k_tag, "Unable to reallocate memory during copy assignment");

		_capacity = other._capacity;
		_count = other._count;
		memcpy(_data, other._data, _capacity * sizeof(Node));
	}

	void operator = (HashMap &&other)
	{
		t_allocator::dealloc(_data);

		_data = other._data;
		_capacity = other._capacity;
		_count = other._count;

		other._data = nullptr;
		other._capacity = 0;
		other._count = 0;
	}

	size_t capacity() const { return _capacity; }
	size_t count() const { return _count; }
	bool isEmpty() const { return _count == 0; }

	bool add(const t_keyType &key, const t_valueType &value)
	{
		const float loadFactor = _count / static_cast<float>(_capacity);
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

		return true;
	}

	t_valueType & operator [] (const t_keyType &key) 
	{
		const i32 index = _findExistingIndex(key);
		g_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _data[_findExistingIndex(key)].value; 
	}

	const t_valueType & operator [] (const t_keyType &key)  const
	{
		const i32 index = _findExistingIndex(key);
		g_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _data[_findExistingIndex(key)].value; 
	}

	bool remove(const t_keyType &key)
	{
		const i32 index = _findExistingIndex(key);
		if (index < 0)
			return false;

		_data[index].state = e_HashNodeState::Removed;
		count--;
		return true;
	}

	bool contains(const t_keyType &key) const { return _findExistingIndex(key) >= 0; }

private:
	using Node = HashNode<t_keyType, t_valueType>;

	void _grow()
	{
		size_t oldCapacity = _capacity;
		Node *oldData = _data;

		_capacity = math::g_max(k_defaultCapacity, _capacity *2);
		_data = t_allocator::alloc(_capacity);
		memset(_data, 0, sizeof(Node) * _capacity);

		for (int i = 0; i < oldCapacity; i++) {
			if (_data[i].state == e_HashNodeState::Full)
				add(oldData[i].key, oldData[i].value);
		}

		t_allocator::dealloc(oldData);
	}

	u32 _hashValue(const t_keyType &key) const
	{
		size_t hash = std::hash<t_keyType>{}(key);
		return static_cast<u32>(hash & (_capacity - 1));
	}

	i32 _findExistingIndex(const t_keyType &key) const
	{
		i32 index = _hashValue(key);
		while(_data[index].state != e_HashNodeState::Empty) {
			if (_data[index].state == e_HashNodeState::Full && _data[index].key == key)
				return index;

			// If Removed it continues
			index = (index + 1) % _capacity;
		}

		return -1;
	}

	Node *_data = nullptr;
	size_t _capacity = 0;
	size_t _count = 0;
};
