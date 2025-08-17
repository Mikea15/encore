#pragma once

#include "core/core_minimal.h"
#include "utils/string_factory.h"

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

namespace utils {
	static const char* GetCurrentWorkingDirectory()
	{
		char buffer[1024];
		if(GetCurrentDir(buffer, sizeof(buffer)) != nullptr)
		{
			return StringFactory::MakeString(buffer);
		}
		return "N/A";
	}
};
