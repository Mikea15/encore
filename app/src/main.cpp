#include "core/core_minimal.h"

#include "game_engine.h"

#define TEST 0
#if TEST
namespace StubWorkload {
	void math_workload(int iterations)
	{
		volatile double result = 1.0;
		for(int i = 0; i < iterations; i++)
		{
			result = sin(result) * cos(result) + sqrt(fabs(result));
			result = fmod(result, 100.0) + 1.0; // Keep result bounded
		}
	}
}
#endif

i32 main(i32 argc, char* argv[])
{
	GameEngine engine;
	return engine.Run();
}
