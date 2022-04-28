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
#include "Core/Profiler/HitProfiler.h"


namespace age
{

template<typename t_keyType, 
		 typename t_valueType,
		 typename t_allocator = DefaultHeapAllocator<byte>,
		 u32 t_loadFactorPercentage = 70>
class HashMap
{
	static_assert(t_loadFactorPercentage > 0 && t_loadFactorPercentage <= 100, "HashMap load factor needs to be > 0 and <= 100.");

	constexpr static char k_tag[] = "HashMap";
	constexpr static u32 k_defaultCapacity = 8;
	constexpr static float k_loadFactor = t_loadFactorPercentage * 0.01f;
	constexpr static u64 k_elementSize = sizeof(t_keyType) + sizeof(byte) + sizeof(t_valueType);

public:
	HashMap() = default;

	HashMap(u32 desiredMaxElements)
	{
		u32 desiredCapacity = static_cast<u32>(desiredMaxElements / k_loadFactor);
		_capacity = math::max(static_cast<u32>(math::nextPow2(desiredCapacity)), k_defaultCapacity);
		alloc();
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
		dealloc();

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

	HashMap(std::initializer_list<std::pair<t_keyType, t_valueType>> &&list) : HashMap(static_cast<u32>(list.size()))
	{
		for (const auto &pair : list) {
			if (!add(pair.first, pair.second))
				age_error(k_tag, "Failed to add element from initializer list.\n""Be sure that there are no values with repeated keys.");
		}
	}

	~HashMap() { dealloc(); }

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
		dealloc();

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


	_force_inline t_valueType & operator [] (const t_keyType &key) 
	{
		const i32 index = findExistingIndex(key);
		age_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _values[index]; 
	}


	_force_inline const t_valueType & operator [] (const t_keyType &key) const
	{
		const i32 index = findExistingIndex(key);
		age_assertFatal(index >= 0, "Item with key %s couldn't be found in the HashMap.", key);
		return _values[index]; 
	}


	const t_valueType *at(const t_keyType &key) const
	{
		const i32 index = findExistingIndex(key);
		return index >= 0 ? &(_values[index]) : nullptr;
	}
	
	
	_force_inline bool contains(const t_keyType &key) const { return findExistingIndex(key) >= 0; }


	// Returns true if new element added.
	bool add(const t_keyType &key, const t_valueType &value)
	{
		AGE_PROFILE_TIME();

		if (reachedLoadFactor() || _capacity == 0)
			grow();

		u32 index = calcProbe(key);
		while (_states[index] == EKeyState::Set)
		{
			// If already exists
			if (hash::isKeyEqual(_keys[index], key))
				return false; 

			index = reProbe(index);
		}

		setElement(index, key, value);
		_count++;

		return true;
	}


	bool add(const t_keyType& key, const t_valueType& value, t_valueType **valuePtr)
	{
		age_assertFatal(valuePtr != nullptr, "valuePtr cannot be a nullptr.");

		AGE_PROFILE_TIME();

		if (reachedLoadFactor() || _capacity == 0)
			grow();

		u32 index = calcProbe(key);
		while (_states[index] == EKeyState::Set)
		{
			// If already exists
			if (hash::isKeyEqual(_keys[index], key))
				return false; 

			index = reProbe(index);
		}

		setElement(index, key, value);
		_count++;
		*valuePtr = &_values[index];

		return true;
	}


	bool remove(const t_keyType &key)
	{
		const i32 index = findExistingIndex(key);
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


	_force_inline u32 capacity() const { return _capacity; }
	_force_inline u32 count() const { return _count; }
	_force_inline bool isEmpty() const { return _count == 0; }

private:
	enum class EKeyState : u8
	{
		Empty,
		Set,
		Removed
	};


	// Grows the HashMap to the next size and readds every element from the old one into the new one
	_force_inline void grow()
	{
		AGE_PROFILE_TIME();

		// For the first add, allocating is enough, since there are no old elements to readd
		if (_capacity == 0) {
			_capacity = k_defaultCapacity;
			alloc();
			return;
		}

		const u32 oldCapacity = _capacity;

		EKeyState *oldStates = _states;
		const t_keyType *oldKeys = _keys;
		const t_valueType *oldValues = _values;

		// Grow strategy is just duplicating the current capacity
		_capacity *= 2;
		alloc();

		// Re add the elements from the old hash map
		// We can do this with a lightweight version of add, since we can be sure of some facts:
		// - Every element in the old map has a unique key;
		// - We won't reach the load factor
		for (u32 i = 0; i < oldCapacity; i++) {
			if (oldStates[i] != EKeyState::Set)
				continue;

			u32 index = calcProbe(oldKeys[i]);
			while(_states[index] == EKeyState::Set)
				index = reProbe(index);

			setElement(index, oldKeys[i], oldValues[i]);
		}

		t_allocator::dealloc(reinterpret_cast<byte *>(oldStates));
	}


	// Sets an element in given index by constructing them in place.
	_force_inline void setElement(int index, const t_keyType& key, const t_valueType& value)
	{
		_states[index] = EKeyState::Set;

		new (&_keys[index]) t_keyType(key);
		new (&_values[index]) t_valueType(value);
	}


	_force_inline i32 findExistingIndex(const t_keyType &key) const
	{	
		if (_states == nullptr)
			return -1;

		i32 index = calcProbe(key);
		while(_states[index] != EKeyState::Empty) {
			// Need to check if it's actualy set, to make sure it wasn't removed
			if (_states[index] == EKeyState::Set && hash::isKeyEqual(_keys[index], key))
				return index;

			// If empty it continues
			index = reProbe(index);
		}

		return -1;
	}


	_force_inline bool reachedLoadFactor() const { return (_count + 1) / static_cast<float>(_capacity) >= k_loadFactor; }
	_force_inline u32 calcProbe(const t_keyType &key) const { return static_cast<u32>(hash::hash(key) & (_capacity - 1)); }
	_force_inline u32 reProbe(u32 index) const { AGE_PROFILE_HIT(); return (index + 1) % (_capacity); }


	// Allocates the necessary memory for the current capacity and sets every pointer to the respective offset
	_force_inline void alloc()
	{
		// States come first because they are always checked before a key.
		// Keys come second because they can be skipped after checking the state, but will always be checked before we need the value.
		// Values come last because we always need to check states and keys first.

		// Use bytePtr to make sure the pointer arithmetic is done at the byte level
		byte *bytePtr = t_allocator::alloc(k_elementSize * _capacity); 
		_states = reinterpret_cast<EKeyState *>(bytePtr);

		bytePtr += _capacity;
		_keys = reinterpret_cast<t_keyType *>(bytePtr);
		IF_MEMORY_DBG(*bytePtr = byte(0xAA));

		bytePtr += _capacity * sizeof(t_keyType);
		_values = reinterpret_cast<t_valueType *>(bytePtr);
		IF_MEMORY_DBG(*bytePtr = byte(0xBB));
		
		memset(_states, static_cast<byte>(EKeyState::Empty), _capacity);
	}


	_force_inline void dealloc()
	{
		t_allocator::dealloc(reinterpret_cast<byte *>(_states));
	}


	EKeyState *_states = nullptr;
	t_keyType *_keys = nullptr;
	t_valueType *_values = nullptr;

	u32 _capacity = 0;
	u32 _count = 0;
};

}    // namespace age
