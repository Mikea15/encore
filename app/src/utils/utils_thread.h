#pragma once

// Platform-specific includes for thread naming
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <pthread.h>
#endif

#include <string>

namespace utils {
	static void NameThread(const std::string& name)
	{
#ifdef _WIN32
		// Windows implementation
		const std::wstring wName(name.begin(), name.end());
		SetThreadDescription(GetCurrentThread(), wName.c_str());
#elif defined(__linux__)
		// Linux implementation
		pthread_setname_np(pthread_self(), name.substr(0, 15).c_str()); // Linux limits to 15 chars
#elif defined(__APPLE__)
		// macOS implementation
		pthread_setname_np(name.c_str());
#endif
	}
}
