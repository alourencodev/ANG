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
	Handle() : _handle(s_nextValue) { s_nextValue++; };

	bool operator == (Handle other) const { return _handle == other._handle; }
	bool operator != (Handle other) const { return _handle != other._handle; }

	operator t_intType() const { return _handle; }

	static const t_intType k_invalid = std::numeric_limits<t_intType>::max();

private:
	static t_intType s_nextValue;

	t_intType _handle = k_invalid;
};

template <typename t_tag, typename t_intType>
t_intType Handle<t_tag, t_intType>::s_nextValue = static_cast<t_intType>(0);

}
