#pragma once

#include "Panels/EditorPanel.h"

// Forward declaration for ImGui types
struct ImVec4;

namespace ignis {

	enum class ConsoleMessageLevel
	{
		Trace = 0,
		Info,
		Warn,
		Error
	};

	struct ConsoleMessage
	{
		ConsoleMessageLevel Level;
		std::string Message;
		std::string Timestamp;
	};

	class EditorConsolePanel : public EditorPanel
	{
	public:
		EditorConsolePanel() = default;
		~EditorConsolePanel() = default;

		void OnImGuiRender() override;
		std::string_view GetName() const override { return "Console"; }
		std::string_view GetID() const override { return "Console"; }

		void AddMessage(ConsoleMessageLevel level, const std::string& message);
		void Clear();

	private:
		void RenderMenuBar();
		void RenderMessages();
		ImVec4 GetLevelColor(ConsoleMessageLevel level) const;

		std::vector<ConsoleMessage> m_messages;
		std::mutex m_message_mutex;

		bool m_show_trace = true;
		bool m_show_info = true;
		bool m_show_warn = true;
		bool m_show_error = true;
		bool m_auto_scroll = true;
	};

} // namespace ignis