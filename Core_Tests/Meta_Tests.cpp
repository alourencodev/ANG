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

class HasCopyOperator {};


TEST_CASE("Check if type has Copy Operator", k_tag)
{
	REQUIRE(isCopyAssignable<HasCopyOperator>::value);
	REQUIRE_FALSE(isCopyAssignable<NoCopyOperator>::value);
}


class HasEqualOperator
{
public:
	bool operator == (const HasEqualOperator &) const { return true; }
};

class NoEqualOperator {};


TEST_CASE("Check if type has equal comparisson operator", k_tag)
{
	REQUIRE(isEqualComparable<HasEqualOperator>::value);
	REQUIRE_FALSE(isEqualComparable<NoEqualOperator>::value);
}


TEST_CASE("Check if the base type is the same", k_tag)
{
	REQUIRE(isSameBaseType<int, int>::value);
	REQUIRE(isSameBaseType<int, int &>::value);
	REQUIRE(isSameBaseType<int, int &&>::value);
	REQUIRE(isSameBaseType<int, const int &>::value);

	REQUIRE_FALSE(isSameBaseType<int, float &>::value);
}


class CopyConstructible
{
public:
	CopyConstructible(const CopyConstructible &) {};
};

class NonCopyConstructible
{
	NonCopyConstructible(const NonCopyConstructible &) = delete;
};


TEST_CASE("Check if class has copy constructor")
{
	REQUIRE(isCopyConstructible<CopyConstructible>::value);
	REQUIRE_FALSE(isCopyConstructible<NonCopyConstructible>::value);
}


TEST_CASE("Check if is class")
{
	REQUIRE(isClass<NoCopyOperator>::value);
	REQUIRE_FALSE(isClass<int>::value);
}
