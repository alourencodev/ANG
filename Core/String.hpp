#pragma once

#include <cstring>

#include <Core/Attributes.hpp>
#include <Core/DArray.hpp>
#include <Core/Log/Assert.hpp>
#include <Core/Memory/Allocator.hpp>
#include <Core/StringUtils.hpp>



namespace age
{

class String
{
	using allocator = DefaultHeapAllocator<char>;

public:
	String() = default;
	String(const char* str)
	{
		_size = strSize(str);
		_str = allocator::alloc(_size + 1);
		memcpy(_str, str, _size + 1);
	}

	String(const String& other)
	{
		if (other.isEmpty()) {
			_str = nullptr;
		} else {
			_str = allocator::alloc(other._size + 1);
			_size = other._size;
			memcpy(_str, other._str, _size + 1);
		}
	}

	String(String&& other)
	{
		_str = other._str;
		other._str = nullptr;
		_size = other._size;
		other._size = 0;
	}

	~String() { if (_size > 0) allocator::dealloc(_str); }

	_force_inline operator const char* () const { return _str;  }

	_force_inline char operator[](size_t index) const
	{
		age_assertFatal(index < _size, "Trying to access char of index %d of a String with length %s.", index, _size);
		return _str[index];
	}

	_force_inline char &operator[](size_t index)
	{
		age_assertFatal(index < _size, "Trying to access char of index %d of a String with length %s.", index, _size);
		return _str[index];
	}

	_force_inline bool isEmpty() const { return _size == 0; }

	void operator = (const String &other)
	{
		if (_str != nullptr)
			allocator::dealloc(_str);

		if (other.isEmpty()) {
			_str = nullptr;
			_size = 0;
		} else {
			_str = allocator::alloc(other._size + 1);
			_size = other._size;
			memcpy(_str, other._str, _size + 1);
		}
	}

	void operator = (String&& other)
	{
		if (_str != nullptr)
			allocator::dealloc(_str);

		_str = other._str;
		other._str = nullptr;
		_size = other._size;
		other._size = 0;
	}
	
	void operator = (const char* other)
	{
		if (_str != nullptr)
			allocator::dealloc(_str);

		size_t otherSize = strSize(other);
		if (otherSize == 0) {
			_str = nullptr;
			_size = 0;
		} else {
			_str = allocator::alloc(otherSize + 1);
			_size = otherSize;
			memcpy(_str , other, _size + 1);
		}
	}

	_force_inline bool operator == (const String &other) const { return operator == (other._str); }
	_force_inline bool operator != (const String &other) const { return operator != (other._str); }

	bool operator == (const char* other) const
	{
		int i;
		for (i = 0; other[i]; i++) {
			if (i >= _size || _str[i] != other[i])
				return false;
		}

		return i == _size;
	}

	bool operator != (const char* other) const
	{
		int i;
		for (i = 0; other[i]; i++) {
			if (i >= _size || _str[i] != other[i])
				return true;
		}

		return i != _size;
	}

	_force_inline size_t size() const { return _size; }

	friend class StringBuilder;
private:
	char *_str = nullptr;
	size_t _size = 0;	// Size without '\0'
};

}	// namespace age
