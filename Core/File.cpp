
#include <Core/DArray.hpp>
#include <Core/File.h>

#include <fstream>
#include <filesystem>


namespace age::file
{

constexpr const char k_tag[] = "File";

DArray<char> readBinary(const char *path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::string currentPath = std::filesystem::current_path().string();
		age_error(k_tag, "Failed to open file %s from directory %s", path, currentPath.c_str());
	}
		
	size_t fileSize = file.tellg();
	DArray<char> buffer(fileSize);
	buffer.addEmpty(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

}