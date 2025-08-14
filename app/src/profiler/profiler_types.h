#pragma once

#include "core/core_minimal.h"

#include <thread>
#include <vector>

enum class FrameCategory
{
	UPDATE,
	INPUT,
	CAMERA,
	RENDER
};

struct ProfilerEntry
{
	const char* section;
	u64 timestamp;
	u64 duration;
	std::thread::id threadId;
	u8 depth;

	ProfilerEntry(const char* name, u64 inTimestamp, u64 inDuration, u8 inDepth, std::thread::id tid)
		: section(name), timestamp(inTimestamp), duration(inDuration), depth(inDepth), threadId(tid)
	{}
};

// Per-thread profiling data
struct ProfilerThreadData
{
	std::vector<ProfilerEntry> entries;
	std::vector<ProfilerEntry> lastEntries;
	u8 depth = 0;
};
