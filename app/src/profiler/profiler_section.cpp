#include "profiler_section.h"

#include "profiler.h"

ProfileSection::ProfileSection(const char* name)
	: timerId(Profiler::GetInstance().BeginSection(name))
{}

ProfileSection::~ProfileSection()
{
	Profiler::GetInstance().EndSection(timerId);
}
