#pragma once

#include <spdlog/sinks/base_sink.h>

namespace ignis {

// Forward declaration
class EditorConsolePanel;
enum class ConsoleMessageLevel;

// Custom spdlog sink that forwards messages to EditorConsolePanel
class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
	EditorConsoleSink(EditorConsolePanel* console_panel);
	
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override;
	void flush_() override {}
	
private:
	EditorConsolePanel* m_console_panel;
	
	ConsoleMessageLevel ConvertLevel(spdlog::level::level_enum level);
};

} // namespace ignis
