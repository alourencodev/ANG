
#include <Core/NonCopyable.hpp>

class TestNonCopyable : public NonCopyable
{
public:
	TestNonCopyable() : n(0) {}
	TestNonCopyable(int inN) : n(inN) {}

	bool operator==(const TestNonCopyable &other) { return other.n == n;}

	int n;
};