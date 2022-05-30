
#include <Core/DArray.hpp>
#include <Core/File.h>

#include <fstream>

#ifdef _WIN64
#	include <direct.h>
#else
#	include <unistd.h>
#endif


namespace age::file
{

constexpr const char k_tag[] = "File";


constexpr const u32 k_charPathSize = 256;

void getCurrentPath(char buffer[], u32 size)
{
#	ifdef _WIN64
		_getcwd(buffer, size);
#	else
		getcwd(buffer, size);
#	endif
}



DArray<byte> readBinary(const char *path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		char currentPath[k_charPathSize];
		getCurrentPath(currentPath, sizeof(currentPath));
		age_error(k_tag, "Failed to open file %s from directory %s", path, currentPath);
	}
		
	size_t fileSize = file.tellg();
	DArray<byte> buffer(fileSize);
	buffer.addEmpty(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}



DArray<char> readText(const char *path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		char currentPath[k_charPathSize];
		getCurrentPath(currentPath, sizeof(currentPath));
		age_error(k_tag, "Failed to open file %s from directory %s", path, currentPath);
	}
		
	size_t fileSize = file.tellg();
	DArray<char> buffer(fileSize + 1);
	buffer.addEmpty(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	buffer.add('\0');

	return buffer;
}



void writeBinary(const char *path, const byte *data, size_t size)
{
	std::ofstream file(path, std::ios::out | std::ios::binary);

	if (!file.is_open()) {
		char currentPath[k_charPathSize];
		getCurrentPath(currentPath, sizeof(currentPath));
		age_error(k_tag, "Failed to open file %s from directory %s", path, currentPath);
	}

	file.write(data, size);
	file.close();

	if (!file.good())
		age_error(k_tag, "Error occurred when writing to %s", path);
}



void writeText(const char *path, const char *data, size_t size)
{
	std::ofstream file(path, std::ios::out);

	if (!file.is_open()) {
		char currentPath[k_charPathSize];
		getCurrentPath(currentPath, sizeof(currentPath));
		age_error(k_tag, "Failed to open file %s from directory %s", path, currentPath);
	}

	file.write(data, size);
	file.close();

	if (!file.good())
		age_error(k_tag, "Error occurred when writing to %s", path);
}

}

