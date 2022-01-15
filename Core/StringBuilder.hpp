#pragma once

#include "Memory/Allocator.hpp"
#include "StringView.hpp"
#include "String.hpp"



namespace age
{

class StringBuilder
{
	using allocator = DefaultHeapAllocator<char>;

public:
	StringBuilder() = default;

	_force_inline void reserve(size_t count) { _segments.reserve(count); }
	_force_inline void clear() { _segments.clear(); _totalLength = 0;}

	_force_inline void append(const ConstStringView &view) 
	{ 
		StringSegment segment = {};
		segment.stringView = view;
		segment.size = view.calcSize();

		_segments.add(segment); 
		_totalLength += segment.size;
	}

	_force_inline void append(const String& str)
	{
		StringSegment segment = {};
		segment.stringView = ConstStringView(str);
		segment.size = str.size();

		_segments.add(segment);
		_totalLength += segment.size;
	}

	_force_inline void append(const char* str)
	{
		StringSegment segment = {};
		segment.stringView = ConstStringView(str);
		segment.size = strSize(str);

		_segments.add(segment);
		_totalLength += segment.size;
	}

	_nodiscard String build()
	{
		String string;
		
		// This memory will then be freed when the String's lifetime ends.
		string._str = allocator::alloc(_totalLength + 1);

		size_t offset = 0;
		for (const StringSegment &segment : _segments) {
			memcpy(&string._str[offset], segment.stringView.str(), segment.size);
			offset += segment.size;
		}

		string._str[_totalLength] = '\0';
		string._size = _totalLength;

		return string;
	}

private:
	struct StringSegment
	{
		ConstStringView stringView;
		size_t size = 0;
	};

	DArray<StringSegment> _segments;
	size_t _totalLength = 0;
};

}	// namespace age
