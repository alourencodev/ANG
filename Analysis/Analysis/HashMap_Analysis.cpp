
#include "HashMap_Analysis.h"

#include <Core/HashMap.hpp>
#include <Core/Log/Log.h>
#include <Core/Profiler/TimeProfiler.h>

#include <string>

namespace age::hashMap
{

constexpr char k_tag[] = "HashMapAnalysis";

void runAddAnalysis()
{
	HashMap<std::string, int> map = {{"aba", 1}, {"baba", 1}, {"other", 3}};
	map.add("first", 4);
	map.add("second", 5);
	map.add("resize", 6);
	map.add("another", 8);
	map.add("yanother", 8);
	map.add("yaanother", 8);
	map.add("yaaaanother", 8);
	map.add("yoanother", 8);
	map.add("yooanother", 8);
	map.add("yoooanother", 8);
	map.add("yeanother", 8);
	map.add("yeeanother", 8);
	map.add("yeeeanother", 8);
	map.add("another1", 8);
	map.add("another2", 8);
	map.add("another3", 8);
}

void runAnalysis()
{
	age::logger::enable(k_tag);

	runAddAnalysis();
	runAddAnalysis();
	runAddAnalysis();
	runAddAnalysis();
}

}