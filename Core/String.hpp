#pragma once

#include <cstring>

#include <Core/Attributes.hpp>
#include <Core/DArray.hpp>
#include <Core/Log/Assert.hpp>
#include <Core/Memory/Allocator.hpp>



namespace age
{

namespace
{
static char k_emptyString[] = "";
}	// namespace anon



size_t strlen(const char* str)
{
	size_t len = 0;
	for (; str[len]; len++);

	return len;
}



template<class t_allocator = DefaultHeapAllocator<char>>
class StringBuilder;



template<class t_allocator = DefaultHeapAllocator<char>>
class String
{
public:
	String() = default;
	String(const char* str)
	{
		_size = strlen(str);
		_str = t_allocator::alloc(_size);
		memcpy(_str, str, _size);
	}

	String(const String& other)
	{
		if (other.isEmpty) {
			_str = k_emptyString;
		} else {
			_str = t_allocator::alloc(other._size);
			_size = other._size;
			memcpy(_str, other._str, _size);
		}
	}

	String(String&& other)
	{
		_str = other._str;
		other._str = k_emptyString;
		_size = other._size;
		other._size = 0;
	}

	~String() { if (_size > 0) t_allocator::dealloc(_str); }

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
		if (_str != k_emptyString)
			t_allocator::free(_str);

		if (other.isEmpty) {
			_str = k_emptyString;
		} else {
			_str = t_allocator::alloc(other._size);
			_size = other._size;
			memcpy(_str, other._str, _size);
		}
	}

	void operator = (String&& other)
	{
		if (_str != k_emptyString)
			t_allocator::free(_str);

		_str = other._str;
		other._str = k_emptyString;
		_size = other._size;
		other._size = 0;
	}

	_force_inline size_t size() const { return _size; }

	static String Empty;

	template<class t_allocator>
	friend class StringBuilder;

private:
	char *_str = k_emptyString;
	size_t _size = 0;
};



template<class t_allocator>
class StringBuilder
{
	using StringType = String<t_allocator>;

public:
	StringBuilder() = default;

	_force_inline void reserve(size_t count) { _segments.reserve(count); }
	_force_inline void clear() { _segments.clear(); }
	_force_inline void append(const StringType &str) 
	{ 
		_segments.add(&str); 
		_totalLength += str.size();
	}

	StringType build()
	{
		StringType string;
		
		// This memory will then be freed when the String's lifetime ends.
		string._str = t_allocator::alloc(_totalLength);

		size_t offset = 0;
		for (const StringType *segment : _segments) {
			memcpy(&string._str[offset], segment._str, segment._size);
			offset += segment._size;
		}

		return string;
	}

private:
	DArray<const StringType *, t_allocator> _segments;
	size_t _totalLength = 0;
};

}	// namespace age
