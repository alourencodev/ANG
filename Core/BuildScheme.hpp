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

#ifndef AGE_RELEASE_DBG_INFO
#	define AGE_RELEASE_DBG_INFO
#endif

#	define AGE_MEMORY_DBG
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


#if defined(AGE_DEBUG)
#	define BUILD_SCHEME_STR "Debug"
#elif defined(AGE_RELEASE_DBG_INFO)
#	define BUILD_SCHEME_STR "ReleaseDbgInfo"
#else
#	define BUILD_SCHEME_STR ""
#endif


#ifdef AGE_MEMORY_DBG
#define IF_MEMORY_DBG(X) X
#else
#define IF_MEMORY_DBG(X)
#endif


#if !defined(AGE_DEBUG) && !defined(AGE_RELEASE_DBG_INFO)
#	define AGE_RELEASE
#endif

