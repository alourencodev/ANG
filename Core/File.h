#pragma once

#include "DArray.hpp"
#include "Types.hpp"



namespace age::file
{

DArray<byte> readBinary(const char *path);
DArray<char> readText(const char *path);

void writeBinary(const char *path, const byte *data, size_t size);

}	// namespace age::file
