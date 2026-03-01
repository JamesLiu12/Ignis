#include "Editor/Panels/EditorConsolePanel.h"
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
		RenderMenuBar();
		ImGui::Separator();
		RenderMessages();
	}

	void EditorConsolePanel::RenderMenuBar()
	{
		ImGui::Checkbox("Trace", &m_show_trace);
		ImGui::SameLine();
		ImGui::Checkbox("Info", &m_show_info);
		ImGui::SameLine();
		ImGui::Checkbox("Warn", &m_show_warn);
		ImGui::SameLine();
		ImGui::Checkbox("Error", &m_show_error);
		
		ImGui::SameLine();
		ImGui::Spacing();
		ImGui::SameLine();
		
		if (ImGui::Button("Clear"))
		{
			Clear();
		}
		
		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &m_auto_scroll);
	}

	void EditorConsolePanel::RenderMessages()
	{
		std::lock_guard<std::mutex> lock(m_message_mutex);

		ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

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