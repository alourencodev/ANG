#pragma once

#include <chrono>

#include <Core/Types.hpp>


namespace age
{

class Timer
{
public:
	void start() { mStartTimePoint = Clock::now(); };
	u64 micros() const { return std::chrono::duration_cast<Micros>(Clock::now() - mStartTimePoint).count(); }
	u64 millis() const { return std::chrono::duration_cast<Millis>(Clock::now() - mStartTimePoint).count(); };
	u64 secs() const { return std::chrono::duration_cast<Secs>(Clock::now() - mStartTimePoint).count(); };

private:
	using Clock = std::chrono::steady_clock;
	using TimePoint = std::chrono::time_point<Clock>;
	using Micros = std::chrono::microseconds;
	using Millis = std::chrono::milliseconds;
	using Secs = std::chrono::seconds;
	
	TimePoint mStartTimePoint = TimePoint::min();
};

}	// namespace age


