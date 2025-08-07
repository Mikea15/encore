#pragma once

#include "core/core_minimal.h"
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <atomic>

namespace task 
{
	using TaskFunction = std::function<void()>;

	class TaskNode
	{
	public:
		TaskNode(const std::string& name, TaskFunction func)
			: m_name(name), m_func(func), m_bCompleted(false)
		{}

		void Execute()
		{
			m_func();
			m_bCompleted = true;
		}

		void AddDependency(std::shared_ptr<TaskNode> dependency)
		{
			m_dependencies.push_back(dependency);
		}

		bool IsComplete() const { return m_bCompleted; }

		bool IsReadyToExecute() const
		{
			for(const auto& dep : m_dependencies)
			{
				if(!dep->IsComplete())
				{
					return false;
				}
			}
			return true;
		}

		void Reset() { m_bCompleted = false; }
		const std::string& GetName() const { return m_name; }

	private:
		std::vector<std::shared_ptr<TaskNode>> m_dependencies;
		std::string m_name;
		TaskFunction m_func;
		std::atomic<bool> m_bCompleted = false;
	};
}
