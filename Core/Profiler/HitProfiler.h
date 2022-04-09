#pragma once

#include <Core/Types.hpp>
#include <Core/BuildScheme.hpp>


namespace age
{

class HitProfiler
{
public:
	HitProfiler(const char *tag) : _tag(tag) {}
	~HitProfiler();

	void ping() { _hits++; };

	class Instance
	{
	public:
		Instance(HitProfiler *profiler) : _profiler(profiler) {}
		~Instance()	{ _profiler->ping(); };

	private:
		HitProfiler *_profiler = nullptr;
	};

private:
	const char *_tag = nullptr;
	u64 _hits = 0;
};


#ifdef AGE_PROFILE_ENABLED
#define AGE_PROFILE_HIT_TAG(TAG)										\
	static HitProfiler __hit_profiler(TAG);								\
	HitProfiler::Instance __hit_profiler_instance(&__hit_profiler)

#define AGE_PROFILE_HIT() AGE_PROFILE_HIT_TAG(__FUNCTION__)

#else
#define AGE_PROFILE_HIT_TAG(TAG)
#define AGE_PROFILE_HIT()
#endif

}
