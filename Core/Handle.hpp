#pragma once

#include <numeric>

#include <Core/Types.hpp>

#define DECLARE_HANDLE(HandleName)  \
struct HandleName ## _struct {};	\
using HandleName = age::Handle<HandleName ## _struct>

#define DECLARE_HANDLE_TYPE(HandleName, HandleType)	\
struct HandleName ## _struct {};					\
using HandleName = age::Handle<HandleName ## _struct, HandleType>

namespace age
{

template <typename t_tag, typename t_intType = u32>
class Handle
{
public:
	Handle() = default;
	Handle(t_intType value) : _value(value) {}

	bool operator == (Handle other) const { return _value == other._value; }
	bool operator != (Handle other) const { return _value != other._value; }

	operator t_intType() const { return _value; }
	static const Handle invalid() { return Handle(k_invalid); }

	bool isValid() const { return _value != k_invalid; }

private:
	static const t_intType k_invalid = std::numeric_limits<t_intType>::max();

	t_intType _value = k_invalid;
};

}
