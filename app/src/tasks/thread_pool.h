#pragma once

#include "core/core_minimal.h"

#include "task_node.h"
#include <thread>
#include <queue>
#include <mutex>

namespace task
{
	class ThreadPool
	{
	public:
		ThreadPool(u32 numThreads)
			: m_bStop(false)
		{
			for(u32 i = 0; i < numThreads; i++)
			{
				m_workerThreads.emplace_back([this] {
					while(true)
					{
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
		std::vector<std::thread> m_workerThreads;
		std::queue<TaskFunction> m_taskQueue;
		std::mutex m_queueMutex;
		std::condition_variable m_cv;
		bool m_bStop;
	};
}
