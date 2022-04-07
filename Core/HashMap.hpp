#pragma once

#include "Core/Attributes.hpp"
#include "Core/Hash.h"
#include "Core/Math/Math.hpp"
#include "Core/Meta.hpp"
#include "Core/Range.hpp"
#include "Core/Types.hpp"
#include "Core/Log/Assert.hpp"
#include "Core/Log/Log.h"
#include "Core/Memory/Allocator.hpp"
#include "Core/Profiler/TimeProfiler.h"



namespace age
{


template<typename t_keyType, 
		 typename t_valueType,
		 typename t_allocator = DefaultHeapAllocator<byte>>
class HashMap
{
	constexpr static char k_tag[] = "HashMap";
	constexpr static size_t k_defaultCapacity = 8;
	constexpr static float k_rehashThreshold = 0.7f;
	constexpr static u32 k_elementSize = sizeof(t_keyType) + sizeof(byte) + sizeof(t_valueType);

public:
	HashMap() = default;
	HashMap(size_t capacity)
	{
		_capacity = math::max(math::nextPow2(capacity), k_defaultCapacity);
		_alloc();
	}

	HashMap(const HashMap &other)
	{
		if (!t_allocator::realloc(&_states, other._capacity * k_elementSize))
			age_error(k_tag, "Unable to reallocate memory during copy constructor");

		_capacity = other._capacity;
		_count = other._count;

		memcpy(_states, other._states, _capacity * sizeof(k_elementSize));
	}

	HashMap(HashMap &&other)
	{
		_dealloc();

		_states = other._states;
		_keys = other._keys;
		_values = other._values;

		_capacity = other._capacity;
		_count = other._count;

		other._states = nullptr;
		other._keys = nullptr;
		other._values = nullptr;

		other._capacity = 0;
		other._count = 0;
	}

	HashMap(std::initializer_list<std::pair<t_keyType, t_valueType>> &&list) : HashMap(list.size())
	{
		for (const auto &pair : list) {
			if (!add(pair.first, pair.second))
				age_error(k_tag, "Failed to add element from initializer list.\n""Be sure that there are no values with repeated keys.");
		}
	}

	~HashMap() { _dealloc(); }

	void operator = (const HashMap &other)
	{
		if (!t_allocator::realloc(&_states, other._capacity * k_elementSize))
			age_error(k_tag, "Unable to reallocate memory during copy assignment");

		_capacity = other._capacity;
		_count = other._count;

		memcpy(_states, other._states, _capacity * sizeof(k_elementSize));
	}

	void operator = (HashMap &&other) noexcept
	{
		_dealloc();

		_states = other._states;
		_keys = other._keys;
		_values = other._values;

		_capacity = other._capacity;
		_count = other._count;

		other._states = nullptr;
		other._keys = nullptr;
		other._values = nullptr;

		other._capacity = 0;
		other._count = 0;
	}

	_force_inline size_t capacity() const { return _capacity; }
	_force_inline size_t count() const { return _count; }
	_force_inline bool isEmpty() const { return _count == 0; }

	/**
	* Returns true if new element added.
	**/
	bool add(const t_keyType &key, const t_valueType &value)
	{
		AGE_PROFILE_TIME();

		if (_hasLoadFactorBeenReached() || _capacity == 0)
			_grow();

		u32 index = _hashValue(key);
		while (_states[index] == EKeyState::Set)
		{
			// If already exists
			if (hash::isKeyEqual(_keys[index], key))
				return false; 

			index = (index + 1) % _capacity;
		}

		_setElement(index, key, value);
		_count++;

		return true;
	}

	bool add(const t_keyType& key, const t_valueType& value, t_valueType **valuePtr)
	{
		age_assertFatal(valuePtr != nullptr, "valuePtr cannot be a nullptr.");

		AGE_PROFILE_TIME();

		if (_hasLoadFactorBeenReached() || _capacity == 0)
			_grow();

		u32 index = _hashValue(key);
		while (_states[index] == EKeyState::Set)
		{
			// If already exists
			if (hash::isKeyEqual(_keys[index], key))
				return false; 

			index = (index + 1) % _capacity;
		}

		_setElement(index, key, value);
		_count++;
		*valuePtr = &_values[index];

		return true;
	}

	t_valueType & operator [] (const t_keyType &key) 
	{
		const i32 index = _findExistingIndex(key);
		age_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _values[index]; 
	}

