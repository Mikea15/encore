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
		u64 start_time;
		u64 duration;

		Entry(const std::string& name, u64 start, u64 dur)
			: section(name), start_time(start), duration(dur)
		{}
	};

	std::vector<Entry> entries;
private:

	// Get current time in microseconds
	static u64 get_time_us()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = now.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	}

public:
	// Start timing a section and return a timer ID
	size_t begin_section(const std::string& section_name)
	{
		entries.emplace_back(section_name, get_time_us(), 0);
		return entries.size() - 1;
	}

	// End timing for a specific section
	void end_section(size_t timer_id)
	{
		if(timer_id < entries.size())
		{
			u64 end_time = get_time_us();
			entries[timer_id].duration = end_time - entries[timer_id].start_time;
		}
	}

	// Clear all entries (call at start of each frame)
	void clear()
	{
		entries.clear();
	}

	// Print profiling results
	void print_results() const
	{
		std::cout << "\n=== Profiling Results ===\n";
		std::cout << std::left << std::setw(20) << "Section"
			<< std::right << std::setw(15) << "Duration (us)" << "\n";
		std::cout << std::string(35, '-') << "\n";

		for(const auto& entry : entries)
		{
			std::cout << std::left << std::setw(20) << entry.section
				<< std::right << std::setw(15) << entry.duration << "\n";
		}
		std::cout << std::string(35, '=') << "\n";
	}

	// Get all entries (for custom processing)
	const std::vector<Entry>& get_entries() const { return entries; }
};

// Global profiler instance
static Profiler g_profiler;

// RAII helper class for automatic timing
class ScopedTimer
{
private:
	size_t timer_id;

public:
	ScopedTimer(const std::string& section_name)
		: timer_id(g_profiler.begin_section(section_name))
	{}

	~ScopedTimer()
	{
		g_profiler.end_section(timer_id);
	}

	// Non-copyable, non-movable to avoid issues
	ScopedTimer(const ScopedTimer&) = delete;
	ScopedTimer& operator=(const ScopedTimer&) = delete;
	ScopedTimer(ScopedTimer&&) = delete;
	ScopedTimer& operator=(ScopedTimer&&) = delete;
};

// Convenience macros
#define PROFILE_SCOPE(name) ScopedTimer _timer(name)
#define PROFILE_FUNCTION() ScopedTimer _timer(__FUNCTION__)

// Manual profiling functions
inline void profile_frame_start() { g_profiler.clear(); }
inline void profile_print() { g_profiler.print_results(); }

