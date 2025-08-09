#pragma once

#include "core/core_minimal.h"
#include "utils/string_factory.h"

namespace utils
{
	static const char* FormatDuration(u64 duration, bool bShowMicroseconds)
	{
		if(bShowMicroseconds)
		{
			if(duration >= SEC_TO_US(1)) // >= 1 second
				return StringFactory::Format("%d s", (US_TO_SEC(duration)));
			
			if(duration >= SEC_TO_MS(1)) // >= 1 ms
				return StringFactory::Format("%d ms", (US_TO_MS(duration)));
			
			return StringFactory::Format("%d us", (duration));
		}

		return StringFactory::Format("%d ms", (US_TO_MS(duration)));
	}
}
