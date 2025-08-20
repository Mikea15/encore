#pragma once

#include "core/core_minimal.h"
#include "globals.h"

#ifdef USE_LPP
#include "LivePP/API/x64/LPP_API_x64_CPP.h"
#endif

#ifdef USE_LPP

#define ABS_PATH_TO_LPP L"C:/Work/encore/app/vendor/LivePP"
#define REL_PATH_TO_LPP L"vendor/LivePP"

class LivePPHandler final
{
public:
	~LivePPHandler()
	{
		Clear();
	}

	bool Init()
	{
		// create a default agent, loading the Live++ agent from the given path, e.g. "ThirdParty/LivePP"
		m_defaultAgent = lpp::LppCreateDefaultAgent(nullptr, REL_PATH_TO_LPP);
		// bail out in case the agent is not valid
		if(!lpp::LppIsValidDefaultAgent(&m_defaultAgent))
		{
			LOG_ERROR("LivePP Default Agent not valid");
			return false;
		}
		m_agentType = AgentType::Default;
		m_defaultAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_NONE, nullptr, nullptr);
		return true;
	}

	bool InitSynchedAgent()
	{
		m_synchedAgent = lpp::LppCreateSynchronizedAgent(nullptr, REL_PATH_TO_LPP);
		if(!lpp::LppIsValidSynchronizedAgent(&m_synchedAgent))
		{
			LOG_ERROR("LivePP Synched Agent not valid");
			return false;
		}
		m_agentType = AgentType::Synched;
		m_synchedAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_NONE, nullptr, nullptr);
		return true;
	}

	void SyncPoint()
	{
		EnsureMsg(m_agentType != AgentType::Invalid, "Should have correctly initialized");

		if(m_synchedAgent.WantsReload(lpp::LPP_RELOAD_OPTION_SYNCHRONIZE_WITH_RELOAD))
		{
			m_synchedAgent.Reload(lpp::LPP_RELOAD_BEHAVIOUR_WAIT_UNTIL_CHANGES_ARE_APPLIED);
		}
		if(m_synchedAgent.WantsRestart())
		{
			m_synchedAgent.Restart(lpp::LPP_RESTART_BEHAVIOUR_INSTANT_TERMINATION, 0u, nullptr);
		}
	}

	void Clear()
	{
		if(m_agentType == AgentType::Default)
		{
			lpp::LppDestroyDefaultAgent(&m_defaultAgent);
		}
		else if(m_agentType == AgentType::Synched)
		{
			lpp::LppDestroySynchronizedAgent(&m_synchedAgent);
		}
	}

private:
	lpp::LppDefaultAgent m_defaultAgent;
	lpp::LppSynchronizedAgent m_synchedAgent;

	enum class AgentType : i8
	{
		Invalid = -1,
		Default,
		Synched
	} m_agentType;
};

#endif

