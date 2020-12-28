#pragma once

#ifdef _DEBUG
#define IF_DEBUG(X) X
#else
#define IF_DEBUG(X)
#endif

#ifdef _TESTS
#define IF_TEST(X) X
#else
#define IF_TEST(X)
#endif
