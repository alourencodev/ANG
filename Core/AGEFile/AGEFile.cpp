#include "AGEFile.h"

#include <sstream>

#include "Core/BuildScheme.hpp"
#include "Core/File.h"
#include "Core/Log/Log.h"
#include "Core/StringBuilder.hpp"
#include "Core/Version.hpp"



namespace age
{

constexpr const char k_tag[] = "AGEFile";
constexpr u8 k_preallocNodes = 32;

constexpr Version k_version = {0, 1, 0};

AGEFile::AGEFile()
{
	_nodePool.reserve(k_preallocNodes);
	_root = requestNode();
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



void writeNode(std::stringstream &stream, AGEFile::Node *node, u8 scopeDepth = 0)
{
	for (int i = 0; i < scopeDepth; i++)
		stream << '\t';

	stream << node->value;
	
	if (node->children.isEmpty())
		return;

	{	// Write Children
		const bool isList = node->children.count() > 1;

		if (isList) {
			stream << "{\n";

			const u8 childScopeDepth = scopeDepth + 1;
			writeNode(stream, node->children[0], childScopeDepth);
			for (int i = 1; i < node->children.count(); i++) {
				stream << ",\n";
				writeNode(stream, node->children[i], childScopeDepth);
			}

			stream << '\n';
			for (int i = 0; i < scopeDepth; i++)
				stream << '\t';

			stream << "}";

		} else {
			const bool childIsValue = node->children[0]->children.isEmpty();
			if (!childIsValue) {
				stream << '\n';
				writeNode(stream, node->children[0], scopeDepth + 1);
			} else {
				stream << ':';
				writeNode(stream, node->children[0]);
			}
		}
	}
}



void AGEFile::write(const char *dir, const Version &version)
{
	if (_root == nullptr || _root->children.isEmpty()) {
		age_error(k_tag, "Trying to write empty AGEFile.");
		return;
	}

	std::stringstream stream;
	stream << "age:" << k_version << '\n';
	stream << "v:" << version << '\n';

	writeNode(stream, _root->children[0]);
	for (int i = 1; i < _root->children.count(); i++) {
		stream << '\n';
		writeNode(stream, _root->children[i]);
	}

	const std::string &str = stream.str();
	file::writeText(dir, str.c_str(), str.size());
}



AGEFile::Node *AGEFile::add(const String &value, Node *parent /*= nullptr*/)
{
	Node *node = requestNode();
	node->value = value;

	Node *parentNode = parent == nullptr ? _root : parent;
	parentNode->children.add(node);

	return node;
}

}	// namespace age