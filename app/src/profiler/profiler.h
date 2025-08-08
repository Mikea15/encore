#pragma once

#include "core/core_minimal.h"

#include <vector>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <unordered_map>
#include <mutex>

enum class FrameCategory
{
	UPDATE,
	INPUT,
	CAMERA,
	RENDER
};

class ThreadSafeProfiler
{
public:
	struct Entry
	{
		std::string section;
		u64 timestamp;
		u64 duration;
		u32 depth;
		std::thread::id threadId;

		Entry(const std::string& name, u64 inTimestamp, u64 inDuration, u32 inDepth, std::thread::id tid)
			: section(name), timestamp(inTimestamp), duration(inDuration), depth(inDepth), threadId(tid)
		{}
	};

	// Per-thread profiling data
	struct ThreadData
	{
		std::vector<Entry> entries;
		std::vector<Entry> lastEntries;
		u32 depth = 0;
	};

private:
	// Thread-local storage for profiling data
	thread_local static ThreadData* tl_threadData;

	// Map to store all thread data (for reporting)
	std::unordered_map<std::thread::id, std::unique_ptr<ThreadData>> m_threadDataMap;
	std::mutex m_threadDataMutex;

	static u64 GetTimeUs()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto duration = now.time_since_epoch();
		return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
	}

	ThreadData* GetOrCreateThreadData()
	{
		if(!tl_threadData)
		{
			std::lock_guard<std::mutex> lock(m_threadDataMutex);
			auto threadId = std::this_thread::get_id();

			auto it = m_threadDataMap.find(threadId);
			if(it == m_threadDataMap.end())
			{
				auto threadData = std::make_unique<ThreadData>();
				tl_threadData = threadData.get();
				m_threadDataMap[threadId] = std::move(threadData);
			}
			else
			{
				tl_threadData = it->second.get();
			}
		}
		return tl_threadData;
	}

public:
	u32 BeginSection(const std::string& section_name)
	{
		ThreadData* data = GetOrCreateThreadData();
		auto threadId = std::this_thread::get_id();

		data->entries.emplace_back(section_name, GetTimeUs(), 0, data->depth++, threadId);
		return (u32)(data->entries.size() - 1);
	}

	void EndSection(u32 timerId)
	{
		ThreadData* data = GetOrCreateThreadData();

		if(timerId < data->entries.size())
		{
			data->depth--;
			data->entries[timerId].duration = GetTimeUs() - data->entries[timerId].timestamp;
		}
	}

	void ClearCurrentThread()
	{
		ThreadData* data = GetOrCreateThreadData();
		data->depth = 0;
		data->lastEntries = data->entries;
		data->entries.clear();
	}

	void ClearAllThreads()
	{
		std::lock_guard<std::mutex> lock(m_threadDataMutex);
		for(auto& [threadId, data] : m_threadDataMap)
		{
			data->depth = 0;
			data->lastEntries = data->entries;
			data->entries.clear();
		}
	}

	// Get profiling data for current thread
	const std::vector<Entry>& GetCurrentThreadEntries()
	{
		ThreadData* data = GetOrCreateThreadData();
		return data->lastEntries;
	}

	// Get profiling data for all threads
	std::vector<Entry> GetAllThreadsEntries()
	{
		std::lock_guard<std::mutex> lock(m_threadDataMutex);
		std::vector<Entry> allEntries;

		for(const auto& [threadId, data] : m_threadDataMap)
		{
			allEntries.insert(allEntries.end(), data->lastEntries.begin(), data->lastEntries.end());
		}

		return allEntries;
	}

	std::vector<Entry> GetThreadEntries(std::thread::id threadId)
	{
		std::lock_guard<std::mutex> lock(m_threadDataMutex);
		auto it = m_threadDataMap.find(threadId);
		if(it != m_threadDataMap.end())
		{
			return it->second->lastEntries;
		}
		return {};
	}

	// Get thread names for better reporting
	void SetThreadName(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(m_threadDataMutex);
		m_threadNames[std::this_thread::get_id()] = name;
	}

	std::string GetThreadName(std::thread::id threadId)
	{
		std::lock_guard<std::mutex> lock(m_threadDataMutex);
		auto it = m_threadNames.find(threadId);
		if(it != m_threadNames.end())
		{
			return it->second;
		}

		// Generate a default name based on thread ID
		std::ostringstream oss;
		oss << "Thread_" << threadId;
		return oss.str();
	}

	// Report profiling data grouped by thread
	void PrintReport()
	{
		auto allEntries = GetAllThreadsEntries();

		// Group entries by thread
		std::unordered_map<std::thread::id, std::vector<Entry>> threadGroups;
		for(const auto& entry : allEntries)
		{
			threadGroups[entry.threadId].push_back(entry);
		}

		// Print report for each thread
		for(const auto& [threadId, entries] : threadGroups)
		{
			std::cout << "\n=== " << GetThreadName(threadId) << " ===\n";

			for(const auto& entry : entries)
			{
				std::string indent(entry.depth * 2, ' ');
				std::cout << indent << entry.section << ": "
					<< std::fixed << std::setprecision(3)
					<< entry.duration / 1000.0f << "ms\n";
			}
		}
	}

private:
	std::unordered_map<std::thread::id, std::string> m_threadNames;
};

// Define the thread_local static member
thread_local ThreadSafeProfiler::ThreadData* ThreadSafeProfiler::tl_threadData = nullptr;

// Global profiler instance
static ThreadSafeProfiler g_profiler;

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

// Enhanced macros with thread support
#define PROFILE_FRAME_START() g_profiler.ClearCurrentThread()
#define PROFILE_FRAME_START_ALL_THREADS() g_profiler.ClearAllThreads()
#define PROFILE_SCOPE(name) ProfileSection _timer(name)
#define PROFILE() ProfileSection _timer(__FUNCTION__)
#define PROFILE_SET_THREAD_NAME(name) g_profiler.SetThreadName(name)
#define PROFILE_PRINT_REPORT() g_profiler.PrintReport()

// Alternative: Simpler atomic-based profiler for high-frequency profiling
class AtomicProfiler
{
public:
	struct Entry
	{
		std::string section;
		u64 duration;
		std::thread::id threadId;
	};

private:
	std::vector<Entry> m_entries;
	std::mutex m_entriesMutex;

public:
	void AddEntry(const std::string& section, u64 duration, std::thread::id threadId)
	{
		std::lock_guard<std::mutex> lock(m_entriesMutex);
		m_entries.emplace_back(Entry{ section, duration, threadId });
	}

	void Clear()
	{
		std::lock_guard<std::mutex> lock(m_entriesMutex);
		m_entries.clear();
	}

	std::vector<Entry> GetEntries()
	{
		std::lock_guard<std::mutex> lock(m_entriesMutex);
		return m_entries;
	}
};

class SimpleProfileSection
{
public:
	SimpleProfileSection(AtomicProfiler& profiler, const std::string& sectionName)
		: m_profiler(profiler), m_sectionName(sectionName)
	{
		m_startTime = std::chrono::high_resolution_clock::now();
	}

	~SimpleProfileSection()
	{
		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime).count();
		m_profiler.AddEntry(m_sectionName, duration, std::this_thread::get_id());
	}

private:
	AtomicProfiler& m_profiler;
	std::string m_sectionName;
	std::chrono::high_resolution_clock::time_point m_startTime;
};
