#include <Core/Profiler/TimeProfiler.h>

#include <Core/Log/Log.h>

#include <sstream>

namespace age
{

constexpr char k_tag[] = "TimeProfiler";

// TODO: Have smart conversion for different time units

TimeProfiler::~TimeProfiler()
{
	float avg = static_cast<float>(_nanos) / static_cast<float>(_count);

	age_forceLog(k_tag, "Profiling %s", _tag);
	age_forceLog(k_tag, "Total Time = %d ns | Count = %d | Average = %.f ns", _nanos, _count, avg);

	std::stringstream sstream;
	sstream << _durations;
	std::string durationsStr = sstream.str();

	age_forceLog(k_tag, "%s", durationsStr.c_str());

	age_forceLog(k_tag, " ----- // ----- // ----- ");
}



void TimeProfiler::ping(u64 nanos)
{
	_durations.add(nanos);
	_nanos += nanos;
	_count++;
}



TimeProfiler::Instance::Instance(TimeProfiler *profiler)
{
	_profiler = profiler;
	_timer.start();
}



TimeProfiler::Instance::~Instance()
{
	_profiler->ping(_timer.nanos());
}


}