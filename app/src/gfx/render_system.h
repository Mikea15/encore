#pragma once

#include "core/minimal.h"
#include <thread>
#include <queue>
#include <mutex>

struct RenderCommand
{

};

class RenderSystem
{
public:
	RenderSystem()
		: m_bShouldStop(false)
	{
		m_renderThread = std::thread(&RenderSystem::RenderLoop, this);
	}

	~RenderSystem()
	{
		m_bShouldStop = true;
		m_renderCondition.notify_all();

		if(m_renderThread.joinable())
		{
			m_renderThread.join();
		}
	}

	void SubmitRenderCommand(const RenderCommand& cmd)
	{

	}

	void ClearScreen()
	{
		SubmitRenderCommand({}); // clear command
	}

	void DrawX() {}

	void Present()
	{
		{
			std::lock_guard<std::mutex> lock(m_renderMutex);
			m_bPresentRequested = true;
		}
		m_renderCondition.notify_one();

		std::unique_lock<std::mutex> lock(m_presentMutex);
		m_presentCondition.wait(lock, [this]() { return !m_bPresentRequested; });
	}

private:
	void RenderLoop()
	{
		while(!m_bShouldStop)
		{
			std::unique_lock<std::mutex> lock(m_renderMutex);
			m_renderCondition.wait(lock, [this] { return m_bShouldStop || !m_bPresentRequested; });

			if(m_bShouldStop) { break; }

			while(!m_commandQueue.empty())
			{
				RenderCommand cmd = m_commandQueue.front();
				m_commandQueue.pop();

				lock.unlock();
				ExecuteRenderCommand(cmd);
				lock.lock();
			}

			{
				std::lock_guard<std::mutex> presentLock(m_presentMutex);
				if(m_bPresentRequested)
				{
					lock.unlock();

					// SDL_RenderPresent(); // Blit

					m_bPresentRequested = false;
					m_presentCondition.notify_one();
					lock.lock();
				}
			}
		}
	}

	void ExecuteRenderCommand(const RenderCommand& cmd)
	{}


	std::thread m_renderThread;
	std::queue<RenderCommand> m_commandQueue;
	std::mutex m_renderMutex;
	std::condition_variable m_renderCondition;
	std::atomic<bool> m_bShouldStop;

	// Present Sync
	std::mutex m_presentMutex;
	std::condition_variable m_presentCondition;
	bool m_bPresentRequested = false;
};
