#include <Core/Profiler/TimeProfiler.h>

#include <Core/Log/Log.h>

#include <sstream>

namespace age
{

constexpr char k_tag[] = "TimeProfiler";

// TODO: Have smart conversion for different time units

TimeProfiler::~TimeProfiler()
{
	const float micros = static_cast<float>(_nanos) * 0.001f;
	const float avg = micros / _count;

	age_forceLog(k_tag, " ---------- // ---------- // ---------- ");
	age_forceLog(k_tag, "Profiling %s", _tag);
	age_forceLog(k_tag, "Total Time = %.3f us | Count = %d | Average = %.3f us", micros, _count, avg);

	std::stringstream sstream;
	sstream << _durations;
	std::string durationsStr = sstream.str();

	age_forceLog(k_tag, "Samples in ns: %s", durationsStr.c_str());

	age_forceLog(k_tag, " ---------- // ---------- // ---------- \n");
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