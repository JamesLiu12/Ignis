#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace ignis {

	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};

		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};

		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_core_logger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return m_client_logger; }

		static bool HasTag(const std::string& tag) { return m_enabled_tags.find(tag) != m_enabled_tags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return m_enabled_tags; }

		// TODO: Add SetTagLevel method for runtime tag configuration
		// TODO: Add PrintAssertMessage methods for debugging assertions
		// TODO: Add EditorConsoleLogger for level editor
		// TODO: Add default tag settings map for better organization
		// TODO: Implement log file rotation and cleanup

		// Compile-time log level filtering (set this based on build configuration)
		static constexpr Level CompileTimeMinLevel = Level::Trace; // Debug: Trace, Release: Info

		// Modern C++ logging interface - Core logging functions
		template<typename... Args>
		static constexpr void CoreTrace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Core, Level::Trace, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreInfo(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Core, Level::Info, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreWarn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Core, Level::Warn, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreError(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Core, Level::Error, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreFatal(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Core, Level::Fatal, fmt, std::forward<Args>(args)...);
			}
		}

		// Client logging functions
		template<typename... Args>
		static constexpr void Trace(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Client, Level::Trace, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void Info(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Client, Level::Info, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void Warn(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Client, Level::Warn, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void Error(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Client, Level::Error, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void Fatal(spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= CompileTimeMinLevel) {
				PrintMessageImpl(Type::Client, Level::Fatal, fmt, std::forward<Args>(args)...);
			}
		}

		// Tagged logging functions - Core
		template<typename... Args>
		static constexpr void CoreTraceTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Core, Level::Trace, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreInfoTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Core, Level::Info, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreWarnTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Core, Level::Warn, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreErrorTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Core, Level::Error, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void CoreFatalTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Core, Level::Fatal, tag, fmt, std::forward<Args>(args)...);
			}
		}

		// Tagged logging functions - Client
		template<typename... Args>
		static constexpr void TraceTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Trace >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Client, Level::Trace, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void InfoTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Info >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Client, Level::Info, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void WarnTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Warn >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Client, Level::Warn, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void ErrorTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Error >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Client, Level::Error, tag, fmt, std::forward<Args>(args)...);
			}
		}

		template<typename... Args>
		static constexpr void FatalTag(const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			if constexpr (Level::Fatal >= CompileTimeMinLevel) {
				PrintMessageTagImpl(Type::Client, Level::Fatal, tag, fmt, std::forward<Args>(args)...);
			}
		}

		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
				case Level::Trace: return "Trace";
				case Level::Info:  return "Info";
				case Level::Warn:  return "Warn";
				case Level::Error: return "Error";
				case Level::Fatal: return "Fatal";
			}
			return "";
		}

		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;
			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> m_core_logger;
		static std::shared_ptr<spdlog::logger> m_client_logger;
		// TODO: Add EditorConsoleLogger for level editor
		
		static std::map<std::string, TagDetails> m_enabled_tags;
		// TODO: Add default tag settings map for better organization

		// Internal implementation functions
		template<typename... Args>
		static inline void PrintMessageImpl(Type type, Level level, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			auto detail = m_enabled_tags[""];
			if (detail.Enabled && detail.LevelFilter <= level) {
				auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
				switch (level) {
					case Level::Trace: logger->trace(fmt, std::forward<Args>(args)...); break;
					case Level::Info:  logger->info(fmt, std::forward<Args>(args)...); break;
					case Level::Warn:  logger->warn(fmt, std::forward<Args>(args)...); break;
					case Level::Error: logger->error(fmt, std::forward<Args>(args)...); break;
					case Level::Fatal: logger->critical(fmt, std::forward<Args>(args)...); break;
				}
			}
		}

		template<typename... Args>
		static inline void PrintMessageTagImpl(Type type, Level level, const std::string& tag, spdlog::format_string_t<Args...> fmt, Args&&... args) {
			auto detail = m_enabled_tags[tag];
			if (detail.Enabled && detail.LevelFilter <= level) {
				auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
				std::string formatted = fmt::format(fmt, std::forward<Args>(args)...);
				switch (level) {
					case Level::Trace: logger->trace("[{}] {}", tag, formatted); break;
					case Level::Info:  logger->info("[{}] {}", tag, formatted); break;
					case Level::Warn:  logger->warn("[{}] {}", tag, formatted); break;
					case Level::Error: logger->error("[{}] {}", tag, formatted); break;
					case Level::Fatal: logger->critical("[{}] {}", tag, formatted); break;
				}
			}
		}
	};

}

// Modern C++ Usage Examples:
// ignis::Log::CoreInfo("Engine initialized successfully");
// ignis::Log::TraceTag("Renderer", "Loaded {} textures", count);
// ignis::Log::Error("Failed to load file: {}", filename);