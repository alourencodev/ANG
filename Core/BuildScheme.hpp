#pragma once

#ifdef AGE_UNIT_TEST
#	define AGE_DEBUG
#	define IF_TEST(X) X
#else
#	define IF_TEST(X)
#endif

#if _MSC_VER
#   ifdef AGE_CMAKE_Debug
#   	ifndef AGE_DEBUG
#   		define AGE_DEBUG
#   	endif
#   endif
#endif

#ifdef AGE_DEBUG
#	define IF_DEBUG(X) X
#	define AGE_RELEASE_DBG_INFO 1
#else
#	define IF_DEBUG(X)
#endif

#if _MSC_VER
#   ifdef AGE_CMAKE_ReleaseDbgInfo
#   	ifndef AGE_RELEASE_DBG_INFO
#   		define AGE_RELEASE_DBG_INFO
#   	endif
#   endif
#endif

#ifdef AGE_RELEASE_DBG_INFO
#	define IF_RELEASE_DBG_INFO(X) X
#	define AGE_ASSERT_ENABLED
#	define AGE_LOG_ENABLED
#else
#	define IF_RELEASE_DBG_INGO(X)
#endif
