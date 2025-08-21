#pragma once
#include "core/core_minimal.h"

#include "editor/editor_widget.h"

class MenuWidget : public EditorWidget
{
public:
	virtual void Run(GameState& rGameStates) override
	{
		if(ImGui::BeginMenuBar())
		{
			if(ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Profiler", nullptr, &rGameStates.editor.bOpenProfiler);
				ImGui::MenuItem("Performance Monitor", nullptr, &rGameStates.editor.bOpenPerformanceMonitor);
				ImGui::MenuItem("Memory Monitor", nullptr, &rGameStates.editor.bOpenMemoryMonitor);

				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("Toggle In-Game ImGui", nullptr, &rGameStates.bShowInGameImGui);
				ImGui::MenuItem("Toggle ImGui", "TAB", &rGameStates.editor.bShowImGui);
				ImGui::MenuItem("Demo Window", nullptr, &rGameStates.editor.bShowDemoWindow);

				ImGui::Text("TAB: Toggle Editor Mode");
				ImGui::Text("ESC: Exit application");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
};
