#pragma once

#include "core/core_minimal.h"

#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>

class Profiler
{
public:
	struct Entry
	{
		std::string section;
		u64 timestamp;
		u64 duration;
		u32 depth;

		Entry(const std::string& name, u64 inTimestamp, u64 inDuration, u32 inDepth)
			: section(name), timestamp(inTimestamp), duration(inDuration), depth(inDepth)
		{}
	};

	std::vector<Entry> entries;
	u32 depth = 0;
private:

	static u64 GetTimeUs()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = now.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	}

public:
	u32 BeginSection(const std::string& section_name)
	{
		entries.emplace_back(section_name, GetTimeUs(), 0, depth++);
		return entries.size() - 1;
	}

	void EndSection(u32 timerId)
	{
		if(timerId < entries.size())
		{
			depth--;
			entries[timerId].duration = GetTimeUs() - entries[timerId].timestamp;
		}
	}

	void clear()
	{
		depth = 0;
		entries.clear();
	}
};

// Global profiler instance
static Profiler g_profiler;

class ProfileSection
{
public:
	ProfileSection(const std::string& sectionName)
		: timerId(g_profiler.BeginSection(sectionName))
	{}

	~ProfileSection()
	{
		g_profiler.EndSection(timerId);
	}

	NO_COPY(ProfileSection);
	NO_MOVE(ProfileSection);

private:
	u32 timerId;
};

// Convenience macros
#define PROFILE_FRAME_START() g_profiler.clear();
#define PROFILE_SCOPE(name) ProfileSection _timer(name)
#define PROFILE() ProfileSection _timer(__FUNCTION__)

