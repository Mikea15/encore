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

extern "C" // Ensures C linkage - prevents C++ name mangling
{
	// Tells NVIDIA driver to use the dedicated GPU for this application
	__declspec(dllexport) // Exports this symbol so the NVIDIA driver can see it
		unsigned long NvOptimusEnablement = 0x00000001; // Setting to 1 enables dedicated GPU

	__declspec(dllexport)
		int AmdPowerXpressRequestHighPerformance = 1;
}
