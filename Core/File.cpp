
#include <Core/DArray.hpp>
#include <Core/File.h>

#include <fstream>

namespace age::file
{

constexpr const char k_tag[] = "File";

DArray<char> readText(const char *path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		age_error(k_tag, "Failed to open file %s", path);
	}
		
	size_t fileSize = file.tellg();
	DArray<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

}