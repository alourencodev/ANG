#pragma once

#include <Core/Attributes.hpp>
#include <Core/Log/Assert.hpp>
#include <Core/StringUtils.hpp>

namespace age
{

template<typename t_charType>
class StringViewBase
{
public:
	StringViewBase() = default;

	StringViewBase(t_charType str[])
	{
		age_assertFatal(str != nullptr, "Cannot construct a StringView with a nullptr.");
		_str = str;
	}

	StringViewBase(const StringViewBase& other)
	{
		_str = other._str;
	}

	StringViewBase(StringViewBase &&other) noexcept
	{
		_str = other._str;
		other._str = nullptr;
	}

	StringViewBase &operator = (const StringViewBase &other)
	{
		_str = other._str;
		return *this;
	}

	StringViewBase &operator = (StringViewBase &&other) noexcept
	{
		_str = other._str;
		other._str = nullptr;

		return *this;
	}

	operator t_charType *() { return _str; }
	operator const t_charType *() const { return _str; }

	bool operator == (const char *str)
	{
		int i = 0;
		for (; _str[i] != '\0'; i++) {
			if (_str[i] != str[i])
				return false;
		}

		return _str[i] == str[i];
	}

	bool operator == (const StringViewBase &other) { return operator=(other._str); }

	bool operator != (const StringViewBase &other) { return !operator==(other); }

	_force_inline size_t calcSize() const { return strSize(_str); }

	void getSubString(char *o_str, u32 length, u32 offset = 0) const
	{
		age_assertFatal((length + 0) <= calcSize(), "Trying to get substring from index %d to %d of a string that only has size %d", offset, length + offset, calcSize());
		memcpy(o_str, _str + offset, length);
		o_str[length] = '\0';
	}

	void getPrefix(char *o_str, u32 length) const
	{
		age_assertFatal(length <= calcSize(), "Trying to get prefix of length %d of a string that only has size %d", length, calcSize());
		memcpy(o_str, _str, length);
		o_str[length] = '\0';
	}

	void getSufix(char *o_str, u32 length) const
	{
		size_t size = calcSize();
		age_assertFatal(length <= size, "Trying to get sufix of length %d of a string that only has size %d", length, size);

		size_t offset = size - length;
		memcpy(o_str, _str + offset, length);
		o_str[length] = '\0';
	}

	_force_inline t_charType *str() { return _str; }
	_force_inline const t_charType *str() const { return _str; }

protected:
	t_charType *_str = nullptr;
};

class StringView : public StringViewBase<char>
{
public:
	StringView() = default;
	StringView(char str[]) : StringViewBase(str) {}
};


class ConstStringView : public StringViewBase<const char>
{
public:
	ConstStringView() = default;
	ConstStringView(const char str[]) : StringViewBase(str) {}
};

}	// namespace age