	const t_valueType & operator [] (const t_keyType &key) const
	{
		const i32 index = _findExistingIndex(key);
		age_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _values[index]; 
	}

	bool remove(const t_keyType &key)
	{
		const i32 index = _findExistingIndex(key);
		if (index < 0)
			return false;

		_states[index] = EKeyState::Removed;
		_count--;
		return true;
	}
	
	_force_inline void clear()
	{
		memset(_states, static_cast<byte>(EKeyState::Empty), _capacity);
		_count = 0;
	}

	bool contains(const t_keyType &key) const { return _findExistingIndex(key) >= 0; }

	const t_valueType *at(const t_keyType &key) const
	{
		const i32 index = _findExistingIndex(key);
		return index >= 0 ? &(_values[index]) : nullptr;
	}

private:

	enum class EKeyState : u8
	{
		Empty,
		Set,
		Removed
	};

	_force_inline void _growthAdd(const t_keyType &key, const t_valueType &value)
	{
		u32 index = _hashValue(key);
		while (_states[index] == EKeyState::Set)
			index++;

		_setElement(index, key, value);
	} 

	_force_inline void _grow()
	{
		AGE_PROFILE_TIME();

		// For the first add, allocating is enough, since there are no old elements to readd
		if (_capacity == 0) {
			_capacity = k_defaultCapacity;
			_alloc();
			return;
		}

		size_t oldCapacity = _capacity;
		EKeyState *oldFlags = _states;
		t_keyType *oldKeys = _keys;
		t_valueType *oldValues = _values;

		_capacity *= 2;
		_alloc();

		for (int i = 0; i < oldCapacity; i++) {
			if (oldFlags[i] == EKeyState::Set)
				_growthAdd(oldKeys[i], oldValues[i]);
		}

		t_allocator::dealloc(reinterpret_cast<byte *>(oldFlags));
	}

	_force_inline u32 _hashValue(const t_keyType &key) const
	{
		return static_cast<u32>(hash::hash(key) & (_capacity - 1));
	}

	_force_inline i32 _findExistingIndex(const t_keyType &key) const
	{	
		if (_states == nullptr)
			return -1;

		i32 index = _hashValue(key);
		while(_states[index] != EKeyState::Empty) {
			if (_states[index] == EKeyState::Set && hash::isKeyEqual(_keys[index], key))
				return index;

			// If empty it continues
			index = (index + 1) % _capacity;
		}

		return -1;
	}

	_force_inline bool _hasLoadFactorBeenReached() const 
	{ 
		return (_count + 1) / static_cast<float>(_capacity) >= k_rehashThreshold; 
	}

	/*
	 *	Allocates the necessary memory for a given capacity and sets every pointer to the respective offset
	 */
	_force_inline void _alloc()
	{
		// States come first because they are always checked before a key.
		// Keys come second because they can be skipped after checking the state, but will always be checked before we need the value.
		// Values come last because we always need to check states and keys first.

		// Use bytePtr to make sure the pointer arithmetic is done at the byte level
		byte *bytePtr = t_allocator::alloc(k_elementSize * _capacity); 
		_states = reinterpret_cast<EKeyState *>(bytePtr);

		bytePtr += _capacity;
		_keys = reinterpret_cast<t_keyType *>(bytePtr);
		IF_MEMORY_DBG(*bytePtr = 170 /*AA*/);

		bytePtr += _capacity * sizeof(t_keyType);
		_values = reinterpret_cast<t_valueType *>(bytePtr);
		IF_MEMORY_DBG(*bytePtr = 187 /*BB*/);
		
		memset(_states, static_cast<byte>(EKeyState::Empty), _capacity);
	}

	_force_inline void _dealloc()
	{
		t_allocator::dealloc(reinterpret_cast<byte *>(_states));
	}

	_force_inline void _setElement(int index, const t_keyType& key, const t_valueType& value)
	{
		_states[index] = EKeyState::Set;

		// Call constructors in the already allocated memory
		new (&_keys[index]) t_keyType(key);
		new (&_values[index]) t_valueType(value);
	}

	EKeyState *_states = nullptr;		// True when offset is being used. // TODO: state only needs a bit, maybe we can encode this somewhere
	t_keyType *_keys = nullptr;
	t_valueType *_values = nullptr;

	size_t _capacity = 0;
	size_t _count = 0;
};

}    // namespace age
