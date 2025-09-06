#include "pch.h"
#include "Log.h"

namespace ignis {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
	std::map<std::string, Log::TagDetails> Log::s_EnabledTags;
	// TODO: Create logger for editor

	void Log::Init()
	{
		std::filesystem::path logsDirectory = std::filesystem::current_path() / "logs";
		if (!std::filesystem::exists(logsDirectory))
			std::filesystem::create_directories(logsDirectory);

		// Common sink creation helper
		auto createSinks = [&logsDirectory](const std::string& filename) {
			std::vector<spdlog::sink_ptr> sinks = {
				std::make_shared<spdlog::sinks::basic_file_sink_mt>((logsDirectory / filename).string(), true),
				std::make_shared<spdlog::sinks::stdout_color_sink_mt>()
			};
			
			// Set patterns
			sinks[0]->set_pattern("[%T] [%l] %n: %v");        // File pattern
			sinks[1]->set_pattern("%^[%T] %n: %v%$");         // Console pattern (with colors)
			
			return sinks;
		};

		// Create loggers with shared sinks
		auto ignisSinks = createSinks("IGNIS.log");
		auto appSinks = createSinks("APP.log");

		s_CoreLogger = std::make_shared<spdlog::logger>("IGNIS", ignisSinks.begin(), ignisSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);

		// TODO: Initialize EditorConsoleLogger for level editor
		// TODO: Set up default tag settings for engine subsystems
		// TODO: Add log file rotation and size limits

		// Initialize default tag settings
		s_EnabledTags[""] = TagDetails{ true, Level::Trace };
		s_EnabledTags["Core"] = TagDetails{ true, Level::Trace };
		s_EnabledTags["Renderer"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Events"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Input"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Audio"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Physics"] = TagDetails{ true, Level::Warn };
		s_EnabledTags["Scene"] = TagDetails{ true, Level::Info };
		s_EnabledTags["Memory"] = TagDetails{ true, Level::Error };
	}

	void Log::Shutdown()
	{
		s_CoreLogger.reset();
		s_ClientLogger.reset();
		spdlog::drop_all();
	}

}
