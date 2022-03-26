#include <Core/Profiler/TimeProfiler.h>

#include <Core/Log/Log.h>

namespace age
{

constexpr char k_tag[] = "TimeProfiler";

TimeProfiler::~TimeProfiler()
{
	float avg = static_cast<float>(_micros) / static_cast<float>(_count);

	age_log(k_tag, "Profiling %s", _tag);
	age_log(k_tag, "Total Time = %d | Count = %d | Average = %.f", _micros, _count, avg);
	age_log(k_tag, " ----- // ----- // ----- ");
}



void TimeProfiler::ping(u64 micros)
{
	_micros += micros;
	_count++;
}



TimeProfiler::Instance::Instance(TimeProfiler *profiler)
{
	_profiler = profiler;
	_timer.start();
}



TimeProfiler::Instance::~Instance()
{
	_profiler->ping(_timer.micros());
}


}