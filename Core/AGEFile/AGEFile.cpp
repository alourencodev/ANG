#include "AGEFile.h"

#include "Core/File.h"

namespace age
{

void AGEFile::read()
{

}



void AGEFile::write()
{

}



AGEFile::Node* AGEFile::addValue(const char* key, const char* value, AGEFile::Node* parent /*= nullptr*/)
{
	Node node(Node::EType::Value);

	// TODO: Populate the node

	if (parent == nullptr) {
		nodes.add(std::move(node));
		return &nodes.back();
	} else {
		parent->children.add(node);
		return &parent->children.back();
	}
}



AGEFile::Node* AGEFile::addList(const char* key, const DArray<String>* list, AGEFile::Node* parent /*= nullptr*/)
{
	Node node(Node::EType::List);

	// TODO: Populate the node

	if (parent == nullptr) {
		nodes.add(std::move(node));
		return &nodes.back();
	} else {
		parent->children.add(node);
		return &parent->children.back();
	}
}



AGEFile::Node* AGEFile::addMap(const char* key, const DArray<Pair<String, String>>* map, AGEFile::Node* parent /*= nullptr*/)
{
	Node node(Node::EType::Map);

	// TODO: Populate the node

	if (parent == nullptr) {
		nodes.add(std::move(node));
		return &nodes.back();
	} else {
		parent->children.add(node);
		return &parent->children.back();
	}
}

}