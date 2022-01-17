#include "Includer.h"

#include <filesystem>

#include <Core/File.h>
#include <Core/String.hpp>

#include "Extensions.hpp"



namespace fs = std::filesystem;



namespace age
{

constexpr char k_tag[] = "Includer";



shaderc_include_result* resolveInclude(void* userData, const char* requestedSource, int type, const char* requestingSource, size_t includeDepth)
{
	Includer *includer = reinterpret_cast<Includer *>(userData);

	const String *sourceCode = includer->getSource(requestedSource);
	if (sourceCode == nullptr) {
		age_error(k_tag, "Unable to find requestedSource %s", requestedSource);
		return nullptr;
	}

	shaderc_include_result *result = reinterpret_cast<shaderc_include_result *>(malloc(sizeof(shaderc_include_result)));
	if (result == NULL) {
		age_assert(false, "Wasn't able to allocate shader include result");
		return nullptr;
	}

	result->source_name = requestedSource;
	result->source_name_length = strSize(requestedSource);
	result->content = *sourceCode;
	result->content_length = sourceCode->size();
	result->user_data = userData;

	return result;
}



void releaseIncludeResult(void *userData, shaderc_include_result *includeResult)
{
	if (includeResult != nullptr)
		free(includeResult);
}


const String *Includer::getSource(const char* requestedSource)
{
	{	// Load From Cache
		const String *sourceCode = _sourceMap.at(requestedSource);
		if (sourceCode != nullptr)
			return sourceCode;
	}

	{ // Load And Cache
		for (const char *includeDir : _includeDirs) {
			DArray<char> sourceCodeRaw = {};

			// Search for file in includeDirectories
			for (const auto &entry : fs::directory_iterator(includeDir)) {
				fs::path sourcePath(entry.path());

				std::string extensionString = sourcePath.extension().string();
				if (k_validExtensions.contains(extensionString.c_str())) {
					sourceCodeRaw = file::readText(sourcePath.string().c_str());
					break;				
				}
			}

			if (!sourceCodeRaw.isEmpty()) {
				String *sourceCode = nullptr;
				_sourceMap.add(requestedSource, String(sourceCodeRaw.data()), &sourceCode);
				age_assert(sourceCode != nullptr, "");

				return sourceCode != nullptr ? sourceCode : nullptr;
			}
		}
	}

	return nullptr;
}

}	// namespace age
