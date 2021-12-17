#include <Tests/Vendor/Catch2/catch.hpp>

#include <Core/BitField.hpp>


constexpr static char k_tag[] = "[BitField]";

using namespace age;

TEST_CASE("BitField Test", k_tag)
{
	BitField bitfield;

	REQUIRE(bitfield.isClear());

	SECTION("Bit manipulation")
	{
		// Set to 0000 0010
		bitfield.set(1);
		REQUIRE(bitfield == static_cast<u8>(0x02));

		// Set to 0000 0110
		bitfield.set(2);
		REQUIRE(bitfield == static_cast<u8>(0x06));

		REQUIRE(bitfield.isSet(1));
		REQUIRE(bitfield.isSet(2));
		REQUIRE(bitfield.isSet(BitField(0x06)));
		REQUIRE_FALSE(bitfield.isSet(BitField(0x02)));

		// Set to 0000 0100
		bitfield.clear(1);
		REQUIRE(bitfield.isSet(2));
		REQUIRE_FALSE(bitfield.isSet(1));
		REQUIRE(bitfield == static_cast<u8>(0x04));

		// Set to 0000 0001
		bitfield.toggle(2);
		bitfield.toggle(0);
		REQUIRE(bitfield == static_cast<u8>(0x01));
	}
}
