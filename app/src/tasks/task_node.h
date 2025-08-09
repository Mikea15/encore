#pragma once

#include "core/core_minimal.h"
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <atomic>

namespace task 
{
	using TaskFunction = std::function<void(float dt)>;

	class TaskNode
	{
	public:
		TaskNode(const std::string& name, TaskFunction func)
			: m_name(name), m_func(func), m_completed(false)
		{}

		void AddDependency(std::shared_ptr<TaskNode> dependency)
		{
			m_dependencies.push_back(dependency);
		}

		void Execute(float dt)
		{
			if(!m_completed.load())
			{
				m_func(dt);
				m_completed.store(true);
			}
		}

		void Reset()
		{
			m_completed.store(false);
		}

		bool IsComplete() const
		{
			return m_completed.load();
		}

		const std::vector<std::shared_ptr<TaskNode>>& GetDependencies() const
		{
			return m_dependencies;
		}

		const std::string& GetName() const { return m_name; }

	private:
		std::string m_name;
		TaskFunction m_func;
		std::atomic<bool> m_completed;
		std::vector<std::shared_ptr<TaskNode>> m_dependencies;
	};
}
