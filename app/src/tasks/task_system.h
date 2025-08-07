#pragma once

#include "core/core_minimal.h"

#include "task_node.h"
#include "thread_pool.h"

namespace task {

	class TaskSystem
	{
	public:
		TaskSystem(u32 numThreads = std::thread::hardware_concurrency())
			: m_threadPool(numThreads) 
		{}

		std::shared_ptr<TaskNode> CreateTask(const std::string& name, TaskFunction func)
		{
			auto task = std::make_shared<TaskNode>(name, func);
			m_taskNodes.push_back(task);
			return task;
		}

		void ExecuteTaskGraph()
		{
			for(auto& task : m_taskNodes)
			{
				task->Reset();
			}

			while(true)
			{
				bool bAllComplete = true;
				std::vector<std::shared_ptr<TaskNode>> tasksReady;

				for(auto& task : m_taskNodes)
				{
					if(!task->IsComplete())
					{
						bAllComplete = false;
						if(task->IsReadyToExecute())
						{
							tasksReady.push_back(task);
						}
					}
				}

				if(bAllComplete)
				{
					break;
				}

				for(auto& task : tasksReady)
				{
					m_threadPool.Enqueue([task]() { task->Execute(); });
				}

				// Wait a bit for tasks to complete
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}

		void ClearTasks()
		{
			m_taskNodes.clear();
		}

		u32 GetTaskCount() const { return (u32) m_taskNodes.size(); }
		
	private:
		ThreadPool m_threadPool;
		std::vector<std::shared_ptr<TaskNode>> m_taskNodes;
	};
}
