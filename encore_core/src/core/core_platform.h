#pragma once

#if defined(_DEBUG) || defined(DEBUG)
#define ENC_DEBUG 1
#else
#define ENC_DEBUG 0
#endif

#if defined(NDEBUG)
#define ENC_RELEASE 1
#else
#define ENC_RELEASE 0
#endif

extern "C"
{
	__declspec(dllexport) extern unsigned long NvOptimusEnablement;
	__declspec(dllexport) extern int AmdPowerXpressRequestHighPerformance;
}
