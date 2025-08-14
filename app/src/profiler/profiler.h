#pragma once

#include "core/core_minimal.h"
#include "manager/base_singleton.h"

#include "profiler_types.h"
#include "profiler_section.h"

#include <vector>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>
#include <unordered_map>
#include <mutex>



class Profiler
{
	DECLARE_SINGLETON(Profiler)

public:

	u32 BeginSection(const char* section_name);
	void EndSection(u32 timerId);
	void ClearCurrentThread();
	void ClearAllThreads();

	const std::vector<ProfilerEntry>& GetCurrentThreadEntries();
	std::vector<ProfilerEntry> GetAllThreadsEntries();
	std::vector<ProfilerEntry> GetThreadEntries(std::thread::id threadId);

	void SetThreadName(const char* name);
	const char* GetThreadName(std::thread::id threadId);

	void PrintReport();

private:
	static u64 GetTimeUs();

	ProfilerThreadData* GetOrCreateThreadData();

private:
	thread_local static ProfilerThreadData* tl_threadData;

	std::unordered_map<std::thread::id, std::unique_ptr<ProfilerThreadData>> m_threadDataMap;
	std::unordered_map<std::thread::id, const char*> m_threadNames;
	std::mutex m_threadDataMutex;
};


// Enhanced macros with thread support
#define PROFILE_FRAME_START() Profiler::GetInstance().ClearCurrentThread()
#define PROFILE_FRAME_START_ALL_THREADS() Profiler::GetInstance().ClearAllThreads()
#define PROFILE_SCOPE(name) ProfileSection _timer(name)
#define PROFILE() ProfileSection _timer(__FUNCTION__)
#define PROFILE_SET_THREAD_NAME(name) Profiler::GetInstance().SetThreadName(name)
#define PROFILE_PRINT_REPORT() Profiler::GetInstance().PrintReport()
