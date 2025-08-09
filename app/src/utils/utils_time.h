#pragma once

#include "core/core_minimal.h"
#include <string>

namespace utils
{
	static std::string FormatDuration(u64 duration, bool bShowMicroseconds)
	{
		if(bShowMicroseconds)
		{
			if(duration >= SEC_TO_US(1)) // >= 1 second
				return std::to_string(US_TO_SEC(duration)) + " s";
			
			if(duration >= SEC_TO_MS(1)) // >= 1 ms
				return std::to_string(US_TO_MS(duration)) + " ms";
			
			return std::to_string(duration) + " us";
		}

		return std::to_string(US_TO_MS(duration)) + " ms";
	}
}
