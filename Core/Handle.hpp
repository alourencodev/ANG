#pragma once

#include <Core/Types.hpp>

#define DECLARE_HANDLE(HandleName)  \
struct HandleName ## _struct {};	\
using HandleName = age::Handle<HandleName ## _struct>

#define DECLARE_HANDLE_TYPE(HandleName, HandleType)	\
struct HandleName ## _struct {};					\
using HandleName = age::Handle<HandleName ## _struct, HandleType>

namespace age
{

template <typename t_tag, typename t_intType = i32>
class Handle
{
public:
	Handle(t_intType handle) : _handle(handle) {};

	bool operator == (Handle other) const { return _handle == other._handle; }
	bool operator != (Handle other) const { return _handle != other._handle; }

	operator t_intType() const { return _handle; }

	static const i32 k_invalid = -1;

private:
	t_intType _handle = -1;
};

}
