#include "AGEFile.h"

#include <sstream>

#include "Core/BuildScheme.hpp"
#include "Core/File.h"
#include "Core/Log/Log.h"
#include "Core/StringBuilder.hpp"



namespace age
{

constexpr const char k_tag[] = "AGEFile";
constexpr u8 k_preallocNodes = 32;

constexpr Version k_version = {0, 1, 0};

AGEFile::AGEFile(const String& dir, const Version &version) : _dir(dir), _version(version)
{
	_nodePool.reserve(k_preallocNodes);
}



AGEFile::Node *AGEFile::requestNode()
{
#ifdef AGE_DEBUG
	if (_nodePool.count() >= k_preallocNodes)
		age_warning(k_tag, "Not preallocating enough nodes. Increase the preallocated nodes amount or remove this warning if it doesn't make sense anymore.");
#endif

	_nodePool.addEmpty();
	Node *node = &_nodePool.back();
	new(node) Node();
	return node;
}



void AGEFile::read()
{

}



void writeNode(std::stringstream &stream, AGEFile::Node *node)
{
	stream << node->value;
	
	if (node->children.isEmpty()) {
		stream << '\n';
	} else {
		stream << ':';
		writeNode(stream, node->children[0]);

		for (int i = 1; i < node->children.count(); i++) {
			stream << ',';
			writeNode(stream, node->children[i]);
		}

		stream << '\n';
	}
}



void AGEFile::write()
{
	if (_root == nullptr) {
		age_error(k_tag, "Trying to write empty AGEFile.");
		return;
	}

	std::stringstream stream;
	stream << "age:" << k_version << '\n';
	stream << "v:" << _version << '\n';

	writeNode(stream, _root);
	const std::string &str = stream.str();
	file::writeText(_dir, str.c_str(), str.size());
}

}	// namespace age