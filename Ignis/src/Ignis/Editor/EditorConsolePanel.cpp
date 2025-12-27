#include "EditorConsolePanel.h"
#include <imgui.h>

namespace ignis {

	void EditorConsolePanel::AddMessage(ConsoleMessageLevel level, const std::string& message)
	{
		std::lock_guard<std::mutex> lock(m_message_mutex);

		// Get current timestamp
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");

		ConsoleMessage msg;
		msg.Level = level;
		msg.Message = message;
		msg.Timestamp = ss.str();

		m_messages.push_back(msg);
	}

	void EditorConsolePanel::Clear()
	{
		std::lock_guard<std::mutex> lock(m_message_mutex);
		m_messages.clear();
	}

	void EditorConsolePanel::OnImGuiRender()
	{
		// Fixed position at bottom of screen
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		float console_height = 200.0f;
		float properties_panel_width = 300.0f;
		
		// Position: bottom, width = window width - properties panel width
		ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + work_size.y - console_height));
		ImGui::SetNextWindowSize(ImVec2(work_size.x - properties_panel_width, console_height));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;

		if (ImGui::Begin("Console", nullptr, window_flags))
		{
			RenderMenuBar();
			ImGui::Separator();
			RenderMessages();
		}
		ImGui::End();
	}

	void EditorConsolePanel::RenderMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			ImGui::Checkbox("Trace", &m_show_trace);
			ImGui::Checkbox("Info", &m_show_info);
			ImGui::Checkbox("Warn", &m_show_warn);
			ImGui::Checkbox("Error", &m_show_error);
			
			ImGui::Separator();
			
			if (ImGui::Button("Clear"))
			{
				Clear();
			}
			
			ImGui::Checkbox("Auto-scroll", &m_auto_scroll);
			
			ImGui::EndMenuBar();
		}
	}

	void EditorConsolePanel::RenderMessages()
	{
		std::lock_guard<std::mutex> lock(m_message_mutex);

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

		for (const auto& msg : m_messages)
		{
			// Filter by level
			bool show = false;
			switch (msg.Level)
			{
				case ConsoleMessageLevel::Trace: show = m_show_trace; break;
				case ConsoleMessageLevel::Info:  show = m_show_info; break;
				case ConsoleMessageLevel::Warn:  show = m_show_warn; break;
				case ConsoleMessageLevel::Error: show = m_show_error; break;
			}

			if (!show) continue;

			// Color code by level
			ImVec4 color = GetLevelColor(msg.Level);
			ImGui::PushStyleColor(ImGuiCol_Text, color);
			ImGui::TextUnformatted(("[" + msg.Timestamp + "] " + msg.Message).c_str());
			ImGui::PopStyleColor();
		}

		// Auto-scroll
		if (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
	}

	ImVec4 EditorConsolePanel::GetLevelColor(ConsoleMessageLevel level) const
	{
		switch (level)
		{
			case ConsoleMessageLevel::Trace: return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);  // Gray
			case ConsoleMessageLevel::Info:  return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White
			case ConsoleMessageLevel::Warn:  return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow
			case ConsoleMessageLevel::Error: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
		}
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

} // namespace ignis