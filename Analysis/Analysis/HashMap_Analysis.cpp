
#include "HashMap_Analysis.h"

#include <Core/HashMap.hpp>
#include <Core/Log/Log.h>
#include <Core/Profiler/TimeProfiler.h>
#include <Core/Random.h>
#include <Core/String.hpp>


namespace age::hashMap
{

constexpr char k_tag[] = "HashMapAnalysis";

void runAddAnalysis()
{
	HashMap<const char *, int> map = {{"aba", 1}, {"baba", 1}, {"other", 3}};
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


void runHitAnalysis()
{
	DArray<u32> existing;
	HashMap<u32, u32> map(100);

	for (int i = 0; i < 100; i++) {
		u32 rand = randomInt<u32>(0, 999);
		
		if (existing.contains(rand)) {
			i--;
			continue;
		}

		existing.add(rand);
		map.add(rand, 0);
	}
}


void runAnalysis()
{
	age::logger::enable(k_tag);

	//for (int i = 0; i < 5; i++)
	//	runAddAnalysis();

	for (int i = 0; i < 10; i++)
		runHitAnalysis();
}

}