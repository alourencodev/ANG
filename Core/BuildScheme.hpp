#pragma once

#ifdef AGE_UNIT_TEST
#	ifndef AGE_DEBUG
#		define AGE_DEBUG
#	endif
#	define IF_TEST(X) X
#else
#	define IF_TEST(X)
#endif

#ifdef AGE_DEBUG
#	define IF_DEBUG(X) X
#	define AGE_RELEASE_DBG_INFO
#else
#	define IF_DEBUG(X)
#endif

#ifdef AGE_RELEASE_DBG_INFO
#	define IF_RELEASE_DBG_INFO(X) X
#	define AGE_ASSERT_ENABLED
#	define AGE_LOG_ENABLED
#else
#	define IF_RELEASE_DBG_INGO(X)
#endif
