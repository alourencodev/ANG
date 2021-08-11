#pragma once

#include "Core/Log/Assert.hpp"
#include "Core/Types.hpp"

#define age_assertBitIndex(bitIndex) age_assertFatal(bitIndex < 8, "Bit index must be smaller than 8.")

namespace age
{
  
class BitField
{
public:
	BitField() = default;
	BitField(byte field) : _field(field) {}
	BitField(const BitField &other) : _field(other._field) {}

	void setAll() { _field = 0xFF; }
	void clearAll() { _field = 0x00; }

	void set(u8 bitIndex) { age_assertBitIndex(bitIndex); _field |= 1 << bitIndex; }
	void clear(u8 bitIndex) { age_assertBitIndex(bitIndex); _field &= ~(1 << bitIndex); }
	void toggle(u8 bitIndex) { age_assertBitIndex(bitIndex); _field ^= 1 << bitIndex; }

	bool isSet(u8 bitIndex) const { age_assertBitIndex(bitIndex); return (_field & (1 << bitIndex)) > 0; }
	bool isSet(BitField other) const { return (_field & other._field) == _field; }
	bool isClear() const { return _field == 0; }
	
	BitField operator & (BitField other) const { return BitField(_field & other._field); }
	void operator &= (BitField other) { _field &= other._field; }

	BitField operator | (BitField other) const { return BitField(_field | other._field); }
	void operator |= (BitField other) { _field |= other._field; }

	BitField operator ^ (BitField other) const { return BitField(_field ^ other._field); }
	void operator ^= (BitField other) { _field ^= other._field; }

	bool operator == (BitField other) { return _field == other._field; }
	bool operator == (byte field) { return _field == field; }

	BitField operator << (i32 offset) { return BitField(_field << offset); }
	BitField operator >> (i32 offset) { return BitField(_field >> offset); }
	BitField operator ~ () const { return BitField(~_field); }

	operator byte () const { return _field; }

private:
	byte _field = 0x00;
};

} // namespace age

#undef age_assertBitIndex
