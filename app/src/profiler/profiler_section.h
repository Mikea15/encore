#pragma once

#include "core/core_minimal.h"

class ProfileSection
{
public:
	ProfileSection(const char* name);
	~ProfileSection();

	NO_COPY(ProfileSection);
	NO_MOVE(ProfileSection);

private:
	u32 timerId;
};
