#pragma once

#include "core/core_minimal.h"

#include "task_node.h"
#include "thread_pool.h"
#include "profiler/profiler_section.h"


class TaskSchedulerSystem
{
public:
	TaskSchedulerSystem(u32 numThreads = std::thread::hardware_concurrency())
		: m_threadPool(numThreads, "TaskWorker")
	{
	}

	std::shared_ptr<TaskNode> CreateTask(const std::string& name, TaskFunction func)
	{
		auto task = std::make_shared<TaskNode>(name, func);
		m_taskNodes.push_back(task);
		DirtyExecutionPlan();
		return task;
	}

	void ExecuteTaskGraph(float deltaTime)
	{
		PROFILE();

		AssertMsg(m_executionPlan.size() != 0 && m_taskNodes.size() > 0, "ExecutionPlan needs to be built");

		// Reset all tasks
		{
			PROFILE_SCOPE("Reset");
			for (auto& task : m_taskNodes)
			{
				task->Reset();
			}
		}

		// Execute using pre-computed plan
		{
			PROFILE_SCOPE("Execute Plan");
			for (const auto& layer : m_executionPlan)
			{
				ExecuteLayer(layer, deltaTime);
			}
		}
	}

	void CreateExecutionPlan()
	{
		PROFILE();
		AssertMsg(m_executionPlan.size() == 0, "ExecutionPlan was already built");
		m_executionPlan = TopologicalSort();
	}

	void DirtyExecutionPlan()
	{
		m_executionPlan.clear();
	}

private:
	// Get all tasks that depend on the given task
	std::vector<std::shared_ptr<TaskNode>> GetDependentTasks(std::shared_ptr<TaskNode> task)
	{
		std::vector<std::shared_ptr<TaskNode>> dependents;

		for (auto& otherTask : m_taskNodes)
		{
			const auto& deps = otherTask->GetDependencies();
			if (std::find(deps.begin(), deps.end(), task) != deps.end())
			{
				dependents.push_back(otherTask);
			}
		}

		return dependents;
	}

	std::vector<std::vector<std::shared_ptr<TaskNode>>> TopologicalSort()
	{
		std::vector<std::vector<std::shared_ptr<TaskNode>>> layers;

		// Calculate in-degrees for each task
		std::unordered_map<std::shared_ptr<TaskNode>, u64> inDegree;
		for (auto& task : m_taskNodes)
		{
			inDegree[task] = task->GetDependencies().size();
		}

		// Find all tasks with no dependencies (in-degree 0)
		std::queue<std::shared_ptr<TaskNode>> ready;
		for (auto& task : m_taskNodes)
		{
			if (inDegree[task] == 0)
			{
				ready.push(task);
			}
		}

		// Process layer by layer
		while (!ready.empty())
		{
			// Current layer: all tasks that are ready now
			std::vector<std::shared_ptr<TaskNode>> currentLayer;
			u64 layerSize = ready.size();

			for (u64 i = 0; i < layerSize; i++)
			{
				auto task = ready.front();
				ready.pop();
				currentLayer.push_back(task);

				// Reduce in-degree of dependent tasks
				for (auto& dependent : GetDependentTasks(task))
				{
					inDegree[dependent]--;
					if (inDegree[dependent] == 0)
					{
						ready.push(dependent);
					}
				}
			}

			layers.push_back(std::move(currentLayer));
		}

		return layers;
	}

	void ExecuteLayer(const std::vector<std::shared_ptr<TaskNode>>& layer, float dt)
	{
		if (layer.empty()) return;

		std::atomic<u64> completedTasks{0};
		const u64 totalTasks = layer.size();

		// Submit all tasks in this layer
		for (auto& task : layer)
		{
			m_threadPool.Enqueue(TaskPayload(
				[task, &completedTasks](float dt)
				{
					PROFILE_SCOPE(task->GetName().c_str());
					task->Execute(dt);
					completedTasks.fetch_add(1, std::memory_order_relaxed);
				}, // task
				dt // deltatime
			));
		}

		// Wait for all tasks in this layer to complete
		while (completedTasks.load(std::memory_order_relaxed) < totalTasks)
		{
			std::this_thread::yield();
		}
	}

	// Same topological sort and execute layer methods...
	ThreadPool m_threadPool;
	std::vector<std::shared_ptr<TaskNode>> m_taskNodes;
	std::vector<std::vector<std::shared_ptr<TaskNode>>> m_executionPlan;
};
