#pragma once

#include "core/core_minimal.h"

#include "task_node.h"
#include <thread>
#include <queue>
#include <mutex>

// Platform-specific includes for thread naming
#ifdef _WIN32
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(__linux__)
#include <pthread.h>
#elif defined(__APPLE__)
#include <pthread.h>
#endif

namespace task
{
	class ThreadPool
	{
	public:
		ThreadPool(u32 numThreads, const std::string& threadNamePrefix = "Worker")
			: m_bStop(false), m_threadNamePrefix(threadNamePrefix)
		{
			for(u32 i = 0; i < numThreads; i++)
			{
				m_workerThreads.emplace_back([this, i] {
					// Set thread name
					SetThreadName(m_threadNamePrefix + "_" + std::to_string(i));
					PROFILE_SET_THREAD_NAME(m_threadNamePrefix + "_" + std::to_string(i));

					while(true)
					{
						PROFILE_SCOPE("Worker running");
						TaskFunction func;
						{
							std::unique_lock<std::mutex> lock(m_queueMutex);
							m_cv.wait(lock, [this]() { return m_bStop || !m_taskQueue.empty(); });

							if(m_bStop && m_taskQueue.empty())
							{
								return;
							}

							func = std::move(m_taskQueue.front());
							m_taskQueue.pop();
						}
						func();
					}
					});
			}
		}

		~ThreadPool()
		{
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_bStop = true;
			}
			m_cv.notify_one();
			
			for(std::thread& thread : m_workerThreads)
			{
				thread.join();
			}
		}

		void Enqueue(TaskFunction task)
		{
			if(m_bStop) { return; }
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_taskQueue.push(task);
			m_cv.notify_one();
		}

		u32 GetQueueSize()
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			return (u32) m_taskQueue.size();
		}

	private:
	private:
		void SetThreadName(const std::string& name)
		{
#ifdef _WIN32
			// Windows implementation
			std::wstring wname(name.begin(), name.end());
			SetThreadDescription(GetCurrentThread(), wname.c_str());
#elif defined(__linux__)
			// Linux implementation
			pthread_setname_np(pthread_self(), name.substr(0, 15).c_str()); // Linux limits to 15 chars
#elif defined(__APPLE__)
			// macOS implementation
			pthread_setname_np(name.c_str());
#endif
		}

	private:
		std::vector<std::thread> m_workerThreads;
		std::queue<TaskFunction> m_taskQueue;
		std::mutex m_queueMutex;
		std::condition_variable m_cv;
		bool m_bStop;
		std::string m_threadNamePrefix;
	};
}
