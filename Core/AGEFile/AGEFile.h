#pragma once

#include "Core/DArray.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"



namespace age
{

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

	AGEFile(const String &dir, const Version &version);

	Node *requestNode();
	void setRoot(Node *root) { _root = root; }

	void read();
	void write();

private:
	Node *_root = nullptr;
	DArray<Node> _nodePool = {};
	String _dir = {};
	Version _version = {0, 0, 0};
};

}
