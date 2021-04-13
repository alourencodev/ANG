#include <catch.hpp>

#include <Core/Meta.hpp>


using namespace meta;

constexpr static char k_tag[] = "[Meta]";


TEST_CASE("Templated constants", k_tag)
{
	REQUIRE(vConst<int, 1>::value == 1);
	REQUIRE(vTrue::value);
	REQUIRE_FALSE(vFalse::value);
}


TEST_CASE("Templated Type comparison", k_tag)
{
	REQUIRE(isSame<int, int>::value);
	REQUIRE_FALSE(isSame<int, float>::value);

	REQUIRE(areSame<int, int, int, int>::value);
	REQUIRE_FALSE(areSame<int, char, float, int>::value);
}


class NoCopyOperator
{
public:
	NoCopyOperator() = default;
	NoCopyOperator &operator = (const NoCopyOperator &) = delete;
};

class HasCopyOperator
{
public:
	HasCopyOperator() = default;
};


TEST_CASE("Check if type has Copy Operator", k_tag)
{
	REQUIRE_FALSE(isCopyAssignable<NoCopyOperator>::value);
	REQUIRE(isCopyAssignable<HasCopyOperator>::value);
}
