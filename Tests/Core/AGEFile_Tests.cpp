#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/AGEFile/AGEFile.h>
#include <Core/Version.hpp>

constexpr static char k_tag[] = "[AGEFile]";

using namespace age;

TEST_CASE("Write Minimal AGEFile")
{
	AGEFile file;
	file.add("Name");
	file.write("Out/Minimal.age", {1, 2, 3});
}



TEST_CASE("Write AGEFile with all types")
{
	AGEFile file;

	auto *nameNode = file.add("Name");
	auto *keyNode = file.add("Key", nameNode);
	auto *valueNode = file.add("Value", keyNode);

	// Chain of Key Values
	auto *chainA = file.add("ChainA");
	auto *chainB = file.add("ChainB", chainA);
	auto *chainC = file.add("ChainC", chainB);
	auto *chainD = file.add("ChainD", chainC);

	// List
	auto *listNode = file.add("List");
	file.add("Element A", listNode);
	file.add("Element B", listNode);
	file.add("Element C", listNode);

	file.write("Out/AllTypes.age", {1, 2, 3});
}



TEST_CASE("Write AGEFile with list of Objects")
{
	AGEFile file;

	// Objects
	auto *objects = file.add("Objects");
		auto *objectA = file.add("Object A", objects);
			auto *parameterA = file.add("ParameterA", objectA);
				auto *listA = file.add("ListA", parameterA);
					auto *listAKeyA = file.add("KeyA", listA);
						file.add("ValueA", listAKeyA);
					auto *listAKeyB = file.add("KeyB", listA);
						file.add("ValueB", listAKeyB);
				auto *keyA = file.add("KeyA", parameterA);
					file.add("Value Of Key A", keyA);
			auto *mapA = file.add("Map A", objectA);
				auto mapKeyA = file.add("MapKeyA", mapA);
					file.add("MapValueA", mapKeyA);
				auto mapKeyB = file.add("MapKeyB", mapA);
					file.add("MapValueB", mapKeyB);
				auto mapKeyC = file.add("MapKeyC", mapA);
					file.add("MapValueC", mapKeyC);
		auto *objectB = file.add("Object B", objects);
			auto *listB = file.add("ListB", objectB);
				auto *listBKeyA = file.add("KeyA", listB);
					file.add("ValueA", listBKeyA);
				auto *listBKeyB = file.add("KeyB", listB);
					file.add("ValueB", listBKeyB);

	file.write("Out/ObjectList.age", {1, 2, 3});
}
