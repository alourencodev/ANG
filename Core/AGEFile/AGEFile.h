#pragma once

#include "Core/DArray.hpp"
#include "Core/Pair.hpp"
#include "Core/String.hpp"
#include "Core/Version.hpp"

namespace age
{

class AGEFile
{
public:
	struct Node
	{
	public:
		DArray<Node> children;


	private:
		enum class EType : u8
		{
			Value,
			List,
			Map
		};

		Node(EType type) : _type(type) {}
		~Node() {};

		String _key;

		union
		{
			String _value;
			DArray<String> _list;
			DArray<Pair<String,String>> _map;
		};

		EType _type;

		friend class AGEFile;
	};

	AGEFile(const char *dir, const Version &version) : _dir(String(dir)), _version(version) {};

	void read();
	void write();

	Node *addValue(const char *key, const char *value, Node *parent = nullptr);
	Node *addList(const char *key, const DArray<String> *list, Node *parent = nullptr);
	Node *addMap(const char *key, const DArray<Pair<String, String>> *map, Node *parent = nullptr);

private:
	String _dir;
	Version _version;

	DArray<Node> nodes;
};

}
