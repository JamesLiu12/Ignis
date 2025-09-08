#include "pch.h"
#include "Log.h"

namespace ignis {

	std::shared_ptr<spdlog::logger> Log::m_core_logger;
	std::shared_ptr<spdlog::logger> Log::m_client_logger;
	std::map<std::string, Log::TagDetails> Log::m_enabled_tags;
	// TODO: Create logger for editor

	void Log::Init()
	{
		std::filesystem::path executableDir;
		
		// Get the directory where the executable is located
		#ifdef _WIN32
			char path[MAX_PATH];
			GetModuleFileNameA(NULL, path, MAX_PATH);
			executableDir = std::filesystem::path(path).parent_path();
		#elif defined(__APPLE__)
			// macOS: Use _NSGetExecutablePath
			char path[1024];
			uint32_t size = sizeof(path);
			if (_NSGetExecutablePath(path, &size) == 0) {
				executableDir = std::filesystem::path(path).parent_path();
			} else {
				executableDir = std::filesystem::current_path(); // Fallback
			}
		#elif defined(__linux__)
			// Linux: Use /proc/self/exe
			char path[1024];
			ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
			if (len != -1) {
				path[len] = '\0';
				executableDir = std::filesystem::path(path).parent_path();
			} else {
				executableDir = std::filesystem::current_path(); // Fallback
			}
		#else
			// Other Unix systems: fallback to current path
			executableDir = std::filesystem::current_path();
		#endif
		
		std::filesystem::path logsDirectory = executableDir / "logs";
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

		m_core_logger = std::make_shared<spdlog::logger>("IGNIS", ignisSinks.begin(), ignisSinks.end());
		m_core_logger->set_level(spdlog::level::trace);

		m_client_logger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		m_client_logger->set_level(spdlog::level::trace);

		// TODO: Initialize EditorConsoleLogger for level editor
		// TODO: Set up default tag settings for engine subsystems
		// TODO: Add log file rotation and size limits

		// Initialize default tag settings
		m_enabled_tags[""] = TagDetails{ true, Level::Trace };
		m_enabled_tags["Core"] = TagDetails{ true, Level::Trace };
		m_enabled_tags["Renderer"] = TagDetails{ true, Level::Info };
		m_enabled_tags["Events"] = TagDetails{ true, Level::Info };
		m_enabled_tags["Input"] = TagDetails{ true, Level::Info };
		m_enabled_tags["Audio"] = TagDetails{ true, Level::Info };
		m_enabled_tags["Physics"] = TagDetails{ true, Level::Warn };
		m_enabled_tags["Scene"] = TagDetails{ true, Level::Info };
		m_enabled_tags["Memory"] = TagDetails{ true, Level::Error };
	}

	void Log::Shutdown()
	{
		m_core_logger.reset();
		m_client_logger.reset();
		spdlog::drop_all();
	}

}
