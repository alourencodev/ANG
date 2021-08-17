#pragma once


namespace age
{

template<typename, typename>
class DArray;

}

namespace age::file
{

DArray<char> readText(const char *path);

}
