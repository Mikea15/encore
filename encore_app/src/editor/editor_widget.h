#pragma once

#include "core/core_minimal.h"

#include <game_state.h>
#include <string>

class EditorWidget
{
public:
	virtual ~EditorWidget() = default;
	virtual void Run(GameState& rGameState) = 0;
	virtual void DrawMenu() {}

	void WithMenu(const std::string& menu, bool* pOpenPanel = nullptr)
	{
		m_pOpenPanel = pOpenPanel;
		m_menu = menu;
	}

	bool ShouldDraw() const { return !m_pOpenPanel || *m_pOpenPanel;}
	const std::string& GetMenu() const { return m_menu; }

protected:
	bool* m_pOpenPanel = nullptr;
	std::string m_menu;
};
