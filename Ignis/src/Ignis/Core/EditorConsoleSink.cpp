#include "pch.h"
#include "EditorConsoleSink.h"
#include "Ignis/Editor/EditorConsolePanel.h"

namespace ignis {

EditorConsoleSink::EditorConsoleSink(EditorConsolePanel* console_panel)
	: m_console_panel(console_panel)
{
}

void EditorConsoleSink::sink_it_(const spdlog::details::log_msg& msg)
{
	if (!m_console_panel)
		return;
	
	// Format the message
	spdlog::memory_buf_t formatted;
	formatter_->format(msg, formatted);
	std::string message = fmt::to_string(formatted);
	
	// Convert spdlog level to ConsoleMessageLevel
	ConsoleMessageLevel level = ConvertLevel(msg.level);
	
	// Forward to EditorConsolePanel
	m_console_panel->AddMessage(level, message);
}

ConsoleMessageLevel EditorConsoleSink::ConvertLevel(spdlog::level::level_enum level)
{
	switch (level)
	{
		case spdlog::level::trace:
		case spdlog::level::debug:
			return ConsoleMessageLevel::Trace;
		case spdlog::level::info:
			return ConsoleMessageLevel::Info;
		case spdlog::level::warn:
			return ConsoleMessageLevel::Warn;
		case spdlog::level::err:
		case spdlog::level::critical:
			return ConsoleMessageLevel::Error;
		default:
			return ConsoleMessageLevel::Info;
	}
}

} // namespace ignis
