#pragma once

#include "Core/DArray.hpp"
#include "Core/String.hpp"



namespace age
{

struct Version;

// TODO: Use pool allocator for Strings and DArrays
class AGEFile
{
public:
	struct Node
	{
		Node() = default;

		String value;
		DArray<Node *> children;
	};

	AGEFile();

	Node *getRoot() { return _root; }

	void read();
	void write(const char *dir, const Version &version);

	Node *add(const String &value, Node *parent = nullptr);

private:
	Node *requestNode();

	Node *_root = nullptr;
	DArray<Node> _nodePool = {};
};

}
