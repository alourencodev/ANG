#pragma once

#ifdef _DEBUG
#	define IF_DEBUG(X) X
#	define _RELEASE_SYMB
#else
#	define IF_DEBUG(X)
#endif

#ifdef _TESTS
#	define IF_TEST(X) X
#else
#	define IF_TEST(X)
#endif

#ifdef _RELEASE_SYMB
#	define IF_RELEASE_SYMB(X) X
#else
#	define IF_RELEASE_SYMB(X)
#endif
