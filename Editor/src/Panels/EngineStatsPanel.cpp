#include "pch.h"
#include "EngineStatsPanel.h"
#include "Ignis.h"
#include <imgui.h>

// ImGui headers
#include <imgui.h>

namespace ignis {

	EngineStatsPanel::EngineStatsPanel()
		: m_last_frame_time(std::chrono::steady_clock::now())
	{
	}

	void EngineStatsPanel::OnImGuiRender()
	{
		if (!ImGui::Begin("Engine Statistics"))
		{
			ImGui::End();
			return;
		}

		UpdateFrameStats();

		if (ImGui::BeginTabBar("StatsTabBar"))
		{
			if (ImGui::BeginTabItem("Performance"))
			{
				ImGui::Text("FPS: %.1f", m_fps);
				ImGui::Text("Frame Time: %.3f ms", m_frame_time);
				ImGui::Separator();
				
				auto& app = Application::Get();
				ImGui::Text("Window: %dx%d", app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
				ImGui::Text("VSync: %s", app.GetWindow().IsVSync() ? "Enabled" : "Disabled");
				
				ImGui::EndTabItem();
			}
			
			if (ImGui::BeginTabItem("System"))
			{
				ImGui::Text("Engine: Ignis");
				ImGui::Text("Platform: GLFW");
				ImGui::Text("Renderer: OpenGL");
				ImGui::EndTabItem();
			}
			
			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	void EngineStatsPanel::UpdateFrameStats()
	{
		auto current_time = std::chrono::steady_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::microseconds>(current_time - m_last_frame_time);
		
		m_frame_time = delta.count() / 1000.0f; // Convert to milliseconds
		m_fps = 1000.0f / m_frame_time;
		
		m_last_frame_time = current_time;
		m_frame_count++;
	}

}