#pragma once

#include <iostream>

#include <Core/Types.hpp>

namespace age
{

struct Version
{
	u8 major = 0;
	u8 minor = 0;
	u8 patch = 0;
};



static std::ostream & operator << (std::ostream &stream, const Version &version)
{
	stream << static_cast<i32>(version.major) << "." << static_cast<i32>(version.minor) << "." << static_cast<i32>(version.patch);
	return stream;
}



static std::istream & operator >> (std::istream &stream, Version &version)
{
	stream >> version.major >> version.minor >> version.patch;
	return stream;
}

}
