#pragma once

#include "core/core_minimal.h"

#include "task_node.h"
#include <thread>
#include <queue>
#include <mutex>
#include <string>
#include <format>

#include <profiler/profiler.h>
#include <utils/utils_thread.h>

namespace task
{
	struct TaskPayload
	{
		TaskFunction func;
		float deltaTime;
	};

	class ThreadPool
	{
	public:
		ThreadPool(u32 numThreads, const std::string& threadNamePrefix = "Worker")
			: m_bStop(false)
		{
			for(u32 i = 0; i < numThreads; i++)
			{
				std::string threadName = std::format("[{}] {}", i, threadNamePrefix);
				m_workerThreads.emplace_back(&ThreadPool::DoWork, this, threadName);
			}
		}

		~ThreadPool()
		{
			m_bStop = true;
			m_cv.notify_all();
			
			for(std::thread& thread : m_workerThreads)
			{
				if(thread.joinable())
				{
					thread.join();
				}
			}
		}

		void Enqueue(TaskPayload payload)
		{
			if(m_bStop) { return; }
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_taskQueue.push(payload);
			m_cv.notify_one();
		}

		u32 GetQueueSize()
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			return (u32) m_taskQueue.size();
		}

	private:
		void DoWork(const std::string& threadName)
		{
			// Set thread name
			utils::NameThread(threadName);

			PROFILE_SET_THREAD_NAME(threadName.c_str());

			while(!m_bStop.load())
			{
				TaskPayload payload;
				{
					std::unique_lock<std::mutex> lock(m_queueMutex);
					m_cv.wait(lock, [this]() { return m_bStop.load() || !m_taskQueue.empty(); });

					if(m_bStop.load() && m_taskQueue.empty())
					{
						return;
					}

					payload = std::move(m_taskQueue.front());
					m_taskQueue.pop();
				}
				payload.func(payload.deltaTime);
			}
		}

	private:
		std::vector<std::thread> m_workerThreads;
		std::queue<TaskPayload> m_taskQueue;
		std::mutex m_queueMutex;
		std::condition_variable m_cv;
		std::atomic<bool> m_bStop;
	};
}
