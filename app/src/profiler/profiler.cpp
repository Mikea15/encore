#include "profiler.h"

#include "utils/string_factory.h"

#include <chrono>

// Define the thread_local static member
thread_local ProfilerThreadData* Profiler::tl_threadData = nullptr;

u32 Profiler::BeginSection(const char* section_name)
{
	ProfilerThreadData* data = GetOrCreateThreadData();
	auto threadId = std::this_thread::get_id();

	data->entries.emplace_back(section_name, GetTimeUs(), 0, data->depth++, threadId);
	return (u32)(data->entries.size() - 1);
}

void Profiler::EndSection(u32 timerId)
{
	ProfilerThreadData* data = GetOrCreateThreadData();

	if(timerId < data->entries.size())
	{
		data->depth--;
		data->entries[timerId].duration = GetTimeUs() - data->entries[timerId].timestamp;
	}
}

void Profiler::ClearCurrentThread()
{
	ProfilerThreadData* data = GetOrCreateThreadData();
	data->depth = 0;
	data->lastEntries = data->entries;
	data->entries.clear();
}

void Profiler::ClearAllThreads()
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
const std::vector<ProfilerEntry>& Profiler::GetCurrentThreadEntries()
{
	ProfilerThreadData* data = GetOrCreateThreadData();
	return data->lastEntries;
}

// Get profiling data for all threads
std::vector<ProfilerEntry> Profiler::GetAllThreadsEntries()
{
	std::lock_guard<std::mutex> lock(m_threadDataMutex);
	std::vector<ProfilerEntry> allEntries;

	for(const auto& [threadId, data] : m_threadDataMap)
	{
		allEntries.insert(allEntries.end(), data->lastEntries.begin(), data->lastEntries.end());
	}

	return allEntries;
}

std::vector<ProfilerEntry> Profiler::GetThreadEntries(std::thread::id threadId)
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
void Profiler::SetThreadName(const char* name)
{
	std::lock_guard<std::mutex> lock(m_threadDataMutex);
	m_threadNames[std::this_thread::get_id()] = name;
}

const char* Profiler::GetThreadName(std::thread::id threadId)
{
	std::lock_guard<std::mutex> lock(m_threadDataMutex);
	auto it = m_threadNames.find(threadId);
	if(it != m_threadNames.end())
	{
		return it->second;
	}

	return StringFactory::TempFormat("Thread_%d", threadId);
}

// Report profiling data grouped by thread
void Profiler::PrintReport()
{
	auto allEntries = GetAllThreadsEntries();

	std::unordered_map<std::thread::id, std::vector<ProfilerEntry>> threadGroups;
	for(const auto& entry : allEntries)
	{
		threadGroups[entry.threadId].push_back(entry);
	}

	for(const auto& [threadId, entries] : threadGroups)
	{
		std::cout << "\n=== " << GetThreadName(threadId) << " ===\n";

		for(const auto& entry : entries)
		{
			for(int i = 0; i < entry.depth; i++)
			{
				printf("\t");
			}
			printf("%s: %.4fms\n", entry.section, NS_TO_MS((f32)entry.duration));
		}
	}
}

u64 Profiler::GetTimeUs()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = now.time_since_epoch();
	return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

ProfilerThreadData* Profiler::GetOrCreateThreadData()
{
	if(!tl_threadData)
	{
		std::lock_guard<std::mutex> lock(m_threadDataMutex);
		auto threadId = std::this_thread::get_id();

		auto it = m_threadDataMap.find(threadId);
		if(it == m_threadDataMap.end())
		{
			auto threadData = std::make_unique<ProfilerThreadData>();
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
