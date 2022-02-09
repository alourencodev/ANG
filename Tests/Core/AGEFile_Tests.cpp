#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/AGEFile/AGEFile.h>

constexpr static char k_tag[] = "[AGEFile]";

using namespace age;

TEST_CASE("Write Minimal AGEFile")
{
	AGEFile file("Out/Minimal.age", {1, 2, 3});
	auto *node = file.requestNode();
	node->value = "Name";
	file.getRoot()->children.add(node);
	file.write();
}



TEST_CASE("Write AGEFile with all types")
{
	AGEFile file("Out/AllTypes.age", {1, 2, 3});

	auto *nameNode = file.requestNode();
	file.getRoot()->children.add(nameNode);
	nameNode->value = "Name";

	// Key Node
	auto *keyNode = file.requestNode();
	nameNode->children.add(keyNode);
	keyNode->value = "Key";

	// Value Node
	auto *valueNode = file.requestNode();
	keyNode->children.add(valueNode);
	valueNode->value = "Value";

	// List Node
	auto *listNode = file.requestNode();
	file.getRoot()->children.add(listNode);
	listNode->value = "List";

	auto *elementA = file.requestNode();
	elementA->value = "Element A";

	auto *elementB = file.requestNode();
	elementB->value = "Element B";

	auto *elementC = file.requestNode();
	elementC->value = "Element C";

	listNode->children.add({elementA, elementB, elementC});

	file.write();
}



TEST_CASE("Write AGEFile with list of Objects")
{

}
