#include <Core/Profiler/HitProfiler.h>

#include <Core/Log/Log.h>


namespace age
{

constexpr char k_tag[] = "HitProfiler";

HitProfiler::~HitProfiler()
{
	age_forceLog(k_tag, " ---------- // ---------- // ---------- ");
	age_forceLog(k_tag, "Profiling %s", _tag);
	age_forceLog(k_tag, "Total hits %d", _hits);
	age_forceLog(k_tag, " ---------- // ---------- // ---------- ");
}

}