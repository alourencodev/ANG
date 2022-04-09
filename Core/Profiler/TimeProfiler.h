#pragma once

#include <Core/DArray.hpp>
#include <Core/Timer.hpp>
#include <Core/BuildScheme.hpp>


namespace age
{

class TimeProfiler
{
public:
	TimeProfiler(const char *tag) : _tag(tag) {}
	~TimeProfiler();

	void ping(u64 micros);

	class Instance
	{
	public:
		Instance(TimeProfiler *profiler);
		~Instance();

	private:
		TimeProfiler *_profiler = nullptr;
		Timer _timer;
	};

private:
	DArray<u64> _durations = {};
	const char *_tag = nullptr;
	u64 _nanos = 0;
	u32 _count = 0;
};


#ifdef AGE_PROFILE_ENABLED
#define AGE_PROFILE_TIME_TAG(TAG)										\
	static TimeProfiler __time_profiler(TAG);							\
	TimeProfiler::Instance __time_profiler_instance(&__time_profiler)

#define AGE_PROFILE_TIME() AGE_PROFILE_TIME_TAG(__FUNCTION__)

#else
#define AGE_PROFILE_TIME_TAG(TAG)
#define AGE_PROFILE_TIME()
#endif

}
